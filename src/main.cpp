#include "sdk/amxxmodule.h"

#include <stdio.h>

#include "brush.h"
#include "world.h"
#include "defs.h"
#include "textures.h"
#include "httpserver.h"
#include "fgdclasses.h"
#include "entity.h"
#include "compiler.h"

editorWorld * g_world;
textureList * g_textures;
httpServer * g_server = NULL;
fgdClasses * g_fgdclasses;
char g_gamedir[16];
bool g_precached = false;

extern int g_compilerstatus;

int DispatchSpawn(edict_t *pEntity)
{
	if (!g_precached) {
		const char models[][32] = {BRUSH_MODELNAME};
		bool ded = false;//IS_DEDICATED_SERVER();
		for(int i = 0; i < sizeof(models)/32; i++){
			PRECACHE_MODEL((char*)STRING(ALLOC_STRING(models[i])));	//precache("ed_brush.mdl")
			//PRECACHE_MODEL((char*)models[i]);
		}
		g_precached = true;
	}
	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void StartFrame_Post(){
	if(g_server)
		g_server->tick();
	if(int stamp = editor_compiled()){
		char cmd[32];
		sprintf(cmd, "changelevel ed_%x\n", stamp);
		SERVER_COMMAND(cmd);
	}
}

void ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax)
{
	g_world->reloadModels();
}

void ServerDeactivate_Post(){
	g_precached = false;
}

static cell AMX_NATIVE_CALL editor_brush_add(AMX *amx, cell *params){
	return g_world->addBrush(new editorBrush(
		params[1],
		CELLS2VEC(MF_GetAmxAddr(amx, params[2])),
		CELLS2VEC(MF_GetAmxAddr(amx, params[3])),
		params[4]
	));
}

static cell AMX_NATIVE_CALL editor_entity_add(AMX *amx, cell *params){
	return g_world->addBrush(new editorEntity(
		params[1],
		CELLS2VEC(MF_GetAmxAddr(amx, params[2])),
		params[3]
	));
}

static cell AMX_NATIVE_CALL editor_brush_remove(AMX *amx, cell *params){
	return g_world->removeBrush(params[1]);
}

static cell AMX_NATIVE_CALL editor_brush_chckown(AMX *amx, cell *params){
	return g_world->checkBrushOwner(params[1], params[2]);
}

static cell AMX_NATIVE_CALL editor_brush_getown(AMX *amx, cell *params){
	return g_world->getBrushOwner(params[1]);
}

static cell AMX_NATIVE_CALL editor_brush_getbbox(AMX *amx, cell *params){
	Vector mins, maxs;
	if(!g_world->getBrushBBox(params[1], &mins, &maxs))
		return false;
	cell* c_mins = MF_GetAmxAddr(amx, params[2]);
	cell* c_maxs = MF_GetAmxAddr(amx, params[3]);
	for(int i = 0; i < 3; i++){
		c_mins[i] = mins[i];
		c_maxs[i] = maxs[i];
	}
	return true;
}

static cell AMX_NATIVE_CALL editor_brush_move(AMX *amx, cell *params){
	return g_world->moveBrush(params[1], CELLS2VEC(MF_GetAmxAddr(amx, params[2])));
}

static cell AMX_NATIVE_CALL editor_brush_clone(AMX *amx, cell *params){
	return g_world->cloneBrush(params[1]);
}

static cell AMX_NATIVE_CALL editor_world_raycast(AMX *amx, cell *params){
	return g_world->rayCast(
		FLOATS2VEC(MF_GetAmxAddr(amx, params[1])),
		FLOATS2VEC(MF_GetAmxAddr(amx, params[2])),
		(int*)&params[3]
	);
}

static cell AMX_NATIVE_CALL editor_world_compile(AMX *amx, cell *params){
	g_world->compile();
	return true;
}

static cell AMX_NATIVE_CALL editor_compiler_status(AMX *amx, cell *params){
	return g_compilerstatus;
}

static cell AMX_NATIVE_CALL editor_texture_getname(AMX *amx, cell *params){
	texlist_t * tex;
	if(!g_textures->IDWadTex(params[1], NULL, &tex))
		return false;
	MF_SetAmxString(amx, params[2], tex->texname, 16);
	return true;
}

static cell AMX_NATIVE_CALL editor_classes_getnext(AMX *amx, cell *params){
	char * name;
	char * desc;
	int ret = g_fgdclasses->getNextClass(params[1], (fgdClassType)params[2], &name, &desc);
	if(ret == -1)
		return -1;
	MF_SetAmxString(amx, params[3], name, 32);
	MF_SetAmxString(amx, params[4], desc, 64);
	return ret;
}

static cell AMX_NATIVE_CALL editor_classes_getname(AMX *amx, cell *params){
	MF_SetAmxString(amx, params[2], g_fgdclasses->getClassName(params[1]), 32);
	return true;
}

AMX_NATIVE_INFO amxxfunctions[] = {
	{"editor_brush_add", editor_brush_add},
	{"editor_entity_add", editor_entity_add},
	{"editor_brush_remove", editor_brush_remove},
	{"editor_brush_chckown", editor_brush_chckown},
	{"editor_brush_getown", editor_brush_getown},
	{"editor_brush_getbbox", editor_brush_getbbox},
	{"editor_brush_move", editor_brush_move},
	{"editor_brush_clone", editor_brush_clone},

	{"editor_world_raycast", editor_world_raycast},
	{"editor_world_compile", editor_world_compile},

	{"editor_compiler_status", editor_compiler_status},

	{"editor_texture_getname", editor_texture_getname},

	{"editor_classes_getnext", editor_classes_getnext},
	{"editor_classes_getname", editor_classes_getname},

	{NULL,NULL}
};

void OnAmxxAttach(){
	ED_LOG("[CS-EDITOR] attached");

	GET_GAME_DIR(g_gamedir);
	g_world = new editorWorld();
	g_textures = new textureList();
	g_server = new httpServer();
	if(!g_server->start()){
		delete g_server;
		g_server = NULL;
	}
	g_fgdclasses = new fgdClasses();

	MF_AddNatives(amxxfunctions);
	RETURN_META(MRES_IGNORED);
}

void OnPluginsLoaded(){
	int forward =  MF_RegisterForward("editor_texturebrowser_set", ET_IGNORE, FP_CELL, FP_CELL, FP_DONE);
	g_server->setForward(forward);
}