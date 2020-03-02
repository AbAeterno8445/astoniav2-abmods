#include <alloc.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"

int init_done=0;
int frame=0;
extern int mx,my;

extern int ticker;

int pskip=0,pidle=0;
extern int t_size;

extern int cursor_type;
extern HCURSOR cursor[10];

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff;
extern int screen_overlay_sprite;
extern short screen_windowed;
extern short screen_renderdist;

int cursedtxt_off = 0;
int col_eff_ticker = 0;
int col_eff_flag = 0;

struct areamap_base *amap_bases;
struct areamap_selected amap_sel = {0};
int maptiers = 5;
int mapdev_level = 1;
long mapdev_exp = 0;
long mapdev_expreq = 200;
int amap_orbs[5] = {0};
int amap_hovering = -1;
int amap_orbhover = -1;
int amap_modhover = -1;
int amap_selected = -1;

// from dd.c
int copysprite(int nr,int effect,int x,int y,int xoff,int yoff);
void dd_flip(void);
void dd_flip_windowed(void);
void dd_showbar(int xf,int yf,int xs,int ys,unsigned short col);
void copyspritex(int nr,int xpos,int ypos,int effect);
void dd_showbox(int xf,int yf,int xs,int ys,unsigned short col);
void dd_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff);
int get_avgcol(int nr);

char *lookup(int nr,unsigned short id);

extern char input[];
extern int in_len;
extern int cur_pos;
extern int view_pos;
extern int logstart;
extern int logtimer;
extern int RED,GREEN,BLUE;
int tput=0;

extern int do_shadow;

void do_msg(void);

char *at_name[5]={
	"Braveness",
	"Willpower",
	"Intuition",
	"Agility",
	"Strength"};

#define AT_BRAVE        0
#define AT_WILL         1
#define AT_INT          2
#define AT_AGIL         3
#define AT_STREN        4

struct skilltab *skilltab;
struct skilltab _skilltab[50]={
	{0,     'C',    "Hand to Hand", "Fighting without weapons.",                    {AT_BRAVE,AT_AGIL,AT_STREN}},
	{1,     'C',    "Karate",       "Fighting without weapons and doing damage.",   {AT_BRAVE,AT_AGIL,AT_STREN}},
	{2,     'C',    "Dagger",       "Fighting with daggers or similiar weapons.",   {AT_BRAVE,AT_AGIL,AT_INT}},
	{3,     'C',    "Sword",        "Fighting with swords or similiar weapons.",    {AT_BRAVE,AT_AGIL,AT_STREN}},
	{4,     'C',    "Axe",          "Fighting with axes or similiar weapons.",      {AT_BRAVE,AT_STREN,AT_STREN}},
	{5,     'C',    "Staff",        "Fighting with staffs or similiar weapons.",    {AT_AGIL,AT_STREN,AT_STREN}},
	{6,     'C',    "Two-Handed",   "Fighting with two-handed weapons.",            {AT_AGIL,AT_STREN,AT_STREN}},

	{7,     'G',    "Lock-Picking", "Opening doors without keys.",                  {AT_INT,AT_WILL,AT_AGIL}},
	{8,     'G',    "Stealth",      "Moving without being seen or heard.",          {AT_INT,AT_WILL,AT_AGIL}},
	{9,     'G',    "Perception",   "Seeing and hearing.",                          {AT_INT,AT_WILL,AT_AGIL}},

	{10,    'M',    "Swimming",     "Moving through water without drowning.",       {AT_INT,AT_WILL,AT_AGIL}},
	{11,    'R',    "Magic Shield", "Spell: Create a magic shield (Cost: 25 Mana).",  {AT_BRAVE,AT_INT,AT_WILL}},

	{12,    'G',    "Bartering",    "Getting good prices from merchants.",          {AT_BRAVE,AT_INT,AT_WILL}},
	{13,    'G',    "Repair",       "Repairing items.",                             {AT_INT,AT_WILL,AT_AGIL}},

	{14,    'R',    "Light",        "Spell: Create light (Cost: 5 Mana).",           {AT_BRAVE,AT_INT,AT_WILL}},
	{15,    'R',    "Recall",       "Spell: Teleport to temple (Cost: 15 Mana).",    {AT_BRAVE,AT_INT,AT_WILL}},
	{16,    'R',    "Guardian Angel","Spell: Avoid loss of HPs and items on death.", {AT_BRAVE,AT_INT,AT_WILL}},
	{17,    'R',    "Protection",   "Spell: Enhance Armor of target (Cost: 15 Mana).", {AT_BRAVE,AT_INT,AT_WILL}},
	{18,    'R',    "Enhance Weapon","Spell: Enhance Weapon of target (Cost: 15 Mana).", {AT_BRAVE,AT_INT,AT_WILL}},
	{19,    'R',    "Stun",         "Spell: Make target motionless (Cost: 20 Mana).",   {AT_BRAVE,AT_INT,AT_WILL}},
	{20,    'R',    "Curse",        "Spell: Decrease attributes of target (Cost: 35 Mana).",  {AT_BRAVE,AT_INT,AT_WILL}},
	{21,    'R',    "Bless",        "Spell: Increase attributes of target (Cost: 35 Mana).", {AT_BRAVE,AT_INT,AT_WILL}},
	{22,    'R',    "Identify",     "Spell: Read stats of item/character. (Cost: 25 Mana)",   {AT_BRAVE,AT_INT,AT_WILL}},

	{23,    'G',    "Resistance",   "Resist against magic.",                        {AT_INT,AT_WILL,AT_STREN}},

	{24,    'R',    "Blast",        "Spell: Inflict injuries to target (Cost: varies).", {AT_INT,AT_WILL,AT_STREN}},
	{25,    'R',    "Dispel Magic", "Spell: Removes curse magic from target (Cost: 25 Mana).", {AT_BRAVE,AT_INT,AT_WILL}},

	{26,    'R',    "Heal",         "Spell: Heal injuries (Cost: 25 Mana).",         {AT_BRAVE,AT_INT,AT_WILL}},
	{27,    'R',    "Ghost Companion","Spell: Create a ghost to attack an enemy.",    {AT_BRAVE,AT_INT,AT_WILL}},

	{28,    'B',    "Regenerate",   "Regenerate Hitpoints faster.",                 {AT_STREN,AT_STREN,AT_STREN}},
	{29,    'B',    "Rest",         "Regenerate Endurance faster.",                 {AT_AGIL,AT_AGIL,AT_AGIL}},
	{30,    'B',    "Meditate",     "Regenerate Mana faster.",                      {AT_INT,AT_WILL,AT_WILL}},

	{31,    'G',    "Sense Magic",  "Find out who casts what at you.",              {AT_BRAVE,AT_INT,AT_WILL}},
	{32,    'G',    "Immunity",     "Partial immunity against negative magic.",     {AT_BRAVE,AT_AGIL,AT_STREN}},
	{33,    'G',    "Surround Hit", "Hit all your enemies at once.",                {AT_BRAVE,AT_AGIL,AT_STREN}},
	{34,    'G',    "Concentrate",  "Reduces mana cost for all spells.",            {AT_WILL,AT_WILL,AT_WILL}},
	{35,    'G',    "Warcry",       "Frighten all enemies in hearing distance.",    {AT_BRAVE,AT_BRAVE,AT_STREN}},

	{36,   'G',   	"Accuracy",		"Increases chance to hit enemies.",				{AT_BRAVE,AT_AGIL,AT_STREN}},
	{37,   'G',   	"Evasion",		"Increases chance to avoid enemies.",			{AT_AGIL,AT_AGIL,AT_AGIL}},
	
	{38,   'C',   	"Shield",		"Allows you to use stronger shields and block more attacks.", {AT_STREN,AT_BRAVE,AT_AGIL}},
	{39,   'C',   	"Wand",			"Fighting with wands or similar weapons.",		{AT_INT,AT_WILL,AT_WILL}},

	{40,   'Z',   "", "", {0,0,0,}},
	{41,   'Z',   "", "", {0,0,0,}},
	{42,   'Z',   "", "", {0,0,0,}},
	{43,   'Z',   "", "", {0,0,0,}},
	{44,   'Z',   "", "", {0,0,0,}},
	{45,   'Z',   "", "", {0,0,0,}},
	{46,   'Z',   "", "", {0,0,0,}},
	{47,   'Z',   "", "", {0,0,0,}},
	{48,   'Z',   "", "", {0,0,0,}},
	{49,   'Z',   "", "", {0,0,0,}}};

int skill_cmp(const void *a,const void *b)
{
	const struct skilltab *c,*d;
	int m1,m2;

	c=a; d=b;

	m1=c->nr; m2=d->nr;

	if (m1==99 && m2!=99) return 1;
	if (m2==99 && m1!=99) return -1;

	if (pl.skill[m1][0]==0 && pl.skill[m2][0]!=0) return 1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]!=0) return -1;

	if (c->sortkey>d->sortkey) return 1;
	if (c->sortkey<d->sortkey) return -1;

	return strcmp(c->name,d->name);
}

// from main.c
extern int quit;

int idle=0;
int ttime=0,xtime=0;
int ctick=0;

int do_exit=0;

int xoff=0,yoff=0;

extern int selected_char;

