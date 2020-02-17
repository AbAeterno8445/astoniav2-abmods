/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "server.h"
#include "driver.h"

#define fx _fx

struct node
{
        int x,y,dir;            // coordinates of current node and dir we originally came from
        int tcost,cost;         // total (guessed) cost, cost of steps so far
        int visited;
        int cdir;               // current direction

        struct node *prev,*next;
};

struct node **nmap=NULL;
struct node *ngo=NULL;

struct node *nodes=NULL;
static int maxnode=0;

#define MAXNODE 4096

struct badtarget
{
	unsigned int tick;
};

struct badtarget *bad;

static int ccn,step;
static unsigned long mapblock;
static int tx1,ty1,tx2,ty2,mode,maxstep;
static int failed=0;

static int xcost(int fx,int fy,int tx,int ty)
{
        int dx,dy;

        dx=abs(fx-tx);
        dy=abs(fy-ty);

        if (dx>dy) return (dx<<1)+dy;
        else return (dy<<1)+dx;
}

static int cost(int fx,int fy,int cdir)
{
        if (mode==0 || mode==1) return xcost(fx,fy,tx1,ty1);
        else {
                int ndir,dirdiff1,dirdiff2;

                ndir=drv_dcoor2dir(tx1-fx,ty1-fy);
                dirdiff1=drv_turncount(cdir,ndir);

                ndir=drv_dcoor2dir(tx2-fx,ty2-fy);
                dirdiff2=drv_turncount(cdir,ndir);

                return min(xcost(fx,fy,tx1,ty1)+12+dirdiff1,xcost(fx,fy,tx2,ty2)+dirdiff2);
        }
}

int init_node(void)
{
        nmap=calloc(MAPX*MAPY,sizeof(struct node *));
        if (!nmap) return 0;

        nodes=malloc(sizeof(struct node)*MAXNODE);
        if (!nodes) return 0;
        
        bad=calloc(MAPX*MAPY,sizeof(struct badtarget));

        return 1;
}

int add_node(int x,int y,int dir,int ccost,int cdir,int inst_id)
{
        int m,tcost,gcost;
        int m_wid, m_hei;
        struct node *node,*tmp,*prev,*next;
        
        if (inst_id == -1) {
                m_wid = MAPX; m_hei = MAPY;
        } else {
                m_wid = map_instances[inst_id].width;
                m_hei = map_instances[inst_id].height;
        }

        if (x<1 || x>=m_wid || y<1 || y>=m_hei) return 0;

        m=x+y*m_wid;
        gcost=cost(x,y,cdir);
        tcost=ccost+gcost;

        if ((tmp=nmap[m])!=NULL) {
                if (tmp->tcost<=tcost) return 0;        // other node is better or equal, no need to try this one

                if (!tmp->visited) {
                        prev=tmp->prev;
                        next=tmp->next;

                        if (prev) prev->next=next;
                        else ngo=next;

                        if (next) next->prev=prev;
                }
                node=tmp;
        } else {
                if (maxnode>=maxstep) { failed=1; return 0; }
                node=&nodes[maxnode++];
                node->x=x;
                node->y=y;
        }

        node->cost=ccost;
        node->tcost=tcost;
        node->dir=dir;
        node->visited=0;
        node->cdir=cdir;

        nmap[m]=node;

        // find correct position in sorted list
        for (tmp=ngo,prev=NULL; tmp; tmp=tmp->next) {
                if (tmp->tcost>=node->tcost) break;
                prev=tmp;
        }

        // previous node -> next
        if (prev) prev->next=node;
        else ngo=node;

        // current node -> prev
        node->prev=prev;

        // next node -> prev
        if (tmp) tmp->prev=node;

        // current node -> next
        node->next=tmp;

        return 1;
}

static inline int dr_check_target(int m,int inst_id)
{
        int in;

        if (inst_id == -1) {
                if (((unsigned long)map[m].flags&mapblock) || map[m].ch || map[m].to_ch ||
                        ((in=map[m].it) && (it[in].flags&IF_MOVEBLOCK) && it[in].driver!=2)) return 0;
        } else {
                if (((unsigned long)map_instancedtiles[inst_id][m].flags&mapblock) || map_instancedtiles[inst_id][m].ch || map_instancedtiles[inst_id][m].to_ch ||
                        ((in=map_instancedtiles[inst_id][m].it) && (it[in].flags&IF_MOVEBLOCK) && it[in].driver!=2)) return 0;
        }

        return 1;
}

