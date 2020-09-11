#include <stdlib.h>

#include "server.h"
#include "npc.h"

int npc_hbeat_test(int cn);
int npc_hbeat_plaguewing(int cn);
int npc_hbeat_plaguewing_fight(int cn);

// High-prio heartbeat
int npc_hbeat_high(int cn)
{
    // Check special driver (data[25])
    switch(ch[cn].data[25]) {
        case 10: return npc_hbeat_plaguewing(cn);   // 10 - Plaguewing boss
    }

    return 0;
}

// Low-prio heartbeat
void npc_hbeat_low(int cn)
{
    return;
}

// Message notif heartbeat
int npc_hbeat_msg(int cn)
{
    return 0;
}

// Fighting heartbeat
int npc_hbeat_fight(int cn)
{
    if (ch[cn].flags&(CF_PLAYER|CF_USURP)) return 0;

    // Template-specific
    switch(ch[cn].temp) {
        case 1172: return npc_hbeat_test(cn);
    }

    // Special driver
    switch(ch[cn].data[25]) {
        case 10: return npc_hbeat_plaguewing_fight(cn);
    }

    return 0;
}

// Spellcaster test
int npc_hbeat_test(int cn)
{
    int co = ch[cn].attack_cn;
    if (!co) return 0;

    if (!RANDOM(20)) {
        // Cast blastclaw
        ch[cn].skill_nr=SSK_BLASTCLAW;
        ch[cn].skill_target1=co;
        return 1;
    }

    return 0;
}

