#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#include "server.h"
#include "map-device.h"

struct areamap_device *amap_devices;
static int loaded_mapdevices = 0;

// Loads/creates a map device for the given character; will get saved to .dat/mapdevices/<cn>.dat
// Returns new mapdevice ID, or -1 if it fails
int load_mapdevice(int cn)
{
    int mapd_id = 0, handle;
    struct areamap_device *tmp_ptr;

    if (ch[cn].used == USE_EMPTY) return -1;

    // Try to find a free spot from existing devices
    if (loaded_mapdevices > 0) {
        for (mapd_id = 0; mapd_id < loaded_mapdevices; mapd_id++) {
            if (amap_devices[mapd_id].used == USE_EMPTY) break;
        }
    }

    if (mapd_id >= loaded_mapdevices) {
        tmp_ptr = realloc(amap_devices, sizeof(struct areamap_device) * (loaded_mapdevices + 1));
        if (tmp_ptr == NULL) {
            xlog("Could not reallocate memory for a new map device.");
            return -1;
        }
        amap_devices = tmp_ptr;
        loaded_mapdevices++;
    }

    // Load/create mapdevice file
    char fpath[60];
    sprintf(fpath, DATDIR"/mapdevices/%d.dat", cn);

    handle=open(fpath, O_RDWR);
    if (handle == -1) {
        xlog("Creating new map device file for %s (%d).", ch[cn].name, cn);
        handle=open(fpath,O_RDWR|O_CREAT,0755);

        if (!extend(handle, sizeof(struct areamap_device), sizeof(struct areamap_device), NULL)) {
            xlog("ERROR - could not map mapdevice data into file.");
            return -1;
        };
        close(handle);

        amap_devices[mapd_id].level = 1;
        amap_devices[mapd_id].experience = 0;

        for (int i=0; i<AMAP_MAXBASES; i++) {
            amap_devices[mapd_id].map_charges[i] = 0;
        }
        
        for (int i=0; i<AMAP_MAXBASES; i++) {
            for (int j=0; j<AMAP_MAXMODS; j++) {
                amap_devices[mapd_id].map_mods[i][j] = 0;
            }
        }

        for (int i=0; i<5; i++) {
            amap_devices[mapd_id].orbs[i] = 0;
        }
    } else {
        // Load from file
        struct areamap_device file_mapdvc;
        if (read(handle, &file_mapdvc, sizeof(struct areamap_device)) == -1) {
            xlog("ERROR - read returned -1 while reading from mapdevice file for %s (%d).", ch[cn].name, cn);
            return -1;
        }
        amap_devices[mapd_id].level = file_mapdvc.level;
        amap_devices[mapd_id].experience = file_mapdvc.experience;

        for (int i=0; i<AMAP_MAXBASES; i++) {
            amap_devices[mapd_id].map_charges[i] = file_mapdvc.map_charges[i];
        }

        for (int i=0; i<AMAP_MAXBASES; i++) {
            for (int j=0; j<AMAP_MAXMODS; j++) {
                amap_devices[mapd_id].map_mods[i][j] = file_mapdvc.map_mods[i][j];
            }
        }

        for (int i=0; i<5; i++) {
            amap_devices[mapd_id].orbs[i] = file_mapdvc.orbs[i];
        }

        close(handle);
    }

    amap_devices[mapd_id].ch = cn;
    amap_devices[mapd_id].used = USE_ACTIVE;

    xlog("Loaded map device for %s (%d). loaded_mapdevices at %d.", ch[cn].name, cn, loaded_mapdevices);

    return mapd_id;
}

// Returns the ID of the mapdevice owned by the given character, or -1 if not found
int get_mapdevice(int cn)
{
    if (ch[cn].used == USE_EMPTY) return -1;
    if (!(ch[cn].flags&CF_PLAYER) || ch[cn].flags&CF_USURP) return -1;

    for (int i=0; i<loaded_mapdevices; i++) {
        if (amap_devices[i].ch == cn) return i;
    }
    return -1;
}

// Saves the mapdevice state of the given character to its respective file
void save_mapdevice(int cn)
{
    int mapd = get_mapdevice(cn);
    if (mapd == -1) return;

    int handle;
    char fpath[60];

    sprintf(fpath, DATDIR"/mapdevices/%d.dat", cn);

    handle=open(fpath,O_WRONLY|O_TRUNC);
    if (handle==-1) {
        xlog("Could not open mapdevice file when saving for %s (%d). Creating new one...", ch[cn].name, cn);
        handle=open(fpath,O_WRONLY|O_CREAT);
    }

    write(handle, &amap_devices[mapd], sizeof(struct areamap_device));
    close(handle);
}

void unload_mapdevice(int cn)
{
    int mapd = get_mapdevice(cn);
    if (mapd != -1) {
        amap_devices[mapd].used = USE_EMPTY;
    }
}

