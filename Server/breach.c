#include <stdlib.h>

#include "server.h"

static int *breaches;
static int total_breaches = 0;

// Stores a new breach for processing, receiving the breach's item ID; returns 0 if it fails
int new_breach(int brc_id)
{
        if (total_breaches > 0) {
                // Check if breach is already in
                for (int i = 0; i < total_breaches; i++) {
                        if (breaches[i] == brc_id) return 0;
                }

                // Try to find existing free spot in array
                for (int i = 0; i < total_breaches; i++) {
                        if (breaches[i] == 0) {
                                breaches[i] = brc_id;
                                return 1;
                        }
                }
        }

        // Allocate new spot
        int* tmp_ptr;
        tmp_ptr = realloc(breaches, sizeof(int) * (total_breaches + 1));
        xlog("new size = %d (%d + 1)", sizeof(int) * (total_breaches + 1), total_breaches);
        if (tmp_ptr == NULL) {
                xlog("Could not allocate memory for a new breach.");
                return 0;
        }
        breaches = tmp_ptr;

        xlog("NEW BREACH ID %d total_breaches=%d", brc_id, total_breaches);
        breaches[total_breaches] = brc_id;
        total_breaches++;
        return 1;
}

/* breach item data[0]: timer for duration
data[1]: radius of total circle
data[2]: travelled distance to radius
data[3]: 0 or 1, opening or closing, respectively */
void breach_tick(int brc_in)
{
        if (it[brc_in].used == USE_EMPTY) return;
        if (it[brc_in].driver != 71) return;

        // Breach timer
        if (it[brc_in].data[0] > 0) it[brc_in].data[0]--;
        else it[brc_in].data[3] = 1;

        int brc_inst = it[brc_in].instance_id;
        if (brc_inst != -1 && !inst_isalive(brc_inst)) return;

        if (it[brc_in].data[3] == 0) {
                // Breach opening
                if (it[brc_in].data[2] < it[brc_in].data[1]) {
                        it[brc_in].data[2]++;

                        int circ_rad = it[brc_in].data[2];
                        int circ_rad_p2 = circ_rad * circ_rad;
                        for(int y = it[brc_in].y-circ_rad; y <= it[brc_in].y+circ_rad; y++){
                                for (int x = it[brc_in].x-circ_rad; x <= it[brc_in].x+circ_rad; x++) {
                                        if ((x-it[brc_in].x)*(x-it[brc_in].x) + (y-it[brc_in].y)*(y-it[brc_in].y) < circ_rad_p2) {
                                                if (brc_inst == -1) {
                                                        int m = x + y * MAPX;
                                                        if (map[m].flags&MF_PURPLE) continue;
                                                        map[m].flags |= MF_PURPLE;
                                                } else {
                                                        int m = x + y * map_instances[brc_inst].width;
                                                        if (map_instancedtiles[brc_inst][m].flags&MF_PURPLE) continue;
                                                        map_instancedtiles[brc_inst][m].flags |= MF_PURPLE;
                                                }

                                                // Spawn breach monsters
                                                if (!RANDOM(30)) {
                                                        int temp;
                                                        int r = RANDOM(5);
                                                        switch(r) {
                                                                case 0: temp=368; break;
                                                                case 1: temp=375; break;
                                                                case 2: temp=554; break;
                                                                case 3: temp=503; break;
                                                                default: temp=505; break;
                                                        }
                                                        int cc = pop_create_char(temp, 0, brc_inst);
                                                        if (!god_drop_char(cc, x, y, brc_inst)) {
                                                                god_destroy_items(cc);
                                                                ch[cc].used = USE_EMPTY;
                                                        } else {
                                                                fx_add_effect(5,0,ch[cc].x,ch[cc].y,0,brc_inst);
                                                                ch[cc].flags |= (CF_BREACH|CF_INST_TEMP);
                                                        }
                                                }
                                        }
                                }
                        }
                }
        } else {
                // Breach closing
                if (it[brc_in].data[2] > 0) {
                        int circ_rad = it[brc_in].data[2];
                        int circ_rad_p2 = circ_rad * circ_rad;

                        it[brc_in].data[2]--;

                        for(int y = it[brc_in].y-circ_rad; y <= it[brc_in].y+circ_rad; y++){
                                for (int x = it[brc_in].x-circ_rad; x <= it[brc_in].x+circ_rad; x++) {
                                        if ((x-it[brc_in].x)*(x-it[brc_in].x) + (y-it[brc_in].y)*(y-it[brc_in].y) >= circ_rad_p2) {
                                                if (brc_inst == -1) {
                                                        int m = x + y * MAPX;
                                                        if (map[m].flags&MF_PURPLE) {
                                                                map[m].flags &=~MF_PURPLE;
                                                        }
                                                } else {
                                                        int m = x + y * map_instances[brc_inst].width;
                                                        if (map_instancedtiles[brc_inst][m].flags&MF_PURPLE) {
                                                                map_instancedtiles[brc_inst][m].flags &=~MF_PURPLE;
                                                        }
                                                }
                                        }
                                }
                        }
                } else {
                        // Breach dies
                        fx_add_effect(5,0,it[brc_in].x,it[brc_in].y,0,brc_inst);
                        if (brc_inst == -1) {
                                int m = it[brc_in].x + it[brc_in].y * MAPX;
                                if (map[m].flags&MF_PURPLE) {
                                        map[m].flags &=~MF_PURPLE;
                                }
                        } else {
                                int m = it[brc_in].x + it[brc_in].y * map_instances[brc_inst].width;
                                if (map_instancedtiles[brc_inst][m].flags&MF_PURPLE) {
                                        map_instancedtiles[brc_inst][m].flags &=~MF_PURPLE;
                                }
                        }
                        it[brc_in].used = USE_EMPTY;
                        for (int i = 0; i < total_breaches; i++) {
                                if (breaches[i] == brc_in) {
                                        breaches[i] = 0;
                                        break;
                                }
                        }
                }
        }
}

void process_breaches(int cltick)
{
        for (int n=0; n<total_breaches; n++) {
                if (it[breaches[n]].used == USE_EMPTY) continue;

                // Opening breaches are processed every second, closing breaches every 1/2 second
                if ((it[breaches[n]].data[3] == 0 && cltick % 18 == 0) || (it[breaches[n]].data[3] == 1 && cltick % 9 == 0)) {
                        breach_tick(breaches[n]);
                }
        }
}