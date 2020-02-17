/*************************************************************************

This file is part of 'Mercenaries of Astonia v2'
Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
All rights reserved.

**************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "server.h"

int diffi=21;

int build_item(int nr,int x,int y,int inst_id,short silent)
{
	int in,n,m;
	int m_wid, m_hei;

	if (inst_id == -1) {
		m_wid = MAPX; m_hei = MAPY;
	} else {
		m_wid = map_instances[inst_id].width;
		m_hei = map_instances[inst_id].height;
	}

	if (x<0 || x>=m_wid || y<0 || y>=m_hei) return 0;

	if (inst_id == -1) {
		if (map[x+y*m_wid].it) return 0;
		if (map[x+y*m_wid].fsprite) map[x+y*m_wid].fsprite=0;
	} else {
		if (map_instancedtiles[inst_id][x+y*m_wid].it) return 0;
		if (map_instancedtiles[inst_id][x+y*m_wid].fsprite) map_instancedtiles[inst_id][x+y*m_wid].fsprite=0;
	}

	if (!silent) xlog("build: add item %d at %d,%d, instance %d",nr,x,y,inst_id);

	if (it_temp[nr].flags&(IF_TAKE|IF_LOOK|IF_LOOKSPECIAL|IF_USE|IF_USESPECIAL)) {
		in=god_create_item(nr);

		if (it[in].driver==33) {
			for (n=1,m=0; n<MAXITEM; n++) {
				if (it[n].used==USE_EMPTY) continue;
				if (it[n].driver==33) m=max(it[n].data[0],m);
			}

			m++;
			if (!silent) {
				if (inst_id == -1) do_area_log(0,0,x,y,1,"Respawn point %d, Level %d.\n",m,diffi);
				else do_area_log_inst(inst_id,0,0,x,y,1,"Respawn point %d, Level %d.\n",m,diffi);
			}
			it[in].data[0]=m;
			it[in].data[9]=diffi;	// difficulty level
			it[in].temp=0; // make sure it isnt reset when doing #reset
		}

		if (inst_id == -1) map[x+y*m_wid].it=in;
		else map_instancedtiles[inst_id][x+y*m_wid].it=in;

		it[in].instance_id=inst_id;
		it[in].x=(short)x;
		it[in].y=(short)y;
		it[in].carried=0;		
	} else {
		if (inst_id == -1) {
			map[x+y*m_wid].fsprite=it_temp[nr].sprite[0];
			if (it_temp[nr].flags&IF_MOVEBLOCK) map[x+y*m_wid].flags|=MF_MOVEBLOCK;
			if (it_temp[nr].flags&IF_SIGHTBLOCK) map[x+y*m_wid].flags|=MF_SIGHTBLOCK;
			return 0;
		} else {
			map_instancedtiles[inst_id][x+y*m_wid].fsprite=it_temp[nr].sprite[0];
			if (it_temp[nr].flags&IF_MOVEBLOCK) map_instancedtiles[inst_id][x+y*m_wid].flags|=MF_MOVEBLOCK;
			if (it_temp[nr].flags&IF_SIGHTBLOCK) map_instancedtiles[inst_id][x+y*m_wid].flags|=MF_SIGHTBLOCK;
			return 0;
		}
	}

	return in;
}

void build_drop(int x,int y,int in,int inst_id,short silent)
{
	int nr;
	int m_wid, m_hei;

	if (inst_id == -1) {
		m_wid = MAPX; m_hei = MAPY;
	} else {
		m_wid = map_instances[inst_id].width;
		m_hei = map_instances[inst_id].height;
	}

	if (x<0 || x>=m_wid || y<0 || y>=m_hei) return;

	if (in&0x40000000) {
		if (inst_id == -1) map[x+y*m_wid].flags^=in&0xfffffff;
		else map_instancedtiles[inst_id][x+y*m_wid].flags^=in&0xfffffff;
		return;
	}
	if (in&0x20000000) {
		nr=in&0xfffffff;
		if (nr==1003) { // hack for jungle ground
			nr+=RANDOM(4);
			if (nr>1003) nr++;
		}
		if (nr==542) { // hack for street ground
			nr+=RANDOM(9);
		}
		if (nr==551) { // hack for grass ground
			nr+=RANDOM(4);
		}
		if (nr==133) { // hack for parkett ground
			nr+=RANDOM(9);
		}
		if (nr==142) { // hack for parkett ground
			nr+=RANDOM(4);
		}
		if (nr==170) { // hack for stone ground
			nr+=RANDOM(6);
		}
		if (nr==704) {
			static int tab[4]={704,659,660,688};
			nr=tab[RANDOM(4)];
		}
		if (nr==950) {
			nr+=RANDOM(9);
		}
		if (nr==959) {
			nr+=(x%3)*3+(y%3);
		}
		if (nr==16670) nr+=RANDOM(4);
		if (nr==16933) {
			static int tab[4]={16933,16957,16958,16959};
			nr=tab[RANDOM(4)];
		}

		if (inst_id == -1) map[x+y*m_wid].sprite=nr;
		else map_instancedtiles[inst_id][x+y*m_wid].sprite=nr;
		return;
	}

	nr=build_item(in,x,y,inst_id,silent);
	if (!nr) return;

	if (it[nr].active) {
		if (it[nr].light[1])
			do_add_light(x,y,it[nr].light[1],inst_id);
	} else {
		if (it[nr].light[0])
			do_add_light(x,y,it[nr].light[0],inst_id);
	}
}


void build_remove(int x,int y,int inst_id)
{
	int nr;
	int m_wid, m_hei;

	if (inst_id == -1) {
		m_wid = MAPX; m_hei = MAPY;
	} else {
		m_wid = map_instances[inst_id].width;
		m_hei = map_instances[inst_id].height;
	}

	if (x<0 || x>=m_wid || y<0 || y>=m_hei) return;

	if (inst_id == -1) {
		map[x+y*m_wid].fsprite=0;
		map[x+y*m_wid].flags&=~(MF_MOVEBLOCK|MF_SIGHTBLOCK);

		nr=map[x+y*m_wid].it;
	} else {
		map_instancedtiles[inst_id][x+y*m_wid].fsprite=0;
		map_instancedtiles[inst_id][x+y*m_wid].flags&=~(MF_MOVEBLOCK|MF_SIGHTBLOCK);

		nr=map_instancedtiles[inst_id][x+y*m_wid].it;
	}

	if (!nr) return;

	if (it[nr].active) {
		if (it[nr].light[1])
			do_add_light(x,y,-it[nr].light[1],inst_id);
	} else {
		if (it[nr].light[0])
			do_add_light(x,y,-it[nr].light[0],inst_id);
	}

	it[nr].used=0;
	if (inst_id == -1) map[x+y*m_wid].it=0;
	else map_instancedtiles[inst_id][x+y*m_wid].it=0;

	xlog("build: remove item from %d,%d",x,y);
}
