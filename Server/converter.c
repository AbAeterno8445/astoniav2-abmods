//Converts old character and old item structure to the new one
//USE WITH CARE, AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "server.h"

int converter_char(void);
int converter_item(void);

struct character *newch;
struct character *ch_temp;
struct oldcharacter *oldch;
struct oldcharacter *oldch_temp;

struct item *newit;
struct item *newit_temp;
struct olditem *oldit;
struct olditem *oldit_temp;

int converter_main(int cmode) {
    printf("Beginning conversion.\n");

    switch(cmode) {
        case 0: return converter_char();
        case 1: return converter_item();
    }

    return 0;
}

int converter_char(void) {
    int cn;
	int handle[4];

    printf("Converting characters.\n");
    printf("Loading data files...\n");
    //LOAD oldch
    printf("Loading OLD CHAR: Item size=%d, file size=%dK\n",
                sizeof(struct oldcharacter),OLDCHARSIZE>>10);

    handle[0]=open(DATDIR"/char.dat",O_RDWR);
    if (handle[0]==-1) {
            printf("Could not open characters data file (%s/char.dat). Stopping Conversion.\n",DATDIR);
            return -1;
    }
    if (!extend(handle[0], OLDCHARSIZE, sizeof(struct oldcharacter), NULL)) return -1;

    oldch=mmap(NULL,OLDCHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[0],0);
    if (oldch==(void*)-1) return -1;

    //LOAD newch
    printf("Loading NEW CHAR: Item size=%d, file size=%dK\n",
            sizeof(struct character),CHARSIZE>>10);

    handle[1]=open(DATDIR"/newchar.dat",O_RDWR);
    if (handle[1]==-1) {
            printf("Building new characters\n");
            handle[1]=open(DATDIR"/newchar.dat",O_RDWR|O_CREAT,0600);
    }
    if (!extend(handle[1], CHARSIZE, sizeof(struct character), NULL)) return -1;

    newch=mmap(NULL,CHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[1],0);
    if (newch==(void*)-1) return -1;

    //LOAD oldtchar
    printf("Loading OLD TCHAR: Item size=%d, file size=%dK\n",
                sizeof(struct oldcharacter),OLDTCHARSIZE>>10);

    handle[2]=open(DATDIR"/tchar.dat",O_RDWR);
    if (handle[2]==-1) {
            printf("Could not open character templates file (%s/tchar.dat). Stopping conversion.\n", DATDIR);
            return -1;
    }
    if (!extend(handle[2], OLDTCHARSIZE, sizeof(struct oldcharacter), NULL)) return -1;

    oldch_temp=mmap(NULL,OLDTCHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[2],0);
    if (oldch_temp==(void*)-1) return -1;

    //LOAD newtchar
    printf("Loading NEW TCHAR: Item size=%d, file size=%dK\n",
                sizeof(struct character),TCHARSIZE>>10);

    handle[3]=open(DATDIR"/newtchar.dat",O_RDWR);
    if (handle[3]==-1) {
            printf("Building new tcharacters\n");
            handle[3]=open(DATDIR"/newtchar.dat",O_RDWR|O_CREAT,0600);
    }
    if (!extend(handle[3], TCHARSIZE, sizeof(struct character), NULL)) return -1;

    ch_temp=mmap(NULL,TCHARSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[3],0);
    if (ch_temp==(void*)-1) return -1;

	//conversion begins
    printf("Converting characters...\n");
    for (cn=0; cn<MAXCHARS; cn++) {

        newch[cn].used=oldch[cn].used;

        for (int i=0; i<40; i++) {
            newch[cn].name[i] = oldch[cn].name[i];
            newch[cn].reference[i] = oldch[cn].reference[i];
        }

        for (int i=0; i<LENDESC; i++) {
            newch[cn].description[i] = oldch[cn].description[i];
        }

        newch[cn].kindred=oldch[cn].kindred;

        newch[cn].player=oldch[cn].player;
        newch[cn].pass1=oldch[cn].pass1;
        newch[cn].pass2=oldch[cn].pass2;

        newch[cn].sprite=oldch[cn].sprite;
        newch[cn].sound=oldch[cn].sound;

        newch[cn].flags=oldch[cn].flags;

        newch[cn].alignment=oldch[cn].alignment;

        newch[cn].temple_x=oldch[cn].temple_x;
        newch[cn].temple_y=oldch[cn].temple_y;

        newch[cn].tavern_x=oldch[cn].tavern_x;
        newch[cn].tavern_y=oldch[cn].tavern_y;

        newch[cn].temp=oldch[cn].temp;

        for (int n=0; n<5; n++) {
            for (int n1=0; n1<6; n1++) {
            newch[cn].attrib[n][n1]=oldch[cn].attrib[n][n1];
            }
        }

        for (int n=0; n<6; n++) {
            newch[cn].hp[n]=oldch[cn].hp[n];
            newch[cn].end[n]=oldch[cn].hp[n];
            newch[cn].mana[n]=oldch[cn].hp[n];
        }

        for (int n=0; n<50; n++) {
            for (int n1=0; n1<6; n1++) {
                newch[cn].skill[n][n1]=oldch[cn].skill[n][n1];
            }
        }

        newch[cn].weapon_bonus=oldch[cn].weapon_bonus;
        newch[cn].armor_bonus=oldch[cn].armor_bonus;

        newch[cn].a_hp=oldch[cn].a_hp;
        newch[cn].a_end=oldch[cn].a_end;
        newch[cn].a_mana=oldch[cn].a_mana;

        newch[cn].light=oldch[cn].light;
        newch[cn].mode=oldch[cn].mode;
        newch[cn].speed=oldch[cn].speed;

        newch[cn].points=oldch[cn].points;
        newch[cn].points_tot=oldch[cn].points_tot;

        newch[cn].armor=oldch[cn].armor;
        newch[cn].weapon=oldch[cn].weapon;

        newch[cn].instance_id=-1; // New
        newch[cn].x=oldch[cn].x;
        newch[cn].y=oldch[cn].y;
        newch[cn].tox=oldch[cn].tox;
        newch[cn].toy=oldch[cn].toy;
        newch[cn].frx=oldch[cn].frx;
        newch[cn].fry=oldch[cn].fry;
        newch[cn].status=oldch[cn].status;
        newch[cn].status2=oldch[cn].status2;
        newch[cn].dir=oldch[cn].dir;

        newch[cn].gold=oldch[cn].gold;

        for (int n=0; n<40; n++) {
            newch[cn].item[n]=oldch[cn].item[n];
        }

        for (int n=0; n<20; n++) {
            newch[cn].worn[n]=oldch[cn].worn[n];
            newch[cn].spell[n]=oldch[cn].spell[n];
        }

        newch[cn].citem=oldch[cn].citem;

        newch[cn].creation_date=oldch[cn].creation_date;
        newch[cn].login_date=oldch[cn].login_date;

        newch[cn].addr=oldch[cn].addr;

        newch[cn].current_online_time=oldch[cn].current_online_time;
        newch[cn].total_online_time=oldch[cn].total_online_time;
        newch[cn].comp_volume=oldch[cn].comp_volume;
        newch[cn].raw_volume=oldch[cn].raw_volume;
        newch[cn].idle=oldch[cn].idle;

        newch[cn].attack_cn=oldch[cn].attack_cn;
        newch[cn].skill_nr=oldch[cn].skill_nr;
        newch[cn].skill_target1=oldch[cn].skill_target1;
        newch[cn].skill_target2=oldch[cn].skill_target2;
        newch[cn].goto_x=oldch[cn].goto_x;
        newch[cn].goto_y=oldch[cn].goto_y;
        newch[cn].use_nr=oldch[cn].use_nr;

        newch[cn].misc_action=oldch[cn].misc_action;
        newch[cn].misc_target1=oldch[cn].misc_target1;
        newch[cn].misc_target2=oldch[cn].misc_target2;

        newch[cn].cerrno=oldch[cn].cerrno;

        newch[cn].escape_timer=oldch[cn].escape_timer;
        for (int n=0; n<4; n++) {
            newch[cn].enemy[n]=oldch[cn].enemy[n];
        }
        newch[cn].current_enemy=oldch[cn].current_enemy;

        newch[cn].retry=oldch[cn].retry;

        newch[cn].stunned=oldch[cn].stunned;

        newch[cn].speed_mod=oldch[cn].speed_mod;
        newch[cn].last_action=oldch[cn].last_action;
        newch[cn].unused=oldch[cn].unused;
        newch[cn].depot_sold=oldch[cn].depot_sold;

        newch[cn].gethit_dam=oldch[cn].gethit_dam;
        newch[cn].gethit_bonus=oldch[cn].gethit_bonus;

        newch[cn].light_bonus=oldch[cn].light_bonus;

        for (int n=0; n<16; n++) {
            newch[cn].passwd[n]=oldch[cn].passwd[n];
        }

        newch[cn].lastattack=oldch[cn].lastattack;
        for (int n=0; n<25; n++) {
            newch[cn].future1[n]=oldch[cn].future1[n];
        }

        newch[cn].sprite_override=oldch[cn].sprite_override;

        for (int n=0; n<49; n++) {
            newch[cn].future2[n]=oldch[cn].future2[n];
        }

        for (int n=0; n<62; n++) {
            newch[cn].depot[n]=oldch[cn].depot[n];
        }

        newch[cn].depot_cost=oldch[cn].depot_cost;

        newch[cn].luck=oldch[cn].luck;

        newch[cn].unreach=oldch[cn].unreach;
        newch[cn].unreachx=oldch[cn].unreachx;
        newch[cn].unreachy=oldch[cn].unreachy;

        newch[cn].class=oldch[cn].class;

        for (int n=0; n<12; n++) {
            newch[cn].future3[n]=oldch[cn].future3[n];
        }

        newch[cn].logout_date=oldch[cn].logout_date;

        for (int n=0; n<100; n++) {
            newch[cn].data[n]=oldch[cn].data[n];
        }

        for (int n=0; n<10; n++) {
            for (int n1=0; n1<160; n1++) {
                newch[cn].text[n][n1]=oldch[cn].text[n][n1];
            }
        }
    }
    
    //ch_temp conversion
    printf("Converting tcharacters...\n");
    for (cn=0; cn<MAXTCHARS; cn++) {

        ch_temp[cn].used=oldch_temp[cn].used;

        strcpy(ch_temp[cn].name, oldch_temp[cn].name);
        strcpy(ch_temp[cn].reference, oldch_temp[cn].reference);
        strcpy(ch_temp[cn].description, oldch_temp[cn].description);

        ch_temp[cn].kindred=oldch_temp[cn].kindred;

        ch_temp[cn].player=oldch_temp[cn].player;
        ch_temp[cn].pass1=oldch_temp[cn].pass1;
        ch_temp[cn].pass2=oldch_temp[cn].pass2;

        ch_temp[cn].sprite=oldch_temp[cn].sprite;
        ch_temp[cn].sound=oldch_temp[cn].sound;

        ch_temp[cn].flags=oldch_temp[cn].flags;

        ch_temp[cn].alignment=oldch_temp[cn].alignment;

        ch_temp[cn].temple_x=oldch_temp[cn].temple_x;
        ch_temp[cn].temple_y=oldch_temp[cn].temple_y;

        ch_temp[cn].tavern_x=oldch_temp[cn].tavern_x;
        ch_temp[cn].tavern_y=oldch_temp[cn].tavern_y;

        ch_temp[cn].temp=oldch_temp[cn].temp;

        for (int n=0; n<5; n++) {
            for (int n1=0; n1<6; n1++) {
                ch_temp[cn].attrib[n][n1]=oldch_temp[cn].attrib[n][n1];
            }
        }

        for (int n=0; n<6; n++) {
            ch_temp[cn].hp[n]=oldch_temp[cn].hp[n];
            ch_temp[cn].end[n]=oldch_temp[cn].hp[n];
            ch_temp[cn].mana[n]=oldch_temp[cn].hp[n];
        }

        for (int n=0; n<50; n++) {
            for (int n1=0; n1<6; n1++) {
                ch_temp[cn].skill[n][n1]=oldch_temp[cn].skill[n][n1];
            }
        }

        ch_temp[cn].weapon_bonus=oldch_temp[cn].weapon_bonus;
        ch_temp[cn].armor_bonus=oldch_temp[cn].armor_bonus;

        ch_temp[cn].a_hp=oldch_temp[cn].a_hp;
        ch_temp[cn].a_end=oldch_temp[cn].a_end;
        ch_temp[cn].a_mana=oldch_temp[cn].a_mana;

        ch_temp[cn].light=oldch_temp[cn].light;
        ch_temp[cn].mode=oldch_temp[cn].mode;
        ch_temp[cn].speed=oldch_temp[cn].speed;

        ch_temp[cn].points=oldch_temp[cn].points;
        ch_temp[cn].points_tot=oldch_temp[cn].points_tot;

        ch_temp[cn].armor=oldch_temp[cn].armor;
        ch_temp[cn].weapon=oldch_temp[cn].weapon;

        ch_temp[cn].instance_id=-1; // New
        ch_temp[cn].x=oldch_temp[cn].x;
        ch_temp[cn].y=oldch_temp[cn].y;
        ch_temp[cn].tox=oldch_temp[cn].tox;
        ch_temp[cn].toy=oldch_temp[cn].toy;
        ch_temp[cn].frx=oldch_temp[cn].frx;
        ch_temp[cn].fry=oldch_temp[cn].fry;
        ch_temp[cn].status=oldch_temp[cn].status;
        ch_temp[cn].status2=oldch_temp[cn].status2;
        ch_temp[cn].dir=oldch_temp[cn].dir;

        ch_temp[cn].gold=oldch_temp[cn].gold;

        for (int n=0; n<40; n++) {
            ch_temp[cn].item[n]=oldch_temp[cn].item[n];
        }

        for (int n=0; n<20; n++) {
            ch_temp[cn].worn[n]=oldch_temp[cn].worn[n];
            ch_temp[cn].spell[n]=oldch_temp[cn].spell[n];
        }

        ch_temp[cn].citem=oldch_temp[cn].citem;

        ch_temp[cn].creation_date=oldch_temp[cn].creation_date;
        ch_temp[cn].login_date=oldch_temp[cn].login_date;

        ch_temp[cn].addr=oldch_temp[cn].addr;

        ch_temp[cn].current_online_time=oldch_temp[cn].current_online_time;
        ch_temp[cn].total_online_time=oldch_temp[cn].total_online_time;
        ch_temp[cn].comp_volume=oldch_temp[cn].comp_volume;
        ch_temp[cn].raw_volume=oldch_temp[cn].raw_volume;
        ch_temp[cn].idle=oldch_temp[cn].idle;

        ch_temp[cn].attack_cn=oldch_temp[cn].attack_cn;
        ch_temp[cn].skill_nr=oldch_temp[cn].skill_nr;
        ch_temp[cn].skill_target1=oldch_temp[cn].skill_target1;
        ch_temp[cn].skill_target2=oldch_temp[cn].skill_target2;
        ch_temp[cn].goto_x=oldch_temp[cn].goto_x;
        ch_temp[cn].goto_y=oldch_temp[cn].goto_y;
        ch_temp[cn].use_nr=oldch_temp[cn].use_nr;

        ch_temp[cn].misc_action=oldch_temp[cn].misc_action;
        ch_temp[cn].misc_target1=oldch_temp[cn].misc_target1;
        ch_temp[cn].misc_target2=oldch_temp[cn].misc_target2;

        ch_temp[cn].cerrno=oldch_temp[cn].cerrno;

        ch_temp[cn].escape_timer=oldch_temp[cn].escape_timer;
        for (int n=0; n<4; n++) {
            ch_temp[cn].enemy[n]=oldch_temp[cn].enemy[n];
        }
        ch_temp[cn].current_enemy=oldch_temp[cn].current_enemy;

        ch_temp[cn].retry=oldch_temp[cn].retry;

        ch_temp[cn].stunned=oldch_temp[cn].stunned;

        ch_temp[cn].speed_mod=oldch_temp[cn].speed_mod;
        ch_temp[cn].last_action=oldch_temp[cn].last_action;
        ch_temp[cn].unused=oldch_temp[cn].unused;
        ch_temp[cn].depot_sold=oldch_temp[cn].depot_sold;

        ch_temp[cn].gethit_dam=oldch_temp[cn].gethit_dam;
        ch_temp[cn].gethit_bonus=oldch_temp[cn].gethit_bonus;

        ch_temp[cn].light_bonus=oldch_temp[cn].light_bonus;

        for (int n=0; n<16; n++) {
            ch_temp[cn].passwd[n]=oldch_temp[cn].passwd[n];
        }

        ch_temp[cn].lastattack=oldch_temp[cn].lastattack;
        for (int n=0; n<25; n++) {
            ch_temp[cn].future1[n]=oldch_temp[cn].future1[n];
        }

        ch_temp[cn].sprite_override=oldch_temp[cn].sprite_override;

        for (int n=0; n<49; n++) {
            ch_temp[cn].future2[n]=oldch_temp[cn].future2[n];
        }

        for (int n=0; n<62; n++) {
            ch_temp[cn].depot[n]=oldch_temp[cn].depot[n];
        }

        ch_temp[cn].depot_cost=oldch_temp[cn].depot_cost;

        ch_temp[cn].luck=oldch_temp[cn].luck;

        ch_temp[cn].unreach=oldch_temp[cn].unreach;
        ch_temp[cn].unreachx=oldch_temp[cn].unreachx;
        ch_temp[cn].unreachy=oldch_temp[cn].unreachy;

        ch_temp[cn].class=oldch_temp[cn].class;

        for (int n=0; n<12; n++) {
            ch_temp[cn].future3[n]=oldch_temp[cn].future3[n];
        }

        ch_temp[cn].logout_date=oldch_temp[cn].logout_date;

        for (int n=0; n<100; n++) {
            ch_temp[cn].data[n]=oldch_temp[cn].data[n];
        }

        for (int n=0; n<10; n++) {
            for (int n1=0; n1<160; n1++) {
                ch_temp[cn].text[n][n1]=oldch_temp[cn].text[n][n1];
            }
        }
    }
	//conversion end

	printf("Conversion done.\n");
	for (int n=0; n<4; n++) close(handle[n]);
	
	printf("Unloading data files...\n");
	if (munmap(oldch,OLDCHARSIZE)) printf("ERROR: munmap(oldch) %s\n",strerror(errno));
	if (munmap(newch,CHARSIZE)) printf("ERROR: munmap(newch) %s\n",strerror(errno));
	if (munmap(oldch_temp,OLDTCHARSIZE)) printf("ERROR: munmap(oldch_temp) %s\n",strerror(errno));
	if (munmap(ch_temp,TCHARSIZE)) printf("ERROR: munmap(ch_temp) %s\n",strerror(errno));
	return 0;
}