struct look look={0,{0,0,0,0,0,0,0,0,0,0},0,0,"",0,0,0,0};

// ************* CHARACTER ****************

struct cplayer pl;

static char *rank[24]={
	"Private",
	"Private First Class",
	"Lance Corporal",
	"Corporal",
	"Sergeant",
	"Staff Sergeant",
	"Master Sergeant",
	"First Sergeant",
	"Sergeant Major",
	"Second Lieutenant",
	"First Lieutenant",
	"Captain",
	"Major",
	"Lieutenant Colonel",
	"Colonel",
	"Brigadier General",
	"Major General",
	"Lieutenant General",
	"General",
	"Field Marshal",
	"Knight",
	"Baron",
	"Earl",
	"Warlord"
};

int stat_raised[108]={0,0,0,0,0,0,0,0,0,0,0,0,0,};
int stat_points_used=0;

int points2rank(int v)
{
	if (v<      50)	return 0;
	if (v<     850)	return 1;
	if (v<    4900)	return 2;
	if (v<   17700)	return 3;
	if (v<   48950)	return 4;
	if (v<  113750)	return 5;
	if (v<  233800)	return 6;
	if (v<  438600)	return 7;
	if (v<  766650)	return 8;
	if (v< 1266650)	return 9;
	if (v< 1998700)	return 10;
	if (v< 3035500)	return 11;
	if (v< 4463550)	return 12;
	if (v< 6384350)	return 13;
	if (v< 8915600)	return 14;
	if (v<12192400)	return 15;
	if (v<16368450)	return 16;
	if (v<21617250)	return 17;
	if (v<28133300)	return 18;
	if (v<36133300)	return 19;

	if (v<49014500)	return 20;
	if (v<63000600)	return 21;
	if (v<80977100)	return 22;

	return 23;
}

int rank2points(int v)
{
	if (v==1)	return 50;
	if (v==2)	return 850;
	if (v==3)	return 4900;
	if (v==4)	return 17700;
	if (v==5)	return 48950;
	if (v==6)	return 113750;
	if (v==7)	return 233800;
	if (v==8)	return 438600;
	if (v==9)	return 766650;
	if (v==10)	return 1266650;
	if (v==11)	return 1998700;
	if (v==12)	return 3035500;
	if (v==13)	return 4463550;
	if (v==14)	return 6384350;
	if (v==15)	return 8915600;
	if (v==16)	return 12192400;
	if (v==17)	return 16368450;
	if (v==18)	return 21617250;
	if (v==19)	return 28133300;

	if (v==20)	return 36133300;
	if (v==21)	return 49014500;
	if (v==22)	return 63000600;
	if (v==23)	return 80977100;

	return 0;
}

/* Calculates experience to next level from current experience and the
   points2rank() function. As no inverse function is supplied we use a
   binary search to determine the experience for the next level.
   If the given number of points corresponds to the highest level,
   return 0. */
int points_tolevel(int curr_exp)
{
        int curr_level, next_level, r, j;  //, p0, p5, p9;

		if (!curr_exp) return 50;	//0 exp
        curr_level = points2rank(curr_exp);
        if (curr_level == 23) return 0;
        next_level = curr_level + 1;

		r = rank2points(next_level);
		j = r-curr_exp;
		
		return j;

        /*p0 = 1;
        p5 = 1;
        p9 = 20 * curr_exp;
        for (j=0; p0<p9 && j<100; j++) {
                p5 = (p0 + p9) / 2;
                r = points2rank(curr_exp + p5);
                if (r < next_level) {
                        p0 = p5 + 1;
                } else {
                        p9 = p5 - 1;
                }
        }
        if (p0 > (20*curr_exp)) return 0;       // Can't do it
        p5++;
        return p5;*/
}

int attrib_needed(int n,int v)
{
	if (v>=pl.attrib[n][2])	return HIGH_VAL;

	return v*v*v*pl.attrib[n][3]/20;
}

int hp_needed(int v)
{
	if (v>=pl.hp[2]) return HIGH_VAL;

	return v*pl.hp[3];
}

int end_needed(int v)
{
	if (v>=pl.end[2]) return HIGH_VAL;

	return v*pl.end[3]/2;
}

int mana_needed(int v)
{
	if (v>=pl.mana[2]) return HIGH_VAL;

	return v*pl.mana[3];
}

int skill_needed(int n,int v)
{
	if (v>=pl.skill[n][2]) return HIGH_VAL;

	return max(v,v*v*v*pl.skill[n][3]/40);
}

// ************* MAP **********************

struct cmap *map=NULL;

void eng_init_map(void)
{
	int n;

	map=calloc(screen_renderdist*screen_renderdist*sizeof(struct cmap),1);

	for (n=0; n<screen_renderdist*screen_renderdist; n++)	map[n].ba_sprite=SPR_EMPTY;
}

void eng_init_player(void)
{
	memset(&pl,0,sizeof(struct cplayer));
}

void eng_init_amap(void)
{
	amap_bases = calloc(AMAP_MAXBASES, sizeof(struct areamap_base));
}

// ************* AREAMAP MODIFIERS ********

// Try to keep the character count <23
const char *amap_modtxt[] = {
	"Skeletal",				//1 (inhabited by skeletons)
	"Undead",				//2 (inhabited by undead)
	"Demonic",				//3 (inhabited by gargoyles)
	"Rock-strewn",			//4 (inhabited by golems)
	"Haunted",				//5 (inhabited by ghosts)
	"Twinned",				//6 (two unique bosses)
	"Stalwart",				//7 (increased mob life)
	"Enfeebling",			//8 (decreased player WV)
	"Savage",				//9 (increased monster damage)
	"Immune",				//10 (monsters gain immunity)
	"Robust",				//11 (monsters gain BWI)
	"Overlord's presence",	//12 (unique boss damage)
	"Titan's presence",		//13 (unique boss life)
	"Armoured",				//14 (increased monster AV)
	"Empowered bless",		//15 (monsters gain bless)
	"Venomous",				//16 (monsters cause poison)
	"Dangerous",			//17 (monsters can critically strike)
	"Smothering",			//18 (reduced player recovery of life/end/mana)
	"Burned",				//19 (area has patches of burning ground)
	"Frozen",				//20 (area has patches of freezing ground)
	"Shocked",				//21 (area has patches of shocking ground)
	"Drying",				//22 (reduced effectiveness of player healing effects)
	"Deadly",				//23 (area contains deathtraps)
	"Vulnerating",			//24 (players have reduced immunity and resistance)
	"Excruciating",			//25 (players have a chance of losing maximum hp on hit)
	"Otherworldly",			//26 (area contains breaches)
	"Glacial",				//27 (inhabited by ice gargoyles)
	"Scaly"					//28 (inhabited by lizards)
};

#define amap_modtxt_size	(sizeof(amap_modtxt) / sizeof(const char *))

// ************* DISPLAY ******************

unsigned int 	subwindow_mode=0;		// For sub-menus (shop, instance manager, etc.)

unsigned int    inv_pos=0;				// scrolling position of small inventory
unsigned int	amap_pos=0;				// scrolling position of map manager

unsigned int    skill_pos=0;

unsigned int   show_look=0,
look_nr=0,			// look at char/item nr
look_type=0,		// 1=char, 2=item
look_timer=0;		// look_timer

