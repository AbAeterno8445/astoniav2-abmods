#include <stdlib.h>

#include "server.h"

int npc_hbeat_test(int cn);

// Template-specific behaviour when fighting
int npc_heartbeat_fight(int cn)
{
    if (ch[cn].flags&(CF_PLAYER|CF_USURP)) return 0;

    switch(ch[cn].temp) {
        case 1172: return npc_hbeat_test(cn);
    }

    return 0;
}

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