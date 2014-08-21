#include "sdk/amxxmodule.h"


//httpserver
#define HTTP_PORT 27015
#define MAX_FILTERED_TEX 768
#define MAX_FIRSTPACKET_LEN 1024
#define MAX_NEXTPACKET_LEN 16384

//brush
#define BRUSH_CLASSNAME "cycler_sprite"
#define BRUSH_MODELNAME "models/editor/ed_brush.mdl"

#define EDITOR_PLACEHOLDER "models/editor/ed_placeholder.mdl"

#define MODEL_COLTEX 6

//textures
#define TEXTURELIST_FILE "/addons/amxmodx/data/editor_texturelist"
#define MAX_WADS 64

//fgdparser
#define EDITORFGD_FILE "/addons/amxmodx/data/editor.fgd"

//main
#define CELLS2VEC(c) Vector(c[0], c[1], c[2])
#define FLOATS2VEC(c) Vector(amx_ctof(c[0]), amx_ctof(c[1]), amx_ctof(c[2]))

#define ADD2COLS(c,p) c[0]+=p[0]; c[1]+=p[1]; c[2]+=p[2];

#define ED_LOG(text) LOG_CONSOLE(PLID, text)

//world
#define fputkv(key, value, file) fprintf(file,"\"%s\" \"%s\"\n", key, value)

extern char g_gamedir[16];

//compiler
#define COMPILERS_PATT "%s/addons/amxmodx/data/editor_compilers/%s.exe %s"
#define VERTICAL_OFFSET 4096
#define EDITOR_INITMAP "/addons/amxmodx/data/editor_init.map"
#define EDITOR_INITENT "{\n"\
"\"classname\" \"info_null\"\n"\
"\"origin\" \"0 0 -2048\"\n"\
"}\n"
#define EDITOR_INITLIGHT "{\n"\
"\"classname\" \"light_environment\"\n"\
"\"_light\" \"255 255 255 200\"\n"\
"\"angles\" \"0 270 0\"\n"\
"\"pitch\" \"-90\"\n"\
"\"_diffuse_light\" \"255 255 128 200\"\n"\
"\"origin\" \"256 -1472 -576\"\n"\
"}\n"