unsigned char   inv_block[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

extern int inv_object;			// object carried with the mouse cursor
extern int mouse_x,mouse_y;	// current mouse coordinates

#define XS	49
#define LL	22
#define XLL (22*10)
static char logtext[XLL][60];
static int logfont[XLL][60];

#define MAXTS            20

int load=0;

void eng_display_win(int plr_sprite,int init)
{
	int y,n,n1,m;
	int mid_x,mid_y;
	int inst_tcolor;
	char *tmp,buf[50];
	static int wntab[20]={
                WN_HEAD,WN_CLOAK,
		WN_BODY,WN_ARMS,
		WN_NECK,WN_BELT,
		WN_RHAND,WN_LHAND,
		WN_RRING,WN_LRING,
		WN_LEGS,WN_FEET,
		0,0,
		0,0,
		0,0,
		0,0};

	//if (load) dd_xputtext(670,300+MAXTS,1,"%3d%%",load);

	if (init) {
		reset_block();

		if (pl.mode==2)	      dd_showbox(screen_width-GUI_QSPELLS_XOFF+3,screen_height-GUI_QSPELLS_YOFF+51,45,12,(unsigned short)(RED));
		else if (pl.mode==1)  dd_showbox(screen_width-GUI_QSPELLS_XOFF+51,screen_height-GUI_QSPELLS_YOFF+51,45,12,(unsigned short)(RED));
		else if (pl.mode==0)  dd_showbox(screen_width-GUI_QSPELLS_XOFF+99,screen_height-GUI_QSPELLS_YOFF+51,45,12,(unsigned short)(RED));
		if (pdata.show_proz)  dd_showbox(screen_width-GUI_QSPELLS_XOFF+147,screen_height-GUI_QSPELLS_YOFF+51,45,12,(unsigned short)(RED));
		if (pdata.show_names) dd_showbox(screen_width-GUI_QSPELLS_XOFF+99,screen_height-GUI_QSPELLS_YOFF+66,45,12,(unsigned short)(RED));
		if (pdata.hide)	      dd_showbox(screen_width-GUI_QSPELLS_XOFF+51,screen_height-GUI_QSPELLS_YOFF+66,45,12,(unsigned short)(RED));

		// inventory
		for (n=0; n<10; n++) {
			if (pl.item[n+inv_pos]) {
				if (hightlight==HL_BACKPACK && hightlight_sub==n+(signed)inv_pos)
					copyspritex(pl.item[n+inv_pos],220+(n%2)*35,2+(n/2)*35,16);
				else
					copyspritex(pl.item[n+inv_pos],220+(n%2)*35,2+(n/2)*35,0);
			}
		}

		// spells
		for (n=0; n<20; n++) {
			if (pl.spell[n]) {
				copyspritex(pl.spell[n],374+(n%5)*24,4+(n/5)*24,15-min(15,pl.active[n]));
			}
		}

		dd_showbar(207,149+(skill_pos*58)/40,11,11,(unsigned short)0x0B00);
		dd_showbar(290,36+(inv_pos*94)/30,11,11,(unsigned short)0x0B00);


		// display info-texts
		dd_xputtext(5,270,1,"Hitpoints         %3d %3d",pl.a_hp,pl.hp[5]);
		dd_xputtext(5,284,1,"Endurance         %3d %3d",pl.a_end,pl.end[5]);
		dd_xputtext(5,298,1,"Mana              %3d %3d",pl.a_mana,pl.mana[5]);

		dd_xputtext(375,190,1,"Money  %8dG %2dS",pl.gold/100,pl.gold%100);
		dd_xputtext(117,256,1,"Update");
		dd_xputtext(162,256,1,"%7d",pl.points-stat_points_used);

		dd_xputtext(screen_width-GUI_XTRADATA_XOFF,GUI_XTRADATA_YOFF,1,"Weapon value   %10d",pl.weapon);
		dd_xputtext(screen_width-GUI_XTRADATA_XOFF,GUI_XTRADATA_YOFF+14,1,"Armor value    %10d",pl.armor);
		dd_xputtext(screen_width-GUI_XTRADATA_XOFF,GUI_XTRADATA_YOFF+28,1,"Experience     %10d",pl.points_tot);

		// display buttons
		for (n=0; n<12; n++) {
			dd_xputtext(screen_width-GUI_QSPELLS_XOFF+6+(n%4)*49,screen_height-GUI_QSPELLS_YOFF+5+(n/4)*15,1,pdata.xbutton[n].name);
		}

		for (n=0; n<5; n++) {
			dd_xputtext(5,4+n*14,1,"%-16.16s  %3d",at_name[n],pl.attrib[n][5]+stat_raised[n]);
			if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])<=pl.points-stat_points_used) dd_putc(136,4+n*14,1,'+');
			if (stat_raised[n]>0) dd_putc(150,4+n*14,1,'-');
			if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])!=HIGH_VAL) dd_xputtext(162,4+n*14,1,"%7d",attrib_needed(n,pl.attrib[n][0]+stat_raised[n]));
		}

		dd_xputtext(5, 74,1,"Hitpoints         %3d",pl.hp[5]+stat_raised[5]);
		if (hp_needed(pl.hp[0]+stat_raised[5])<=pl.points-stat_points_used)	dd_putc(136,74,1,'+');
		if (stat_raised[5]>0) dd_putc(150,74,1,'-');
		if (hp_needed(pl.hp[0]+stat_raised[5])!=HIGH_VAL) dd_xputtext(162,74,1,"%7d",hp_needed(pl.hp[0]+stat_raised[5]));

		dd_xputtext(5, 88,1,"Endurance         %3d",pl.end[5]+stat_raised[6]);
		if (end_needed(pl.end[0]+stat_raised[6])<=pl.points-stat_points_used) dd_putc(136,88,1,'+');
		if (stat_raised[6]>0) dd_putc(150,88,1,'-');
		if (end_needed(pl.end[0]+stat_raised[6])!=HIGH_VAL)	dd_xputtext(162,88,1,"%7d",end_needed(pl.end[0]+stat_raised[6]));

		dd_xputtext(5,102,1,"Mana              %3d",pl.mana[5]+stat_raised[7]);
		if (mana_needed(pl.mana[0]+stat_raised[7])<=pl.points-stat_points_used)	dd_putc(136,102,1,'+');
		if (stat_raised[7]>0) dd_putc(150,102,1,'-');
		if (mana_needed(pl.mana[0]+stat_raised[7])!=HIGH_VAL) dd_xputtext(162,102,1,"%7d",mana_needed(pl.mana[0]+stat_raised[7]));

		for (n=0; n<10; n++) {
			m=skilltab[n+skill_pos].nr;
			if (!pl.skill[m][0]) {
				dd_xputtext(5,116+n*14,1,"unused");
				continue;
			}
			dd_xputtext(5,116+n*14,1,"%-16.16s  %3d",skilltab[n+skill_pos].name,pl.skill[m][5]+stat_raised[n+8+skill_pos]);
			if (skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos])<=pl.points-stat_points_used) dd_putc(136,116+n*14,1,'+');
			if (stat_raised[n+8+skill_pos]>0) dd_putc(150,116+n*14,1,'-');
			if (skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos])!=HIGH_VAL)
				dd_xputtext(162,116+n*14,1,"%7d",skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos]));
		}
	}

	// logtext
	if (logtimer) logtimer--;
	else logstart=0;

	for (y=0; y<LL; y++) {
		dd_puttext(screen_width-GUI_CHAT_XOFF,4+y*10,logfont[LL-y-1+logstart],logtext[LL-y-1+logstart]);
	}

	input[in_len]=0;
	if (cur_pos-view_pos>45) view_pos=cur_pos-45;
	if (cur_pos-5<view_pos)	view_pos=max(0,cur_pos-5);
	memcpy(buf,input+view_pos,48);
	buf[48]=0;

	dd_puttext_1f(screen_width-GUI_CHAT_XOFF,9+10*LL,FNT_YELLOW,buf);
	dd_putc(screen_width-GUI_CHAT_XOFF+6*(cur_pos-view_pos),9+10*LL,FNT_YELLOW,127);

	if (init) {
		if (subwindow_mode == SUBWND_SHOP) show_look=0;
		if (!show_look) {
			for (n=0; n<12; n++) {
				if (pl.worn[wntab[n]]) {
					if (hightlight==HL_EQUIPMENT && hightlight_sub==wntab[n])
						copyspritex(pl.worn[wntab[n]],303+(n%2)*35,2+(n/2)*35,16);
					else
						copyspritex(pl.worn[wntab[n]],303+(n%2)*35,2+(n/2)*35,0);
				}
				if (inv_block[wntab[n]]) copyspritex(4,303+(n%2)*35,2+(n/2)*35,0);
			}

			if (selected_char) tmp=lookup(selected_char,0);
			else tmp=pl.name;
			dd_xputtext(374+(125-strlen(tmp)*6)/2,28,1,tmp);

			if (pl.hp[5]>0)	n=min(124,pl.hp[5]*62/pl.hp[5]);
			else n=0;
			dd_showbar(373,127,n,6,(unsigned short)0x00B0);

			if (pl.hp[5]>0)	n=min(124,pl.a_hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(373,127,n,6,(unsigned short)0x0B00);

			if (pl.end[5]>0) n=min(124,pl.end[5]*62/pl.end[5]);
			else n=0;
			dd_showbar(373,134,n,6,(unsigned short)0x00B0);
			if (pl.end[5]>0) n=min(124,pl.a_end*62/pl.end[5]);
			else n=0;
			dd_showbar(373,134,n,6,(unsigned short)0x0B00);

			if (pl.mana[5]>0) n=min(124,pl.mana[5]*62/pl.mana[5]);
			else n=0;
			dd_showbar(373,141,n,6,(unsigned short)0x00B0);

			if (pl.mana[5]>0) n=min(124,pl.a_mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(373,141,n,6,(unsigned short)0x0B00);

			//experience bar
			if (pl.points_tot>0) n=min(153,(pl.points_tot-rank2points(points2rank(pl.points_tot)))*153/(pl.points_tot+points_tolevel(pl.points_tot)-rank2points(points2rank(pl.points_tot))));
			else n=0;
			dd_showbar(screen_width-GUI_XTRADATA_XOFF-4,GUI_XTRADATA_YOFF+43,n,3,(unsigned short)0xBB00);

			if (subwindow_mode != SUBWND_SHOP) {
				copyspritex(10+min(20,points2rank(pl.points_tot)),463,54-16,0);
				copyspritex(plr_sprite,402,32,0);
				dd_xputtext(374+(125-strlen(pl.name)*6)/2,152,1,pl.name);
				dd_xputtext(374+(125-strlen(rank[points2rank(pl.points_tot)])*6)/2,172,1,rank[points2rank(pl.points_tot)]);
			}

		} else {
			if (look.worn[WN_HEAD])	copyspritex(look.worn[WN_HEAD],303,2,0);
			if (look.worn[WN_CLOAK]) copyspritex(look.worn[WN_CLOAK],338,2,0);
			if (look.worn[WN_BODY])	copyspritex(look.worn[WN_BODY],303,37,0);
			if (look.worn[WN_ARMS])	copyspritex(look.worn[WN_ARMS],338,37,0);
			if (look.worn[WN_BELT])	copyspritex(look.worn[WN_BELT],338,72,0);
			if (look.worn[WN_NECK])	copyspritex(look.worn[WN_NECK],303,72,0);
			if (look.worn[WN_RHAND]) copyspritex(look.worn[WN_RHAND],303,107,0);
			if (look.worn[WN_LHAND]) copyspritex(look.worn[WN_LHAND],338,107,0);
			if (look.worn[WN_LEGS])	copyspritex(look.worn[WN_RRING],303,142,0);
			if (look.worn[WN_FEET])	copyspritex(look.worn[WN_LRING],338,142,0);
			if (look.worn[WN_LEGS])	copyspritex(look.worn[WN_LEGS],303,142+35,0);
			if (look.worn[WN_FEET])	copyspritex(look.worn[WN_FEET],338,142+35,0);

			if (look.sprite) copyspritex(look.sprite,402,32,0);

			dd_xputtext(374+(125-strlen(rank[points2rank(look.points)])*6)/2,172,1,rank[points2rank(look.points)]);
			dd_xputtext(374+(125-strlen(look.name)*6)/2,152,1,look.name);

			if (pl.hp[5]) n=min(124,look.hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(373,127,n,6,(unsigned short)0x00B0);

			if (pl.hp[5]) n=min(124,look.a_hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(373,127,n,6,(unsigned short)0xB000);

			if (pl.end[5]) n=min(124,look.end*62/pl.end[5]);
			else n=0;
			dd_showbar(373,134,n,6,(unsigned short)0x00B0);
			if (pl.end[5]) n=min(124,look.a_end*62/pl.end[5]);
			else n=0;
			dd_showbar(373,134,n,6,(unsigned short)0xB000);

			if (pl.mana[5])	n=min(124,look.mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(373,141,n,6,(unsigned short)0x00B0);
			if (pl.mana[5])	n=min(124,look.a_mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(373,141,n,6,(unsigned short)0xB000);

			copyspritex(10+min(20,points2rank(look.points)),463,54-16,0);
		}

		if (subwindow_mode == SUBWND_SHOP) {
			// Shop sub-window
			mid_x = screen_width / 2 - 160;
			mid_y = screen_height / 2 - 80;
			copyspritex(92,mid_x,mid_y,0);
			for (n=0; n<62; n++) {
				if (!shop.item[n]) continue;
				if (hightlight==HL_SHOP && hightlight_sub==n) {
					copyspritex(shop.item[n],mid_x+2+(n%8)*35,mid_y+2+(n/8)*35,16);
					if (shop.price[n]) dd_xputtext(mid_x+5,mid_y+289,1,"Sell: %dG %dS",shop.price[n]/100,shop.price[n]%100);
				} else
					copyspritex(shop.item[n],mid_x+2+(n%8)*35,mid_y+2+(n/8)*35,0);
			}
			if (pl.citem && shop.pl_price)
				dd_xputtext(mid_x+5,mid_y+289,1,"Buy:  %dG %dS",shop.pl_price/100,shop.pl_price%100);

			if (shop.sprite) copyspritex(shop.sprite,402,32,0);
			copyspritex(10+min(20,points2rank(shop.points)),463,54-16,0);
			dd_xputtext(374+(125-strlen(rank[points2rank(shop.points)])*6)/2,172,1,rank[points2rank(shop.points)]);
			dd_xputtext(374+(125-strlen(shop.name)*6)/2,152,1,shop.name);
		}
		else if (subwindow_mode == SUBWND_INSTMNG) {
			// Instance manager sub-window
			mid_x = screen_width / 2 - 160;
			mid_y = screen_height / 2 - 80;
			copyspritex(GUI_INSTMENU,mid_x,mid_y,0);
			dd_xputtext(mid_x+106,mid_y+4,1,"Instance manager");
			dd_xputtext(mid_x+8,mid_y+34,1,"You are about to enter area \"%s\".", area_instname);
			dd_xputtext(mid_x+8,mid_y+46,1,"Choose or create an instance below.");

			for (n=0; n<loaded_insts; n++) {
				inst_tcolor = 1;
				dd_xputtext(mid_x+17+(48-strlen("Enter")*6)/2,mid_y+n*36+71,1,"Enter");

				if (area_insts[n].time_left <= 0) {
					inst_tcolor = 0;
					dd_xputtext(mid_x+77,mid_y+n*36+66,inst_tcolor,"Time left: Expired / Created: %02d:%02d",
						(int)floor(area_insts[n].creation_time/100), area_insts[n].creation_time%100);
				} else {
					dd_xputtext(mid_x+77,mid_y+n*36+66,inst_tcolor,"Time left: %02d:%02d / Created: %02d:%02d",
						(int)floor(abs(area_insts[n].time_left)/60), abs(area_insts[n].time_left)%60,
						(int)floor(area_insts[n].creation_time/100), area_insts[n].creation_time%100);
				}

				dd_xputtext(mid_x+77,mid_y+n*36+78,inst_tcolor,"Monsters remaining: %d%s", min(50, area_insts[n].mobs_remaining),
					area_insts[n].mobs_remaining > 50 ? "+" : "");
			}

			if (loaded_insts < 7) {
				dd_xputtext(mid_x+17+(48-strlen("New")*6)/2,mid_y+n*36+71,1,"New");
				dd_xputtext(mid_x+77,mid_y+n*36+66,1,"This option will put you in a new");
				dd_xputtext(mid_x+77,mid_y+n*36+78,1,"instance of %s.", area_instname);
			}
		}
		else if (subwindow_mode == SUBWND_AMAPS) {
			// Areamap manager sub-window
			mid_x = screen_width / 2 - 256;
			mid_y = screen_height / 2 - 80;
			copyspritex(GUI_AMAPSMENU,mid_x,mid_y,0);

			dd_xputtext(mid_x+364,mid_y+5,FNT_ORANGE,"Map manager");

			if (amap_selected != -1) {
				dd_xputtext(mid_x+395-strlen(amap_sel.name)*3,mid_y+71,FNT_YELLOW,amap_sel.name);

				dd_xputtext(mid_x+379,mid_y+250,FNT_YELLOW,"Enter");
			} else {
				dd_xputtext(mid_x+379,mid_y+250,FNT_SILVER,"Enter");
			}

			if (mapdev_level < 10) dd_xputtext(mid_x+393,mid_y+33,FNT_ORANGE,"%d",mapdev_level);
			else if (mapdev_level < 100) dd_xputtext(mid_x+390,mid_y+33,FNT_ORANGE,"%d",mapdev_level);
			else if (mapdev_level < 1000) dd_xputtext(mid_x+387,mid_y+33,FNT_ORANGE,"%d",mapdev_level);
			else dd_xputtext(mid_x+384,mid_y+33,FNT_ORANGE,"%d",mapdev_level);

			if (mapdev_exp>0) n=min(156,156*(mapdev_exp/mapdev_expreq));
			else n=0;
			dd_showbar(mid_x+317,mid_y+25,n,3,(unsigned short)0xBB00);

			// Map-modifying orbs
			for (n=0; n<5; n++) {
				if (amap_orbhover == n) copyspritex(6606 + n, mid_x + 480, mid_y + 114 + n * 31, 16);
				else copyspritex(6606 + n, mid_x + 480, mid_y + 114 + n * 31, 0);

				n1 = 0;
				if (amap_orbs[n] > 9) n1++;
				if (amap_orbs[n] > 99) n1++;
				dd_xputtext(mid_x+498-n1*6,mid_y+135+n*31,FNT_ORANGE,"x%d",amap_orbs[n]);
			}

			// Selected map mods
			if (amap_selected != -1) {
				n1 = 0;
				for (n=0; n<6; n++) {
					while (n1 < AMAP_MAXMODS) {
						if (amap_sel.mods[n1] != 0) {
							if (amap_modhover == n) copyspritex(6611, mid_x + 317, mid_y + 94 + n * 19, 16);
							else copyspritex(6611, mid_x + 317, mid_y + 94 + n * 19, 0);

							if (n1 < amap_modtxt_size) dd_xputtext(mid_x + 335, mid_y + 97 + n * 19, FNT_BLUE, amap_modtxt[n1]);
							else dd_xputtext(mid_x + 335, mid_y + 97 + n * 19, FNT_BLUE, "%d", amap_sel.mods[n1]);
							n1++;
							break;
						}
						n1++;
					}
					if (n1 >= AMAP_MAXMODS) break;
				}
			}

			for (n=0; n<5; n++) {
				for (n1=0; n1<4; n1++) {
					m = n1 + (n + amap_pos) * 4;
					if (m >= AMAP_MAXBASES) continue;
					if (!amap_bases[m].used) continue;

					// Draw connections
					// Horizontal
					if (n1<3) {
						if (amap_bases[m].conn_flags&AMAP_CONN_E && amap_bases[m+1].used == 1 && amap_bases[m+1].conn_flags&AMAP_CONN_W) {
							copyspritex(6600, mid_x + 50 + n1 * 68, mid_y + 25 + n * 54, 0);
						}
					}
					// Vertical
					if (n<4) {
						if (m+4 < AMAP_MAXBASES) {
							if (amap_bases[m].conn_flags&AMAP_CONN_S && amap_bases[m+4].used == 1 && amap_bases[m+4].conn_flags&AMAP_CONN_N) {
								copyspritex(6603, mid_x + 50 + n1 * 68, mid_y + 25 + n * 54, 0);
							}
						}
						// Diagonal (SE)
						if (m+5 < AMAP_MAXBASES) {
							if (amap_bases[m].conn_flags&AMAP_CONN_SE && amap_bases[m+5].used == 1 && amap_bases[m+5].conn_flags&AMAP_CONN_NW) {
								copyspritex(6601, mid_x + 50 + n1 * 68, mid_y + 25 + n * 54, 0);
							}
						}
						// Diagonal (SW)
						if (n1>0 && m+3 < AMAP_MAXBASES) {
							if (amap_bases[m].conn_flags&AMAP_CONN_SW && amap_bases[m+3].used == 1 && amap_bases[m+3].conn_flags&AMAP_CONN_NE) {
								copyspritex(6602, mid_x - 43 + n1 * 68, mid_y + 25 + n * 54, 0);
							}
						}
					}

					copyspritex(amap_bases[m].sprite, mid_x + 36 + n1 * 68, mid_y + 11 + n * 54, 0);

					// Hovering
					if (amap_hovering == m && amap_selected != m) {
						dd_showbox(mid_x + 35 + n1 * 68, mid_y + 10 + n * 54, 34, 34, (unsigned short)0xB000);
					}
					// Selected
					if (amap_selected == m) {
						dd_showbox(mid_x + 35 + n1 * 68, mid_y + 10 + n * 54, 34, 34, (unsigned short)0xD900);
					}

					// Charges
					copyspritex(6604, mid_x + 36 + n1 * 68, mid_y + 11 + n * 54, 0);
					if (amap_bases[m].charges < 10) dd_xputtext(mid_x + 49 + n1 * 68, mid_y + 41 + n * 54, FNT_SILVER, "%d", amap_bases[m].charges);
					else if (amap_bases[m].charges < 100) dd_xputtext(mid_x + 46 + n1 * 68, mid_y + 41 + n * 54, FNT_SILVER, "%d", amap_bases[m].charges);
					else dd_xputtext(mid_x + 43 + n1 * 68, mid_y + 41 + n * 54, FNT_SILVER, "%d", amap_bases[m].charges);
				}
			}
		}
	}
}

struct looks {
	char known;
	char name[21];
	char proz;
	unsigned short id;
};

struct looks *looks=NULL;
int lookmax=0;
int lookat=0;

char *lookup(int nr,unsigned short id)
{
	static char buf[40];
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
			looks[n].proz=0;
		}
		lookmax=nr+10;
	}

	if (id && id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
		looks[nr].id=id;
	}

	if (!looks[nr].known) lookat=nr;

	if (!id) return looks[nr].name;

	if (pdata.show_names && pdata.show_proz) {
		if (looks[nr].proz) {
			sprintf(buf,"%s %d%%",looks[nr].name,looks[nr].proz);
			return buf;
		} else return looks[nr].name;
	} else if (pdata.show_names) return looks[nr].name;
	else if (pdata.show_proz) {
		if (looks[nr].proz) {
			sprintf(buf,"%d%%",looks[nr].proz);
			return buf;
		} else return "";
	} else return "";
}

void add_look(unsigned short nr,char *name,unsigned short id)
{
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
			looks[n].proz=0;
		}
		lookmax=nr+10;
	}

	if (id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
	}

	strncpy(looks[nr].name,name,16);
	looks[nr].name[16]=0;
	looks[nr].known=1;
	looks[nr].proz=0;
	looks[nr].id=id;
}

void set_look_proz(unsigned short nr,unsigned short id,int proz)
{
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
		}
		lookmax=nr+10;
	}
	if (id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
		looks[nr].id=id;
	}
	looks[nr].proz=(unsigned char)proz;
}