void delete_mapdevice(int cn)
{
    int mapd = get_mapdevice(cn);
    if (mapd != -1) {
        unload_mapdevice(cn);

        char fpath[60];
        sprintf(fpath, DATDIR"/mapdevices/%d.dat", cn);
        remove(fpath);
    }
}

long mapdev_expreq(int mapd)
{
    if (mapd < 0 || mapd >= loaded_mapdevices) return 200;
    return 200 + (amap_devices[mapd].level - 1) * (200 * (amap_devices[mapd].level / 2));
}

void mapdev_addexp(int cn, int xp)
{
    int mapd = get_mapdevice(cn);
    if (mapd == -1) return;

    int xpreq = mapdev_expreq(cn);

    amap_devices[mapd].experience += xp;
    if (amap_devices[mapd].experience >= xpreq) {
        int leftover = amap_devices[mapd].experience - xpreq;
        amap_devices[mapd].experience = 0;
        amap_devices[mapd].level++;
        if (leftover < mapdev_expreq(mapd)) {
            do_char_log(cn, 1, "/|%d|Your map device becomes more attuned to reality. It is now level %d.", FNT_ORANGE, amap_devices[mapd].level);
        }
        mapdev_addexp(cn, leftover);
    }
    send_mapdev_data(ch[cn].player, 0);
}

// Convert the given coordinate ID (x+y*width) to its respective areamap index; returns -1 if not found
int amap_coords_to_ind(int coords)
{
    int amap;
    for (amap=0; amap<AMAP_MAXBASES; amap++) {
        if (amap_bases[amap].used == USE_EMPTY) continue;
        if (amap_bases[amap].quadrant == coords%4 && amap_bases[amap].tier == floor(coords/4)) break;
    }
    if (amap >= AMAP_MAXBASES) return -1;

    return amap;
}

// Sends an areamap base to the given player
void send_amapbase(int nr, int amap)
{
    if (!player[nr].sock) return;
    if (amap < 0 || amap >= AMAP_MAXBASES) return;
    if (amap_bases[amap].used == USE_EMPTY) return;

    unsigned char buf[16];

    buf[0]=SV_AMAPBASE;
    *(unsigned short*)(buf+1) = amap_bases[amap].quadrant + amap_bases[amap].tier * 4;
    *(unsigned char*)(buf+3) = amap_bases[amap].conn_flags;
    *(unsigned int*)(buf+4) = amap_bases[amap].sprite;
    xsend(nr,buf,8);
}

// Sends all active areamap bases to the given player
void send_amapbase_all(int nr)
{
    if (!player[nr].sock) return;

    for (int i=0; i<AMAP_MAXBASES; i++) {
        send_amapbase(nr, i);
    }
}

// Sends areamap charges to the given player
void send_amapcharges(int nr, int amap)
{
    if (!player[nr].sock) return;
    if (amap < 0 || amap >= AMAP_MAXBASES) return;
    if (amap_bases[amap].used == USE_EMPTY) return;
    
    int cn = player[nr].usnr;
    int amap_dev = get_mapdevice(cn);
    unsigned char buf[16];

    if (amap_dev == -1) return;

    buf[0]=SV_AMAPCHG;
    *(unsigned short*)(buf+1) = amap_bases[amap].quadrant + amap_bases[amap].tier * 4;
    *(unsigned char*)(buf+3) = amap_devices[amap_dev].map_charges[amap];
    xsend(nr,buf,4);
}

void send_amapcharges_all(int nr)
{
    if (!player[nr].sock) return;

    for (int i=0; i<AMAP_MAXBASES; i++) {
        send_amapcharges(nr, i);
    }
}

// Sends mapdevice data
void send_mapdev_data(int nr, int open_gui)
{
    if (!player[nr].sock) return;

    int cn = player[nr].usnr;
    int amap_dev = get_mapdevice(cn);
    if (amap_dev == -1) return;

    unsigned char buf[16];

    // Send level & exp
    buf[0] = SV_MAPDEVDATA;
    *(unsigned char*)(buf+1) = 0;
    *(unsigned short*)(buf+2) = amap_devices[amap_dev].level;
    *(unsigned long*)(buf+4) = amap_devices[amap_dev].experience;
    xsend(nr,buf,12);

    // Send map-modifying orbs
    buf[0] = SV_MAPDEVDATA;
    *(unsigned char*)(buf+1) = 1;
    *(unsigned short*)(buf+2) = amap_devices[amap_dev].orbs[AMAP_ORB_ADD1];
    *(unsigned short*)(buf+4) = amap_devices[amap_dev].orbs[AMAP_ORB_ADD2];
    *(unsigned short*)(buf+6) = amap_devices[amap_dev].orbs[AMAP_ORB_ADD3];
    *(unsigned short*)(buf+8) = amap_devices[amap_dev].orbs[AMAP_ORB_SCOUR];
    *(unsigned short*)(buf+10) = amap_devices[amap_dev].orbs[AMAP_ORB_SHIVA];
    xsend(nr,buf,12);

    // Whether to open gui
    buf[0] = SV_MAPDEVDATA;
    *(unsigned char*)(buf+1) = 2;
    *(unsigned char*)(buf+2) = open_gui;
    xsend(nr,buf,3);
}

