#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#include "server.h"

struct mapinstance *map_instances;
struct map **map_instancedtiles;

static int inst_expire_y[INST_MAX]; // Used in instance item expiration ticker

void init_instances(void)
{
        map_instances = malloc(sizeof(struct mapinstance) * INST_MAX);

        map_instancedtiles = malloc(sizeof(struct map *) * INST_MAX);

        for (int i=0; i<INST_MAX; i++) {
                map_instances[i].used = USE_EMPTY;
        }

        // Remove all leftover items from previous instances
        for (int in=0; in<MAXITEM; in++) {
                if (it[in].used == USE_EMPTY) continue;
                if (it[in].instance_id != -1) it[in].used = USE_EMPTY;
        }

        // Remove all leftover NPCs/Players from previous instances
        for (int cn=0; cn<MAXCHARS; cn++) {
                if (ch[cn].used == USE_EMPTY || ch[cn].flags&(CF_PLAYER|CF_USURP)) continue;
                if (ch[cn].instance_id != -1) {
                        god_destroy_items(cn);
                        ch[cn].used = USE_EMPTY;
                }
        }

        // Remove all leftover effects from previous instances
        for (int f=0; f<MAXEFFECT; f++) {
                if (fx[f].used == USE_EMPTY) continue;
                if (fx[f].instance_id != -1) fx[f].used = USE_EMPTY;
        }

        xlog("Initialized instances.");
}

// Create a new instance base area; fname is file name to save instance in; returns the id of the new instance or -1 if failed
// Take care of having inst_name and fname lengths be lower than the instance base struct's respective fields
int create_instance_base(char *inst_name, char *fname, int wid, int hei)
{
        if (wid < 0 || hei < 0 || wid >= MAPX || hei >= MAPY) {
                xlog("Could not create a new instance base - wrong size (wid=%d hei=%d).", wid, hei);
                return -1;
        }
        
        int i;
        for (i=0; i<INST_MAXBASES; i++) {
                if (map_instancebases[i].used == USE_EMPTY) break;
        }

        if (i >= INST_MAXBASES) {
                xlog("Could not create a new instance base - limit reached!");
                return -1;
        }

        map_instancebases[i].used = USE_ACTIVE;
        map_instancebases[i].id = i;
        strcpy(map_instancebases[i].name, inst_name);
        strcpy(map_instancebases[i].fname, fname);
        map_instancebases[i].width = wid;
        map_instancebases[i].height = hei;

        // Set creation time
        time_t now;
        struct tm *now_tm;
        now = time(NULL);
        now_tm = localtime(&now);
        map_instancebases[i].creation_time = now_tm->tm_hour*100 + now_tm->tm_min;
        xlog("Created base at %d.", map_instancebases[i].creation_time);

        // Create file for instance base if it doesn't exist
        int handle, ninst_size;
        char fpath[60];
        struct map tmap;

        sprintf(fpath, DATDIR"/inst_basedata/%s.dat", fname);

        handle=open(fpath,O_RDWR);
        if (handle==-1) {
                ninst_size = sizeof(struct map) * wid * hei;

                xlog("Creating file for instance base %s(%d): file size=%dK",
                        inst_name,i,ninst_size>>10);

                handle=open(fpath,O_RDWR|O_CREAT,0655);
                bzero(&tmap, sizeof(struct map));
                tmap.sprite = SPR_GROUND1;
                if (!extend(handle, ninst_size, sizeof(struct map), &tmap)) {
                        xlog("ERROR - could not map instance base data into file.");
                        return -1;
                }
                close(handle);
        }

        return i;
}

// Looks for an instance base with the given name, and returns its ID, or -1 if not found
int get_instance_base(char *bname)
{
        for (int i=0; i<INST_MAXBASES; i++) {
                if (map_instancebases[i].used == USE_EMPTY) continue;
                if (strcmp(map_instancebases[i].name, bname) == 0) return i;
        }
        return -1;
}

// Delete an instance base map, existing instances copied from this map stay alive
void delete_instance_base(char *bname)
{
        int base_id = get_instance_base(bname);
        if (base_id == -1) return;

        map_instancebases[base_id].used = USE_EMPTY;
}

short inst_isalive(int inst_id)
{
        if (inst_id < 0 || inst_id >= INST_MAX) return 0;
        if (map_instances[inst_id].used == USE_EMPTY) return 0;
        return 1;
}

int inst_totalalive() {
        int al = 0;
        for (int i=0; i<INST_MAX; i++) {
                if (map_instances[i].used != USE_EMPTY) al++;
        }
        return al;
}

int inst_getnewid()
{
        for (int i=0; i<INST_MAX; i++) {
                if (map_instances[i].used == USE_EMPTY) return i;
        }
        return -1;
}