int tile_x=-1,tile_y=-1,tile_type=-1;

void dd_show_map(unsigned short *src,int xo,int yo);

int autohide(int x,int y)
{
	if (x>=(screen_renderdist/2) || (y<=screen_renderdist/2)) return 0;
	return 1;
}

int facing(int x,int y,int dir)
{
	if (dir==1 && x==screen_renderdist/2+1 && y==screen_renderdist/2) return 1;
	if (dir==2 && x==screen_renderdist/2-1 && y==screen_renderdist/2) return 1;
	if (dir==4 && x==screen_renderdist/2 && y==screen_renderdist/2+1) return 1;
	if (dir==3 && x==screen_renderdist/2 && y==screen_renderdist/2-1) return 1;

	return 0;
}

int mapxy_rand(int x,int y,int dur)
{
    int val,tim;

    val=(x*43+y*77+x*y+x*24+y*39)%666;
    tim=(ticker/dur)%666;

    if (tim==val) return ticker%dur;
    else return 0;
}

void display_floortile(int tile,int light,int x,int y,int xoff,int yoff,int mx,int my)
{
    switch(tile) {
	case 16980:	tile+=mapxy_rand(mx,my,10)/2; break;
    }
    copysprite(tile,light,x,y,xoff,yoff);
}

void eng_display(int init)	// optimize me!!!!!
{
	int x,y,rx,ry,m,plr_sprite,tmp,mapx,mapy,selected_visible=0,alpha,alphastr;
	extern int dd_cache_hit,dd_cache_miss,swap,MAXCACHE;
	static unsigned short xmap[MAPX_MAX*MAPY_MAX];
	static xm_flag=1;

	if (xm_flag) {
		for (m=0; m<MAPX_MAX*MAPY_MAX; m++)	xmap[m]=0;
		xm_flag=0;
	}

	// check if we're visible. If not, just leave.
	if (!dd_isvisible()) return;

	mouse(mx,my,0);
	SetCursor(cursor[cursor_type]);

	// *******
	// * map *
	// *******

	if (init) {
		if (do_shadow) dd_shadow_clear();
		xoff=-map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj_xoff-176; //-176;
		yoff=-map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj_yoff; //-176;
		plr_sprite=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj2;

		mapx=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].x;
		mapy=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].y;

		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {
				// background
				m=x+y*screen_renderdist;

				if (hightlight==HL_MAP && tile_type==0 && tile_x==x && tile_y==y) tmp=16;
				else tmp=0;
				if (map[m].flags&INVIS)	tmp|=64;
				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;
				if (map[m].flags&TPURPLE) tmp|=1024;

				display_floortile(map[m].back,map[m].light|tmp,x*32,y*32,xoff,yoff,map[x+y*screen_renderdist].x,map[x+y*screen_renderdist].y);

				if (map[m].x<MAPX_MAX && map[m].y<MAPY_MAX && !(map[m].flags&INVIS)) {
					if (!xmap[map[m].y+map[m].x*MAPX_MAX] || xmap[map[m].y+map[m].x*MAPX_MAX]==0xffff)

						xmap[map[m].y+map[m].x*MAPX_MAX]=(unsigned short)get_avgcol(map[m].back);
				}

				// Floor warning effects
				if (map[m].flags3&F3_FLRWARN_SQ) copysprite(6621,0,x*32,y*32,xoff,yoff);
				if (map[m].flags3&F3_FLRWARN_TR) copysprite(6620,0,x*32,y*32,xoff,yoff);
				if (map[m].flags3&F3_FLRWARN_CR1) copysprite(6622,0,x*32,y*32,xoff,yoff);
				if (map[m].flags3&F3_FLRWARN_CR2) copysprite(6623,0,x*32,y*32,xoff,yoff);

				if (pl.goto_x > 0 && pl.goto_y > 0 && pl.goto_x==map[m].x && pl.goto_y==map[m].y)
					copysprite(31,0,x*32,y*32,xoff,yoff);
			}
		}

		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {

				m=x+y*screen_renderdist;

				if (map[x+y*screen_renderdist].flags&INVIS) continue; //tmp=128;
				else tmp=0;

				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;
				if (map[m].flags&TPURPLE) tmp|=1024;

				// object
                                if (pdata.hide==0 || (map[m].flags&ISITEM) || autohide(x,y)) {
					int tmp2;

					if (map[m].obj1>16335 && map[m].obj1<16422 && map[m].obj1!=16357 &&
						map[m].obj1!=16365 && map[m].obj1!=16373 && map[m].obj1!=16381 &&
						map[m].obj1!=16357 && map[m].obj1!=16389 && map[m].obj1!=16397 &&
						map[m].obj1!=16405 && map[m].obj1!=16413 && map[m].obj1!=16421 &&
						!facing(x,y,pl.dir) && !autohide(x,y) && pdata.hide) { // mine hack

						if (map[m].obj1<16358) tmp2=457;
						else if (map[m].obj1<16366)	tmp2=456;
						else if (map[m].obj1<16374)	tmp2=455;
						else if (map[m].obj1<16382)	tmp2=466;
						else if (map[m].obj1<16390)	tmp2=459;
						else if (map[m].obj1<16398)	tmp2=458;
						else if (map[m].obj1<16398)	tmp2=449;
						else if (map[m].obj1<16406)	tmp2=468;
						else tmp2=467;

                                                if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) copysprite(tmp2,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else copysprite(tmp2,map[m].light|tmp,x*32,y*32,xoff,yoff);
					} else {
                                                if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) copysprite(map[m].obj1,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else copysprite(map[m].obj1,map[m].light|tmp,x*32,y*32,xoff,yoff);
					}					

				} else if (map[m].obj1) {					
					copysprite(map[m].obj1+1,map[m].light|tmp,x*32,y*32,xoff,yoff);					
				}

				if (map[m].obj1 && map[m].x<MAPX_MAX && map[m].y<MAPY_MAX) {
					xmap[map[m].y+map[m].x*MAPX_MAX]=(unsigned short)get_avgcol(map[m].obj1);
				}

				// character
				if (tile_type==2 && tile_x==x && tile_y==y)	tmp=16;
				else tmp=0;
				if (map[m].ch_nr==selected_char) {
					tmp|=32; selected_visible=1;
				}
				if (map[m].flags&INVIS)	tmp|=64;
				if (map[m].flags&STONED) tmp|=128;
				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;
				if (map[m].flags&TPURPLE) tmp|=1024;

				if (do_shadow) dd_shadow(map[m].obj2,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff+4);
				copysprite(map[m].obj2,map[m].light|tmp,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);				

				if (pl.attack_cn && pl.attack_cn==map[m].ch_nr)
					copysprite(34,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if (pl.misc_action==DR_GIVE && pl.misc_target1==map[m].ch_id)
					copysprite(45,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if ((pdata.show_names|pdata.show_proz) && map[m].ch_nr) {
					set_look_proz(map[m].ch_nr,map[m].ch_id,map[m].ch_proz);
					dd_gputtext(x*32,y*32,1,lookup(map[m].ch_nr,map[m].ch_id),xoff+map[m].obj_xoff,yoff+map[m].obj_yoff-8);
					
					// Healthbar over characters
					if (pdata.show_proz) {
						rx=((x*32)/2)+((y*32)/2)+32-HPBAR_WIDTH/2+screen_tilexoff-((screen_renderdist-34)/2*32)+xoff+map[m].obj_xoff;
						ry=((x*32)/4)-((y*32)/4)+screen_tileyoff-60+yoff+map[m].obj_yoff;
						dd_showbar(rx,ry,(int)(HPBAR_WIDTH*((float)looks[map[m].ch_nr].proz/100.0)),1,(unsigned short)0xF000);
					}
				}

				if (pl.misc_action==DR_DROP && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(32,0,x*32,y*32,xoff,yoff);
				if (pl.misc_action==DR_PICKUP && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(33,0,x*32,y*32,xoff,yoff);
				if (pl.misc_action==DR_USE && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(45,0,x*32,y*32,xoff,yoff);

				// effects
				if (map[m].flags2&MF_MOVEBLOCK)	copysprite(55,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_SIGHTBLOCK) copysprite(84,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_INDOORS) copysprite(56,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_UWATER) copysprite(75,0,x*32,y*32,xoff,yoff);
//				if (map[m].flags2&MF_NOFIGHT) copysprite(58,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOMONST) copysprite(59,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_BANK) copysprite(60,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_TAVERN) copysprite(61,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOMAGIC) copysprite(62,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_DEATHTRAP)	copysprite(73,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOLAG)	copysprite(57,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_ARENA)	copysprite(76,0,x*32,y*32,xoff,yoff);
//				if (map[m].flags2&MF_TELEPORT2) copysprite(77,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOEXPIRE) copysprite(82,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&0x80000000) copysprite(72,0,x*32,y*32,xoff,yoff);

				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==INJURED)
					copysprite(1079,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED1))
					copysprite(1080,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED2))
					copysprite(1081,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED1|INJURED2))
					copysprite(1082,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if (map[m].flags&DEATH) {
					if (map[m].obj2) copysprite(280+((map[m].flags&DEATH)>>17)-1,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
					else copysprite(280+((map[m].flags&DEATH)>>17)-1,0,x*32,y*32,xoff,yoff);
				}
				if (map[m].flags&TOMB) {
					copysprite(240+((map[m].flags&TOMB)>>12)-1,map[m].light,x*32,y*32,xoff,yoff);
				}

				alpha=0; alphastr=0;

				if (map[m].flags&EMAGIC) {
					alpha|=1;
					alphastr=max((unsigned)alphastr,((map[m].flags&EMAGIC)>>22));
				}

				if (map[m].flags&GMAGIC) {
					alpha|=2;
					alphastr=max((unsigned)alphastr,((map[m].flags&GMAGIC)>>25));
				}

				if (map[m].flags&CMAGIC) {
					alpha|=4;
					alphastr=max((unsigned)alphastr,((map[m].flags&CMAGIC)>>28));
				}
				if (alpha) dd_alphaeffect_magic(alpha,alphastr,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
			}
		}
	} else {
		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {
				// background
				copysprite(SPR_EMPTY,0,x*32,y*32,-176,0);
			}
		}
	}

	if (!selected_visible) selected_char=0;

	copyspritex(screen_overlay_sprite,0,0,0);

	if (init) {
		xmap[mapy+mapx*MAPX_MAX]=0xffff;

		mapx-=64;
		if (mapx<0)	mapx=0;
		if (mapx>MAPX_MAX-129) mapx=MAPX_MAX-129;

		mapy=mapy-64;
		if (mapy<0)	mapy=0;
		if (mapy>MAPY_MAX-129) mapy=MAPY_MAX-129;

		dd_show_map(xmap,mapy,mapx);
	}

	eng_display_win(plr_sprite,init);

	// ********************
	// display cursors etc.
	// ********************

	if (init && pl.citem) {
		if (cursor_type==CT_DROP || cursor_type==CT_SWAP || cursor_type==CT_USE)
			copyspritex(pl.citem,mouse_x-16,mouse_y-16,16);
		else
			copyspritex(pl.citem,mouse_x-16,mouse_y-16,0);
	}
}