/*static int rem_node(struct node *node)
{
        struct node *prev,*next;

        prev=node->prev;
        next=node->next;

        if (prev) prev->next=next;
        else ngo=next;
        if (next) next->prev=prev;

        return 1;
}*/

static void add_suc(struct node *node, int inst_id)
{
        int m_wid;
        if (inst_id == -1) m_wid = MAPX;
        else m_wid = map_instances[inst_id].width;

        if (!node->dir) {
                if (dr_check_target(node->x+node->y*m_wid+1,inst_id)) add_node(node->x+1,node->y,DX_RIGHT,node->cost+2+drv_turncount(node->cdir,DX_RIGHT),DX_RIGHT,inst_id);
                if (dr_check_target(node->x+node->y*m_wid-1,inst_id)) add_node(node->x-1,node->y,DX_LEFT,node->cost+2+drv_turncount(node->cdir,DX_LEFT),DX_LEFT,inst_id);
                if (dr_check_target(node->x+node->y*m_wid+m_wid,inst_id)) add_node(node->x,node->y+1,DX_DOWN,node->cost+2+drv_turncount(node->cdir,DX_DOWN),DX_DOWN,inst_id);
                if (dr_check_target(node->x+node->y*m_wid-m_wid,inst_id)) add_node(node->x,node->y-1,DX_UP,node->cost+2+drv_turncount(node->cdir,DX_UP),DX_UP,inst_id);

                if (dr_check_target(node->x+node->y*m_wid+1+m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+m_wid,inst_id))
                        add_node(node->x+1,node->y+1,DX_RIGHTDOWN,node->cost+3+drv_turncount(node->cdir,DX_RIGHTDOWN),DX_RIGHTDOWN,inst_id);

                if (dr_check_target(node->x+node->y*m_wid+1-m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-m_wid,inst_id))
                        add_node(node->x+1,node->y-1,DX_RIGHTUP,node->cost+3+drv_turncount(node->cdir,DX_RIGHTUP),DX_RIGHTUP,inst_id);

                if (dr_check_target(node->x+node->y*m_wid-1+m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+m_wid,inst_id))
                        add_node(node->x-1,node->y+1,DX_LEFTDOWN,node->cost+3+drv_turncount(node->cdir,DX_LEFTDOWN),DX_LEFTDOWN,inst_id);

                if (dr_check_target(node->x+node->y*m_wid-1-m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-m_wid,inst_id))
                        add_node(node->x-1,node->y-1,DX_LEFTUP,node->cost+3+drv_turncount(node->cdir,DX_LEFTUP),DX_LEFTUP,inst_id);
        } else {
                if (dr_check_target(node->x+node->y*m_wid+1,inst_id)) add_node(node->x+1,node->y,node->dir,node->cost+2+drv_turncount(node->cdir,DX_RIGHT),DX_RIGHT,inst_id);
                if (dr_check_target(node->x+node->y*m_wid-1,inst_id)) add_node(node->x-1,node->y,node->dir,node->cost+2+drv_turncount(node->cdir,DX_LEFT),DX_LEFT,inst_id);
                if (dr_check_target(node->x+node->y*m_wid+m_wid,inst_id)) add_node(node->x,node->y+1,node->dir,node->cost+2+drv_turncount(node->cdir,DX_DOWN),DX_DOWN,inst_id);
                if (dr_check_target(node->x+node->y*m_wid-m_wid,inst_id)) add_node(node->x,node->y-1,node->dir,node->cost+2+drv_turncount(node->cdir,DX_UP),DX_UP,inst_id);

                if (dr_check_target(node->x+node->y*m_wid+1+m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+m_wid,inst_id))
                        add_node(node->x+1,node->y+1,node->dir,node->cost+3+drv_turncount(node->cdir,DX_RIGHTDOWN),DX_RIGHTDOWN,inst_id);

                if (dr_check_target(node->x+node->y*m_wid+1-m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-m_wid,inst_id))
                        add_node(node->x+1,node->y-1,node->dir,node->cost+3+drv_turncount(node->cdir,DX_RIGHTUP),DX_RIGHTUP,inst_id);

                if (dr_check_target(node->x+node->y*m_wid-1+m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid+m_wid,inst_id))
                        add_node(node->x-1,node->y+1,node->dir,node->cost+3+drv_turncount(node->cdir,DX_LEFTDOWN),DX_LEFTDOWN,inst_id);

                if (dr_check_target(node->x+node->y*m_wid-1-m_wid,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-1,inst_id) &&
                    dr_check_target(node->x+node->y*m_wid-m_wid,inst_id))
                        add_node(node->x-1,node->y-1,node->dir,node->cost+3+drv_turncount(node->cdir,DX_LEFTUP),DX_LEFTUP,inst_id);
        }
}