// Send data about selected map (takes map position instead of index)
void send_selmap_data(int nr, int sel_pos)
{
    if (!player[nr].sock) return;

    int amap = amap_coords_to_ind(sel_pos);
    if (amap == -1) return;
    
    int cn = player[nr].usnr;
    int amap_dev = get_mapdevice(cn);
    if (amap_dev == -1) return;

    unsigned char buf[16];

    // Send name
    buf[0] = SV_SELMAPDATA;
    *(unsigned char*)(buf+1)=0;
    for (int n=0; n<14; n++) buf[n+2]=amap_bases[amap].name[n];
    xsend(nr,buf,16);

    if (strlen(amap_bases[amap].name) > 14) {
        buf[0] = SV_SELMAPDATA;
        *(unsigned char*)(buf+1)=1;
        for (int n=0; n<14; n++) buf[n+2]=amap_bases[amap].name[n+14];
        xsend(nr,buf,16);
    }

    if (strlen(amap_bases[amap].name) > 28) {
        buf[0] = SV_SELMAPDATA;
        *(unsigned char*)(buf+1)=2;
        for (int n=0; n<12; n++) buf[n+2]=amap_bases[amap].name[n+28];
        xsend(nr,buf,14);
    }

    // Send mods
    for (int i=0; i<AMAP_MAXMODS; i++) {
        buf[0] = SV_SELMAPDATA;
        *(unsigned char*)(buf+1)=3;
        *(unsigned char*)(buf+2)=i;
        *(unsigned int*)(buf+3)=amap_devices[amap_dev].map_mods[amap][i];
        xsend(nr,buf,7);
    }
}

// Applies map-related features and modifiers to the given instance, additionally using data from the given player's map device
void instance_makemap(int cn, int inst_id, int amap)
{
    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) {
        do_char_log(cn, 0, "Could not load your map device data. Map remains unmodified.\n");
    }
    
    // Apply monster modifiers
    if (map_instances[inst_id].mobs_remaining > 0) {
        for (int i=0; i<map_instances[inst_id].height; i++) {
            for (int j=0; j<map_instances[inst_id].width; j++) {
                int co = map_instancedtiles[inst_id][j+i*map_instances[inst_id].width].ch;
                if (!co) continue;
                if (!(ch[co].flags&(CF_PLAYER|CF_USURP)) && ch[co].alignment < 0) {
                    if (mapdev != -1) {
                        int modval = 0;
                        // Mob life
                        if ((modval=amap_getmod(cn, amap, MMOD_MOBLIFE)) > 0) {
                            ch[co].hp[0] = (float)ch[co].hp[0] * (1 + (float)modval / 100);
                        }

                        //TODO - mob damage applies directly on hit and spell hits

                        // Mob immunity
                        if ((modval=amap_getmod(cn, amap, MMOD_MOBIMM)) > 0) {
                            ch[co].skill[SK_IMMUN][0] += modval;
                        }

                        // Mob BWI
                        if ((modval=amap_getmod(cn, amap, MMOD_MOBBWI)) > 0) {
                            ch[co].attrib[AT_BRAVE][0] += modval;
                            ch[co].attrib[AT_WILL][0] += modval;
                            ch[co].attrib[AT_INT][0] += modval;
                        }

                        // Mob AV
                        if ((modval=amap_getmod(cn, amap, MMOD_MOBIMM)) > 0) {
                            ch[co].armor_bonus += modval;
                        }

                        // Mob bless
                        if ((modval=amap_getmod(cn, amap, MMOD_MOBIMM)) > 0) {
                            ch[co].skill[SK_BLESS][0] += modval;
                        }

                        //TODO - unique boss life and damage, twinned mod
                    }
                    ch[co].flags |= CF_MAPBORN;
                    do_update_char(co);
                }
            }
        }
    }
}

int amap_getmod(int cn, int amap, int mod)
{
    if (amap < 0 || amap >= AMAP_MAXBASES) return 0;
    if (amap_bases[amap].used == USE_EMPTY) return 0;

    if (mod < 0 || mod >= AMAP_MAXMODS) return 0;

    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return 0;

    return amap_devices[amap].map_mods[amap][mod];
}