void appendc(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

// DISPLAY: TEXT OUTPUT
void tlog(char *text,int font)
{
	int n,panic=0;
	int cmode=0, txt_len, copiedchars=0;

	char *tmp_col[5]={0};
	char *src_text=calloc(XLL, sizeof(char));
	int *line_font=calloc(XLL, sizeof(int));
	static int flag=0;

	if (!flag) {
		for (n=0; n<XLL*60; n++) {
			logtext[0][n]=0;
		}
		for (n=0; n<XLL*60; n++) {
			logfont[0][n]=FNT_YELLOW;
		}
		flag=1;
	}

	if (strlen(text)<1)	return;

	// Process color coding system
	txt_len = min(XLL, strlen(text)+1);
	for (n=0; n<txt_len; n++) {
		if (n < txt_len - 2) {
			if (text[n] == '/' && text[n+1] == '|') {
				n+=2;
				cmode=n;
			} else if (cmode > 0 && (text[n] == '|' || n-cmode > 4)) {
				n++;
				strncpy(tmp_col, &text[cmode], n-cmode-1);
				font = atoi(tmp_col);
				tmp_col[0]=0;
				cmode=0;
			}
		}

		if (cmode == 0) {
			line_font[copiedchars] = font;
			appendc(src_text, text[n]);
			copiedchars++;
		}
	}

	while (panic++<XLL) {
		do_msg();
		memmove(logtext[1],logtext[0],XLL*60-60);
		memmove(logfont[1],logfont[0],XLL*60*sizeof(int)-60*sizeof(int));
		memcpy(logtext[0],src_text,min(XS-1,strlen(src_text)+1));
		//memcpy(logfont[0],line_font,min(XS-1,strlen(src_text)+1));
		logtext[0][XS-1]=0;

		for (n=0; n<min(XS-1,strlen(src_text)+1); n++) {
			logfont[0][n] = line_font[n];
		}

		if (strlen(src_text)<XS-1) return;

		for (n=XS-1; n>0; n--) {
			if (logtext[0][n]==' ') break;
		}

		if (n!=0) {
			logtext[0][n]=0;
			src_text+=n+1;
			line_font+=n+1;
		} else {
			src_text+=XS-1;
			line_font+=XS-1;
		}
	}

	free(src_text);
	free(line_font);
}

void xlog(char font,char *format,...)
{
	va_list args;
	char buf[1024];

	va_start(args,format);
	vsprintf(buf,format,args);
	tlog(buf,font);
	va_end(args);
}


// ************* MAIN *********************

void init_engine(void)
{
	eng_init_map();
	eng_init_player();
	eng_init_amap();
}

void do_msg(void)
{
	MSG msg;

	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void eng_flip(unsigned int t)
{
	int diff;
	MSG msg;

	diff=t-GetTickCount();
	if (diff>0)	idle+=diff;

	do {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else Sleep(1);
	} while (t>GetTickCount());

	if (screen_windowed == 1) {
		dd_flip_windowed();
	} else {
		dd_flip();
	}

	frame++;
}

unsigned char speedtab[20][20]=
{
//  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},	//20
	{1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1},	//19
	{1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1},	//18
	{1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1},	//17
	{1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1},	//16
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},	//15
	{1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1},	//14
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0},	//13
	{0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1},	//12
	{0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1},	//11
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0},	//10
	{1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0},	//9
	{1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,1,0},	//8
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},	//7
	{0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0},	//6
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0},	//5
	{0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0},	//4
	{0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0},	 //3
	{0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0},	//2
	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0}		//1
};

