#define MMOD_SKEL               1       // Area is inhabited by skeletons
#define MMOD_UNDEAD             2       // Area is inhabited by undead
#define MMOD_GARGS              3       // Area is inhabited by gargoyles
#define MMOD_GOLEMS             4       // Area is inhabited by golems
#define MMOD_GHOSTS             5       // Area is inhabited by ghosts
#define MMOD_TWINNED            6       // Area has two unique bosses
#define MMOD_MOBLIFE            7       // X% more monster life
#define MMOD_PLRDCWV            8       // Player WV is reduced by X% against monsters
#define MMOD_MOBDMG             9       // X% increased monster damage
#define MMOD_MOBIMM             10      // Monsters gain +X immunity
#define MMOD_MOBBWI             11      // Monsters gain +X BWI (brav, will, int)
#define MMOD_BOSSDMG            12      // Unique boss deals X% increased damage
#define MMOD_BOSSLIFE           13      // Unique boss has X% increased life
#define MMOD_MOBAV              14      // Monsters gain +X AV
#define MMOD_MOBBLESS           15      // Monsters gain +X bless
#define MMOD_MOBPOISON          16      // Monsters have a X% chance on hit to poison, dealing Y% of the damage dealt over time
#define MMOD_MOBCRITS           17      // Monsters have a X% chance to critically strike, dealing Y% more damage
#define MMOD_PLRDCRECOV         18      // Players have X% less recovery rate of life, endurance and mana
#define MMOD_BRNGROUND          19      // Area has patches of burning ground (radius X, freq Y, debuff lasts Z seconds)
#define MMOD_FRZGROUND          20      // Area has patches of freezing ground (radius X, freq Y, debuff lasts Z seconds)
#define MMOD_SHKGROUND          21      // Area has patches of shocking ground (radius X, freq Y, debuff lasts Z seconds)
#define MMOD_PLRDCHEAL          22      // Players have X% reduced effectiveness of healing effects (affects heal spell and potions)
#define MMOD_DEATHTRAPS         23      // Area contains deathtraps (X freq)
#define MMOD_PLRDCIMM           24      // Players lose X immunity and Y resistance
#define MMOD_PLHPONHIT          25      // Players have a X% chance of losing Y% of their maximum hp when hit
#define MMOD_BREACH             26      // Area contains X breaches
#define MMOD_ICEGARGS           27      // Area is inhabited by ice gargoyles
#define MMOD_LIZARDS            28      // Area is inhabited by lizards

#define AMAP_MAXBASES           100
#define AMAP_MAXMODS            40

// Holds map-device related data for player characters
struct areamap_device
{
        unsigned char used;

        unsigned int ch;                        // character ID of owner

        unsigned int level;
        unsigned long experience;

        unsigned char map_charges[AMAP_MAXBASES]; // Amount of charges for each map base

        unsigned int map_mods[AMAP_MAXBASES][AMAP_MAXMODS]; // Modifiers for each map base
        
} __attribute__ ((packed));

#define AMAP_CONN_NW            (1<<0)
#define AMAP_CONN_N             (1<<1)
#define AMAP_CONN_NE            (1<<2)
#define AMAP_CONN_E             (1<<3)
#define AMAP_CONN_SE            (1<<4)
#define AMAP_CONN_S             (1<<5)
#define AMAP_CONN_SW            (1<<6)
#define AMAP_CONN_W             (1<<7)

#define AMAP_BASES_SIZE         (sizeof(struct areamap_base) * AMAP_MAXBASES)

// Map bases
struct areamap_base
{
        unsigned char used;

        char name[40];
        char description[200];

        char inst_base_name[40];        // Name of instance base for map
        unsigned char layouts;          // # of available layouts for this map

        unsigned short tier;
        unsigned char quadrant;         // Column position of map

        unsigned char conn_flags;

        unsigned int sprite;

} __attribute__ ((packed));

extern struct areamap_device *amap_devices;
extern struct areamap_base *amap_bases;