void orb_description(int cn, int orb)
{
    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return;

    do_char_log(cn, 1, "/|%d|To apply a map-modifying orb to the selected map, shift+click it.\n", FNT_ORANGE);
    switch(orb) {
        case AMAP_ORB_ADD1:
            do_char_log(cn, 1, "/|%d|Lesser orb of map alteration: /|%d|Adds 1 random modifier to the selected map.\n",
                FNT_ORANGE, FNT_YELLOW);
        break;

        case AMAP_ORB_ADD2:
            do_char_log(cn, 1, "/|%d|Orb of map alteration: /|%d|Adds 2 random modifiers to the selected map.\n",
                FNT_ORANGE, FNT_YELLOW);
        break;

        case AMAP_ORB_ADD3:
            do_char_log(cn, 1, "/|%d|Greater orb of map alteration: /|%d|Adds 3 random modifiers to the selected map.\n",
                FNT_ORANGE, FNT_YELLOW);
        break;

        case AMAP_ORB_SCOUR:
            do_char_log(cn, 1, "/|%d|Orb of map scouring: /|%d|Removes all modifiers from the selected map.\n",
                FNT_ORANGE, FNT_YELLOW);
        break;

        case AMAP_ORB_SHIVA:
            if (amap_devices[mapdev].orbs[AMAP_ORB_SHIVA] == 0) {
                do_char_log(cn, 1, "/|%d|Unknown orb: The purpose of this dark orb is not yet known...\n", FNT_DEMON);
            } else {
                do_char_log(cn, 1, "/|%d|Orb of Shiva: Spreads Shiva's influence on the selected map, with chaotic results.\n", FNT_DEMON);
            }
        break;
    }
}