// Plaguewing boss heartbeat
int npc_hbeat_plaguewing(int cn)
{
    // Special skills
    switch(ch[cn].data[1]) {
        case 1: // Backblink skill process
            if (ch[cn].data[1] == 1 && ch[cn].data[0] > 0) {
                int xx = 0, yy = 0;
                int xo = ch[cn].x, yo = ch[cn].y;
                switch(ch[cn].dir) {
                    case    DX_RIGHT:   if (check_map_go(xo - 1, yo, ch[cn].instance_id)) xx=-1; else ch[cn].data[0] = 0; break;
                    case    DX_LEFT:    if (check_map_go(xo + 1, yo, ch[cn].instance_id)) xx=1; else ch[cn].data[0] = 0; break;
                    case    DX_UP:      if (check_map_go(xo, yo + 1, ch[cn].instance_id)) yy=1; else ch[cn].data[0] = 0; break;
                    case    DX_DOWN:    if (check_map_go(xo, yo - 1, ch[cn].instance_id)) yy=-1; else ch[cn].data[0] = 0; break;
                    default: break;
                }

                if (ch[cn].data[0] == 0) return 0;

                if (god_transfer_char(cn, xo + xx, yo + yy, ch[cn].instance_id)) {
                    add_spellfx(FX_FLRWARN_TR, TICKS*3, xo + yy, yo + xx, cn, FXS_BLAST, 200, SFX_BLAST, ch[cn].instance_id);
                    add_spellfx(FX_FLRWARN_TR, TICKS*2 + 14, xo, yo, cn, FXS_BLAST, 240, SFX_BLAST, ch[cn].instance_id);
                    add_spellfx(FX_FLRWARN_TR, TICKS*3, xo - yy, yo - xx, cn, FXS_BLAST, 200, SFX_BLAST, ch[cn].instance_id);

                    fx_add_effect(12, 0, xo, yo, 0, ch[cn].instance_id);
                    ch[cn].data[0]--;
                } else {
                    ch[cn].data[0] = 0;
                }
                return 1;
            }
        break;

        case 2: // Hiding skill process
            if (ch[cn].data[0] > 0) {
                ch[cn].data[0]--;
                if (ch[cn].data[0] <= TICKS*3 && ch[cn].data[2] == 0) {
                    // Pick player
                    int co = 0;
                    if (ch[cn].instance_id == -1) {
                        for (int i=651; i<=681; i++) {
                            if (co) break;
                            for (int j=601; j<=631; j++) {
                                if ((co = map[j+i*MAPX].ch) != 0 && ch[co].flags&(CF_PLAYER|CF_USURP)) {
                                    break;
                                }
                            }
                        }
                    } else {
                        //TODO - handle instanced version
                    }
                    int side = RANDOM(4);
                    int fails = 0;
                    int xx = 0, yy = 0;

                    // Check what spot adjacent to target is free, randomly picking first one
                    for (int z=0; z<4; z++) {
                        //TODO - handle boss getting stuck if no players in arena
                        if (!co) {
                            fails++;
                            continue;
                        }

                        switch(side % 4) {
                            case 0: if (check_map_go(ch[co].x + 1, ch[co].y, ch[co].instance_id)) xx++; break;
                            case 1: if (check_map_go(ch[co].x - 1, ch[co].y, ch[co].instance_id)) xx--; break;
                            case 2: if (check_map_go(ch[co].x, ch[co].y - 1, ch[co].instance_id)) yy--; break;
                            case 3: if (check_map_go(ch[co].x, ch[co].y + 1, ch[co].instance_id)) yy++; break;
                        }

                        if (xx != 0 || yy != 0) {
                            int new_x = ch[co].x + xx;
                            int new_y = ch[co].y + yy;
                            ch[cn].data[2] = new_x;
                            ch[cn].data[3] = new_y;

                            // Cross effect at new position
                            add_spellfx(FX_FLRWARN_CR1, ch[cn].data[0], new_x, new_y, cn, FXS_BLAST, 0, SFX_SPELLXC, ch[cn].instance_id);
                            // Blast cross
                            for (int i=1; i<=3; i++) {
                                add_spellfx(FX_FLRWARN_TR, ch[cn].data[0] + i*2, new_x + i, new_y, cn, FXS_BLAST, 240, SFX_BLAST, ch[cn].instance_id);
                            }
                            for (int i=1; i<=3; i++) {
                                add_spellfx(FX_FLRWARN_TR, ch[cn].data[0] + i*2, new_x - i, new_y, cn, FXS_BLAST, 240, 0, ch[cn].instance_id);
                            }
                            for (int i=1; i<=3; i++) {
                                add_spellfx(FX_FLRWARN_TR, ch[cn].data[0] + i*2, new_x, new_y - i, cn, FXS_BLAST, 240, 0, ch[cn].instance_id);
                            }
                            for (int i=1; i<=3; i++) {
                                add_spellfx(FX_FLRWARN_TR, ch[cn].data[0] + i*2, new_x, new_y + i, cn, FXS_BLAST, 240, 0, ch[cn].instance_id);
                            }
                            break;
                        } else {
                            fails++;
                        }
                        side++;
                    }
                    if (fails == 4) ch[cn].data[0]+=TICKS;
                }
                return 1;
            }

            // Reveal from hiding
            god_transfer_char(cn, ch[cn].data[2], ch[cn].data[3], ch[cn].instance_id);

            ch[cn].data[1] = 0;
            ch[cn].data[2] = 0;
            ch[cn].data[3] = 0;
            fx_add_effect(FX_EVILMAGIC, 0, ch[cn].x, ch[cn].y, 0, ch[cn].instance_id);
            add_exhaust(cn, TICKS + TICKS/2);
        break;
    }

    return 0;
}