// Saves the given instance state to its base map
void save_inst_to_base(int inst_id)
{
        if (!inst_isalive(inst_id)) return;
        
        int handle;
        char fpath[60];

        // Save map tile data / map items
        sprintf(fpath, DATDIR"/inst_basedata/%s.dat", map_instances[inst_id].fname);

        handle=open(fpath,O_WRONLY|O_TRUNC);
        if (handle==-1) {
                xlog("Could not open file at (%s), saving instance %d mapdata to base. Creating new file...", fpath, inst_id);
                handle=open(fpath,O_WRONLY|O_CREAT,0655);
        }

        for (int i=0; i<map_instances[inst_id].height; i++) {
                for (int j=0; j<map_instances[inst_id].width; j++) {
                        int m = j + i * map_instances[inst_id].width;
                        int in;
                        struct map tmp_tile;

                        bzero(&tmp_tile, sizeof(struct map));
                        tmp_tile.flags = map_instancedtiles[inst_id][m].flags;
                        tmp_tile.fsprite = map_instancedtiles[inst_id][m].fsprite;
                        in = map_instancedtiles[inst_id][m].it;
                        if (in && it[in].used != USE_EMPTY) {
                                if (!(it[in].flags&(IF_TAKE|IF_MONEY))) {
                                        tmp_tile.it = it[in].temp;
                                } else {
                                        tmp_tile.it = 0;
                                }
                        }
                        tmp_tile.sprite = map_instancedtiles[inst_id][m].sprite;

                        write(handle, &tmp_tile, sizeof(struct map));
                }
        }
        close(handle);

        // Save characters
        sprintf(fpath, DATDIR"/inst_basedata/%s.chr", map_instances[inst_id].fname);

        handle=open(fpath,O_WRONLY|O_TRUNC);
        if (handle==-1) {
                xlog("Could not open file at (%s), saving instance %d chars to base. Creating new file...", fpath, inst_id);
                handle=open(fpath,O_WRONLY|O_CREAT,0655);
        }

        for (int cn=0; cn<MAXCHARS; cn++) {
                if (ch[cn].used == USE_EMPTY || !(ch[cn].flags&CF_INST_TEMP) || ch[cn].instance_id != inst_id || !ch[cn].temp) continue;

                struct instcharacter tmp_char;

                tmp_char.x = ch[cn].x;
                tmp_char.y = ch[cn].y;
                tmp_char.dir = ch[cn].dir;
                tmp_char.temp = ch[cn].temp;

                write(handle, &tmp_char, sizeof(struct instcharacter));
        }
        close(handle);
}