int astar(int fx,int fy,int cdir,int inst_id)
{
        struct node *node;

        node=&nodes[maxnode++];
        node->x=fx;
        node->y=fy;
        node->dir=0;
        node->cost=0;
        node->tcost=node->cost+cost(fx,fy,cdir);
        node->cdir=cdir;

        ngo=node;
        if (inst_id == -1) nmap[fx + fy * MAPX]=node;
        else nmap[fx + fy * map_instances[inst_id].width]=node;

        while (ngo && !failed) {

                // get first node in go list
                node=ngo;

                // remove node from go list
                ngo=ngo->next;
                if (ngo) ngo->prev=NULL;

                node->visited=1; step++;

                if (mode==0 && node->x==tx1 && node->y==ty1) return node->dir;
                if (mode!=0 && abs(node->x-tx1)+abs(node->y-ty1)==1) return node->dir;
                if (mode==2 && abs(node->x-tx2)+abs(node->y-ty2)==1) return node->dir;
                add_suc(node,inst_id);
        }

        return -1;
}

int is_bad_target(int x,int y,int inst_id)
{
        if (inst_id == -1) return bad[x + y * MAPX].tick>globs->ticker;
        return bad[x + y * map_instances[inst_id].width].tick>globs->ticker;
}

void add_bad_target(int x,int y,int inst_id)
{
	if (inst_id == -1) bad[x + y * MAPX].tick=globs->ticker+1;
        else bad[x + y * map_instances[inst_id].width].tick=globs->ticker+1;
}

int pathfinder(int cn,int x1,int y1,int flag,int x2,int y2,int inst_id)
{
        int tmp,n;
        int m_wid, m_hei;

        if (inst_id == -1) {
                m_wid = MAPX; m_hei = MAPY;
        } else {
                m_wid = map_instances[inst_id].width;
                m_hei = map_instances[inst_id].height;
        }

        if (ch[cn].x<1 || ch[cn].x>=m_wid) return -1;
        if (ch[cn].y<1 || ch[cn].y>=m_hei) return -1;
        if (x1<1 || x1>=m_wid) return -1;
        if (y1<1 || y1>=m_hei) return -1;
        if (x2<0 || x2>=m_wid) return -1;
        if (y2<0 || y2>=m_hei) return -1;
        
        if (is_bad_target(x1,y1,inst_id)) return -1;

        ccn=cn;
        if ((ch[cn].kindred&KIN_MONSTER) && !(ch[cn].flags&(CF_USURP|CF_THRALL))) mapblock=MF_NOMONST|MF_MOVEBLOCK; else mapblock=MF_MOVEBLOCK;
        if (!(ch[cn].flags&(CF_PLAYER|CF_USURP))) mapblock|=MF_DEATHTRAP;

        step=0;
        tx1=x1; tx2=x2;
        ty1=y1; ty2=y2;
        mode=flag;

        if (flag==0 && !dr_check_target(tx1+ty1*m_wid,inst_id)) return -1;

        if (ch[cn].attack_cn || (!(ch[cn].flags&(CF_PLAYER|CF_USURP)) && ch[cn].data[78])) maxstep=max(abs(ch[cn].x-x1),abs(ch[cn].y-y1))*4+50;
        else maxstep=max(abs(ch[cn].x-x1),abs(ch[cn].y-y1))*8+100;

        if (ch[cn].temp==498) maxstep+=4000;    // hack for grolmy (stunrun.c)

        if (maxstep>MAXNODE) maxstep=MAXNODE;

        tmp=astar(ch[cn].x,ch[cn].y,ch[cn].dir,inst_id);

      	//if (cn==20) do_sayx(cn,"astar=%d, steps=%d, nodes=%d (%d)",tmp,step,maxnode,maxstep);

	//if (step>100) do_sayx(20,"%s (%d) astar=%d, steps=%d, nodes=%d (%d)",ch[cn].name,cn,tmp,step,maxnode,maxstep);

        for (n=0; n<maxnode; n++) {
                nmap[nodes[n].x+nodes[n].y*m_wid]=NULL;
        }

        ngo=NULL;
        maxnode=0;
        failed=0;
        
        if (tmp==-1) add_bad_target(x1,y1,inst_id);

        return tmp;
}
