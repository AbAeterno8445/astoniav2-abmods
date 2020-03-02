#define MMOD_SKEL               0       // Area is inhabited by skeletons
#define MMOD_UNDEAD             1       // Area is inhabited by undead
#define MMOD_GARGS              2       // Area is inhabited by gargoyles
#define MMOD_GOLEMS             3       // Area is inhabited by golems
#define MMOD_GHOSTS             4       // Area is inhabited by ghosts
#define MMOD_TWINNED            5       // Area has two unique bosses
#define MMOD_MOBLIFE            6       // X% more monster life
#define MMOD_PLRDCWV            7       // Player WV is reduced by X% against monsters
#define MMOD_MOBDMG             8       // X% increased monster damage
#define MMOD_MOBIMM             9       // Monsters gain +X immunity
#define MMOD_MOBBWI             10      // Monsters gain +X BWI (brav, will, int)
#define MMOD_BOSSDMG            11      // Unique boss deals X% increased damage
#define MMOD_BOSSLIFE           12      // Unique boss has X% increased life
#define MMOD_MOBAV              13      // Monsters gain +X AV
#define MMOD_MOBBLESS           14      // Monsters gain +X bless
#define MMOD_MOBPOISON          15      // Monsters have a X% chance on hit to poison, dealing Y% of the damage dealt over time
#define MMOD_MOBCRITS           16      // Monsters have a X% chance to critically strike, dealing Y% more damage
#define MMOD_PLRDCRECOV         17      // Players have X% less recovery rate of life, endurance and mana
#define MMOD_BRNGROUND          18      // Area has patches of burning ground (radius X, freq Y, debuff lasts Z seconds)
#define MMOD_FRZGROUND          19      // Area has patches of freezing ground (radius X, freq Y, debuff lasts Z seconds)
#define MMOD_SHKGROUND          20      // Area has patches of shocking ground (radius X, freq Y, debuff lasts Z seconds)
#define MMOD_PLRDCHEAL          21      // Players have X% reduced effectiveness of healing effects (affects heal spell and potions)
#define MMOD_DEATHTRAPS         22      // Area contains deathtraps (X freq)
#define MMOD_PLRDCIMM           23      // Players lose X immunity and Y resistance
#define MMOD_PLHPONHIT          24      // Players have a X% chance of losing Y% of their maximum hp when hit
#define MMOD_BREACH             25      // Area contains X breaches
#define MMOD_ICEGARGS           26      // Area is inhabited by ice gargoyles
#define MMOD_LIZARDS            27      // Area is inhabited by lizards
#define MMOD_SHIVA              28      // Area is influenced by Shiva

#define AMAP_XPMULT             8       // Experience multiplier applied to mapborn monsters'
#define AMAP_MEDTIER            10      // Map tiers equal to or higher than this level are considered medium
#define AMAP_HIGHTIER           18      // Map tiers equal to or higher than this level are considered high

#define AMAP_ORB_ADD1           0       // Map-modifying orb IDs
#define AMAP_ORB_ADD2           1
#define AMAP_ORB_ADD3           2
#define AMAP_ORB_SCOUR          3
#define AMAP_ORB_SHIVA          4

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

        unsigned short orbs[5];
        
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