void amap_mod_description(int cn, int amap, int mod_n)
{
    if (amap < 0 || amap >= AMAP_MAXBASES) return;
    if (amap_bases[amap].used == USE_EMPTY) return;

    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return;

    int mod = 0, i = 0;
    while (mod < AMAP_MAXMODS) {
        if (amap_devices[mapdev].map_mods[amap][mod] != 0) i++;
        if (i == mod_n + 1) break;
        mod++;
    }
    if (mod >= AMAP_MAXMODS) return;

    int modval = amap_devices[mapdev].map_mods[amap][mod];
    int val1=0, val2=0;
    switch(mod) {
        case MMOD_SKEL:
            do_char_log(cn, 1, "/|%d|Skeletal: /|%d|Area is inhabited by skeletons.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_UNDEAD:
            do_char_log(cn, 1, "/|%d|Undead: /|%d|Area is inhabited by undead.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_GARGS:
            do_char_log(cn, 1, "/|%d|Demonic: /|%d|Area is inhabited by gargoyles.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_GOLEMS:
            do_char_log(cn, 1, "/|%d|Rock-strewn: /|%d|Area is inhabited by golems.\n", FNT_ORANGE, FNT_BLUE);
        break;
        
        case MMOD_GHOSTS:
            do_char_log(cn, 1, "/|%d|Haunted: /|%d|Area is inhabited by ghosts.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_TWINNED:
            do_char_log(cn, 1, "/|%d|Twinned: /|%d|Area contains two unique bosses.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_MOBLIFE:
            do_char_log(cn, 1, "/|%d|Stalwart: /|%d|%d%% more monster life.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_PLRDCWV:
            do_char_log(cn, 1, "/|%d|Enfeebling: /|%d|Player WV is reduced by %d%% against monsters.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_MOBDMG:
            do_char_log(cn, 1, "/|%d|Savage: /|%d|%d%% increased monster damage.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_MOBIMM:
            do_char_log(cn, 1, "/|%d|Immune: /|%d|Monsters gain +%d immunity.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_MOBBWI:
            do_char_log(cn, 1, "/|%d|Robust: /|%d|Monsters gain +%d brave/will/int.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_BOSSDMG:
            do_char_log(cn, 1, "/|%d|Overlord's presence: /|%d|Unique boss deals %d%% increased damage.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_BOSSLIFE:
            do_char_log(cn, 1, "/|%d|Titan's presence: /|%d|Unique boss has %d%% increased life.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_MOBAV:
            do_char_log(cn, 1, "/|%d|Armoured: /|%d|Monsters gain +%d AV.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_MOBBLESS:
            do_char_log(cn, 1, "/|%d|Empowered bless: /|%d|Monsters gain +%d bless.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_MOBPOISON:
            val1 = modval % 100;
            val2 = floor(modval / 100);
            do_char_log(cn, 1, "/|%d|Venomous: /|%d|Monsters have a %d%% chance on hit to poison, dealing %d%% of the damage dealt over time.\n", FNT_ORANGE, FNT_BLUE,
                val1, val2);
        break;

        case MMOD_MOBCRITS:
            val1 = modval % 100;
            val2 = floor(modval / 100);
            do_char_log(cn, 1, "/|%d|Dangerous: /|%d|Monsters have a %d%% chance on hit to critically strike, dealing %d%% damage.\n", FNT_ORANGE, FNT_BLUE,
                val1, val2);
        break;

        case MMOD_PLRDCRECOV:
            val1 = modval % 1000;
            val2 = floor(modval / 1000);
            if (val1 == val2) {
                do_char_log(cn, 1, "/|%d|Smothering: /|%d|Players have %d%% less recovery rate of life, endurance and mana.\n", FNT_ORANGE, FNT_BLUE,
                    modval);
            } else {
                if (val1 == 100) {
                    do_char_log(cn, 1, "/|%d|Smothering: /|%d|Players don't regenerate life, and have %d%% less recovery rate of endurance and mana.\n", FNT_ORANGE, FNT_BLUE,
                        val2);
                } else {
                    do_char_log(cn, 1, "/|%d|Smothering: /|%d|Players have %d%% less recovery rate of life and %d%% less recovery rate of endurance and mana.\n", FNT_ORANGE, FNT_BLUE,
                        val1, val2);
                }
            }
        break;

        case MMOD_BRNGROUND:
            val1 = modval % 10;
            if (val1 == 2) do_char_log(cn, 1, "/|%d|Burned: /|%d|Area contains some patches of burning ground.\n", FNT_ORANGE, FNT_BLUE);
            else if (val1 == 3) do_char_log(cn, 1, "/|%d|Burned: /|%d|Area contains patches of burning ground.\n", FNT_ORANGE, FNT_BLUE);
            else if (val1 == 4) do_char_log(cn, 1, "/|%d|Burned: /|%d|Area contains numerous patches of burning ground.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_FRZGROUND:
            val1 = modval % 10;
            if (val1 == 2) do_char_log(cn, 1, "/|%d|Frozen: /|%d|Area contains some patches of freezing ground.\n", FNT_ORANGE, FNT_BLUE);
            else if (val1 == 3) do_char_log(cn, 1, "/|%d|Frozen: /|%d|Area contains patches of freezing ground.\n", FNT_ORANGE, FNT_BLUE);
            else if (val1 == 4) do_char_log(cn, 1, "/|%d|Frozen: /|%d|Area contains numerous patches of freezing ground.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_SHKGROUND:
            val1 = modval % 10;
            if (val1 == 2) do_char_log(cn, 1, "/|%d|Shocked: /|%d|Area contains some patches of shocking ground.\n", FNT_ORANGE, FNT_BLUE);
            else if (val1 == 3) do_char_log(cn, 1, "/|%d|Shocked: /|%d|Area contains patches of shocking ground.\n", FNT_ORANGE, FNT_BLUE);
            else if (val1 == 4) do_char_log(cn, 1, "/|%d|Shocked: /|%d|Area contains numerous patches of shocking ground.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_PLRDCHEAL:
            do_char_log(cn, 1, "/|%d|Drying: /|%d|Players have %d%% reduced effectiveness of healing effects.\n", FNT_ORANGE, FNT_BLUE,
                modval);
        break;

        case MMOD_DEATHTRAPS:
            do_char_log(cn, 1, "/|%d|Deadly: /|%d|Area contains %s deathtraps.\n", FNT_ORANGE, FNT_BLUE,
                (modval < 100) ? "" : "many");
        break;
        
        case MMOD_PLRDCIMM:
            val1 = modval % 100;
            val2 = floor(modval / 100);
            do_char_log(cn, 1, "/|%d|Vulnerating: /|%d|Players lose %d immunity and %d resistance.\n", FNT_ORANGE, FNT_BLUE,
                val1, val2);
        break;

        case MMOD_PLHPONHIT:
            val1 = modval % 100;
            val2 = floor(modval / 100);
            do_char_log(cn, 1, "/|%d|Excruciating: /|%d|Players have a %d%% chance of losing %d%% of their maximum life when hit.\n", FNT_ORANGE, FNT_BLUE,
                val1, val2);
        break;

        case MMOD_BREACH:
            do_char_log(cn, 1, "/|%d|Otherworldly: /|%d|Area contains %d breach%s.\n", FNT_ORANGE, FNT_BLUE,
                modval, (modval > 1) ? "es" : "");
        break;

        case MMOD_ICEGARGS:
            do_char_log(cn, 1, "/|%d|Glacial: /|%d|Area is inhabited by ice gargoyles.\n", FNT_ORANGE, FNT_BLUE);
        break;

        case MMOD_LIZARDS:
            do_char_log(cn, 1, "/|%d|Scaly: /|%d|Area is inhabited by lizards.\n", FNT_ORANGE, FNT_BLUE);
        break;
    }
}

int amap_get_totalmods(int cn, int amap)
{
    int mapdev = get_mapdevice(cn);
    int tot_mods = 0;
    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (amap_devices[mapdev].map_mods[amap][i]) tot_mods++;
    }
    return tot_mods;
}

// Get a random modifier that hasn't been applied to the given map for the given character
// Returns the new modifier's ID, or -1 if it fails
int amap_getrandmod(int cn, int amap)
{
    if (amap < 0 || amap >= AMAP_MAXBASES) return -1;
    if (amap_bases[amap].used == USE_EMPTY) return -1;

    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return -1;

    // List of weights for each mod
    int rollmod_list[AMAP_MAXMODS] = {0};

    // Only one "inhabited by..." mod per map
    int inhabmod = 0;
    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (amap_devices[mapdev].map_mods[amap][i] == 0) continue;

        switch(amap_devices[mapdev].map_mods[amap][i]) {
            case MMOD_SKEL:
            case MMOD_UNDEAD:
            case MMOD_GARGS:
            case MMOD_GOLEMS:
            case MMOD_GHOSTS:
            case MMOD_LIZARDS:
            case MMOD_ICEGARGS:
                inhabmod = 1;
            break;
        }
    }

    // All tiers
    if (!inhabmod) {
        rollmod_list[MMOD_SKEL] = 100;
        rollmod_list[MMOD_UNDEAD] = 100;
        rollmod_list[MMOD_GARGS] = 100;
        rollmod_list[MMOD_GOLEMS] = 100;
        rollmod_list[MMOD_GHOSTS] = 100;
    }
    rollmod_list[MMOD_TWINNED] = 100;
    rollmod_list[MMOD_BREACH] = 50;

    rollmod_list[MMOD_MOBLIFE] = 100;
    rollmod_list[MMOD_PLRDCWV] = 100;
    rollmod_list[MMOD_MOBDMG] = 100;
    rollmod_list[MMOD_MOBIMM] = 100;
    rollmod_list[MMOD_MOBBWI] = 100;
    rollmod_list[MMOD_BOSSDMG] = 100;
    rollmod_list[MMOD_BOSSLIFE] = 100;
    rollmod_list[MMOD_MOBAV] = 100;
    rollmod_list[MMOD_PLRDCRECOV] = 100;
    rollmod_list[MMOD_BRNGROUND] = 100;
    rollmod_list[MMOD_FRZGROUND] = 100;
    rollmod_list[MMOD_SHKGROUND] = 100;
    rollmod_list[MMOD_PLRDCHEAL] = 100;

    // Medium tier maps (non-exclusive)
    if (amap_bases[amap].tier >= AMAP_MEDTIER) {
        rollmod_list[MMOD_MOBBLESS] = 100;
        rollmod_list[MMOD_MOBPOISON] = 100;
        rollmod_list[MMOD_MOBCRITS] = 100;
        rollmod_list[MMOD_DEATHTRAPS] = 100;
        rollmod_list[MMOD_PLRDCIMM] = 100;
        rollmod_list[MMOD_PLHPONHIT] = 100;
        if (!inhabmod) rollmod_list[MMOD_LIZARDS] = 100;
    }

    // High tier maps (non-exclusive)
    if (amap_bases[amap].tier >= AMAP_HIGHTIER) {
        if (!inhabmod) rollmod_list[MMOD_ICEGARGS] = 100;
    }

    // Add up weights
    int weight_tot = 0;

    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (rollmod_list[i] <= 0) continue;
        if (amap_devices[mapdev].map_mods[amap][i] != 0) {
            rollmod_list[i] = 0;
            continue;
        }
        weight_tot += rollmod_list[i];
    }

    // Pick mod
    int rand_pick = RANDOM(weight_tot) + 1;
    int ret = -1;

    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (rollmod_list[i] <= 0) continue;

        rand_pick -= rollmod_list[i];
        if (rand_pick <= 0) {
            ret = i;
            break;
        }
    }

    return ret;
}

// Adds a random modifier to the given map, returning 1 on success, 0 on failure
int amap_addmod(int cn, int amap)
{
    // getrandmod() already checks for validity of amap and cn's map-device
    int newmod = amap_getrandmod(cn, amap);
    if (newmod == -1) return 0;

    int mapdev = get_mapdevice(cn);

    int maptier = 0;
    if (amap_bases[amap].tier >= AMAP_MEDTIER) maptier = 1;
    if (amap_bases[amap].tier >= AMAP_HIGHTIER) maptier = 2;

    // Mod-specific rolls
    int modroll = 0;

    switch(newmod) {
        // Roll-less mods (these just need to be 1 to be present)
        case MMOD_SKEL:
        case MMOD_UNDEAD:
        case MMOD_GARGS:
        case MMOD_GOLEMS:
        case MMOD_GHOSTS:
        case MMOD_TWINNED:
        case MMOD_LIZARDS:
        case MMOD_ICEGARGS:
            modroll = 1;
        break;

        // X% more monster life
        case MMOD_MOBLIFE:
            if (maptier == 0) modroll = 10 + RANDOM(6);
            else if (maptier == 1) modroll = 16 + RANDOM(10);
            else if (maptier == 2) modroll = 26 + RANDOM(8);
        break;

        // Player WV is reduced by X% against monsters
        case MMOD_PLRDCWV:
            if (maptier == 0) modroll = 5 + RANDOM(6);
            else if (maptier == 1) modroll = 15 + RANDOM(6);
            else if (maptier == 2) modroll = 21 + RANDOM(5);
        break;

        // X% increased monster damage
        case MMOD_MOBDMG:
            if (maptier == 0) modroll = 10 + RANDOM(6);
            else if (maptier == 1) modroll = 16 + RANDOM(13);
            else if (maptier == 2) modroll = 29 + RANDOM(7);
        break;

        // Monsters gain +X immunity
        case MMOD_MOBIMM:
            if (maptier == 0) modroll = 10;
            else if (maptier == 1) modroll = 20;
            else if (maptier == 2) modroll = 35 + RANDOM(6);
        break;

        // Monsters gain +X brave/will/int
        case MMOD_MOBBWI:
            if (maptier == 0) modroll = 5 + RANDOM(4);
            else if (maptier == 1) modroll = 9 + RANDOM(6);
            else if (maptier == 2) modroll = 15 + RANDOM(6);
        break;

        // Unique boss deals X% increased damage
        case MMOD_BOSSDMG:
            if (maptier < 2) modroll = 15 + RANDOM(6);
            else if (maptier == 2) modroll = 21 + RANDOM(10);
        break;

        // Unique boss has X% increased life
        case MMOD_BOSSLIFE:
            if (maptier < 2) modroll = 20 + RANDOM(5);
            else if (maptier == 2) modroll = 25 + RANDOM(9);
        break;

        // Monsters gain +X AV
        case MMOD_MOBAV:
            if (maptier == 0) modroll = 5 + RANDOM(3);
            else if (maptier == 1) modroll = 8 + RANDOM(6);
            else if (maptier == 2) modroll = 14 + RANDOM(7);
        break;

        // Monsters gain +X bless
        case MMOD_MOBBLESS:
            if (maptier == 1) modroll = 5 + RANDOM(4);
            else if (maptier == 2) modroll = 10 + RANDOM(6);
        break;

        // Monsters have a X% chance on hit to poison, dealing Y% of the damage dealt over time
        case MMOD_MOBPOISON:
            if (maptier == 1) {
                modroll = 15 + RANDOM(11);  // on-hit chance
                modroll += 1500;            // damage buffer

            } else if (maptier == 2) {
                modroll = 18 + RANDOM(16);
                modroll += 2500;
            }
        break;

        // Monsters have a X% chance to critically strike, dealing Y% more damage
        case MMOD_MOBCRITS:
            if (maptier == 1) {
                modroll = 4 + RANDOM(3);    // crit chance
                modroll += 15000;           // crit damage multiplier

            } else if (maptier == 2) {
                modroll = 7 + RANDOM(6);
                modroll += 22000;
            }
        break;

        // Players have X% less recovery rate of life, endurance and mana
        // Internally used as XXYYY, XX: mana and end recovery rate reduction, YYY: life recovery rate reduction
        case MMOD_PLRDCRECOV:
            if (maptier == 0) modroll = 20020;
            else if (maptier == 1) modroll = 40040;
            else if (maptier == 2) modroll = 60100;
        break;

        // Area has patches of burning ground
        // Internally used as XXXYYYZ, XXX: buff duration, YYY: freq, Z: patch radius
        case MMOD_BRNGROUND:
            if (maptier == 0) {
                modroll = 2;        // patch radius
                modroll += 500;     // frequency (this /10000 per tile)
                modroll += 360000;   // buff duration (in 1/18s of a second)

            } else if (maptier == 1) {
                modroll = 3;
                modroll += 1500;
                modroll += 540000;

            } else if (maptier == 2) {
                modroll = 4;
                modroll += 2500;
                modroll += 900000;
            }
        break;

        // Area has patches of freezing ground
        case MMOD_FRZGROUND:
            if (maptier == 0) {
                modroll = 2;
                modroll += 500;
                modroll += 450000;

            } else if (maptier == 1) {
                modroll = 3;
                modroll += 1500;
                modroll += 720000;

            } else if (maptier == 2) {
                modroll = 4;
                modroll += 2500;
                modroll += 900000;
            }
        break;

        // Area has patches of shocking ground
        case MMOD_SHKGROUND:
            if (maptier == 0) {
                modroll = 2;
                modroll += 500;
                modroll += 540000;

            } else if (maptier == 1) {
                modroll = 3;
                modroll += 1500;
                modroll += 810000;

            } else if (maptier == 2) {
                modroll = 4;
                modroll += 2500;
                modroll += 1260000;
            }
        break;

        // Players have X% reduced effectiveness of healing effects
        case MMOD_PLRDCHEAL:
            if (maptier == 0) modroll = 10;
            else if (maptier == 1) modroll = 20;
            else if (maptier == 2) modroll = 40;
        break;

        // Area contains deathtraps
        case MMOD_DEATHTRAPS:
            if (maptier == 1) modroll = 70;         // frequency (this /1000 per tile)
            else if (maptier == 2) modroll = 125;
        break;

        // Players lose X immunity and Y resistance
        case MMOD_PLRDCIMM:
            if (maptier == 1) {
                modroll = 8 + RANDOM(7);            // imm reduction
                modroll += (10 + RANDOM(7)) * 100;  // res reduction

            } else if (maptier == 2) {
                modroll = 19 + RANDOM(12);
                modroll += (20 + RANDOM(14)) * 100;
            }
        break;

        // Players have a X% chance of losing Y% of their maximum life when hit
        case MMOD_PLHPONHIT:
            if (maptier == 1) {
                modroll = 15;     // on-hit chance
                modroll += 100;   // % of maximum hp lost

            } else if (maptier == 2) {
                modroll = 33;
                modroll += (1 + RANDOM(2)) * 100;
            }
        break;

        // Area contains X breaches
        case MMOD_BREACH:
            modroll = 1 + RANDOM(2);
        break;
    }

    amap_devices[mapdev].map_mods[amap][newmod] = modroll;
    return 1;
}

// Randomly picks an existing mod from the selected map, removes it, and adds a new one
// Returns 1 on success, 0 on failure
int amap_rerollmod(int cn, int amap)
{
    if (amap < 0 || amap >= AMAP_MAXBASES) return 0;
    if (amap_bases[amap].used == USE_EMPTY) return 0;

    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return 0;

    int tot_mods = 0;
    int last_mod = -1;
    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (amap_devices[mapdev].map_mods[amap][i] != 0) {
            tot_mods++;
            last_mod = i;
        }
    }
    if (!tot_mods) return 0;

    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (amap_devices[mapdev].map_mods[amap][i] == 0) continue;

        if (!RANDOM(tot_mods) || (last_mod != -1 && i == last_mod)) {
            amap_devices[mapdev].map_mods[amap][i] = 0;
            amap_addmod(cn, amap);
            return 1;
        }
    }
    return 0;
}

// Removes all mods from the given map, returns amount of mods removed
int amap_scourmods(int cn, int amap)
{
    if (amap < 0 || amap >= AMAP_MAXBASES) return 0;
    if (amap_bases[amap].used == USE_EMPTY) return 0;

    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return 0;

    int rem = 0;
    for (int i=0; i<AMAP_MAXMODS; i++) {
        if (amap_devices[mapdev].map_mods[amap][i] != 0) {
            rem++;
        }
        amap_devices[mapdev].map_mods[amap][i] = 0;
    }
    return rem;
}

// Use a map-modifying orb on the given map
int amap_useorb(int cn, int amap, int orb)
{
    if (amap < 0 || amap >= AMAP_MAXBASES) return 0;
    if (amap_bases[amap].used == USE_EMPTY) return 0;

    int mapdev = get_mapdevice(cn);
    if (mapdev == -1) return 0;

    if (amap_devices[mapdev].orbs[AMAP_ORB_ADD1] < 1) {
            do_char_log(cn, 0, "You don't have any more of those.\n");
            return 0;
    }

    int madd = 0, mmod = 0, mrem = 0;
    int success = 0;
    int iter = 0;
    int map_mod_amt = amap_get_totalmods(cn, amap);
    switch(orb) {
        case AMAP_ORB_ADD1:
            if (map_mod_amt > 3) {
                do_char_log(cn, 0, "You can only add up to 4 modifiers with this orb.\n");
                return 0;
            }
            do_char_log(cn, 1, "/|%d|Applied a lesser orb of map alteration.\n", FNT_ORANGE);
            madd += amap_addmod(cn, amap);
            success = 1;
        break;

        case AMAP_ORB_ADD2:
        case AMAP_ORB_ADD3:
            iter = 2;
            if (orb == AMAP_ORB_ADD3) {
                do_char_log(cn, 1, "/|%d|Applied a greater orb of map alteration.\n", FNT_ORANGE);
                iter = 3;
            } else {
                do_char_log(cn, 1, "/|%d|Applied an orb of map alteration.\n", FNT_ORANGE);
            }

            for (int i=0; i<iter; i++) {
                if (map_mod_amt >= 6) {
                    mmod += amap_rerollmod(cn, amap);
                } else if (amap_addmod(cn, amap)) {
                    map_mod_amt++;
                    madd++;
                }
            }
            success = 1;
        break;

        case AMAP_ORB_SCOUR:
            if (map_mod_amt < 1) {
                do_char_log(cn, 0, "That map has no modifiers to remove.\n");
                return 0;
            }
            do_char_log(cn, 1, "/|%d|Applied an orb of map scouring.\n", FNT_ORANGE);
            mrem = amap_scourmods(cn, amap);
            success = 1;
        break;
    }

    if (!mmod && !madd && !mrem) {
        do_char_log(cn, 0, "However, nothing seems to happen.\n");
        return 0;
    }

    if (success) {
        if (madd) do_char_log(cn, 1, "> Added %d modifier%s.\n", madd, (madd == 1) ? "" : "s");
        if (mmod) do_char_log(cn, 1, "> Re-rolled %d modifier%s.\n", mmod, (mmod == 1) ? "" : "s");
        if (mrem) do_char_log(cn, 1, "> Removed %d modifier%s.\n", mrem, (mrem == 1) ? "" : "s");
        amap_devices[mapdev].orbs[orb]--;
        send_mapdev_data(ch[cn].player, 0);
        return 1;
    }

    return 0;
}