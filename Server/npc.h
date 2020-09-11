/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

int npc_dist(int cn,int co);
int npc_try_spell(int cn,int co,int spell);
int npc_check_target(int x,int y);

// -- stunrun.c
int npc_stunrun_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int npc_stunrun_high(int cn);
void npc_stunrun_low(int cn);

// -- cityattack.c
int npc_cityattack_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int npc_cityattack_high(int cn);
void npc_cityattack_low(int cn);

// -- npc_malte.c
int npc_malte_msg(int cn,int type,int dat1,int dat2,int dat3,int dat4);
int npc_malte_high(int cn);
void npc_malte_low(int cn);

// -- npc heartbeat --
int npc_hbeat_msg(int cn);
int npc_hbeat_fight(int cn);
int npc_hbeat_high(int cn);
void npc_hbeat_low(int cn);

// -- special npc skills --
void skill_blastclaw(int cn);
void skill_blaststar(int cn);

#define SSK_BLASTCLAW   200
#define SSK_BLASTSTAR   201