unsigned char speedsteptab[20][20]=
{
//  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
	{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},	//20
	{4,4,4,4,4,4,4,4,4,2,4,4,4,4,4,4,4,4,4,4},	//19
	{4,4,4,4,4,2,4,4,4,4,4,4,4,4,2,4,4,4,4,4},	//18
	{4,4,4,2,4,4,4,4,4,2,4,4,4,4,4,4,2,4,4,4},	//17
	{4,4,2,4,4,4,4,2,4,4,4,4,2,4,4,4,4,2,4,4},	//16
	{4,4,2,4,4,4,2,4,4,4,2,4,4,4,2,4,4,4,2,4},	//15
	{4,2,4,4,2,4,4,4,2,4,4,2,4,4,2,4,4,2,4,4},	//14
	{4,2,4,4,2,4,4,2,4,4,2,4,4,2,4,4,2,4,4,2},	//13
	{2,4,4,2,4,4,2,4,2,4,4,2,4,4,2,4,2,4,2,4},	//12
	{2,4,2,4,2,4,2,4,4,2,4,2,4,2,4,2,4,4,2,4},	//11
	{4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2},	//10
	{4,2,4,2,4,2,4,1,3,4,2,4,2,4,2,4,1,2,4,2},	//9
	{4,1,2,4,1,3,4,2,4,1,2,4,1,3,4,2,4,2,4,2},	//8
	{2,4,1,3,4,1,3,4,1,3,4,1,3,4,1,3,4,1,3,4},	//7
	{3,4,1,3,4,1,2,3,4,1,3,4,1,3,4,1,3,4,1,2},	//6
	{2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1},	//5
	{2,3,4,1,1,2,3,4,1,1,2,3,4,1,1,2,3,4,1,1},	//4
	{2,3,4,4,1,1,2,2,3,4,1,1,2,2,3,4,4,1,1,2},	 //3
	{2,3,3,4,4,4,1,1,1,2,2,2,3,3,4,1,1,1,2,2},	//2
	{3,3,3,3,3,4,4,4,4,4,1,1,1,1,1,2,2,2,2,2}		//1
};

