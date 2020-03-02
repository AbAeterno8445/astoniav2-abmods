#define VERSION 0x020E06

#define MAXSPRITE 2000+(128*1024)

//#define MAPX			TILEX
//#define MAPY			TILEY

#define MAPX_MAX		1024
#define MAPY_MAX		1024

#define MS_MOVE		    0
#define MS_LB_DOWN	    1
#define MS_RB_DOWN	    2
#define MS_LB_UP		3
#define MS_RB_UP		4

#define HPBAR_WIDTH		42

#define TICK			(1000/TICKS)

#define HIGH_VAL		(1<<30)

#define QSIZE			8

// Rendering distance (Must match server-side)
#define RENDERDIST			54

// Resolution-specific data
// 800x600
#define VIEWSIZE_800		34			// View distance in tiles (must be lower than RENDERDIST)
#define VIEW_SUBEDGES_800	4			// Amount of tiles to make un-clickable from edges, in southern and eastern corners (use if your overlay doesn't display those)
#define XPOS_800			0			// X origin for tile drawing
#define YPOS_800			440			// Y origin for tile drawing
#define XWALK_NX_800		164			// Auto-walk button coordinates, format XWALK_<dir><X/Y>_<res>
#define XWALK_NY_800		315
#define XWALK_EX_800		635
#define XWALK_EY_800		315
#define XWALK_SX_800		569
#define XWALK_SY_800		518
#define XWALK_WX_800		230
#define XWALK_WY_800		518

// 1280x720
#define VIEWSIZE_1280		48
#define VIEW_SUBEDGES_1280	0
#define XPOS_1280			224
#define YPOS_1280			460
#define XWALK_NX_1280		307
#define XWALK_NY_1280		256
#define XWALK_EX_1280		944
#define XWALK_EY_1280		256
#define XWALK_SX_1280		943
#define XWALK_SY_1280		638
#define XWALK_WX_1280		307
#define XWALK_WY_1280		638

// 1600x900
#define VIEWSIZE_1600		54
#define VIEW_SUBEDGES_1600	0
#define XPOS_1600			440
#define YPOS_1600			520
#define XWALK_NX_1600		415
#define XWALK_NY_1600		325
#define XWALK_EX_1600		1260
#define XWALK_EY_1600		325
#define XWALK_SX_1600		1259
#define XWALK_SY_1600		747
#define XWALK_WX_1600		415
#define XWALK_WY_1600		747

// GUI overlay sprite number, based on resolution
#define GUI_OVERLAY_800		370
#define GUI_OVERLAY_1280	371
#define GUI_OVERLAY_1600	372

#define GUI_INSTMENU		373
#define GUI_AMAPSMENU		374

// Offsets from screen width/screen height for positioning GUI elements
#define GUI_QSPELLS_XOFF	198  // Spells quickbar
#define GUI_QSPELLS_YOFF	98

#define GUI_XTRADATA_XOFF	153  // WV/AV/Exp table below chat
#define GUI_XTRADATA_YOFF	243

#define GUI_CHAT_XOFF		299  // Chatbox

#define GUI_MINIMAP_YOFF	130  // Minimap

// Font macros
#define FNT_RED			0
#define FNT_YELLOW		1
#define FNT_GREEN		2
#define FNT_BLUE		3
#define FNT_OBFUSCATED	4
#define FNT_PURPLE		1960
#define FNT_TURQUOISE	1961
#define FNT_PINK		1962
#define FNT_ORANGE		1963
#define FNT_AQUA		1964
#define FNT_SILVER		1965
#define FNT_EMERALD		1966
#define FNT_DEMON       1967

// Sub-window modes
#define SUBWND_SHOP			1
#define SUBWND_INSTMNG		2
#define SUBWND_AMAPS		3

struct xbutton
{
   char name[8];
   int skill_nr;
//   int skill_strength;
};


struct pdata
{
	char cname[80];
	char ref[80];
	char desc[160];

	char changed;

  int hide;
  int show_names;
  int show_proz;
  struct xbutton xbutton[12];
};

extern struct pdata pdata;

extern struct cplayer pl;
extern struct cmap *map;