// Create an instance from the given map base, returns its new id, or -1 if it failed
int create_instance_frombase(char *mname, short nochars)
{
        int new_id, base_id, handle;
        char fpath[60];
        //struct map **tmp_map_ptr;

        base_id = get_instance_base(mname);
        if (base_id == -1) {
                xlog("ERROR - could not create instance, base %s not found!", mname);
                return -1;
        }

        new_id = inst_getnewid();
        if (new_id < 0) {
                xlog("ERROR - could not create instance, max reached!");
                return -1;
        }

        // Open instance mapdata file
        sprintf(fpath, DATDIR"/inst_basedata/%s.dat", map_instancebases[base_id].fname);
        handle=open(fpath,O_RDONLY);
        if (handle==-1) {
                xlog("ERROR - could not find base file for %s when creating new instance.", map_instancebases[base_id].fname);
                return -1;
        }

        // Create base instance object, copying from given base
        map_instances[new_id].id = new_id;
        map_instances[new_id].width = map_instancebases[base_id].width;
        map_instances[new_id].height = map_instancebases[base_id].height;
        strcpy(map_instances[new_id].name, map_instancebases[base_id].name);
        strcpy(map_instances[new_id].fname, map_instancebases[base_id].fname);
        map_instances[new_id].last_activity_time = globs->ticker;
        map_instances[new_id].mobs_remaining = 0;

        // Set creation time
        time_t now;
        struct tm *now_tm;
        now = time(NULL);
        now_tm = localtime(&now);
        map_instances[new_id].creation_time = now_tm->tm_hour*100 + now_tm->tm_min;

        // Set instanced tiles for new map
        // Allocate individual tiles
        map_instancedtiles[new_id] = calloc(map_instances[new_id].width * map_instances[new_id].height, sizeof(struct map));
        if (map_instancedtiles[new_id] == NULL) {
                xlog("ERROR - could not reallocate memory for instance %d (%s)'s individual tile data.", new_id, map_instances[new_id].name);
                return -1;
        }

        // Load mapdata
        for (int i = 0; i < map_instances[new_id].height; i++) {
                for (int j = 0; j < map_instances[new_id].width; j++) {
                        //map_instancedtiles[new_id][j + i * map_instances[new_id].width].sprite = SPR_GROUND1;

                        struct map tmp_tile;
                        if (read(handle, &tmp_tile, sizeof(struct map)) == -1) {
                                xlog("ERROR - read returned -1 while reading from (%s) for instance %d.", fpath, new_id);
                                unload_instance(new_id);
                                return -1;
                        }
                        int m = j + i * map_instances[new_id].width;
                        map_instancedtiles[new_id][m].flags = tmp_tile.flags;
                        map_instancedtiles[new_id][m].fsprite = tmp_tile.fsprite;
                        map_instancedtiles[new_id][m].sprite = tmp_tile.sprite;

                        // Use build function to add instance-independent items
                        if (tmp_tile.it) {
                                build_drop(j, i, tmp_tile.it, new_id, 1);
                        }
                }
        }
        close(handle);

        // Load characters
        if (!nochars) {
                sprintf(fpath, DATDIR"/inst_basedata/%s.chr", map_instancebases[base_id].fname);
                handle=open(fpath,O_RDONLY);
                if (handle==-1) {
                        xlog("No character file found for %s when creating new instance.", map_instancebases[base_id].fname);
                } else {
                        struct instcharacter tmp_char;
                        while (read(handle, &tmp_char, sizeof(struct instcharacter)) > 0) {
                                int co = pop_create_char(tmp_char.temp, 0, new_id);
                                if (!god_drop_char_fuzzy(co, tmp_char.x, tmp_char.y, new_id)) {
                                        xlog("Could not drop char %d at [%d;%d] for instance %d.", tmp_char.temp, tmp_char.x, tmp_char.y, new_id);
                                        ch[co].used = USE_EMPTY;
                                        continue;
                                }
                                ch[co].dir = tmp_char.dir;
                                ch[co].flags&=~CF_RESPAWN;

                                ch[co].data[29] = tmp_char.x + tmp_char.y * map_instances[new_id].width;

                                if (ch[co].alignment < 0) {
                                        map_instances[new_id].mobs_remaining++;
                                }
                        }
                        close(handle);
                }
        }

        // Set to active only after successful loading
        map_instances[new_id].used = USE_ACTIVE;

        init_lights_inst(new_id);

        xlog("Created instance %d from base %s.", new_id, map_instances[new_id].name);

        return new_id;
}

void unload_instance(int inst_id)
{
        if (!inst_isalive(inst_id)) return;

        xlog("Unloading instance %d (%s).", inst_id, map_instances[inst_id].name);

        // Remove characters and items
        for (int i=0; i<map_instances[inst_id].height; i++) {
                for (int j=0; j<map_instances[inst_id].width; j++) {
                        int m = j + i * map_instances[inst_id].width;
                        int cn = map_instancedtiles[inst_id][m].ch;
                        if (cn) {
                                // Kick players out, making them recall
                                if (ch[cn].flags&(CF_PLAYER|CF_USURP)) {
                                        do_char_log(cn, 0, "Instance is shutting down, you will be returned to your recall position.\n");
                                        if (!god_transfer_char(cn, ch[cn].temple_x, ch[cn].temple_y, -1)) {
                                                do_char_log(cn, 0, "Could not drop you in your recall position, try logging in later.\n");
                                                plr_logout(cn, ch[cn].player, LO_NOROOM);
                                                plog(ch[cn].player, "unload_instance(): could not drop character in recall position");
                                        } else {
                                                fx_add_effect(6,0,ch[cn].x,ch[cn].y,0,-1);
                                        }
                                } else {
                                        god_destroy_items(cn);
                                        ch[cn].used = USE_EMPTY;
                                }
                        }

                        int in = map_instancedtiles[inst_id][m].it;
                        if (in) it[in].used = USE_EMPTY;
                }
        }

        // Remove effects
        for (int f=0; f<MAXEFFECT; f++) {
                if (fx[f].used == USE_EMPTY) continue;
                if (fx[f].instance_id != inst_id) continue;
                fx[f].used = USE_EMPTY;
        }

        // For instance ticker update
        inst_expire_y[inst_id] = 0;

        map_instances[inst_id].used = USE_EMPTY;
        free(map_instancedtiles[inst_id]);
}