int speedo(int n)
{
	return speedtab[map[n].ch_speed][ctick];
}


int speedstep(int n,int d,int s,int update)
{
	int hard_step;
	int soft_step;
	int total_step;
	int speed;
	int dist;
	int z,m;

	speed=map[n].ch_speed;
	hard_step=map[n].ch_status-d;

	if (!update) return 32*hard_step/s;

	z=ctick;
	soft_step=0;
	m=hard_step;

	while (m) {
		z--;
		if (z<0) z=19;
		soft_step++;
		if (speedtab[speed][z])	m--;
	}
	while (1) {
		z--;
		if (z<0) z=19;
		if (speedtab[speed][z])	break;
		soft_step++;
	}

	z=ctick;
	total_step=soft_step;
	m=s-hard_step;

	while (1) {
		if (speedtab[speed][z])	m--;
		if (m<1) break;
		z++;
		if (z>19) z=0;
		total_step++;
	}
	dist=32*(soft_step)/(total_step+1);

	return dist;
}
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10
static int stattab[]={ 0, 1, 1, 6, 6, 2, 3, 4, 5, 7, 4};

#define do_idle(ani,sprite)  (sprite==22480 ? ani : 0)

int eng_char(int n)
{
	int tmp,update=1;

	if (map[n].flags&STUNNED) update=0;

	switch (map[n].ch_status) {
		// idle up
		case    0:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			return map[n].ch_sprite+0+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle down
		case    1:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+8+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle left
		case    2:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+16+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle right
		case    3:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+24+do_idle(map[n].idle_ani,map[n].ch_sprite);

			// idle left-up
		case    4:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+32+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle left-down
		case    5:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+40+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle right-up
		case    6:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+48+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle right-down
		case    7:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+56+do_idle(map[n].idle_ani,map[n].ch_sprite);

			// walk up
		case    16:
		case    17:
		case    18:
		case    19:
		case    20:
		case    21:
		case    22: map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-16)+64;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    23:   map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-16)+64;
			if (speedo(n) && update) map[n].ch_status=16;
			return tmp;

			// walk down
		case    24:
		case    25:
		case    26:
		case    27:
		case    28:
		case    29:
		case    30: map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-24)+72;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    31:   map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-24)+72;
			if (speedo(n) && update) map[n].ch_status=24;
			return tmp;

			// walk left
		case    32:
		case    33:
		case    34:
		case    35:
		case    36:
		case    37:
		case    38: map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-32)+80;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    39:   map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-32)+80;
			if (speedo(n) && update) map[n].ch_status=32;
			return tmp;

			// walk right
		case    40:
		case    41:
		case    42:
		case    43:
		case    44:
		case    45:
		case    46: map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-40)+88;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    47:   map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-40)+88;
			if (speedo(n) && update) map[n].ch_status=40;
			return tmp;


			// left+up
		case    48:
		case    49:
		case    50:
		case    51:
		case    52:
		case    53:
		case    54:
		case    55:
		case    56:
		case    57:
		case    58:   map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-48)*8/12+96;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    59:   map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-48)*8/12+96;
			if (speedo(n) && update) map[n].ch_status=48;
			return tmp;


			// left+down
		case    60:
		case    61:
		case    62:
		case    63:
		case    64:
		case    65:
		case    66:
		case    67:
		case    68:
		case    69:
		case    70:   map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-60)*8/12+104;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    71:   map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-60)*8/12+104;
			if (speedo(n) && update) map[n].ch_status=60;
			return tmp;


			// right+up
		case    72:
		case    73:
		case    74:
		case    75:
		case    76:
		case    77:
		case    78:
		case    79:
		case    80:
		case    81:
		case  82:   map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-72)*8/12+112;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    83:   map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-72)*8/12+112;
			if (speedo(n) && update) map[n].ch_status=72;
			return tmp;

			// right+down
		case    84:
		case    85:
		case    86:
		case    87:
		case    88:
		case    89:
		case    90:
		case    91:
		case    92:
		case    93:
		case    94:   map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-84)*8/12+120;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    95:   map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-84)*8/12+120;
			if (speedo(n) && update) map[n].ch_status=84;
			return tmp;

			// turn up to left-up
		case    96:
		case    97:
		case    98: map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-96)+128;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case    99: map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-96)+128;
			if (speedo(n) && update) map[n].ch_status=96;
			return tmp;

			// turn left-up to up
		case 100:
		case 101:
		case 102:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-100)+132;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 103:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-100)+132;
			if (speedo(n) && update) map[n].ch_status=100;
			return tmp;

			// turn up to right-up
		case 104:
		case 105:
		case 106:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-104)+136;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 107:   tmp=map[n].ch_sprite+(map[n].ch_status-104)+136;
			if (speedo(n) && update) map[n].ch_status=104;
			return tmp;

			// turn right-up to right
		case 108:
		case 109:
		case 110:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-108)+140;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 111:   tmp=map[n].ch_sprite+(map[n].ch_status-108)+140;
			if (speedo(n) && update) map[n].ch_status=108;
			return tmp;

			// turn down to left-down
		case 112:
		case 113:
		case 114:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-112)+144;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 115:   tmp=map[n].ch_sprite+(map[n].ch_status-112)+144;
			if (speedo(n) && update) map[n].ch_status=112;
			return tmp;

			// turn left-down to left
		case 116:
		case 117:
		case 118:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-116)+148;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 119:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-116)+148;
			if (speedo(n) && update) map[n].ch_status=116;
			return tmp;

			// turn down to right-down
		case 120:
		case 121:
		case 122:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-120)+152;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 123:   tmp=map[n].ch_sprite+(map[n].ch_status-120)+152;
			if (speedo(n) && update) map[n].ch_status=120;
			return tmp;

			// turn right-down to down
		case 124:
		case 125:
		case 126:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-124)+156;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 127:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-124)+156;
			if (speedo(n) && update) map[n].ch_status=124;
			return tmp;

			// turn left to left-up
		case 128:
		case 129:
		case 130:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-128)+160;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 131:   tmp=map[n].ch_sprite+(map[n].ch_status-128)+160;
			if (speedo(n) && update) map[n].ch_status=128;
			return tmp;

			// turn left-up to up
		case 132:
		case 133:
		case 134:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-132)+164;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 135:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-132)+164;
			if (speedo(n) && update) map[n].ch_status=132;
			return tmp;

			// turn left to left-down
		case 136:
		case 137:
		case 138:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-136)+168;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 139:   tmp=map[n].ch_sprite+(map[n].ch_status-136)+168;
			if (speedo(n) && update) map[n].ch_status=136;
			return tmp;

			// turn left-down to down
		case 140:
		case 141:
		case 142:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-140)+172;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 143:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-140)+172;
			if (speedo(n) && update) map[n].ch_status=140;
			return tmp;

			// turn right to right-up
		case 144:
		case 145:
		case 146:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-144)+176;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 147:   tmp=map[n].ch_sprite+(map[n].ch_status-144)+176;
			if (speedo(n) && update) map[n].ch_status=144;
			return tmp;

			// turn right-up to up
		case 148:
		case 149:
		case 150:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-148)+180;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 151:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-148)+180;
			if (speedo(n) && update) map[n].ch_status=148;
			return tmp;

			// turn right to right-down
		case 152:
		case 153:
		case 154:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-152)+184;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 155:   tmp=map[n].ch_sprite+(map[n].ch_status-152)+184;
			if (speedo(n) && update) map[n].ch_status=152;
			return tmp;

			// turn right-down to down
		case 156:
		case 157:
		case 158:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-156)+188;
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 159:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-156)+188;
			if (speedo(n) && update) map[n].ch_status=156;
			return tmp;

			// misc up
		case 160:
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
		case 166:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-160)+192+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 167:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-160)+192+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status=160;
			return tmp;

			// misc down
		case 168:
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:
		case 174:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-168)+200+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 175:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-168)+200+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status=168;
			return tmp;

			// misc left
		case 176:
		case 177:
		case 178:
		case 179:
		case 180:
		case 181:
		case 182:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-176)+208+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 183:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-176)+208+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status=176;
			return tmp;

			// misc right
		case 184:
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:
		case 190:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-184)+216+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status++;
			return tmp;
		case 191:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-184)+216+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedo(n) && update) map[n].ch_status=184;
			return tmp;

		default:        xlog(0,"Unknown ch_status %d",map[n].ch_status);
			return map[n].ch_sprite;
	}
}