void mouse(int x,int y,int state);

void cmd1s(int cmd,int val);
void add_look(unsigned short nr,char *name,unsigned short id);
int attrib_needed(int n,int v);
int hp_needed(int v);
int end_needed(int v);
int mana_needed(int v);
int skill_needed(int n,int v);
void xlog(char font,char *format,...);
int play_sound(char *file,int vol,int pan);
void reset_block(void);

void setres_800(void);
void setres_1600(void);
void setres_default(void);

void save_options(void);
void load_options(void);
void options(void);

void xsend(unsigned char *buf);
void engine_tick(void);
void so_perf_report(int ticksize,int skip,int idle);
int game_loop(void);
int tick_do(void);
void init_engine(void);

void button_command(int nr);
void cmd_exit(void);
void cmds(int cmd,int x,int y);

struct key
{
	unsigned int usnr;
	unsigned int pass1,pass2;
	char name[40];
  int race;
};

struct look
{
	unsigned char autoflag;
	unsigned short worn[20];
	unsigned short sprite;
	unsigned int points;
	char name[40];
	unsigned int hp;
	unsigned int end;
	unsigned int mana;
	unsigned int a_hp;
	unsigned int a_end;
   unsigned int a_mana;
	unsigned short nr;
	unsigned short id;
	unsigned char extended;
	unsigned short item[62];
	unsigned int price[62];
   unsigned int pl_price;
};

struct areainst
{
	int creation_time;
	int time_left;
	int mobs_remaining;
};

extern struct look look;
extern struct look shop;
extern unsigned int subwindow_mode;

extern struct areainst area_insts[7];
extern unsigned int loaded_insts;
extern char area_instname[40];

#define HL_BUTTONBOX	1
#define HL_STATBOX	2
#define HL_BACKPACK	3
#define HL_EQUIPMENT	4
#define HL_SPELLBOX	5
#define HL_CITEM		6
#define HL_MONEY		7
#define HL_MAP			8
#define HL_SHOP		9
#define HL_STATBOX2	10

extern int hightlight;
extern int hightlight_sub;

#define CT_NONE		1
#define CT_TAKE		2
#define CT_DROP		3
#define CT_USE			4
#define CT_GIVE		5
#define CT_WALK		6
#define CT_HIT			7
#define CT_SWAP		8
#define CT_SEL			9

struct skilltab
{
		int nr;
		  char sortkey;
		  char name[40];
		  char desc[200];

		  int attrib[3];
};

extern struct skilltab *skilltab;
extern char *at_name[];

#define AMAP_CONN_NW            (1<<0)
#define AMAP_CONN_N             (1<<1)
#define AMAP_CONN_NE            (1<<2)
#define AMAP_CONN_E             (1<<3)
#define AMAP_CONN_SE            (1<<4)
#define AMAP_CONN_S             (1<<5)
#define AMAP_CONN_SW            (1<<6)
#define AMAP_CONN_W             (1<<7)

#define AMAP_ORB_ADD1			0
#define AMAP_ORB_ADD2			1
#define AMAP_ORB_ADD3			2
#define AMAP_ORB_SCOUR			3
#define AMAP_ORB_SHIVA			4

#define AMAP_MAXBASES			100

struct areamap_base
{
	unsigned char used;

	int tier;
	int quadrant;

	unsigned char conn_flags;

	unsigned char charges;

	unsigned int sprite;
};

#define AMAP_MAXMODS			40

struct areamap_selected
{
	char name[40];

	int mods[AMAP_MAXMODS];
};

extern struct areamap_base *amap_bases;
extern struct areamap_selected amap_sel;

void dd_puttext(int x,int y,int *font,char *text);
void dd_puttext_1f(int x,int y,int font,char *text);
void dd_gputc(int xpos,int ypos,int font,int c);
void dd_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff);
void dd_putc(int xpos,int ypos,int font,int c);
void dd_xputtext(int x,int y,int font,char *format,...);
void say(char *input);
void cmd1(int cmd,int x);
void init_xalloc(void);
void tlog(char *text,int font);
void do_msg(void);
