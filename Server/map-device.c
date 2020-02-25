#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#include "server.h"
#include "map-device.h"

struct areamap_device *amap_devices;
static int loaded_mapdevices = 0;

// Loads/creates a map device for the given character; will get saved to .dat/mapdevices/<cn>.dat
// Returns new mapdevice ID, or -1 if it fails
int load_mapdevice(int cn)
{
    int mapd_id = 0, handle;
    struct areamap_device *tmp_ptr;

    if (ch[cn].used == USE_EMPTY) return -1;

    // Try to find a free spot from existing devices
    if (loaded_mapdevices > 0) {
        for (mapd_id = 0; mapd_id < loaded_mapdevices; mapd_id++) {
            if (amap_devices[mapd_id].used == USE_EMPTY) break;
        }
    }

    if (mapd_id >= loaded_mapdevices) {
        tmp_ptr = realloc(amap_devices, sizeof(struct areamap_device) * (loaded_mapdevices + 1));
        if (tmp_ptr == NULL) {
            xlog("Could not reallocate memory for a new map device.");
            return -1;
        }
        amap_devices = tmp_ptr;
        loaded_mapdevices++;
    }

    // Load/create mapdevice file
    char fpath[60];
    sprintf(fpath, DATDIR"/mapdevices/%d.dat", cn);

    handle=open(fpath, O_RDWR);
    if (handle == -1) {
        xlog("Creating new map device file for %s (%d).", ch[cn].name, cn);
        handle=open(fpath,O_RDWR|O_CREAT,0755);

        if (!extend(handle, sizeof(struct areamap_device), sizeof(struct areamap_device), NULL)) {
            xlog("ERROR - could not map mapdevice data into file.");
            return -1;
        };
        close(handle);

        amap_devices[mapd_id].level = 1;
        amap_devices[mapd_id].experience = 0;

        for (int i=0; i<AMAP_MAXBASES; i++) {
            amap_devices[mapd_id].map_charges[i] = 0;
        }
        
        for (int i=0; i<AMAP_MAXBASES; i++) {
            for (int j=0; j<AMAP_MAXMODS; j++) {
                amap_devices[mapd_id].map_mods[i][j] = 0;
            }
        }
    } else {
        // Load from file
        struct areamap_device file_mapdvc;
        if (read(handle, &file_mapdvc, sizeof(struct areamap_device)) == -1) {
            xlog("ERROR - read returned -1 while reading from mapdevice file for %s (%d).", ch[cn].name, cn);
            return -1;
        }
        amap_devices[mapd_id].level = file_mapdvc.level;
        amap_devices[mapd_id].experience = file_mapdvc.experience;

        for (int i=0; i<AMAP_MAXBASES; i++) {
            amap_devices[mapd_id].map_charges[i] = file_mapdvc.map_charges[i];
        }

        for (int i=0; i<AMAP_MAXBASES; i++) {
            for (int j=0; j<AMAP_MAXMODS; j++) {
                amap_devices[mapd_id].map_mods[i][j] = file_mapdvc.map_mods[i][j];
            }
        }

        close(handle);
    }

    amap_devices[mapd_id].ch = cn;
    amap_devices[mapd_id].used = USE_ACTIVE;

    xlog("Loaded map device for %s (%d). loaded_mapdevices at %d.", ch[cn].name, cn, loaded_mapdevices);

    return mapd_id;
}

// Returns the ID of the mapdevice owned by the given character, or -1 if not found
int get_mapdevice(int cn)
{
    if (ch[cn].used == USE_EMPTY) return -1;
    if (!(ch[cn].flags&CF_PLAYER) || ch[cn].flags&CF_USURP) return -1;

    for (int i=0; i<loaded_mapdevices; i++) {
        if (amap_devices[i].ch == cn) return i;
    }
    return -1;
}

// Saves the mapdevice state of the given character to its respective file
void save_mapdevice(int cn)
{
    int mapd = get_mapdevice(cn);
    if (mapd == -1) return;

    int handle;
    char fpath[60];

    sprintf(fpath, DATDIR"/mapdevices/%d.dat", cn);

    handle=open(fpath,O_WRONLY|O_TRUNC);
    if (handle==-1) {
        xlog("Could not open mapdevice file when saving for %s (%d). Creating new one...", ch[cn].name, cn);
        handle=open(fpath,O_WRONLY|O_CREAT);
    }

    write(handle, &amap_devices[mapd], sizeof(struct areamap_device));
    close(handle);
}

void unload_mapdevice(int cn)
{
    int mapd = get_mapdevice(cn);
    if (mapd != -1) {
        amap_devices[mapd].used = USE_EMPTY;
    }
}

void delete_mapdevice(int cn)
{
    int mapd = get_mapdevice(cn);
    if (mapd != -1) {
        unload_mapdevice(cn);

        char fpath[60];
        sprintf(fpath, DATDIR"/mapdevices/%d.dat", cn);
        remove(fpath);
    }
}

