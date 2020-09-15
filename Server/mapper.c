#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "cgi-lib.h"
#include "html-lib.h"
#include "gendefs.h"
#include "data.h"

struct map *map;
struct item *it;
struct item *it_temp;
struct mapedit_queue *maped_queue;

static int load(void)
{
        int handle;
        char cwd[128];

        // Load map
        handle=open("../"DATDIR"/map.dat",O_RDWR);
        if (handle==-1) {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                fprintf(stderr, "cwd: %s\n", cwd);
            perror("../"DATDIR"/map.dat");
            return -1;
        }
        map=mmap(NULL,MAPSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (map==(void*)-1) {
            fprintf(stderr,"cannot mmap map.dat.\n");
            return -1;
        }
        close(handle);

        // Load map editor queue
        handle=open("../"DATDIR"/mapedQ.dat",O_RDWR);
        if (handle==-1) {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                fprintf(stderr, "cwd: %s\n", cwd);
            perror("../"DATDIR"/mapedQ.dat");
            return -1;
        }
        maped_queue=mmap(NULL,MAPED_QUEUE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (maped_queue==(void*)-1) {
            fprintf(stderr,"cannot mmap mapedQ.dat.\n");
            return -1;
        }
        close(handle);

        // Load item templates
        handle=open("../"DATDIR"/titem.dat",O_RDWR);
        if (handle==-1) {
                fprintf(stderr,"titem.dat does not exist.\n");
                return -1;
        }

        it_temp=mmap(NULL,TITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it_temp==(void*)-1) {
                fprintf(stderr,"cannot mmap titem.dat.\n");
                return -1;
        }
        close(handle);

        // Load items
        handle=open("../"DATDIR"/item.dat",O_RDWR);
        if (handle==-1) {
                fprintf(stderr,"item.dat does not exist.\n");
                return -1;
        }

        it=mmap(NULL,ITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it==(void*)-1) {
                fprintf(stderr,"cannot mmap item.dat.\n");
                return -1;
        }
        close(handle);

        return 0;
}

static void unload(void)
{
    if (munmap(map,MAPSIZE)) perror("munmap(map)");
    if (munmap(maped_queue,MAPED_QUEUE_SIZE)) perror("munmap(maped_queue)");
    if (munmap(it,ITEMSIZE)) perror("munmap(it)");
    if (munmap(it_temp,TITEMSIZE)) perror("munmap(it_temp)");
}

void cssSetup()
{
    printf("<style>");
    printf("body { background-color: darkslategray; }");
    printf("body, html { height: 100%%; overflow: hidden; }");
    printf(".cv-base {");
    printf("overflow: auto;");
    printf("border: 1px white solid;");
    printf("background-color: black;");
    printf("flex: 1;");
    printf("min-height: 0;");
    printf("min-width: 0;");
    printf("margin: 4px;");
    printf("}");
    printf("#div-flexmain { display: flex; height: 99%%; }");
    printf("#div-palette {");
    printf("border: 1px white solid;");
    printf("margin: 4px;");
    printf("width: 300px;");
    printf("overflow: auto;");
    printf("}");
    printf("#div-mapload {");
    printf("background-color: rgb(40, 40, 40);");
    printf("border: 1px white solid;");
    printf("margin: 2px;");
    printf("color: wheat;");
    printf("font-size: 10pt;");
    printf("}");
    printf("#button-mapload {");
    printf("float: right;");
    printf("margin-top: 5px;");
    printf("}");
    printf("#div-toolbox {");
    printf("height: 40px;");
    printf("border: 1px white solid;");
    printf("margin: 2px;");
    printf("display: flex;");
    printf("align-items: center;");
    printf("background-color: rgb(40, 40, 40);");
    printf("}");
    printf(".toolbox-button {");
    printf("width: 32px;");
    printf("height: 32px;");
    printf("border: 1px white solid;");
    printf("background-color: black;");
    printf("background-size: auto; /**NEW*/");
    printf("background-position: center; /**NEW*/");
    printf("margin: 2px;");
    printf("}");
    printf(".toolbox-button:hover { /**NEW*/");
    printf("border: 1px rgb(0, 153, 255) solid;");
    printf("background-color: gray;");
    printf("}");
    printf("#div-filter, #div-temp-picker {");
    printf("background-color: rgb(40, 40, 40);");
    printf("border: 1px white solid;");
    printf("margin: 0px 2px 2px 2px;");
    printf("color: wheat;");
    printf("font-size: 10pt;");
    printf("padding: 2px;");
    printf("}");
    printf("#div-temp-picker {");
    printf("display: flex;");
    printf("align-items: center;");
    printf("flex-direction: column;");
    printf("}");
    printf("input::-webkit-outer-spin-button,\n");
    printf("input::-webkit-inner-spin-button {\n");
    printf("-webkit-appearance: none;\n");
    printf("margin: 0;\n");
    printf("}\n");
    printf("#div-palette-container {");
    printf("display: flex;");
    printf("flex-wrap: wrap;");
    printf("overflow: auto;");
    printf("justify-content: center;\n");
    printf("}");
    printf("#div-canvastable {");
    printf("display: flex;");
    printf("flex: 3;");
    printf("flex-direction: column;");
    printf("min-width: 0;");
    printf("}");
    printf("#tiles-table {");
    printf("border-spacing: 0;");
    printf("position: relative;");
    printf("margin: 4px;");
    printf("}");
    printf("td {");
    printf("margin: 0;");
    printf("padding: 0;");
    printf("}");
    printf(".tile-cell {");
    printf("border: 1px rgb(120, 120, 120) solid;");
    printf("min-width: 32px;");
    printf("height: 32px;");
    printf("}");
    printf(".tile-cell:hover {");
    printf("border: 1px rgb(0, 153, 255) solid;");
    printf("}");
    printf(".temp-cell {");
    printf("width: 64px;");
    printf("height: 64px;");
    printf("border: 1px white solid;");
    printf("margin: 2px;");
    printf("background-color: black;");
    printf("background-repeat: no-repeat;");
    printf("background-position: center;");
    printf("background-size: contain;");
    printf("}");
    printf(".temp-cell:hover {");
    printf("background-color: rgb(65, 65, 65);");
    printf("border: 1px rgb(0, 153, 255) solid;");
    printf("}");
    printf(".temp-cell-color {");
    printf("width: 8px;");
    printf("height: 8px;");
    printf("border: 1px white solid;");
    printf("position: relative;");
    printf("top: -1px;");
    printf("left: -1px;");
    printf("}");
    printf(".temp-icon {");
    printf("width: 12px;");
    printf("height: 12px;");
    printf("border: 2px orange solid;");
    printf("border-radius: 100%%;");
    printf("margin: auto;");
    printf("pointer-events: none;");
    printf("}");
    printf(".temp-typespan {");
    printf("float: right;");
    printf("color: wheat;");
    printf("font-size: 10pt;");
    printf("margin-top: -1px;");
    printf("margin-right: 2px;");
    printf("}");
    printf(".rb-border {");
    printf("border-image: linear-gradient(to bottom right, #b827fc 0%%, #2c90fc 25%%, #b8fd33 50%%, #fec837 75%%, #fd1892 100%%);");
    printf("border-image-slice: 1;");
    printf("}");
    printf(".unselectable {");
    printf("-webkit-touch-callout: none;");
    printf("-webkit-user-select: none;");
    printf("-khtml-user-select: none;");
    printf("-moz-user-select: none;");
    printf("-ms-user-select: none;");
    printf("user-select: none;");
    printf("}");
    printf("</style>");
}

void loadFloors()
{
    printf("item_templates[\"it_temp\" + 100116] = new ItemTemp(100116, 116, \"floor\", \"rgb(107, 86, 46)\");\n");
    printf("item_templates[\"it_temp\" + 100117] = new ItemTemp(100117, 117, \"floor\", \"rgb(40, 35, 36)\");\n");
    printf("item_templates[\"it_temp\" + 100118] = new ItemTemp(100118, 118, \"floor\", \"rgb(38, 15, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100130] = new ItemTemp(100130, 130, \"floor\", \"rgb(177, 155, 133)\");\n");
    printf("item_templates[\"it_temp\" + 100131] = new ItemTemp(100131, 131, \"floor\", \"rgb(177, 155, 133)\");\n");
    printf("item_templates[\"it_temp\" + 100132] = new ItemTemp(100132, 132, \"floor\", \"rgb(178, 156, 134)\");\n");
    printf("item_templates[\"it_temp\" + 100133] = new ItemTemp(100133, 133, \"floor\", \"rgb(92, 33, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100134] = new ItemTemp(100134, 134, \"floor\", \"rgb(93, 33, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100135] = new ItemTemp(100135, 135, \"floor\", \"rgb(100, 35, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100136] = new ItemTemp(100136, 136, \"floor\", \"rgb(89, 32, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100137] = new ItemTemp(100137, 137, \"floor\", \"rgb(97, 35, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100138] = new ItemTemp(100138, 138, \"floor\", \"rgb(97, 35, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100139] = new ItemTemp(100139, 139, \"floor\", \"rgb(91, 33, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100140] = new ItemTemp(100140, 140, \"floor\", \"rgb(102, 36, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100141] = new ItemTemp(100141, 141, \"floor\", \"rgb(93, 34, 14)\");\n");
    printf("item_templates[\"it_temp\" + 100142] = new ItemTemp(100142, 142, \"floor\", \"rgb(211, 200, 182)\");\n");
    printf("item_templates[\"it_temp\" + 100143] = new ItemTemp(100143, 143, \"floor\", \"rgb(209, 199, 183)\");\n");
    printf("item_templates[\"it_temp\" + 100144] = new ItemTemp(100144, 144, \"floor\", \"rgb(212, 202, 186)\");\n");
    printf("item_templates[\"it_temp\" + 100145] = new ItemTemp(100145, 145, \"floor\", \"rgb(210, 201, 186)\");\n");
    printf("item_templates[\"it_temp\" + 100170] = new ItemTemp(100170, 170, \"floor\", \"rgb(137, 129, 118)\");\n");
    printf("item_templates[\"it_temp\" + 100171] = new ItemTemp(100171, 171, \"floor\", \"rgb(140, 131, 120)\");\n");
    printf("item_templates[\"it_temp\" + 100172] = new ItemTemp(100172, 172, \"floor\", \"rgb(134, 127, 116)\");\n");
    printf("item_templates[\"it_temp\" + 100173] = new ItemTemp(100173, 173, \"floor\", \"rgb(140, 131, 120)\");\n");
    printf("item_templates[\"it_temp\" + 100174] = new ItemTemp(100174, 174, \"floor\", \"rgb(141, 132, 121)\");\n");
    printf("item_templates[\"it_temp\" + 100175] = new ItemTemp(100175, 175, \"floor\", \"rgb(136, 127, 116)\");\n");
    printf("item_templates[\"it_temp\" + 100402] = new ItemTemp(100402, 402, \"floor\", \"rgb(35, 35, 35)\");\n");
    printf("item_templates[\"it_temp\" + 100499] = new ItemTemp(100499, 499, \"floor\", \"rgb(196, 5, 196)\");\n");
    printf("item_templates[\"it_temp\" + 100500] = new ItemTemp(100500, 500, \"floor\", \"rgb(31, 31, 31)\");\n");
    printf("item_templates[\"it_temp\" + 100520] = new ItemTemp(100520, 520, \"floor\", \"rgb(237, 0, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100521] = new ItemTemp(100521, 521, \"floor\", \"rgb(233, 0, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100522] = new ItemTemp(100522, 522, \"floor\", \"rgb(234, 0, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100523] = new ItemTemp(100523, 523, \"floor\", \"rgb(149, 44, 57)\");\n");
    printf("item_templates[\"it_temp\" + 100524] = new ItemTemp(100524, 524, \"floor\", \"rgb(181, 22, 33)\");\n");
    printf("item_templates[\"it_temp\" + 100525] = new ItemTemp(100525, 525, \"floor\", \"rgb(151, 22, 42)\");\n");
    printf("item_templates[\"it_temp\" + 100526] = new ItemTemp(100526, 526, \"floor\", \"rgb(181, 22, 33)\");\n");
    printf("item_templates[\"it_temp\" + 100527] = new ItemTemp(100527, 527, \"floor\", \"rgb(149, 44, 57)\");\n");
    printf("item_templates[\"it_temp\" + 100528] = new ItemTemp(100528, 528, \"floor\", \"rgb(181, 22, 33)\");\n");
    printf("item_templates[\"it_temp\" + 100529] = new ItemTemp(100529, 529, \"floor\", \"rgb(145, 24, 49)\");\n");
    printf("item_templates[\"it_temp\" + 100530] = new ItemTemp(100530, 530, \"floor\", \"rgb(181, 22, 33)\");\n");
    printf("item_templates[\"it_temp\" + 100531] = new ItemTemp(100531, 531, \"floor\", \"rgb(159, 0, 78)\");\n");
    printf("item_templates[\"it_temp\" + 100532] = new ItemTemp(100532, 532, \"floor\", \"rgb(158, 0, 78)\");\n");
    printf("item_templates[\"it_temp\" + 100533] = new ItemTemp(100533, 533, \"floor\", \"rgb(157, 0, 77)\");\n");
    printf("item_templates[\"it_temp\" + 100534] = new ItemTemp(100534, 534, \"floor\", \"rgb(100, 33, 72)\");\n");
    printf("item_templates[\"it_temp\" + 100535] = new ItemTemp(100535, 535, \"floor\", \"rgb(119, 19, 77)\");\n");
    printf("item_templates[\"it_temp\" + 100536] = new ItemTemp(100536, 536, \"floor\", \"rgb(100, 20, 71)\");\n");
    printf("item_templates[\"it_temp\" + 100537] = new ItemTemp(100537, 537, \"floor\", \"rgb(119, 19, 77)\");\n");
    printf("item_templates[\"it_temp\" + 100538] = new ItemTemp(100538, 538, \"floor\", \"rgb(100, 33, 72)\");\n");
    printf("item_templates[\"it_temp\" + 100539] = new ItemTemp(100539, 539, \"floor\", \"rgb(119, 19, 77)\");\n");
    printf("item_templates[\"it_temp\" + 100540] = new ItemTemp(100540, 540, \"floor\", \"rgb(95, 24, 74)\");\n");
    printf("item_templates[\"it_temp\" + 100541] = new ItemTemp(100541, 541, \"floor\", \"rgb(119, 19, 77)\");\n");
    printf("item_templates[\"it_temp\" + 100542] = new ItemTemp(100542, 542, \"floor\", \"rgb(69, 69, 69)\");\n");
    printf("item_templates[\"it_temp\" + 100543] = new ItemTemp(100543, 543, \"floor\", \"rgb(69, 69, 69)\");\n");
    printf("item_templates[\"it_temp\" + 100544] = new ItemTemp(100544, 544, \"floor\", \"rgb(71, 71, 71)\");\n");
    printf("item_templates[\"it_temp\" + 100545] = new ItemTemp(100545, 545, \"floor\", \"rgb(66, 66, 66)\");\n");
    printf("item_templates[\"it_temp\" + 100546] = new ItemTemp(100546, 546, \"floor\", \"rgb(69, 70, 69)\");\n");
    printf("item_templates[\"it_temp\" + 100547] = new ItemTemp(100547, 547, \"floor\", \"rgb(66, 66, 66)\");\n");
    printf("item_templates[\"it_temp\" + 100548] = new ItemTemp(100548, 548, \"floor\", \"rgb(64, 63, 64)\");\n");
    printf("item_templates[\"it_temp\" + 100549] = new ItemTemp(100549, 549, \"floor\", \"rgb(68, 68, 68)\");\n");
    printf("item_templates[\"it_temp\" + 100550] = new ItemTemp(100550, 550, \"floor\", \"rgb(63, 63, 63)\");\n");
    printf("item_templates[\"it_temp\" + 100551] = new ItemTemp(100551, 551, \"floor\", \"rgb(15, 128, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100552] = new ItemTemp(100552, 552, \"floor\", \"rgb(15, 127, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100553] = new ItemTemp(100553, 553, \"floor\", \"rgb(15, 128, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100554] = new ItemTemp(100554, 554, \"floor\", \"rgb(15, 127, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100558] = new ItemTemp(100558, 558, \"floor\", \"rgb(53, 16, 15)\");\n");
    printf("item_templates[\"it_temp\" + 100596] = new ItemTemp(100596, 596, \"floor\", \"rgb(0, 152, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100659] = new ItemTemp(100659, 659, \"floor\", \"rgb(3, 6, 3)\");\n");
    printf("item_templates[\"it_temp\" + 100660] = new ItemTemp(100660, 660, \"floor\", \"rgb(0, 5, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100688] = new ItemTemp(100688, 688, \"floor\", \"rgb(2, 6, 2)\");\n");
    printf("item_templates[\"it_temp\" + 100704] = new ItemTemp(100704, 704, \"floor\", \"rgb(2, 6, 2)\");\n");
    printf("item_templates[\"it_temp\" + 100705] = new ItemTemp(100705, 705, \"floor\", \"rgb(7, 9, 7)\");\n");
    printf("item_templates[\"it_temp\" + 100706] = new ItemTemp(100706, 706, \"floor\", \"rgb(19, 12, 8)\");\n");
    printf("item_templates[\"it_temp\" + 100707] = new ItemTemp(100707, 707, \"floor\", \"rgb(4, 9, 4)\");\n");
    printf("item_templates[\"it_temp\" + 100708] = new ItemTemp(100708, 708, \"floor\", \"rgb(29, 10, 6)\");\n");
    printf("item_templates[\"it_temp\" + 100709] = new ItemTemp(100709, 709, \"floor\", \"rgb(32, 12, 5)\");\n");
    printf("item_templates[\"it_temp\" + 100710] = new ItemTemp(100710, 710, \"floor\", \"rgb(52, 13, 6)\");\n");
    printf("item_templates[\"it_temp\" + 100711] = new ItemTemp(100711, 711, \"floor\", \"rgb(95, 12, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100712] = new ItemTemp(100712, 712, \"floor\", \"rgb(19, 7, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100713] = new ItemTemp(100713, 713, \"floor\", \"rgb(17, 11, 8)\");\n");
    printf("item_templates[\"it_temp\" + 100714] = new ItemTemp(100714, 714, \"floor\", \"rgb(14, 11, 8)\");\n");
    printf("item_templates[\"it_temp\" + 100715] = new ItemTemp(100715, 715, \"floor\", \"rgb(10, 8, 2)\");\n");
    printf("item_templates[\"it_temp\" + 100716] = new ItemTemp(100716, 716, \"floor\", \"rgb(51, 9, 0)\");\n");
    printf("item_templates[\"it_temp\" + 100717] = new ItemTemp(100717, 717, \"floor\", \"rgb(68, 10, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100718] = new ItemTemp(100718, 718, \"floor\", \"rgb(68, 12, 2)\");\n");
    printf("item_templates[\"it_temp\" + 100719] = new ItemTemp(100719, 719, \"floor\", \"rgb(115, 16, 2)\");\n");
    printf("item_templates[\"it_temp\" + 100720] = new ItemTemp(100720, 720, \"floor\", \"rgb(43, 8, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100721] = new ItemTemp(100721, 721, \"floor\", \"rgb(63, 10, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100722] = new ItemTemp(100722, 722, \"floor\", \"rgb(72, 10, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100723] = new ItemTemp(100723, 723, \"floor\", \"rgb(28, 6, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100724] = new ItemTemp(100724, 724, \"floor\", \"rgb(12, 6, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100725] = new ItemTemp(100725, 725, \"floor\", \"rgb(139, 16, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100726] = new ItemTemp(100726, 726, \"floor\", \"rgb(136, 19, 3)\");\n");
    printf("item_templates[\"it_temp\" + 100727] = new ItemTemp(100727, 727, \"floor\", \"rgb(135, 18, 1)\");\n");
    printf("item_templates[\"it_temp\" + 100728] = new ItemTemp(100728, 728, \"floor\", \"rgb(143, 19, 2)\");\n");
    printf("item_templates[\"it_temp\" + 100808] = new ItemTemp(100808, 808, \"floor\", \"rgb(48, 47, 46)\");\n");
    printf("item_templates[\"it_temp\" + 100809] = new ItemTemp(100809, 809, \"floor\", \"rgb(51, 51, 49)\");\n");
    printf("item_templates[\"it_temp\" + 100810] = new ItemTemp(100810, 810, \"floor\", \"rgb(45, 43, 41)\");\n");
    printf("item_templates[\"it_temp\" + 100811] = new ItemTemp(100811, 811, \"floor\", \"rgb(49, 47, 46)\");\n");
    printf("item_templates[\"it_temp\" + 100950] = new ItemTemp(100950, 950, \"floor\", \"rgb(74, 65, 65)\");\n");
    printf("item_templates[\"it_temp\" + 100951] = new ItemTemp(100951, 951, \"floor\", \"rgb(75, 67, 67)\");\n");
    printf("item_templates[\"it_temp\" + 100952] = new ItemTemp(100952, 952, \"floor\", \"rgb(75, 66, 66)\");\n");
    printf("item_templates[\"it_temp\" + 100953] = new ItemTemp(100953, 953, \"floor\", \"rgb(72, 63, 64)\");\n");
    printf("item_templates[\"it_temp\" + 100954] = new ItemTemp(100954, 954, \"floor\", \"rgb(71, 63, 63)\");\n");
    printf("item_templates[\"it_temp\" + 100955] = new ItemTemp(100955, 955, \"floor\", \"rgb(84, 75, 75)\");\n");
    printf("item_templates[\"it_temp\" + 100956] = new ItemTemp(100956, 956, \"floor\", \"rgb(73, 64, 64)\");\n");
    printf("item_templates[\"it_temp\" + 100957] = new ItemTemp(100957, 957, \"floor\", \"rgb(85, 76, 76)\");\n");
    printf("item_templates[\"it_temp\" + 100958] = new ItemTemp(100958, 958, \"floor\", \"rgb(74, 65, 64)\");\n");
    printf("item_templates[\"it_temp\" + 100959] = new ItemTemp(100959, 959, \"floor\", \"rgb(98, 86, 87)\");\n");
    printf("item_templates[\"it_temp\" + 100960] = new ItemTemp(100960, 960, \"floor\", \"rgb(94, 83, 83)\");\n");
    printf("item_templates[\"it_temp\" + 100961] = new ItemTemp(100961, 961, \"floor\", \"rgb(103, 92, 92)\");\n");
    printf("item_templates[\"it_temp\" + 100962] = new ItemTemp(100962, 962, \"floor\", \"rgb(99, 87, 87)\");\n");
    printf("item_templates[\"it_temp\" + 100963] = new ItemTemp(100963, 963, \"floor\", \"rgb(96, 85, 84)\");\n");
    printf("item_templates[\"it_temp\" + 100964] = new ItemTemp(100964, 964, \"floor\", \"rgb(102, 90, 90)\");\n");
    printf("item_templates[\"it_temp\" + 100965] = new ItemTemp(100965, 965, \"floor\", \"rgb(107, 94, 94)\");\n");
    printf("item_templates[\"it_temp\" + 100966] = new ItemTemp(100966, 966, \"floor\", \"rgb(99, 88, 87)\");\n");
    printf("item_templates[\"it_temp\" + 100967] = new ItemTemp(100967, 967, \"floor\", \"rgb(94, 83, 83)\");\n");
    printf("item_templates[\"it_temp\" + 100985] = new ItemTemp(100985, 985, \"floor\", \"rgb(93, 88, 87)\");\n");
    printf("item_templates[\"it_temp\" + 100986] = new ItemTemp(100986, 986, \"floor\", \"rgb(87, 82, 81)\");\n");
    printf("item_templates[\"it_temp\" + 100987] = new ItemTemp(100987, 987, \"floor\", \"rgb(89, 83, 83)\");\n");
    printf("item_templates[\"it_temp\" + 100988] = new ItemTemp(100988, 988, \"floor\", \"rgb(85, 79, 79)\");\n");
    printf("item_templates[\"it_temp\" + 100989] = new ItemTemp(100989, 989, \"floor\", \"rgb(52, 71, 19)\");\n");
    printf("item_templates[\"it_temp\" + 100999] = new ItemTemp(100999, 999, \"floor\", \"rgb(0, 0, 0)\");\n");
    printf("item_templates[\"it_temp\" + 101001] = new ItemTemp(101001, 1001, \"floor\", \"rgb(135, 77, 13)\");\n");
    printf("item_templates[\"it_temp\" + 101002] = new ItemTemp(101002, 1002, \"floor\", \"rgb(144, 119, 32)\");\n");
    printf("item_templates[\"it_temp\" + 101003] = new ItemTemp(101003, 1003, \"floor\", \"rgb(24, 54, 7)\");\n");
    printf("item_templates[\"it_temp\" + 101005] = new ItemTemp(101005, 1005, \"floor\", \"rgb(21, 49, 7)\");\n");
    printf("item_templates[\"it_temp\" + 101006] = new ItemTemp(101006, 1006, \"floor\", \"rgb(26, 56, 8)\");\n");
    printf("item_templates[\"it_temp\" + 101007] = new ItemTemp(101007, 1007, \"floor\", \"rgb(15, 44, 1)\");\n");
    printf("item_templates[\"it_temp\" + 101008] = new ItemTemp(101008, 1008, \"floor\", \"rgb(49, 87, 48)\");\n");
    printf("item_templates[\"it_temp\" + 101010] = new ItemTemp(101010, 1010, \"floor\", \"rgb(84, 84, 84)\");\n");
    printf("item_templates[\"it_temp\" + 101011] = new ItemTemp(101011, 1011, \"floor\", \"rgb(191, 3, 214)\");\n");
    printf("item_templates[\"it_temp\" + 101013] = new ItemTemp(101013, 1013, \"floor\", \"rgb(129, 74, 49)\");\n");
    printf("item_templates[\"it_temp\" + 101014] = new ItemTemp(101014, 1014, \"floor\", \"rgb(32, 0, 0)\");\n");
    printf("item_templates[\"it_temp\" + 101034] = new ItemTemp(101034, 1034, \"floor\", \"rgb(24, 66, 0)\");\n");
    printf("item_templates[\"it_temp\" + 101092] = new ItemTemp(101092, 1092, \"floor\", \"rgb(39, 34, 91)\");\n");
    printf("item_templates[\"it_temp\" + 101099] = new ItemTemp(101099, 1099, \"floor\", \"rgb(97, 0, 2)\");\n");
    printf("item_templates[\"it_temp\" + 101100] = new ItemTemp(101100, 1100, \"floor\", \"rgb(103, 104, 103)\");\n");
    printf("item_templates[\"it_temp\" + 101109] = new ItemTemp(101109, 1109, \"floor\", \"rgb(71, 0, 0)\");\n");
    printf("item_templates[\"it_temp\" + 101118] = new ItemTemp(101118, 1118, \"floor\", \"rgb(8, 92, 14)\");\n");
    printf("item_templates[\"it_temp\" + 101141] = new ItemTemp(101141, 1141, \"floor\", \"rgb(68, 53, 68)\");\n");
    printf("item_templates[\"it_temp\" + 101145] = new ItemTemp(101145, 1145, \"floor\", \"rgb(86, 91, 86)\");\n");
    printf("item_templates[\"it_temp\" + 101158] = new ItemTemp(101158, 1158, \"floor\", \"rgb(203, 203, 203)\");\n");
    printf("item_templates[\"it_temp\" + 116430] = new ItemTemp(116430, 16430, \"floor\", \"rgb(71, 48, 49)\");\n");
    printf("item_templates[\"it_temp\" + 116431] = new ItemTemp(116431, 16431, \"floor\", \"rgb(70, 21, 22)\");\n");
    printf("item_templates[\"it_temp\" + 116432] = new ItemTemp(116432, 16432, \"floor\", \"rgb(65, 19, 20)\");\n");
    printf("item_templates[\"it_temp\" + 116433] = new ItemTemp(116433, 16433, \"floor\", \"rgb(53, 12, 13)\");\n");
    printf("item_templates[\"it_temp\" + 116434] = new ItemTemp(116434, 16434, \"floor\", \"rgb(76, 14, 14)\");\n");
    printf("item_templates[\"it_temp\" + 116435] = new ItemTemp(116435, 16435, \"floor\", \"rgb(76, 21, 22)\");\n");
    printf("item_templates[\"it_temp\" + 116436] = new ItemTemp(116436, 16436, \"floor\", \"rgb(62, 14, 14)\");\n");
    printf("item_templates[\"it_temp\" + 116437] = new ItemTemp(116437, 16437, \"floor\", \"rgb(64, 20, 21)\");\n");
    printf("item_templates[\"it_temp\" + 116438] = new ItemTemp(116438, 16438, \"floor\", \"rgb(73, 54, 54)\");\n");
    printf("item_templates[\"it_temp\" + 116439] = new ItemTemp(116439, 16439, \"floor\", \"rgb(77, 60, 61)\");\n");
    printf("item_templates[\"it_temp\" + 116440] = new ItemTemp(116440, 16440, \"floor\", \"rgb(79, 26, 27)\");\n");
    printf("item_templates[\"it_temp\" + 116441] = new ItemTemp(116441, 16441, \"floor\", \"rgb(83, 15, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116442] = new ItemTemp(116442, 16442, \"floor\", \"rgb(83, 53, 54)\");\n");
    printf("item_templates[\"it_temp\" + 116443] = new ItemTemp(116443, 16443, \"floor\", \"rgb(56, 45, 46)\");\n");
    printf("item_templates[\"it_temp\" + 116444] = new ItemTemp(116444, 16444, \"floor\", \"rgb(63, 38, 39)\");\n");
    printf("item_templates[\"it_temp\" + 116445] = new ItemTemp(116445, 16445, \"floor\", \"rgb(82, 46, 46)\");\n");
    printf("item_templates[\"it_temp\" + 116446] = new ItemTemp(116446, 16446, \"floor\", \"rgb(95, 42, 43)\");\n");
    printf("item_templates[\"it_temp\" + 116447] = new ItemTemp(116447, 16447, \"floor\", \"rgb(97, 51, 52)\");\n");
    printf("item_templates[\"it_temp\" + 116448] = new ItemTemp(116448, 16448, \"floor\", \"rgb(79, 63, 64)\");\n");
    printf("item_templates[\"it_temp\" + 116449] = new ItemTemp(116449, 16449, \"floor\", \"rgb(118, 30, 31)\");\n");
    printf("item_templates[\"it_temp\" + 116450] = new ItemTemp(116450, 16450, \"floor\", \"rgb(88, 61, 62)\");\n");
    printf("item_templates[\"it_temp\" + 116451] = new ItemTemp(116451, 16451, \"floor\", \"rgb(118, 30, 31)\");\n");
    printf("item_templates[\"it_temp\" + 116452] = new ItemTemp(116452, 16452, \"floor\", \"rgb(88, 61, 62)\");\n");
    printf("item_templates[\"it_temp\" + 116453] = new ItemTemp(116453, 16453, \"floor\", \"rgb(121, 24, 25)\");\n");
    printf("item_templates[\"it_temp\" + 116454] = new ItemTemp(116454, 16454, \"floor\", \"rgb(90, 47, 48)\");\n");
    printf("item_templates[\"it_temp\" + 116455] = new ItemTemp(116455, 16455, \"floor\", \"rgb(78, 68, 69)\");\n");
    printf("item_templates[\"it_temp\" + 116456] = new ItemTemp(116456, 16456, \"floor\", \"rgb(108, 47, 48)\");\n");
    printf("item_templates[\"it_temp\" + 116457] = new ItemTemp(116457, 16457, \"floor\", \"rgb(149, 28, 29)\");\n");
    printf("item_templates[\"it_temp\" + 116458] = new ItemTemp(116458, 16458, \"floor\", \"rgb(85, 60, 61)\");\n");
    printf("item_templates[\"it_temp\" + 116459] = new ItemTemp(116459, 16459, \"floor\", \"rgb(111, 32, 32)\");\n");
    printf("item_templates[\"it_temp\" + 116460] = new ItemTemp(116460, 16460, \"floor\", \"rgb(84, 73, 74)\");\n");
    printf("item_templates[\"it_temp\" + 116461] = new ItemTemp(116461, 16461, \"floor\", \"rgb(74, 55, 56)\");\n");
    printf("item_templates[\"it_temp\" + 116462] = new ItemTemp(116462, 16462, \"floor\", \"rgb(81, 64, 65)\");\n");
    printf("item_templates[\"it_temp\" + 116463] = new ItemTemp(116463, 16463, \"floor\", \"rgb(100, 44, 44)\");\n");
    printf("item_templates[\"it_temp\" + 116464] = new ItemTemp(116464, 16464, \"floor\", \"rgb(64, 55, 57)\");\n");
    printf("item_templates[\"it_temp\" + 116465] = new ItemTemp(116465, 16465, \"floor\", \"rgb(120, 28, 28)\");\n");
    printf("item_templates[\"it_temp\" + 116466] = new ItemTemp(116466, 16466, \"floor\", \"rgb(69, 47, 48)\");\n");
    printf("item_templates[\"it_temp\" + 116467] = new ItemTemp(116467, 16467, \"floor\", \"rgb(87, 48, 49)\");\n");
    printf("item_templates[\"it_temp\" + 116468] = new ItemTemp(116468, 16468, \"floor\", \"rgb(81, 17, 19)\");\n");
    printf("item_templates[\"it_temp\" + 116469] = new ItemTemp(116469, 16469, \"floor\", \"rgb(81, 60, 60)\");\n");
    printf("item_templates[\"it_temp\" + 116470] = new ItemTemp(116470, 16470, \"floor\", \"rgb(86, 24, 25)\");\n");
    printf("item_templates[\"it_temp\" + 116471] = new ItemTemp(116471, 16471, \"floor\", \"rgb(78, 65, 65)\");\n");
    printf("item_templates[\"it_temp\" + 116472] = new ItemTemp(116472, 16472, \"floor\", \"rgb(76, 49, 50)\");\n");
    printf("item_templates[\"it_temp\" + 116473] = new ItemTemp(116473, 16473, \"floor\", \"rgb(65, 28, 29)\");\n");
    printf("item_templates[\"it_temp\" + 116474] = new ItemTemp(116474, 16474, \"floor\", \"rgb(68, 35, 36)\");\n");
    printf("item_templates[\"it_temp\" + 116475] = new ItemTemp(116475, 16475, \"floor\", \"rgb(51, 28, 29)\");\n");
    printf("item_templates[\"it_temp\" + 116476] = new ItemTemp(116476, 16476, \"floor\", \"rgb(57, 38, 39)\");\n");
    printf("item_templates[\"it_temp\" + 116477] = new ItemTemp(116477, 16477, \"floor\", \"rgb(70, 41, 42)\");\n");
    printf("item_templates[\"it_temp\" + 116478] = new ItemTemp(116478, 16478, \"floor\", \"rgb(86, 75, 76)\");\n");
    printf("item_templates[\"it_temp\" + 116479] = new ItemTemp(116479, 16479, \"floor\", \"rgb(76, 24, 26)\");\n");
    printf("item_templates[\"it_temp\" + 116480] = new ItemTemp(116480, 16480, \"floor\", \"rgb(80, 57, 58)\");\n");
    printf("item_templates[\"it_temp\" + 116481] = new ItemTemp(116481, 16481, \"floor\", \"rgb(62, 11, 14)\");\n");
    printf("item_templates[\"it_temp\" + 116482] = new ItemTemp(116482, 16482, \"floor\", \"rgb(81, 14, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116483] = new ItemTemp(116483, 16483, \"floor\", \"rgb(80, 14, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116484] = new ItemTemp(116484, 16484, \"floor\", \"rgb(85, 15, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116485] = new ItemTemp(116485, 16485, \"floor\", \"rgb(80, 14, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116486] = new ItemTemp(116486, 16486, \"floor\", \"rgb(73, 13, 15)\");\n");
    printf("item_templates[\"it_temp\" + 116487] = new ItemTemp(116487, 16487, \"floor\", \"rgb(86, 16, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116488] = new ItemTemp(116488, 16488, \"floor\", \"rgb(90, 16, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116489] = new ItemTemp(116489, 16489, \"floor\", \"rgb(82, 15, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116490] = new ItemTemp(116490, 16490, \"floor\", \"rgb(82, 15, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116491] = new ItemTemp(116491, 16491, \"floor\", \"rgb(89, 16, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116492] = new ItemTemp(116492, 16492, \"floor\", \"rgb(109, 20, 21)\");\n");
    printf("item_templates[\"it_temp\" + 116493] = new ItemTemp(116493, 16493, \"floor\", \"rgb(98, 18, 18)\");\n");
    printf("item_templates[\"it_temp\" + 116494] = new ItemTemp(116494, 16494, \"floor\", \"rgb(96, 17, 18)\");\n");
    printf("item_templates[\"it_temp\" + 116495] = new ItemTemp(116495, 16495, \"floor\", \"rgb(90, 17, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116496] = new ItemTemp(116496, 16496, \"floor\", \"rgb(82, 15, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116497] = new ItemTemp(116497, 16497, \"floor\", \"rgb(116, 21, 22)\");\n");
    printf("item_templates[\"it_temp\" + 116498] = new ItemTemp(116498, 16498, \"floor\", \"rgb(108, 20, 21)\");\n");
    printf("item_templates[\"it_temp\" + 116499] = new ItemTemp(116499, 16499, \"floor\", \"rgb(108, 20, 20)\");\n");
    printf("item_templates[\"it_temp\" + 116500] = new ItemTemp(116500, 16500, \"floor\", \"rgb(112, 20, 21)\");\n");
    printf("item_templates[\"it_temp\" + 116501] = new ItemTemp(116501, 16501, \"floor\", \"rgb(102, 18, 18)\");\n");
    printf("item_templates[\"it_temp\" + 116502] = new ItemTemp(116502, 16502, \"floor\", \"rgb(122, 22, 22)\");\n");
    printf("item_templates[\"it_temp\" + 116503] = new ItemTemp(116503, 16503, \"floor\", \"rgb(113, 20, 22)\");\n");
    printf("item_templates[\"it_temp\" + 116504] = new ItemTemp(116504, 16504, \"floor\", \"rgb(111, 20, 20)\");\n");
    printf("item_templates[\"it_temp\" + 116505] = new ItemTemp(116505, 16505, \"floor\", \"rgb(107, 19, 20)\");\n");
    printf("item_templates[\"it_temp\" + 116506] = new ItemTemp(116506, 16506, \"floor\", \"rgb(113, 20, 21)\");\n");
    printf("item_templates[\"it_temp\" + 116507] = new ItemTemp(116507, 16507, \"floor\", \"rgb(108, 19, 20)\");\n");
    printf("item_templates[\"it_temp\" + 116584] = new ItemTemp(116584, 16584, \"floor\", \"rgb(97, 66, 60)\");\n");
    printf("item_templates[\"it_temp\" + 116585] = new ItemTemp(116585, 16585, \"floor\", \"rgb(87, 65, 62)\");\n");
    printf("item_templates[\"it_temp\" + 116586] = new ItemTemp(116586, 16586, \"floor\", \"rgb(107, 61, 49)\");\n");
    printf("item_templates[\"it_temp\" + 116587] = new ItemTemp(116587, 16587, \"floor\", \"rgb(114, 71, 61)\");\n");
    printf("item_templates[\"it_temp\" + 116588] = new ItemTemp(116588, 16588, \"floor\", \"rgb(104, 64, 55)\");\n");
    printf("item_templates[\"it_temp\" + 116589] = new ItemTemp(116589, 16589, \"floor\", \"rgb(99, 71, 68)\");\n");
    printf("item_templates[\"it_temp\" + 116590] = new ItemTemp(116590, 16590, \"floor\", \"rgb(107, 72, 67)\");\n");
    printf("item_templates[\"it_temp\" + 116591] = new ItemTemp(116591, 16591, \"floor\", \"rgb(106, 61, 53)\");\n");
    printf("item_templates[\"it_temp\" + 116592] = new ItemTemp(116592, 16592, \"floor\", \"rgb(108, 57, 49)\");\n");
    printf("item_templates[\"it_temp\" + 116593] = new ItemTemp(116593, 16593, \"floor\", \"rgb(92, 60, 55)\");\n");
    printf("item_templates[\"it_temp\" + 116594] = new ItemTemp(116594, 16594, \"floor\", \"rgb(121, 65, 49)\");\n");
    printf("item_templates[\"it_temp\" + 116595] = new ItemTemp(116595, 16595, \"floor\", \"rgb(128, 70, 56)\");\n");
    printf("item_templates[\"it_temp\" + 116596] = new ItemTemp(116596, 16596, \"floor\", \"rgb(129, 62, 45)\");\n");
    printf("item_templates[\"it_temp\" + 116597] = new ItemTemp(116597, 16597, \"floor\", \"rgb(86, 62, 57)\");\n");
    printf("item_templates[\"it_temp\" + 116598] = new ItemTemp(116598, 16598, \"floor\", \"rgb(96, 72, 69)\");\n");
    printf("item_templates[\"it_temp\" + 116634] = new ItemTemp(116634, 16634, \"floor\", \"rgb(40, 77, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116635] = new ItemTemp(116635, 16635, \"floor\", \"rgb(42, 77, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116636] = new ItemTemp(116636, 16636, \"floor\", \"rgb(34, 80, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116637] = new ItemTemp(116637, 16637, \"floor\", \"rgb(41, 77, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116638] = new ItemTemp(116638, 16638, \"floor\", \"rgb(38, 78, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116639] = new ItemTemp(116639, 16639, \"floor\", \"rgb(32, 81, 16)\");\n");
    printf("item_templates[\"it_temp\" + 116640] = new ItemTemp(116640, 16640, \"floor\", \"rgb(37, 79, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116641] = new ItemTemp(116641, 16641, \"floor\", \"rgb(38, 78, 17)\");\n");
    printf("item_templates[\"it_temp\" + 116670] = new ItemTemp(116670, 16670, \"floor\", \"rgb(142, 144, 207)\");\n");
    printf("item_templates[\"it_temp\" + 116671] = new ItemTemp(116671, 16671, \"floor\", \"rgb(143, 146, 209)\");\n");
    printf("item_templates[\"it_temp\" + 116672] = new ItemTemp(116672, 16672, \"floor\", \"rgb(144, 146, 209)\");\n");
    printf("item_templates[\"it_temp\" + 116673] = new ItemTemp(116673, 16673, \"floor\", \"rgb(144, 146, 209)\");\n");
    printf("item_templates[\"it_temp\" + 116728] = new ItemTemp(116728, 16728, \"floor\", \"rgb(0, 47, 0)\");\n");
    printf("item_templates[\"it_temp\" + 116859] = new ItemTemp(116859, 16859, \"floor\", \"rgb(15, 73, 10)\");\n");
    printf("item_templates[\"it_temp\" + 116933] = new ItemTemp(116933, 16933, \"floor\", \"rgb(0, 0, 77)\");\n");
    printf("item_templates[\"it_temp\" + 116934] = new ItemTemp(116934, 16934, \"floor\", \"rgb(17, 5, 38)\");\n");
    printf("item_templates[\"it_temp\" + 116937] = new ItemTemp(116937, 16937, \"floor\", \"rgb(17, 5, 37)\");\n");
    printf("item_templates[\"it_temp\" + 116957] = new ItemTemp(116957, 16957, \"floor\", \"rgb(0, 0, 73)\");\n");
    printf("item_templates[\"it_temp\" + 116958] = new ItemTemp(116958, 16958, \"floor\", \"rgb(1, 1, 74)\");\n");
    printf("item_templates[\"it_temp\" + 116959] = new ItemTemp(116959, 16959, \"floor\", \"rgb(1, 1, 74)\");\n");
    printf("item_templates[\"it_temp\" + 116980] = new ItemTemp(116980, 16980, \"floor\", \"rgb(69, 69, 69)\");\n");
    printf("item_templates[\"it_temp\" + 116981] = new ItemTemp(116981, 16981, \"floor\", \"rgb(79, 79, 79)\");\n");
    printf("item_templates[\"it_temp\" + 116982] = new ItemTemp(116982, 16982, \"floor\", \"rgb(79, 80, 79)\");\n");
    printf("item_templates[\"it_temp\" + 116983] = new ItemTemp(116983, 16983, \"floor\", \"rgb(79, 79, 79)\");\n");
    printf("item_templates[\"it_temp\" + 116984] = new ItemTemp(116984, 16984, \"floor\", \"rgb(78, 78, 78)\");\n");
}

void scriptMapper()
{
    printf("<script>\n");
    printf("function padSpriteNum(num) { return (\"00000\" + num).substr(-5); }\n");
    
    printf("class CanvasHandler {\n");
    printf("constructor(cv) {\n");
    printf("this.cv = cv;\n");
    printf("this.cv.width = cv.offsetWidth;\n");
    printf("this.cv.height = cv.offsetHeight;\n");
    printf("this.ctx = this.cv.getContext(\"2d\");\n");
    
    printf("this.drawXOffset = 200;\n");
    printf("this.drawYOffset = 200;\n");
    
    printf("this.loadedImages = {};\n");
    printf("this.redrawTimeout = 100;\n");
    printf("}\n");
    
    printf("/** Load an image for future drawing */\n");
    printf("loadImage(path) {\n");
    printf("if (this.loadedImages.hasOwnProperty(path)) {\n");
    printf("return;\n");
    printf("}\n");
    printf("var newImg = new Image();\n");
    printf("newImg.setAttribute(\"loaded\", \"0\");\n");
    printf("newImg.onload = () => {\n");
    printf("newImg.setAttribute(\"loaded\", \"1\");\n");
    printf("};\n");
    printf("newImg.src = path;\n");
    printf("this.loadedImages[path] = newImg;\n");
    printf("}\n");
    
    printf("/** Load a list of images for future drawing */\n");
    printf("loadImages(pathList) {\n");
    printf("for (let imgP of pathList) {\n");
    printf("this.loadImage(imgP);\n");
    printf("}\n");
    printf("}\n");
    
    printf("getImage(path) {\n");
    printf("if (!this.loadedImages.hasOwnProperty(path)) {\n");
    printf("return null;\n");
    printf("}\n");
    printf("return this.loadedImages[path];\n");
    printf("}\n");
    
    printf("clearLoadedImages() {\n");
    printf("this.loadedImages = {};\n");
    printf("}\n");
    
    printf("clearContext() {\n");
    printf("if (this.cv.offsetWidth != this.cv.width) this.cv.width = this.cv.offsetWidth;\n");
    printf("if (this.cv.offsetHeight != this.cv.height) this.cv.height = this.cv.offsetHeight;\n");
    printf("this.ctx.clearRect(0, 0, this.cv.width, this.cv.height);\n");
    printf("}\n");
    
    printf("/** If enqueue is 1, will queue given image for loading before drawing it (if it hasn\"t been loaded before) */\n");
    printf("drawImage(imgPath, x, y, enqueue) {\n");
    printf("// Image not loaded\n");
    printf("if (!this.loadedImages.hasOwnProperty(imgPath) && enqueue) {\n");
    printf("// Queue for re-drawing\n");
    printf("this.loadImage(imgPath);\n");
    printf("setTimeout(() => {\n");
    printf("this.drawImage(imgPath, x, y, enqueue);\n");
    printf("}, this.redrawTimeout);\n");
    printf("return;\n");
    printf("}\n");
    printf("var img = this.loadedImages[imgPath];\n");
    printf("if (img.getAttribute(\"loaded\") === \"0\" && enqueue) {\n");
    printf("setTimeout(() => {\n");
    printf("this.drawImage(imgPath, x, y, enqueue);\n");
    printf("}, this.redrawTimeout);\n");
    printf("return;\n");
    printf("}\n");
    printf("if (x + 100 < img.width || y + 100 < img.height || x > this.cv.width || y > this.cv.height) {\n");
    printf("return;\n");
    printf("}\n");
    printf("this.ctx.drawImage(img, x, y);\n");
    printf("}\n");
    
    printf("/** X and Y in tiles */\n");
    printf("drawImageIsometric(imgPath, x, y) {\n");
    printf("// Image not loaded\n");
    printf("if (!this.loadedImages.hasOwnProperty(imgPath) || (this.loadedImages.hasOwnProperty(imgPath) && this.loadedImages[imgPath].width == 0)) {\n");
    printf("this.loadImage(imgPath);\n");
    printf("setTimeout(() => {\n");
    printf("this.drawImageIsometric(imgPath, x, y);\n");
    printf("}, this.redrawTimeout);\n");
    printf("return;\n");
    printf("}\n");
    printf("var img = this.loadedImages[imgPath];\n");
    printf("var drawX = this.drawXOffset + (x + y) * 16 - (img.width - 32);\n");
    printf("var drawY = this.drawYOffset + (y - x) * 8 - (img.height - 32);\n");
    printf("this.drawImage(imgPath, drawX, drawY, false);\n");
    printf("}\n");
    printf("}\n");
    
    printf("class MapTile {\n");
    printf("constructor(id, x, y, floor, item) {\n");
    printf("this.id = id;\n");
    printf("this.x = x;\n");
    printf("this.y = y;\n");
    printf("this.map_x = 0;\n");
    printf("this.map_y = 0;\n");
    printf("this.floor = floor; // ID of floor template\n");
    printf("this.item = item;   // ID of template\n");
    printf("this.dom_elem = null;\n");
    printf("}\n");
    
    printf("getItemTemp() {\n");
    printf("var it_id = \"it_temp\" + this.item;\n");
    printf("if (!item_templates.hasOwnProperty(it_id)) return null;\n");
    printf("return item_templates[it_id];\n");
    printf("}\n");
    
    printf("getFloorSprite() {\n");
    printf("var fl_id = \"it_temp\" + this.floor;\n");
    printf("if (item_templates.hasOwnProperty(fl_id)) return \"/assets/\" + padSpriteNum(item_templates[fl_id].item_spr) + \".png\";\n");
    printf("}\n");
    printf("getItemSprite() {\n");
    printf("var it_temp = this.getItemTemp();\n");
    printf("if (!it_temp) return null;\n");
    
    printf("var tmp_item = it_temp.item_spr;\n");
    printf("// Hiding walls toggle\n");
    printf("if (hideWalls) {\n");
    printf("if (it_temp.type == \"wall\") tmp_item++;\n");
    printf("}\n");
    printf("return \"/assets/\" + padSpriteNum(tmp_item) + \".png\";\n");
    printf("}\n");
    
    printf("updateElem() {\n");
    printf("if (!this.dom_elem) return;\n");
    
    printf("var it_temp = this.getItemTemp();\n");
    
    printf("this.dom_elem.innerHTML = \"\";\n");
    
    printf("// If tile has wall, set bg to its color, otherwise set to floor\n");
    printf("if (it_temp && it_temp.type == \"wall\") {\n");
    printf("this.dom_elem.style.backgroundColor = it_temp.color;\n");
    printf("} else if (this.floor) {\n");
    printf("var flr_temp = \"it_temp\" + this.floor;\n");
    printf("if (item_templates.hasOwnProperty(flr_temp) && item_templates[flr_temp].type == \"floor\") {\n");
    printf("this.dom_elem.style.backgroundColor = item_templates[flr_temp].color;\n");
    printf("} else {\n");
    printf("this.dom_elem.style.backgroundColor = \"black\";\n");
    printf("}\n");
    printf("}\n");
    
    printf("// Non-wall item shape\n");
    printf("if (it_temp && it_temp.type == \"item\") {\n");
    printf("var itemIcon = document.createElement(\"div\");\n");
    printf("itemIcon.className = \"temp-icon\";\n");
    printf("itemIcon.style.backgroundColor = it_temp.color;\n");
    printf("this.dom_elem.appendChild(itemIcon);\n");
    printf("}\n");
    printf("}\n");
    printf("}\n");
    
    printf("class ItemTemp {\n");
    printf("constructor(temp_id, item_spr, type, color) {\n");
    printf("this.temp_id = temp_id;\n");
    printf("this.item_spr = item_spr;\n");
    printf("this.type = type;   // 3 types: \"floor\", \"wall\" and \"item\" (candles, portals, etc.)\n");
    printf("this.color = color;\n");
    printf("this.dom_elem = null;\n");
    printf("}\n");
    
    printf("updateElem() {\n");
    printf("if (!this.dom_elem) return;\n");
    printf("var it_temp_prev = document.getElementById(this.dom_elem.id + \"_prev\");\n");
    
    printf("it_temp_prev.style.backgroundColor = this.color;\n");
    
    printf("var span_txt = \"F\";\n");
    printf("if (this.type == \"wall\") span_txt = \"W\";\n");
    printf("if (this.type == \"item\") {\n");
    printf("span_txt = \"It\";\n");
    printf("it_temp_prev.style.borderRadius = \"100%%\";\n");
    printf("it_temp_prev.style.border = \"2px orange solid\";\n");
    printf("} else {\n");
    printf("it_temp_prev.style.borderRadius = null;\n");
    printf("it_temp_prev.style.border = null;\n");
    printf("}\n");
    printf("document.getElementById(this.dom_elem.id + \"_typespan\").innerHTML = span_txt;\n");
    printf("}\n");
    printf("}\n");
    
    printf("const prevCanvas = new CanvasHandler(document.getElementById(\"cv-preview\"));\n");
    printf("const tileTable = document.getElementById(\"tiles-table\");\n");
    printf("const paletteDiv = document.getElementById(\"div-palette-container\");\n");
    
    printf("var tilemap = {};\n");
    printf("var tilemap_width = 25;\n");
    printf("var tilemap_height = 25;\n");
    printf("var item_templates = {};\n");
    
    printf("// Mouse clicking\n");
    printf("var mouseDown = 0;\n");
    printf("document.body.onmousedown = function(eventData) {\n");
    printf("if (eventData.button == 0) {\n");
    printf("mouseDown = 1;\n");
    printf("} else if (eventData.button == 2) {\n");
    printf("mouseDown = 2;\n");
    printf("}\n");
    printf("}\n");
    printf("document.body.onmouseup = function() {\n");
    printf("mouseDown = 0;\n");
    printf("}\n");
    
    printf("// Shift/Ctrl press\n");
    printf("var shiftDown = 0;\n");
    printf("var ctrlDown = 0;\n");
    printf("document.addEventListener(\"keydown\", function(event) {\n");
    printf("switch(event.keyCode) {\n");
    printf("case 16: shiftDown = 1; break;  // Shift\n");
    printf("case 17: ctrlDown = 1; break;   // Ctrl\n");
    printf("}\n");
    printf("});\n");
    printf("document.addEventListener(\"keyup\", function(event) {\n");
    printf("switch(event.keyCode) {\n");
    printf("case 16: shiftDown = 0; break;\n");
    printf("case 17: ctrlDown = 0; break;\n");
    printf("}\n");
    printf("});\n");

    // Load item/wall templates from server
    for (int i=0; i<MAXTITEM; i++) {
        if (it_temp[i].used == USE_EMPTY) continue;
        if (it_temp[i].flags&(IF_TAKE)) continue;

        char *it_type = "wall";
        char *it_color = "gray";

        // Considers templates with any of these flags as 'item' type
        if (it_temp[i].flags&(IF_TAKE|IF_LOOK|IF_LOOKSPECIAL|IF_USE|IF_USESPECIAL) || !(it_temp[i].flags&(IF_MOVEBLOCK))) {
            it_type = "item";
            it_color = "yellow";
        }
        printf("item_templates[\"it_temp\" + %d] = new ItemTemp(%d, %d, \"%s\", \"%s\");\n", i, i, it_temp[i].sprite[0], it_type, it_color);
    }

    // Load floors
    loadFloors();
    
    printf("function loadMapArea() {\n");
    printf("var x1 = document.getElementById(\"inp-map-x1\").value;\n");
    printf("var y1 = document.getElementById(\"inp-map-y1\").value;\n");
    printf("var x2 = document.getElementById(\"inp-map-x2\").value;\n");
    printf("var y2 = document.getElementById(\"inp-map-y2\").value;\n");
    
    printf("if (!x1.match(/^[0-9]+$/)) { alert(\"X1 contains non-number characters.\"); return; }\n");
    printf("if (!y1.match(/^[0-9]+$/)) { alert(\"Y1 contains non-number characters.\"); return; }\n");
    printf("if (!x2.match(/^[0-9]+$/)) { alert(\"X2 contains non-number characters.\"); return; }\n");
    printf("if (!y2.match(/^[0-9]+$/)) { alert(\"Y2 contains non-number characters.\"); return; }\n");
    
    printf("if (x1 < 0 || x1 > 1024 || x1 >= x2) { alert(\"X1 must be between 0 and 1024, and must be lower than X2.\"); return; }\n");
    printf("if (y1 < 0 || y1 > 1024 || y1 >= y2) { alert(\"Y1 must be between 0 and 1024, and must be lower than Y2.\"); return; }\n");
    printf("if (x2 < 0 || x2 > 1024) { alert(\"X2 must be between 0 and 1024.\"); return; }\n");
    printf("if (y2 < 0 || y2 > 1024) { alert(\"Y2 must be between 0 and 1024.\"); return; }\n");
    
    printf("// Load x1,y1 -> x2,y2 from server map into the tilemap, then redirect...\n");
    printf("window.location.replace(\"/cgi-imp/mapper.cgi?step=1&x1=\" + x1 + \"&y1=\" + y1 + \"&x2=\" + x2 + \"&y2=\" + y2);\n");
    printf("}\n");
    
    printf("function loadMapCells(x1, y1, x2, y2) {\n");
    printf("tilemap_width = x2 - x1;\n");
    printf("tilemap_height = y2 - y1;\n");
    printf("tileTable.innerHTML = \"\";\n");
    printf("tilemap = {};\n");
    
    printf("var x = 0, y = 0;\n");
    printf("for (var i = y1; i < y2; i++) {\n");
    printf("var tileRow = document.createElement(\"tr\");\n");
    printf("tileRow.draggable = false;\n");
    printf("tileTable.appendChild(tileRow);\n");
    
    printf("for (var j = x1; j < x2; j++) {\n");
    printf("var id = x + y * tilemap_width;\n");
    printf("var tile_id = \"maptile\" + id;\n");
    printf("tilemap[tile_id] = new MapTile(id, x, y, 0, 0);\n");
    printf("tilemap[tile_id].map_x = x1 + y;\n");
    printf("tilemap[tile_id].map_y = y1 + x;\n");
    
    printf("var tileCell = document.createElement(\"td\");\n");
    printf("tileCell.id = tile_id;\n");
    printf("tileCell.className = \"tile-cell unselectable\";\n");
    printf("tileCell.draggable = false;\n");
    printf("tileCell.oncontextmenu = function() { return false; }\n");
    printf("tileCell.onmousedown = function(obj) { mapCellClick(obj.srcElement.id, Math.max(1, obj.button)); }\n");
    printf("tileCell.onmouseenter = function(obj) {\n");
    printf("if (mouseDown) mapCellClick(obj.srcElement.id, mouseDown);\n");
    printf("}\n");
    printf("tileRow.appendChild(tileCell);\n");
    
    printf("tilemap[tile_id].dom_elem = tileCell;\n");
    
    printf("x++;\n");
    printf("}\n");
    printf("x = 0;\n");
    printf("y++;\n");
    printf("}\n");
    printf("}\n");
    
    printf("function updateAllCellElems() {\n");
    printf("for (var tile in tilemap) tilemap[tile].updateElem();\n");
    printf("}\n");
    
    printf("function loadTemplates() {\n");
    printf("paletteDiv.innerHTML = \"\";\n");
    printf("for (var temp in item_templates) {\n");
    printf("var item = item_templates[temp];\n");
    printf("var sprite_url = \"/assets/\" + padSpriteNum(item.item_spr) + \".png\";\n");
    
    printf("// Load image into preview renderer\n");
    printf("prevCanvas.loadImage(sprite_url);\n");
    printf("if (item.type == \"wall\") {\n");
    printf("// Load hidden wall image\n");
    printf("var hidden_spr = item.item_spr + 1;\n");
    printf("prevCanvas.loadImage(\"/assets/\" + padSpriteNum(hidden_spr) + \".png\");\n");
    printf("}\n");
    
    printf("var temp_cell = document.createElement(\"div\");\n");
    printf("temp_cell.id = temp;\n");
    printf("temp_cell.className = \"temp-cell\";\n");
    printf("temp_cell.style.backgroundImage = \"url(\" + sprite_url + \")\";\n");
    printf("temp_cell.onmousedown = function(obj) { itemTempClick(obj.srcElement.id); }\n");
    
    printf("var tmp_typespan = document.createElement(\"span\");\n");
    printf("tmp_typespan.id = temp + \"_typespan\";\n");
    printf("tmp_typespan.className = \"temp-typespan unselectable\";\n");
    printf("switch(item.type) {\n");
    printf("case \"floor\": tmp_typespan.innerHTML = \"F\"; break;\n");
    printf("case \"wall\": tmp_typespan.innerHTML = \"W\"; break;\n");
    printf("case \"item\": tmp_typespan.innerHTML = \"It\"; break;\n");
    printf("}\n");
    printf("temp_cell.appendChild(tmp_typespan);\n");
    
    printf("var tmp_colorprev = document.createElement(\"div\");\n");
    printf("tmp_colorprev.id = temp + \"_prev\";\n");
    printf("tmp_colorprev.className = \"temp-cell-color\";\n");
    printf("tmp_colorprev.style.backgroundColor = item.color;\n");
    printf("if (item.type == \"item\") {\n");
    printf("tmp_colorprev.style.borderRadius = \"100%%\";\n");
    printf("tmp_colorprev.style.border = \"2px orange solid\";\n");
    printf("}\n");
    printf("temp_cell.appendChild(tmp_colorprev);\n");
    
    printf("paletteDiv.appendChild(temp_cell);\n");
    printf("item.dom_elem = temp_cell;\n");
    printf("}\n");
    printf("}\n");
    
    printf("var selected_item = null;\n");
    printf("var selected_item_elem = null;\n");
    printf("function itemTempClick(temp_id) {\n");
    printf("if (!item_templates.hasOwnProperty(temp_id)) return;\n");
    
    printf("if (shiftDown) {\n");
    printf("var it_temp = item_templates[temp_id];\n");
    
    printf("if (document.getElementById(\"inp-apply-colpicker\").checked) {\n");
    printf("it_temp.color = pickr_button.style.backgroundColor;\n");
    printf("}\n");
    
    printf("if (it_temp.type != \"floor\") {\n");
    printf("if (document.getElementById(\"rad-settype-wall\").checked) it_temp.type = \"wall\";\n");
    printf("else if (document.getElementById(\"rad-settype-item\").checked) it_temp.type = \"item\";\n");
    printf("}\n");
    printf("it_temp.updateElem();\n");
    printf("updateAllCellElems();\n");
    printf("return;\n");
    printf("}\n");
    
    printf("if (selected_item_elem != null) {\n");
    printf("selected_item_elem.style.border = null;\n");
    printf("}\n");
    printf("selected_item = item_templates[temp_id];\n");
    printf("selected_item_elem = document.getElementById(temp_id);\n");
    printf("if (selected_item_elem) {\n");
    printf("selected_item_elem.style.border = \"1px blue solid\";\n");
    printf("}\n");
    printf("document.getElementById(\"span-seltemp\").innerHTML = \"Selected: \" + selected_item.temp_id;\n");
    printf("}\n");
    
    printf("function applyTempFilter() {\n");
    printf("var type = \"\";\n");
    printf("var min = 0;\n");
    printf("var max = 0;\n");
    
    printf("// Get type filter\n");
    printf("if (document.getElementById(\"rad-type-wall\").checked) type = \"wall\";\n");
    printf("else if (document.getElementById(\"rad-type-floor\").checked) type = \"floor\";\n");
    printf("else if (document.getElementById(\"rad-type-item\").checked) type = \"item\";\n");
    
    printf("// Get min/max temp id filter\n");
    printf("var tmp_val = document.getElementById(\"inp-minitem\").value;\n");
    printf("if (tmp_val.match(/^[0-9]+$/)) min = tmp_val;\n");
    
    printf("tmp_val = document.getElementById(\"inp-maxitem\").value;\n");
    printf("if (tmp_val.match(/^[0-9]+$/)) max = tmp_val;\n");
    
    printf("var temp_elems = document.getElementsByClassName(\"temp-cell\");\n");
    printf("for (var temp of temp_elems) {\n");
    printf("if (!item_templates.hasOwnProperty(temp.id)) continue;\n");
    
    printf("var hide = 0;\n");
    printf("var it_temp = item_templates[temp.id];\n");
    printf("if (type != \"\" && it_temp.type != type) hide = 1;\n");
    printf("if (max > 0 && it_temp.temp_id > max) hide = 1;\n");
    printf("if (min > 0 && it_temp.temp_id < min) hide = 1;\n");
    
    printf("if (hide) temp.style.display = \"none\";\n");
    printf("else temp.style.display = \"\";\n");
    printf("}\n");
    printf("}\n");
    
    printf("function mapCellClick(id, clickType) {\n");
    printf("if (!tilemap.hasOwnProperty(id)) return;\n");
    
    printf("if (clickType == 1) {\n");
    printf("if (ctrlDown) {\n");
    printf("// Pick tile\n");
    printf("if (tilemap[id].item) itemTempClick(\"it_temp\" + tilemap[id].item);\n");
    printf("else if (tilemap[id].floor) itemTempClick(\"it_temp\" + tilemap[id].floor);\n");
    printf("return;\n");
    printf("} else if (selected_item) {\n");
    printf("if (selected_item.type == \"floor\") {\n");
    printf("tilemap[id].floor = selected_item.temp_id;\n");
    printf("} else {\n");
    printf("tilemap[id].item = selected_item.temp_id;\n");
    printf("}\n");
    printf("}\n");
    printf("} else if (clickType == 2) {\n");
    printf("// Remove item/wall\n");
    printf("tilemap[id].item = 0;\n");
    printf("}\n");
    printf("tilemap[id].updateElem();\n");
    printf("renderPreview();\n");

    printf("var it_temp_val = 0;\n");
    printf("var tmp_type = \"NA\";\n");
    printf("if (clickType == 1 && selected_item) {\n");
    printf("tmp_type = selected_item.type;\n");
    printf("switch(tmp_type) {\n");
    printf("case \"floor\": it_temp_val = selected_item.item_spr; break;\n");
    printf("case \"wall\":\n");
    printf("case \"item\": it_temp_val = selected_item.temp_id; break;\n");
    printf("}\n");
    printf("} else if (clickType == 2) {\n");
    printf("tmp_type = \"remove\";\n");
    printf("} $.ajax({\n");
    printf("url: \"/cgi-imp/mapper.cgi?step=2\",\n");
    printf("type: \"GET\",\n");
    printf("data: { x: tilemap[id].map_x, y: tilemap[id].map_y, it_type: tmp_type, it_val: it_temp_val },\n");
    printf("dataType: \"html\"});\n");
    printf("}\n");
    
    printf("function saveTempConfig() {\n");
    printf("var dl_elem = document.getElementById(\"downloadAnchorElem\");\n");
    printf("var dataStr = \"data:text/json;charset=utf-8,\" + encodeURIComponent(JSON.stringify(item_templates, 0, 4));\n");
    printf("dl_elem.setAttribute(\"href\", \"data:\"+dataStr);\n");
    printf("dl_elem.setAttribute(\"download\", \"template_config.json\");\n");
    printf("dl_elem.click();\n");
    printf("}\n");
    
    printf("document.getElementById(\"inp-configfile\").onchange = function(e) {\n");
    printf("console.log(e.srcElement.value);\n");
    
    printf("if (e.srcElement.value.substr(-5) != \".json\") {\n");
    printf("alert(\"Invalid file type (expecting .json)\");\n");
    printf("return;\n");
    printf("}\n");
    
    printf("var reader = new FileReader();\n");
    printf("reader.onload = function(ev_onload) {\n");
    printf("var jsonData = JSON.parse(ev_onload.target.result);\n");
    
    printf("for (var temp in jsonData) {\n");
    printf("var loaded_temp = jsonData[temp];\n");
    printf("if (item_templates.hasOwnProperty(temp)) {\n");
    printf("item_templates[temp].type = loaded_temp.type;\n");
    printf("item_templates[temp].color = loaded_temp.color;\n");
    printf("item_templates[temp].updateElem();\n");
    printf("}\n");
    printf("}\n");
    printf("updateAllCellElems();\n");
    printf("};\n");
    printf("reader.readAsText(e.target.files[0]);\n");
    printf("}\n");
    
    printf("var hideWalls = false;\n");
    printf("function toggleWalls() {\n");
    printf("hideWalls = !hideWalls;\n");
    printf("renderPreview();\n");
    printf("}\n");

    printf("function renderPreview() {\n");
    printf("prevCanvas.clearContext();\n");

    printf("var cam_x = prevCanvas.cv.width / 2 - prevCanvas.drawXOffset;\n");
    printf("var cam_y = prevCanvas.cv.height / 2 - prevCanvas.drawYOffset;\n");
    printf("var x1 = Math.max(0, Math.round((cam_x - prevCanvas.cv.width - cam_y*2) / 32));\n");
    printf("var x2 = Math.max(0, Math.min(tilemap_width - 1, Math.round((cam_x + prevCanvas.cv.width - cam_y*2) / 32)));\n");
    printf("var y1 = Math.max(0, Math.round((cam_y - prevCanvas.cv.height + cam_x/2) / 16));\n");
    printf("var y2 = Math.max(0, Math.min(tilemap_height - 1, Math.round((cam_y + prevCanvas.cv.height + cam_x/2) / 16)));\n");
    
    printf("for (var i = y1; i < y2; i++) {\n");
    printf("for (var j = x2; j >= x1; j--) {\n");
    printf("var tile_id = j + i * tilemap_width;\n");
    printf("if (!tilemap.hasOwnProperty(\"maptile\" + tile_id)) continue;\n");
    printf("var tile = tilemap[\"maptile\" + tile_id];\n");
    
    printf("// Floor\n");
    printf("if (tile.floor) {\n");
    printf("prevCanvas.drawImageIsometric(tile.getFloorSprite(), j, i);\n");
    printf("}\n");
    printf("// Item/wall\n");
    printf("if (tile.item) {\n");
    printf("var it_spr = tile.getItemSprite();\n");
    printf("if (it_spr) prevCanvas.drawImageIsometric(it_spr, j, i);\n");
    printf("}\n");
    printf("}\n");
    printf("}\n");
    printf("}\n");
    
    printf("function toggleDiv(tgt_div) {\n");
    printf("var tempFilterDiv = document.getElementById(tgt_div);\n");
    printf("if (tempFilterDiv.style.display == \"none\") {\n");
    printf("tempFilterDiv.style.display = null;\n");
    printf("} else {\n");
    printf("tempFilterDiv.style.display = \"none\";\n");
    printf("}\n");
    printf("}\n");
    
    printf("var viewdivs_mode = 0;\n");
    printf("function toggleViewDivs() {\n");
    printf("viewdivs_mode = (viewdivs_mode + 1) %% 3;\n");
    
    printf("var div_mapgrid = document.getElementById(\"div-grid\");\n");
    printf("switch(viewdivs_mode) {\n");
    printf("case 0:\n");
    printf("div_mapgrid.style.display = null;\n");
    printf("prevCanvas.cv.style.display = null;\n");
    printf("break;\n");
    printf("case 1:\n");
    printf("prevCanvas.cv.style.display = \"none\";\n");
    printf("div_mapgrid.style.display = null;\n");
    printf("break;\n");
    printf("case 2:\n");
    printf("div_mapgrid.style.display = \"none\";\n");
    printf("prevCanvas.cv.style.display = null;\n");
    printf("break;\n");
    printf("}\n");
    printf("renderPreview();\n");
    printf("}\n");
    
    printf("var grid_enabled = true;\n");
    printf("function toggleMapGrid() {\n");
    printf("grid_enabled = !grid_enabled;\n");
    
    printf("var cells = document.getElementsByClassName(\"tile-cell\");\n");
    printf("if (grid_enabled) {\n");
    printf("for (var c of cells) {\n");
    printf("c.style.border = null;\n");
    printf("}\n");
    printf("} else {\n");
    printf("for (var c of cells) c.style.border = \"none\";\n");
    printf("}\n");
    printf("}\n");
    
    printf("toggleDiv(\"div-filter\");\n");
    printf("toggleDiv(\"div-temp-picker\");\n");
    
    printf("loadTemplates();\n");
    printf("</script>\n");
}

void scriptPrevCanvas()
{
    printf("<script>\n");
    printf("prevCanvas.cv.addEventListener('mousedown', canvasMouseClick, false);\n");
    printf("prevCanvas.cv.addEventListener('mousemove', canvasMouseMove, false);\n");

    printf("var clickX = 0, clickY = 0;\n");
    printf("var cameraX = 0, cameraY = 0;\n");
    printf("var cam_startx = cameraX, cam_starty = cameraY;\n");
    //printf("var dragRender = 1;\n");

    printf("function canvasMouseMove(event) {\n");
    printf("if (mouseDown) {\n");
    //printf("dragRender = 0;\n");
    //printf("setTimeout(() => { dragRender = 1; }, 100);\n");
    printf("cameraX = cam_startx - (clickX - event.clientX);\n");
    printf("cameraY = cam_starty - (clickY - event.clientY);\n");
    printf("prevCanvas.drawXOffset = 200 + cameraX;\n");
    printf("prevCanvas.drawYOffset = 200 + cameraY;\n");

    printf("renderPreview();\n");
    printf("}\n");
    printf("}\n");

    printf("function canvasMouseClick(event) {\n");
    printf("// Shift+click to reset camera\n");
    printf("if (shiftDown) {\n");
    printf("cameraX = 0;\n");
    printf("cameraY = 0;\n");
    printf("prevCanvas.drawXOffset = 200;\n");
    printf("prevCanvas.drawYOffset = 200;\n");
    printf("renderPreview();\n");
    printf("}\n");
    printf("clickX = event.clientX;\n");
    printf("clickY = event.clientY;\n");
    printf("cam_startx = cameraX;\n");
    printf("cam_starty = cameraY;\n");
    printf("};\n");
    printf("</script>\n");
}

void scriptCPicker()
{
    printf("<script>\n");
    printf("const pickr = Pickr.create({\n");
    printf("el: '.color-picker',\n");
    printf("theme: 'nano',\n");
    printf("useAsButton: true,\n");
    printf("lockOpacity: true,\n");
    printf("comparison: false,\n");

    printf("default: '#000000',\n");

    printf("components: {\n");
    printf("// Main components\n");
    printf("preview: true,\n");
    printf("hue: true,\n");

    printf("// Input / output Options\n");
    printf("interaction: {\n");
    printf("hex: true,\n");
    printf("rgba: true,\n");
    printf("input: true,\n");
    printf("save: true\n");
    printf("}\n");
    printf("}\n");
    printf("});\n");

    printf("const pickr_button = document.getElementById('button-temp-pickr');\n");
    printf("pickr.on('change', (color, instance) => {\n");
    printf("var col = color.toRGBA();\n");
    printf("pickr_button.style.backgroundColor = `rgb(${col[0]}, ${col[1]}, ${col[2]})`;\n");
    printf("});\n");
    printf("</script>\n");
}

void addQueueInstruction(char type, int x, int y, int it_val)
{
    for (int i=0; i<MAX_MAPED_QUEUE; i++) {
        if (maped_queue[i].used == USE_ACTIVE) continue;

        maped_queue[i].op_type = type;
        maped_queue[i].x = x;
        maped_queue[i].y = y;
        maped_queue[i].it_temp = it_val;
        maped_queue[i].used = USE_ACTIVE;
        break;
    }
}

int main(int argc, char *args[])
{
    LIST *head;
    head = is_form_empty() ? NULL : cgi_input_parse();

    load();

    // Head
    printf("Content-Type: text/html\n\n\n");
    printf("<html><head>\n");
    printf("<meta content=\"text/html;charset=utf-8\" http-equiv=\"Content-Type\">\n");
    printf("<title>v2 Map Editor</title>\n");
    cssSetup();
    printf("<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/@simonwep/pickr/dist/themes/nano.min.css\"/>\n");
    printf("<script src=\"https://cdn.jsdelivr.net/npm/@simonwep/pickr/dist/pickr.min.js\"></script>\n");
    printf("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script>\n");
    printf("</head>\n");

    // Body
    printf("<body ondragstart=\"return false;\" ondrop=\"return false;\">\n");
    printf("<a id=\"downloadAnchorElem\" style=\"display:none\"></a>\n");
    printf("<div id=\"div-flexmain\">\n");
    printf("<div id=\"div-palette\">\n");
    printf("<div style=\"display: flex; flex-direction: column; height: 100%%;\">\n");
    printf("<div id=\"div-mapload\">\n");
    printf("<button id=\"button-mapload\" onclick=\"loadMapArea()\">Load Area</button>\n");
    printf("X1:<input id=\"inp-map-x1\" style=\"width:40px\" type=\"number\" maxlength=\"4\">\n");
    printf("Y1:<input id=\"inp-map-y1\" style=\"width:40px\" type=\"number\" maxlength=\"4\"><br>\n");
    printf("X2:<input id=\"inp-map-x2\" style=\"width:40px\" type=\"number\" maxlength=\"4\">\n");
    printf("Y2:<input id=\"inp-map-y2\" style=\"width:40px\" type=\"number\" maxlength=\"4\">\n");
    printf("</div>\n");
    printf("<div id=\"div-toolbox\">\n");
    printf("<button class=\"toolbox-button\" style=\"background-image: url('/assets/ui/hidew.png')\" onclick=\"toggleWalls()\" title=\"Hide walls\"></button>\n");
    printf("<button class=\"toolbox-button\" style=\"background-image: url('/assets/ui/filter.png');\" onclick=\"toggleDiv('div-filter')\" title=\"Filter item templates\"></button>\n");
    printf("<button class=\"toolbox-button\" style=\"background-image: url('/assets/ui/togglegrid.png');\" onclick=\"toggleMapGrid()\" title=\"Toggle map grid\"></button>\n");
    printf("<button class=\"toolbox-button\" style=\"background-image: url('/assets/ui/cpicker.png');\" onclick=\"toggleDiv('div-temp-picker')\" title=\"Change template type and color\"></button>\n");
    printf("<button class=\"toolbox-button\" style=\"background-image: url('/assets/ui/toggle_divs.png');\" onclick=\"toggleViewDivs()\" title=\"Toggle between grid-only and preview-only\"></button>\n");
    printf("<span id=\"span-seltemp\" style=\"color:wheat;font-size:10pt;margin-left:2px;\">Selected: </span>\n");
    printf("</div>\n");
    printf("<div id=\"div-temp-picker\">\n");
    printf("<div style=\"display: flex;width: 100%%;justify-content: space-evenly;align-items:center;\">\n");
    printf("<input id=\"inp-apply-colpicker\" type=\"checkbox\">\n");
    printf("<button id=\"button-temp-pickr\" class=\"toolbox-button rb-border color-picker\"></button>\n");
    printf("<div style=\"width: 100%%;display: flex;flex-direction: column;text-align: center;flex: 1;\">\n");
    printf("Set template type to:\n");
    printf("<div style=\"width: 100%%;\">\n");
    printf("<input id=\"rad-settype-na\" type=\"radio\" name=\"rad-temp-type\" checked>N/A\n");
    printf("<input id=\"rad-settype-wall\" type=\"radio\" name=\"rad-temp-type\">Wall\n");
    printf("<input id=\"rad-settype-item\" type=\"radio\" name=\"rad-temp-type\">Item\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div style=\"margin-top: 8px;\">\n");
    printf("<button style=\"padding: 0;margin: 4px;\" onclick=\"saveTempConfig()\">Save config</button>\n");
    printf("<button style=\"padding: 0;margin: 4px;\" onclick=\"document.getElementById('inp-configfile').click()\">Load config</button>\n");
    printf("<input id=\"inp-configfile\" type=\"file\" accept=\".json\" style=\"display: none\">\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div id=\"div-filter\">\n");
    printf("<div>\n");
    printf("Type:\n");
    printf("<input id=\"rad-type-na\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\" checked>N/A\n");
    printf("<input id=\"rad-type-wall\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\">Wall\n");
    printf("<input id=\"rad-type-floor\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\">Floor\n");
    printf("<input id=\"rad-type-item\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\">Item\n");
    printf("</div>\n");
    printf("<div style=\"margin-top:4px\">\n");
    printf("Item ID:\n");
    printf("<input id=\"inp-minitem\" type=\"number\" style=\"width:32px\" onchange=\"applyTempFilter()\">~\n");
    printf("<input id=\"inp-maxitem\" type=\"number\" style=\"width:32px\" onchange=\"applyTempFilter()\">\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div id=\"div-palette-container\"></div>\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div id=\"div-canvastable\">\n");
    printf("<div id=\"div-grid\" class=\"cv-base\">\n");
    printf("<table id=\"tiles-table\"></table>\n");
    printf("</div>\n");
    printf("<canvas id=\"cv-preview\" class=\"cv-base\"></canvas>\n");
    printf("</div>\n");
    printf("</div>\n");

    scriptCPicker();
    scriptMapper();
    scriptPrevCanvas();

    int step = 0;
    if (head) {
        char *tmp = find_val(head,"step");
        if (tmp) step = atoi(tmp);
    }

    int x1, y1, x2, y2;
    int x = 0, y = 0;
    char *it_type;
    switch(step) {
        case 1: // Load map area, receives coordinates as "x1", "y1", "x2", "y2"
            x1 = atoi(find_val(head,"x1"));
            y1 = atoi(find_val(head,"y1"));
            x2 = atoi(find_val(head,"x2"))+1;
            y2 = atoi(find_val(head,"y2"))+1;

            if (x1 < 0 || x1 > 1024 || x1 > x2 || y1 < 0 || y1 > 1024 || y1 > y2 ||
                x2 < 0 || x2 > 1024 || y2 < 0 || y2 > 1024) {
                    printf("<script>alert(\"Received invalid input for map area.\");</script>\n");
                    break;
            }
            printf("<script>loadMapCells(%d, %d, %d, %d);\n", x1, y1, x2, y2);

            for (int i=x1; i<x2; i++) {
                for (int j=y1; j<y2; j++) {
                    int map_tileid = i + j * MAPX;
                    if (map_tileid < 0 || map_tileid > MAPX * MAPY) continue;

                    printf("var cell_tileid = %d + %d * tilemap_width;\n", x, y);
                    printf("if (tilemap.hasOwnProperty(\"maptile\" + cell_tileid)) {\n");
                    printf("var tile = tilemap[\"maptile\" + cell_tileid];\n");

                    if (map[map_tileid].sprite) {
                        int fl_id = 100000 + map[map_tileid].sprite;
                        printf("tile.floor = %d;\n", fl_id);
                    }

                    int it_id = map[map_tileid].it;
                    if (it_id > 1 && it[it_id].used != USE_EMPTY) {
                        printf("tile.item = %d;\n", it[it_id].temp);
                    } else {
                        // Find item template with the same sprite (since it's not stored in map.it, it shouldn't be an usable/take-able item)
                        for (int in=2; in<MAXTITEM; in++) {
                            if (it_temp[in].used == USE_EMPTY) continue;

                            if (it_temp[in].sprite[0] == map[map_tileid].fsprite) {
                                printf("tile.item = %d;\n", in);
                                break;
                            }
                        }
                    }
                    printf("}\n");

                    x++;
                }
                x = 0;
                y++;
            }
            printf("renderPreview(); updateAllCellElems();</script>\n");
        break;

        case 2: // Process tile instruction
            x = atoi(find_val(head, "x"));
            y = atoi(find_val(head, "y"));
            x1 = atoi(find_val(head, "it_val"));
            it_type = find_val(head, "it_type");

            if (x<0 || x>MAPX || y<0 || y>MAPY) break;

            if (strcmp(it_type, "floor") == 0) {
                //map[x + y * MAPX].sprite = x1;
                addQueueInstruction(MAPED_SETFLOOR, x, y, x1);

            } else if (strcmp(it_type, "wall") == 0) {
                /*map[x + y * MAPX].fsprite = 0;
                map[x + y * MAPX].flags&=~MF_MOVEBLOCK;
                map[x + y * MAPX].flags&=~MF_SIGHTBLOCK;

                if (x1 < 2 || x1 >= MAXTITEM) break;
                if (it_temp[x1].used == USE_EMPTY) break;

                // Remove old item
                y1 = map[x + y * MAPX].it;
                if (y1) {
                    it[y1].used = USE_EMPTY;
                    map[x + y * MAPX].it = 0;
                }

                map[x + y * MAPX].fsprite = it_temp[x1].sprite[0];
                if (it_temp[x1].flags&IF_MOVEBLOCK) map[x + y * MAPX].flags|=MF_MOVEBLOCK;
                if (it_temp[x1].flags&MF_SIGHTBLOCK) map[x + y * MAPX].flags|=MF_SIGHTBLOCK;*/
                if (map[x + y * MAPX].it) {
                    addQueueInstruction(MAPED_RMVITEM, x, y, 0);
                }
                addQueueInstruction(MAPED_PLACEITEM, x, y, x1);
            } else if (strcmp(it_type, "item") == 0) {
                // Remove old item
                /*y1 = map[x + y * MAPX].it;
                if (y1) {
                    it[y1].used = USE_EMPTY;
                    map[x + y * MAPX].it = 0;
                }
                //TODO - handle light when removing item? (do_add_light is used in build.c->build_remove)

                // Create item
                if (x1 < 2 || x1 >= MAXTITEM) break;
                if (it_temp[x1].used == USE_EMPTY) break;
                if (it_temp[x1].driver == 33) break; //TODO - ability to place pentagrams?
                
                for (x2=1; x2<MAXITEM; x2++) {
                    if (it[x2].used == USE_EMPTY) break;
                }
                if (x2 == MAXITEM) break;
                it[x2] = it_temp[x1];
                it[x2].temp = x1;

                map[x + y * MAPX].it = x2;

                it[x2].instance_id = -1;
                it[x2].x = x;
                it[x2].y = y;
                it[x2].carried = 0;*/
                if (map[x + y * MAPX].it) {
                    addQueueInstruction(MAPED_RMVITEM, x, y, 0);
                }
                addQueueInstruction(MAPED_PLACEITEM, x, y, x1);
            } else if (strcmp(it_type, "remove") == 0) {
                /*y1 = map[x + y * MAPX].it;
                if (y1) {
                    it[y1].used = USE_EMPTY;
                    map[x + y * MAPX].it = 0;
                }

                map[x + y * MAPX].fsprite = 0;
                map[x + y * MAPX].flags&=~MF_MOVEBLOCK;
                map[x + y * MAPX].flags&=~MF_SIGHTBLOCK;*/
                addQueueInstruction(MAPED_RMVITEM, x, y, 0);
            }
        break;
    }
    
    unload();

    printf("</body>\n");
    printf("</html>\n");
    return 0;
}