// Item expiration for instances
void item_tick_expire_inst(void)
{
    int x,in,m,act,cn,inst;
    int exp_time;

    for (inst = 0; inst < INST_MAX; inst++) {
        if (map_instances[inst].used == USE_EMPTY) continue;

        exp_time = map_instances[inst].height / 4;

        for (x = 0,m = inst_expire_y[inst] * map_instances[inst].width; x < map_instances[inst].width; x++, m++) {
                if ((in=map_instancedtiles[inst][m].it)!=0) {
                        if ((it[in].flags&IF_REACTIVATE) && !it[in].active) {
                                if (!map_instancedtiles[inst][m].ch && !map_instancedtiles[inst][m].to_ch) {
                                        it[in].active=it[in].duration;
                                        if (it[in].light[0]!=it[in].light[1])
                                                do_add_light(x,inst_expire_y[inst],it[in].light[1]-it[in].light[0],inst);
                                }
                        }
                        // active and expire
                        if (it[in].active && it[in].active!=0xffffffff) {
                                if (it[in].active<=exp_time) {
                                        if (may_deactivate(in) && !map_instancedtiles[inst][m].ch && !map_instancedtiles[inst][m].to_ch) {
                                                use_driver(0,in,0);
                                                it[in].active=0;
                                                if (it[in].light[0]!=it[in].light[1])
                                                        do_add_light(x,inst_expire_y[inst],it[in].light[0]-it[in].light[1],inst);
                                        }
                                } else  it[in].active-=exp_time;
                        }

                        // legacy drivers, replace by IF_EXPIREPROC!
                        if (it[in].driver==33) pentagram(in);
                        if (it[in].driver==43) spiderweb(in);
                        if (it[in].driver==56) greenlingball(in);

                        if (it[in].flags&IF_EXPIREPROC) expire_driver(in);

                        if (!(it[in].flags&IF_TAKE) && it[in].driver!=7) goto noexpire_inst;
                        if ((map_instancedtiles[inst][m].flags&MF_NOEXPIRE) && it[in].driver!=7) goto noexpire_inst;      // yuck!
                        if (it[in].driver==37) goto noexpire_inst;

                        if (it[in].flags&IF_NOEXPIRE) goto noexpire_inst;

                        if (it[in].active) act=1;
                        else act=0;

                        it[in].current_age[act]+=exp_time;      // each place is only checked every MAPY ticks
                                                                // so we add MAPY instead of one

                        if (it[in].flags&IF_LIGHTAGE) lightage(in,exp_time);

                        if (item_age(in) && it[in].damage_state==5) {
                                if (it[in].light[act]) do_add_light(x,inst_expire_y[inst],-it[in].light[act],inst);
                                map_instancedtiles[inst][m].it=0;
                                it[in].used=USE_EMPTY;

                                if (it[in].driver==7) { // tomb
                                        int co,temp;

                                        co=it[in].data[0];
                                        temp=ch[co].temp;

                                        god_destroy_items(co);
                                        ch[co].used=USE_EMPTY;

                                        if (temp && (ch[co].flags&CF_RESPAWN)) {
                                            if (temp==189 || temp==561) {
                                                fx_add_effect(2,TICKS*60*20+RANDOM(TICKS*60*5),ch_temp[temp].x,ch_temp[temp].y,temp,inst);
                                            } else {
                                                fx_add_effect(2,TICKS*60*1+RANDOM(TICKS*60*1),ch_temp[temp].x,ch_temp[temp].y,temp,inst);
                                            }
                                            xlog("respawn %d (%s): YES",co,ch[co].name);
                                        } else xlog("respawn %d (%s): NO",co,ch[co].name);
                                }
                        }
                }

                noexpire_inst:
                // checker
                if ((cn=map_instancedtiles[inst][m].ch)!=0) {
                        if (ch[cn].x!=x || ch[cn].y!=inst_expire_y[inst] || ch[cn].used!=USE_ACTIVE) {
                                xlog("map_instancedtiles[%d][%d,%d].ch reset from %d (%s) to 0",inst,x,inst_expire_y[inst],cn,ch[cn].reference);
                                map_instancedtiles[inst][m].ch=0;
                        }
                }
                if ((cn=map_instancedtiles[inst][m].to_ch)!=0) {
                        if (ch[cn].tox!=x || ch[cn].toy!=inst_expire_y[inst] || ch[cn].used!=USE_ACTIVE) {
                                xlog("map_instancedtiles[%d][%d,%d].to_ch reset from %d (%s) to 0",inst,x,inst_expire_y[inst],cn,ch[cn].reference);
                                map_instancedtiles[inst][m].to_ch=0;
                        }
                }
                if ((in=map_instancedtiles[inst][m].it)!=0) {
                        if (it[in].x!=x || it[in].y!=inst_expire_y[inst] || it[in].used!=USE_ACTIVE) {
                                xlog("map_instancedtiles[%d][%d,%d].it reset from %d (%s) to 0",inst,x,inst_expire_y[inst],in,it[in].reference);
                                map_instancedtiles[inst][m].it=0;
                        }
                }
        }

        inst_expire_y[inst]++; if (inst_expire_y[inst]>=map_instances[inst].height) { inst_expire_y[inst]=0; }
    }
}

//TODO - fix minimap for instances client-side