long mapdev_expreq(int mapd)
{
    if (mapd < 0 || mapd >= loaded_mapdevices) return 200;
    return 200 + (amap_devices[mapd].level - 1) * (200 * (amap_devices[mapd].level / 2));
}

void mapdev_addexp(int cn, int xp)
{
    int mapd = get_mapdevice(cn);
    if (mapd == -1) return;

    int xpreq = mapdev_expreq(cn);

    amap_devices[mapd].experience += xp;
    if (amap_devices[mapd].experience >= xpreq) {
        int leftover = amap_devices[mapd].experience - xpreq;
        amap_devices[mapd].experience = 0;
        amap_devices[mapd].level++;
        if (leftover < mapdev_expreq(mapd)) {
            do_char_log(cn, 1, "/|%d|Your map device becomes more attuned to reality. It is now level %d.", FNT_ORANGE, amap_devices[mapd].level);
        }
        mapdev_addexp(cn, leftover);
    }
    send_mapdev_data(ch[cn].player);
}

// Sends an areamap base to the given player
void send_amapbase(int nr, int amap)
{
    if (!player[nr].sock) return;
    if (amap < 0 || amap >= AMAP_MAXBASES) return;
    if (amap_bases[amap].used == USE_EMPTY) return;

    unsigned char buf[16];

    buf[0]=SV_AMAPBASE;
    *(unsigned short*)(buf+1) = amap_bases[amap].quadrant + amap_bases[amap].tier * 4;
    *(unsigned char*)(buf+3) = amap_bases[amap].conn_flags;
    *(unsigned int*)(buf+4) = amap_bases[amap].sprite;
    xsend(nr,buf,8);
}

// Sends all active areamap bases to the given player
void send_amapbase_all(int nr)
{
    if (!player[nr].sock) return;

    for (int i=0; i<AMAP_MAXBASES; i++) {
        send_amapbase(nr, i);
    }
}

// Sends areamap charges to the given player
void send_amapcharges(int nr, int amap)
{
    if (!player[nr].sock) return;
    if (amap < 0 || amap >= AMAP_MAXBASES) return;
    if (amap_bases[amap].used == USE_EMPTY) return;
    
    int cn = player[nr].usnr;
    int amap_dev = get_mapdevice(cn);
    unsigned char buf[16];

    if (amap_dev == -1) return;

    buf[0]=SV_AMAPCHG;
    *(unsigned short*)(buf+1) = amap_bases[amap].quadrant + amap_bases[amap].tier * 4;
    *(unsigned char*)(buf+3) = amap_devices[amap_dev].map_charges[amap];
    xsend(nr,buf,4);
}

void send_amapcharges_all(int nr)
{
    if (!player[nr].sock) return;

    for (int i=0; i<AMAP_MAXBASES; i++) {
        send_amapcharges(nr, i);
    }
}

// Sends mapdevice data
void send_mapdev_data(int nr)
{
    if (!player[nr].sock) return;

    int cn = player[nr].usnr;
    int amap_dev = get_mapdevice(cn);
    unsigned char buf[16];

    if (amap_dev == -1) return;

    buf[0] = SV_MAPDEVDATA;
    *(unsigned short*)(buf+1) = amap_devices[amap_dev].level;
    *(unsigned long*)(buf+3) = amap_devices[amap_dev].experience;
    xsend(nr,buf,11);
}

// Send data about selected map
void send_selmap_data(int nr, int sel_pos)
{
    if (!player[nr].sock) return;

    int amap;
    for (amap=0; amap<AMAP_MAXBASES; amap++) {
        if (amap_bases[amap].used == USE_EMPTY) continue;
        if (amap_bases[amap].quadrant == sel_pos%4 && amap_bases[amap].tier == floor(sel_pos/4)) break;
    }
    if (amap >= AMAP_MAXBASES) return;

    int cn = player[nr].usnr;
    int amap_dev = get_mapdevice(cn);
    unsigned char buf[16];

    if (amap_dev == -1) return;

    // Send name
    buf[0] = SV_SELMAPDATA;
    *(unsigned char*)(buf+1)=0;
    for (int n=0; n<14; n++) buf[n+2]=amap_bases[amap].name[n];
    xsend(nr,buf,16);

    if (strlen(amap_bases[amap].name) > 14) {
        buf[0] = SV_SELMAPDATA;
        *(unsigned char*)(buf+1)=1;
        for (int n=0; n<14; n++) buf[n+2]=amap_bases[amap].name[n+14];
        xsend(nr,buf,16);
    }

    if (strlen(amap_bases[amap].name) > 28) {
        buf[0] = SV_SELMAPDATA;
        *(unsigned char*)(buf+1)=2;
        for (int n=0; n<12; n++) buf[n+2]=amap_bases[amap].name[n+28];
        xsend(nr,buf,14);
    }
}