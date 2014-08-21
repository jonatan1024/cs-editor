#include "world.h"

#include "compiler.h"
#include "fgdclasses.h"
extern fgdClasses * g_fgdclasses;

//TODO: AABB tree?

editorWorld::editorWorld(){
	list_c = 0;
	list = NULL;

	char filename[64];
	sprintf(filename,"%s/maps/ed_latest",g_gamedir);
	if(FILE * f = fopen(filename,"r")){
		fclose(f);
		//yep, openin' 'n' closin' just for shits 'n' giggles
		loadFromFile(filename);
	}
}

int editorWorld::addBrush(editorBrush*obj){
	list_c++;
	if(list = (editorBrush**)realloc(list,sizeof(editorBrush*)*list_c)){
		list[list_c-1]=obj;
		return list_c-1;
	}
	return -1;
}

int editorWorld::cloneBrush(int brush){
	if(!brushOK(brush))
		return 0;
	editorBrush * obj;
	if(list[brush]->entType){
		editorEntity * ent = (editorEntity*)list[brush];
		obj = new editorEntity(*ent);
	}else{
		obj = new editorBrush(*list[brush]);
	}
	return addBrush(obj);
}

editorBrush * editorWorld::getBrush(int brush){
	if(brushOK(brush))
		return list[brush];
	return NULL;
}

editorEntity * editorWorld::getEntity(int entity){
	editorEntity * ent = (editorEntity*)getBrush(entity);
	if(ent && ent->entType)
		return ent;
	return NULL;
}

inline bool editorWorld::brushOK(int brush){
	return (brush < list_c && brush >= 0);
}

bool editorWorld::removeBrush(int obj){
	if(!brushOK(obj))
		return false;
	delete list[obj];
	list[obj] = list[--list_c];
	return true;
}

bool editorWorld::checkBrushOwner(int owner, int brush){
	if(!brushOK(brush))
		return false;
	return list[brush]->checkOwner(owner);
}

int editorWorld::getBrushOwner(int brush){
	if(!brushOK(brush))
		return 0;
	return list[brush]->getOwner();
}

int editorWorld::rayCast(Vector org, Vector dir, int * face){
	float mindist = FLT_MAX;
	int obj = -1;
	for(int i = 0; i < list_c; i++){
		int l_face;
		float dist = list[i]->intersect(org, dir, &l_face);
		if(dist > 0 && dist < mindist){
			mindist = dist;
			obj = i;
			*face = l_face;
		}
	}
	return obj;
}

bool editorWorld::getBrushBBox(int brush, Vector * mins, Vector * maxs){
	if(!brushOK(brush))
		return false;
	list[brush]->getBBox(mins,maxs);
	return true;
}

bool editorWorld::moveBrush(int brush, Vector offset){
	if(!brushOK(brush))
		return false;
	list[brush]->move(offset);
	return true;
}

void editorWorld::compile(){
	int stamp = time(NULL);

	char latest[64];
	char timestmpd[64];
	sprintf(latest, "%s/maps/ed_latest", g_gamedir);
	strcpy(timestmpd, latest);
	char * pos = timestmpd + strlen(timestmpd) - 6;
	pos += sprintf(pos, "%x", stamp);

	saveToFile(latest);
	CopyFile(latest, timestmpd, false);

	strcpy(pos, ".map");

	saveToMAP(timestmpd);
	editor_compile(timestmpd, stamp);
}

void editorWorld::saveToFile(char * filename){
	FILE * f;
	f = fopen(filename,"wb");
	fwrite(&list_c, sizeof(int), 1, f);
	for(int i = 0; i < list_c; i++){
		fwrite(&list[i]->entType, sizeof(char), 1, f);
		list[i]->toFile(f);
	}
	fclose(f);
}

void editorWorld::loadFromFile(char * filename){
	FILE * f;
	f = fopen(filename, "rb");
	fread(&list_c, sizeof(int), 1, f);
	list = (editorBrush**)malloc(sizeof(editorBrush*)*list_c);
	for(int i = 0; i < list_c; i++){
		char entType;
		fread(&entType, sizeof(char), 1, f);
		if(!entType)
			list[i] = new editorBrush(f);
		else
			list[i] = new editorEntity(f, entType);
	}
	fclose(f);
}

void editorWorld::saveToMAP(char * filename){
	FILE * f;
	f = fopen(filename, "w");

	fputs("{\n", f);
	fputkv("classname", "worldspawn", f);
	fputkv("mapversion", "220", f);
	g_textures->saveMAPwads(f);
	//world brushes
	for(int i = 0; i < list_c; i++)
		if(!list[i]->entType)
			list[i]->toMAP(f);
	//copy world brushes from init map
	char init_fname[16 + sizeof(EDITOR_INITMAP)];
	strcpy(init_fname, g_gamedir);
	strcat(init_fname, EDITOR_INITMAP);
	FILE * finit = fopen(init_fname, "r");
	int count;
	char buffer[256];
	while(count = fread(buffer, 1, sizeof(buffer), finit)){
		fwrite(buffer, 1, count, f);
	}
	fclose(finit);

	fputs("}\n", f);
	
	//entities
	int light = g_fgdclasses->getClassIndex("light_environment");
	bool haveLight = false;

	for(int i = 0; i < list_c; i++){
		if(list[i]->entType){
			list[i]->toMAP(f);
			if(!haveLight){
				editorEntity* ent = (editorEntity*)list[i];
				haveLight = (ent->entclass == light);
			}
		}
	}
	//copy init entity
	if(haveLight)
		fwrite(EDITOR_INITENT, 1, sizeof(EDITOR_INITENT), f);
	else
		fwrite(EDITOR_INITLIGHT, 1, sizeof(EDITOR_INITLIGHT), f);
	fclose(f);
}

void editorWorld::reloadModels(){
	for(int i = 0; i < list_c; i++){
		list[i]->reloadModel();
	}
}

editorWorld::~editorWorld(){
	for(int i = 0; i < list_c; i++)
		delete list[i];
	free(list);
}