int converter_item(void) {
    int in;
	int handle[4];

    printf("Converting items.\n");
    printf("Loading data files...\n");
    //LOAD oldit
    printf("Loading OLD ITEM: Item size=%d, file size=%dK\n",
                sizeof(struct olditem),OLDITEMSIZE>>10);

    handle[0]=open(DATDIR"/item.dat",O_RDWR);
    if (handle[0]==-1) {
        printf("Could not find items data file (%s/item.dat). Stopping conversion.\n", DATDIR);
        return -1;
    }
    if (!extend(handle[0], OLDITEMSIZE, sizeof(struct olditem), NULL)) return -1;

    oldit=mmap(NULL,OLDITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[0],0);
    if (oldit==(void*)-1) return -1;

    //LOAD newit
    printf("Loading NEW ITEM: Item size=%d, file size=%dK\n",
            sizeof(struct item),ITEMSIZE>>10);

    handle[1]=open(DATDIR"/newitem.dat",O_RDWR);
    if (handle[1]==-1) {
            printf("Building new items\n");
            handle[1]=open(DATDIR"/newitem.dat",O_RDWR|O_CREAT,0600);
    }
    if (!extend(handle[1], ITEMSIZE, sizeof(struct item), NULL)) return -1;

    newit=mmap(NULL,ITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[1],0);
    if (newit==(void*)-1) return -1;

    //LOAD oldtitem
    printf("Loading OLD TITEM: Item size=%d, file size=%dK\n",
                sizeof(struct olditem),OLDTITEMSIZE>>10);

    handle[2]=open(DATDIR"/titem.dat",O_RDWR);
    if (handle[2]==-1) {
        printf("Could not find item templates data file (%s/titem.dat). Stopping conversion.\n", DATDIR);
        return -1;
    }
    if (!extend(handle[2], OLDTITEMSIZE, sizeof(struct olditem), NULL)) return -1;

    oldit_temp=mmap(NULL,OLDTITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[2],0);
    if (oldit_temp==(void*)-1) return -1;

    //LOAD newtitem
    printf("Loading NEW TITEM: Item size=%d, file size=%dK\n",
                sizeof(struct item),TITEMSIZE>>10);

    handle[3]=open(DATDIR"/newtitem.dat",O_RDWR);
    if (handle[3]==-1) {
            printf("Building new titems\n");
            handle[3]=open(DATDIR"/newtitem.dat",O_RDWR|O_CREAT,0600);
    }
    if (!extend(handle[3], TITEMSIZE, sizeof(struct item), NULL)) return -1;

    newit_temp=mmap(NULL,TITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle[3],0);
    if (newit_temp==(void*)-1) return -1;

	//conversion begins
    printf("Converting items...\n");
    for (in=0; in<MAXITEM; in++) {
        newit[in].used = oldit[in].used;

        for (int i=0; i<40; i++) {
            newit[in].name[i] = oldit[in].name[i];
            newit[in].reference[i] = oldit[in].reference[i];
        }

        for (int i=0; i<200; i++) {
            newit[in].description[i] = oldit[in].description[i];
        }

        newit[in].flags = oldit[in].flags;
        
        newit[in].value = oldit[in].value;
        newit[in].placement = oldit[in].placement;

        newit[in].temp = oldit[in].temp;

        newit[in].damage_state = oldit[in].damage_state;

        for (int i=0; i<2; i++) {
            newit[in].max_age[i] = oldit[in].max_age[i];
            newit[in].current_age[i] = oldit[in].current_age[i];
        }

        newit[in].max_damage = oldit[in].max_damage;
        newit[in].current_damage = oldit[in].current_damage;

        for (int i=0; i<5; i++) {
            for (int j=0; j<3; j++) {
                newit[in].attrib[i][j] = oldit[in].attrib[i][j];
            }
        }

        for (int i=0; i<3; i++) {
            newit[in].hp[i] = oldit[in].hp[i];
            newit[in].end[i] = oldit[in].end[i];
            newit[in].mana[i] = oldit[in].mana[i];
        }

        for (int i=0; i<50; i++) {
            for (int j=0; j<3; j++) {
                newit[in].skill[i][j] = oldit[in].skill[i][j];
            }
        }

        for (int i=0; i<2; i++) {
            newit[in].armor[i] = oldit[in].armor[i];
            newit[in].weapon[i] = oldit[in].weapon[i];

            newit[in].light[i] = oldit[in].light[i];
        }

        newit[in].duration = oldit[in].duration;
        newit[in].cost = oldit[in].cost;
        newit[in].power = oldit[in].power;
        newit[in].active = oldit[in].active;

        newit[in].instance_id = -1;  // New
        newit[in].x = oldit[in].x;
        newit[in].y = oldit[in].y;
        newit[in].carried = oldit[in].carried;
        newit[in].sprite_override = oldit[in].sprite_override;

        for (int i=0; i<2; i++) {
            newit[in].sprite[i] = oldit[in].sprite[i];
            newit[in].status[i] = oldit[in].status[i];

            newit[in].gethit_dam[i] = oldit[in].gethit_dam[i];
        }

        newit[in].min_rank = oldit[in].min_rank;

        for (int i=0; i<3; i++) {
            newit[in].future[i] = oldit[in].future[i];
        }

        for (int i=0; i<9; i++) {
            newit[in].future3[i] = oldit[in].future3[i];
        }

        newit[in].t_bought = oldit[in].t_bought;
        newit[in].t_sold = oldit[in].t_sold;

        newit[in].driver = oldit[in].driver;
        for (int i=0; i<10; i++) {
            newit[in].data[i] = oldit[in].data[i];
        }
    }

    printf("Converting titems...\n");
    for (in=0; in<MAXTITEM; in++) {
        newit_temp[in].used = oldit_temp[in].used;

        for (int i=0; i<40; i++) {
            newit_temp[in].name[i] = oldit_temp[in].name[i];
            newit_temp[in].reference[i] = oldit_temp[in].reference[i];
        }

        for (int i=0; i<200; i++) {
            newit_temp[in].description[i] = oldit_temp[in].description[i];
        }

        newit_temp[in].flags = oldit_temp[in].flags;
        
        newit_temp[in].value = oldit_temp[in].value;
        newit_temp[in].placement = oldit_temp[in].placement;

        newit_temp[in].temp = oldit_temp[in].temp;

        newit_temp[in].damage_state = oldit_temp[in].damage_state;

        for (int i=0; i<2; i++) {
            newit_temp[in].max_age[i] = oldit_temp[in].max_age[i];
            newit_temp[in].current_age[i] = oldit_temp[in].current_age[i];
        }

        newit_temp[in].max_damage = oldit_temp[in].max_damage;
        newit_temp[in].current_damage = oldit_temp[in].current_damage;

        for (int i=0; i<5; i++) {
            for (int j=0; j<3; j++) {
                newit_temp[in].attrib[i][j] = oldit_temp[in].attrib[i][j];
            }
        }

        for (int i=0; i<3; i++) {
            newit_temp[in].hp[i] = oldit_temp[in].hp[i];
            newit_temp[in].end[i] = oldit_temp[in].end[i];
            newit_temp[in].mana[i] = oldit_temp[in].mana[i];
        }

        for (int i=0; i<50; i++) {
            for (int j=0; j<3; j++) {
                newit_temp[in].skill[i][j] = oldit_temp[in].skill[i][j];
            }
        }

        for (int i=0; i<2; i++) {
            newit_temp[in].armor[i] = oldit_temp[in].armor[i];
            newit_temp[in].weapon[i] = oldit_temp[in].weapon[i];

            newit_temp[in].light[i] = oldit_temp[in].light[i];
        }

        newit_temp[in].duration = oldit_temp[in].duration;
        newit_temp[in].cost = oldit_temp[in].cost;
        newit_temp[in].power = oldit_temp[in].power;
        newit_temp[in].active = oldit_temp[in].active;

        newit_temp[in].instance_id = -1;  // New
        newit_temp[in].x = oldit_temp[in].x;
        newit_temp[in].y = oldit_temp[in].y;
        newit_temp[in].carried = oldit_temp[in].carried;
        newit_temp[in].sprite_override = oldit_temp[in].sprite_override;

        for (int i=0; i<2; i++) {
            newit_temp[in].sprite[i] = oldit_temp[in].sprite[i];
            newit_temp[in].status[i] = oldit_temp[in].status[i];

            newit_temp[in].gethit_dam[i] = oldit_temp[in].gethit_dam[i];
        }

        newit_temp[in].min_rank = oldit_temp[in].min_rank;

        for (int i=0; i<3; i++) {
            newit_temp[in].future[i] = oldit_temp[in].future[i];
        }

        for (int i=0; i<9; i++) {
            newit_temp[in].future3[i] = oldit_temp[in].future3[i];
        }

        newit_temp[in].t_bought = oldit_temp[in].t_bought;
        newit_temp[in].t_sold = oldit_temp[in].t_sold;

        newit_temp[in].driver = oldit_temp[in].driver;
        for (int i=0; i<10; i++) {
            newit_temp[in].data[i] = oldit_temp[in].data[i];
        }
    }
    //conversion end

	printf("Conversion done.\n");
	for (int n=0; n<4; n++) close(handle[n]);
	
	printf("Unloading data files...\n");
	if (munmap(oldit,OLDITEMSIZE)) printf("ERROR: munmap(oldit) %s\n",strerror(errno));
	if (munmap(newit,ITEMSIZE)) printf("ERROR: munmap(newit) %s\n",strerror(errno));
	if (munmap(oldit_temp,OLDTITEMSIZE)) printf("ERROR: munmap(oldit_temp) %s\n",strerror(errno));
	if (munmap(newit_temp,TITEMSIZE)) printf("ERROR: munmap(newit_temp) %s\n",strerror(errno));

    return 0;
}