int npc_hbeat_plaguewing_fight(int cn)
{
    int co = ch[cn].attack_cn;
    if (!co) return 0;

    if (is_exhausted(cn)) return 0;

    int dist = npc_dist(cn, co);
    if (dist <= 2) {
        if (!RANDOM(22)) {
            // Cast blaststar
            ch[cn].skill_nr = SSK_BLASTSTAR;
            add_exhaust(cn, TICKS*4);
            return 1;
        }
    }

    if (!RANDOM(25)) {
        // Hiding skill
        int old_x = ch[cn].x;
        int old_y = ch[cn].y;
        if (god_transfer_char(cn, 610, 660, -1)) {
            ch[cn].data[1] = 2;
            ch[cn].data[0] = TICKS*4 + TICKS/2 + TICKS*RANDOM(4);
            npc_reset_orders(cn);

            fx_add_effect(FX_DEATHMIST, 0, old_x, old_y, 0, ch[cn].instance_id);
            if (ch[cn].instance_id == -1) do_area_sound(0, 0, old_x, old_y, SFX_MANACLEAVE);
            else do_area_sound_inst(ch[cn].instance_id, 0, 0, old_x, old_y, SFX_MANACLEAVE);
            return 1;
        }
    }

    // Melee skills
    if (is_facing(cn,co)) {
        if (!RANDOM(15)) {
            // Backblink skill
            ch[cn].data[0] = 6;
            ch[cn].data[1] = 1;
            npc_reset_orders(cn);
            add_exhaust(cn, TICKS*4);
            return 1;
        }
        else if (!RANDOM(15)) {
            // Piercer skill
            int xx = 0, yy = 0;
            int xo = ch[cn].x, yo = ch[cn].y;
            switch(ch[cn].dir) {
                case    DX_RIGHT:   if (check_map_go(ch[co].x + 1, ch[co].y, ch[cn].instance_id)) xx=1; break;
                case    DX_LEFT:    if (check_map_go(ch[co].x - 1, ch[co].y, ch[cn].instance_id)) xx=-1; break;
                case    DX_UP:      if (check_map_go(ch[co].x, ch[co].y - 1, ch[cn].instance_id)) yy=-1; break;
                case    DX_DOWN:    if (check_map_go(ch[co].x, ch[co].y + 1, ch[cn].instance_id)) yy=1; break;
            }

            if ((xx != 0 || yy != 0) && check_map_go(ch[co].x + xx, ch[co].y + yy, ch[cn].instance_id)) {
                if (god_transfer_char(cn, ch[co].x + xx, ch[co].y + yy, ch[co].instance_id)) {
                    add_spellfx(FX_FLRWARN_TR, TICKS*2 + 8, xo, yo, cn, FXS_BLAST, 200, SFX_BLAST, ch[cn].instance_id);
                    add_spellfx(FX_FLRWARN_TR, TICKS*2 + 9, ch[co].x, ch[co].y, cn, FXS_BLAST, 200, SFX_BLAST, ch[cn].instance_id);
                    add_spellfx(FX_FLRWARN_TR, TICKS*2 + 10, ch[cn].x, ch[cn].y, cn, FXS_BLAST, 200, SFX_BLAST, ch[cn].instance_id);

                    fx_add_effect(12, 0, xo, yo, 0, ch[cn].instance_id);
                    add_exhaust(cn, TICKS*2);

                    ch[cn].attack_cn = co;
                    return 1;
                }
            }
        }
    }

    return 0;
}

void skill_blastclaw(int cn)
{
    int co = ch[cn].skill_target1;
    if (!co) return;
    if (!do_char_can_see(cn, co)) return;

    int sp_proc = 0;
    for (int i=ch[co].y-2; i<=ch[co].y+2; i++) {
        sp_proc++;
        if (sp_proc % 2 == 0) continue;

        int pos = 0;
        for (int j=ch[co].x-2; j<=ch[co].x+2; j++) {
            int sfx = 0;
            if (pos % 2 == 0 && sp_proc == 3) sfx = SFX_BLAST;

            add_spellfx(FX_FLRWARN_TR, TICKS*2 + pos*2, j, i, cn, FXS_BLAST, 100, sfx, ch[co].instance_id);
            pos++;
        }
    }
}

void skill_blaststar(int cn)
{
    for (int i=-2; i<=2; i++) {
        for (int j=-2; j<=2; j++) {
            if (j == 0 && i == 0) continue;
            
            if ((abs(j) == 1 && abs(i) != 2) || (abs(i) == 1 && abs(j) != 2) || j == 0 || i == 0) {
                int timer = TICKS*2 + 8;
                if (abs(j) == 2 || abs(i) == 2) timer = TICKS*2 + 12;

                int sfx = 0;
                if (j == 0 || i == 0) sfx = SFX_BLAST;
                add_spellfx(FX_FLRWARN_TR, timer, ch[cn].x + j, ch[cn].y + i, cn, FXS_BLAST, 230, sfx, ch[cn].instance_id);
            }
        }
    }
}