int eng_item(int n)
{
	switch (map[n].it_status) {
		case    0:      return map[n].it_sprite;
		case    1:      return map[n].it_sprite;

			// four sprite animation, 2-step
		case    2:    if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    3:    if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case    4:    if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case    5:    if (speedtab[10][ctick]) map[n].it_status=2;
			return map[n].it_sprite+6;

			// two sprite animation, 1-step
		case    6:    if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    7:    if (speedtab[10][ctick]) map[n].it_status=6;
			return map[n].it_sprite+1;

			// eight sprite animation, 1-step
		case    8:    if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    9:    if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+1;

		case    10:   if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case    11:   if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+3;

		case   12:   if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case    13:   if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+5;

		case    14:   if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite+6;

		case    15:   if (speedtab[10][ctick]) map[n].it_status=8;
			return map[n].it_sprite+7;

			// five sprite animation, 1-step, random
		case    16:   if (speedtab[10][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    17:   if (speedtab[10][ctick]) map[n].it_status++;
				return map[n].it_sprite+1;

		case    18:   	if (speedtab[10][ctick]) map[n].it_status++;
				return map[n].it_sprite+2;

		case    19:   	if (speedtab[10][ctick]) map[n].it_status++;
				return map[n].it_sprite+3;

		case   20:   	if (speedtab[10][ctick]) map[n].it_status=16;
				return map[n].it_sprite+4;

		case   21:  return map[n].it_sprite+(ticker&63);

		default:        xlog(0,"Unknown it_status");
				return map[n].it_sprite;
	}
}

void engine_tick(void)
{
	int n,tmp;

	ticker++;

	for (n=0; n<screen_renderdist*screen_renderdist; n++) {
		map[n].back=0;
		map[n].obj1=0;
		map[n].obj2=0;
		map[n].ovl_xoff=0;
		map[n].ovl_yoff=0;
	}

	for (n=0; n<screen_renderdist*screen_renderdist; n++) {

		map[n].back=map[n].ba_sprite;

		// item
		if (map[n].it_sprite) {
			tmp=eng_item(n);
			map[n].obj1=tmp;
		}

		// character
		if (map[n].ch_sprite) {
			tmp=eng_char(n);
			map[n].obj2=tmp;
		}
	}
}

void send_opt(void)
{
	static int state=0;
	unsigned char buf[16];
	int n;

	buf[0]=CL_CMD_SETUSER;

	switch (state) {
		case    0:  buf[1]=0; buf[2]=0; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n];
			xlog(1,"Transfering user data..."); break;
		case    1: buf[1]=0; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+13]; break;
		case    2:  buf[1]=0; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+26]; break;
		case    3:  buf[1]=0; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+39]; break;
		case    4: buf[1]=0; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+52]; break;
		case    5: buf[1]=0; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+65]; break;

		case  6:    buf[1]=1; buf[2]=0; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n]; break;
		case  7: buf[1]=1; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+13]; break;
		case  8:    buf[1]=1; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+26]; break;
		case  9:    buf[1]=1; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+39]; break;
		case 10: buf[1]=1; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+52]; break;
		case 11: buf[1]=1; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+65]; break;

		case 12:    buf[1]=2; buf[2]=0; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+78]; break;
		case 13: buf[1]=2; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+91]; break;
		case 14:    buf[1]=2; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+104]; break;
		case 15:    buf[1]=2; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+117]; break;
		case 16: buf[1]=2; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+130]; break;
		case 17: buf[1]=2; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+143];
			pdata.changed=0; save_options();
			xlog(1,"Transfer done."); break;
	}
	xsend(buf);
	state++;
}

int firstquit=0;
int wantquit=0;
int maynotquit=TICKS*5;

void cmd_exit(void)
{
	if (do_exit || !maynotquit) {
		quit=1;
		return;
	}
	if (!firstquit) {
		xlog(0," ");
		xlog(0,"Leaving the game without entering a tavern will make you lose money and possibly life. Click again if you still want to leave the hard way.");
		xlog(0,"A tavern is located west of the Temple of Skua (the starting point).");
		firstquit=1;
		return;
	}

	if (!wantquit) {
		cmd1(CL_CMD_EXIT,0);
		wantquit=1;
		xlog(0," ");
		xlog(0,"Exit request acknowledged. Please wait for server to enter exit state.");
	}
}

int noshop=0;
extern int xmove;
extern int do_ticker;

void engine(void)
{
	int tmp,tick,init=0;
	int step=0,skip=0,lookstep=0,optstep=0,skipinrow=0,n,panic,xtimer=0;
	extern int cmd_count,tick_count;
	unsigned int t;

	skilltab=malloc(sizeof(struct skilltab)*50);
	for (n=0; n<50; n++) {
		skilltab[n]=_skilltab[n];
		skilltab[n].attrib[0]=1;
	}

	init_done=1;

	t=GetTickCount();

	while (!quit) {
		do_msg();
		if (wantquit && maynotquit)	maynotquit--;

		if (do_ticker && (ticker&15)==0) cmd1s(CL_CMD_CTICK,ticker);

		if (ticker%4==0) cursedtxt_off = (cursedtxt_off + random(10)) % 10 + cursedtxt_off + 1;

		// For colors that flash back and forth
		if (!col_eff_flag) {
			col_eff_ticker+=7;
			if (col_eff_ticker >= 46) col_eff_flag = 1;
		} else {
			col_eff_ticker-=7;
			if (col_eff_ticker <= 0) col_eff_flag = 0;
		}

		if (step++>16) {
			pskip=100.0*(float)skip/(float)frame;
			pidle=100.0*(float)idle/(float)xtime;
            skip=frame=0;
			idle=xtime=0;
			step=0;
		}

		frame++;

		lookstep++;
		if (lookat && lookstep>QSIZE*3) {
			if (lookat>=lookmax || looks[lookat].known==0)
				cmd1s(CL_CMD_AUTOLOOK,lookat);
			lookat=0;
			lookstep=0;
		}

		if (look_timer)	look_timer--;
		else show_look=0;

		if (subwindow_mode == SUBWND_SHOP && lookstep>QSIZE) {
			cmd1s(CL_CMD_LOOK,shop.nr);
			lookstep=0;
		}

		optstep++;
		if (optstep>4 && pdata.changed) {
			send_opt();
			optstep=0;
		}

		if (xtime>0) xtimer--;

		if (xmove && xtimer<1) {
			switch (xmove) {
				case  1:     cmds(CL_CMD_MOVE,map[(RENDERDIST/2-7)+screen_renderdist*RENDERDIST/2].x,map[(RENDERDIST/2-7)+screen_renderdist*RENDERDIST/2].y); break;
				case  3:     cmds(CL_CMD_MOVE,map[(RENDERDIST/2+7)+screen_renderdist*RENDERDIST/2].x,map[(RENDERDIST/2+7)+screen_renderdist*RENDERDIST/2].y); break;
				case  2:     cmds(CL_CMD_MOVE,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2-7)].x,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2-7)].y); break;
				case  4:     cmds(CL_CMD_MOVE,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2+7)].x,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2+7)].y); break;
			}
			xtimer=4 * (TICKS / 18);
		}

		// Update area instances timer locally
		if ((ticker&15)==0 && loaded_insts > 0) {
			for (n=0; n<loaded_insts; n++) {
				area_insts[n].time_left--;
			}
		}

		panic=0;
		do {
			do_msg();
			tmp=game_loop();
			panic++;
		} while (tmp && panic<8192);

		tmp=tick_do();
        if (tmp) init=1;
		if (do_exit) init=0;

		do_msg();

		if (noshop>0) {
			noshop--;
			subwindow_mode=0;
		}
		if (t>GetTickCount() || skipinrow>100) {	// display frame only if we've got enough time
			eng_display(init);
			eng_flip(t);
			skipinrow=0;
		} else {
			skip++; skipinrow++;
		}

		do_msg();

		if (t_size) tick=TICK*QSIZE/t_size;
		else tick=TICK;

		t+=tick; ttime+=tick; xtime+=tick;
	}
}
