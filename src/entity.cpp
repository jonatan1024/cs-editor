#include "entity.h"

#include "fgdclasses.h"
extern fgdClasses * g_fgdclasses;

void editorEntity::newModel(){
	//awfuly duplicated code
	model = CREATE_NAMED_ENTITY(ALLOC_STRING(BRUSH_CLASSNAME));
	SET_MODEL(model,BRUSH_MODELNAME);
	model->v.origin = mins;
	Vector size = (maxs-mins)/4;
	for(int i = 0; i < 3; i++){
		model->v.controller[i] = size[i];
	}

	//awfuly copied from textures.cpp
	float r = 255.0/(MODEL_COLTEX-1);
	int skin = 0;
	for(int i = 0; i < 3; i++){
		int s = 1;
		if(i==0)
			s = MODEL_COLTEX*MODEL_COLTEX;
		else if(i==1)
			s = MODEL_COLTEX;
		skin += floor((color[i]/r)+0.5)*s;
	}

	model->v.skin = skin;
	MDLL_Spawn(model);
}

void editorEntity::move(Vector offset){
	this->mins = this->mins + offset;
	this->maxs = this->maxs + offset;
	this->origin = this->origin + offset;
	Vector * origin = &(model->v.origin);
	*origin = *origin + offset;
}

void editorEntity::reloadModel(){
	newModel();
}

editorEntity::editorEntity(int owner, Vector origin, int entclass) : editorBrush(owner){
	Vector mins(-8,-8,-8), maxs(8,8,8);
	color = Vector(220, 0, 180);

	g_fgdclasses->getSizeColor(entclass, &mins, &maxs, &color);
	this->mins = mins + origin;
	this->maxs = maxs + origin;
	
	this->entType = 1;
	this->isBox = true;
	this->origin = origin;
	this->entclass = entclass;
	this->flags = -1;

	newModel();
}

editorEntity::editorEntity(const editorEntity& entity) : editorBrush(entity){
	entType = entity.entType;
	color = entity.color;
	origin = entity.origin;
	entclass = entity.entclass;
	flags = entity.entclass;
	attr = entity.attr;

	newModel();
}

void editorEntity::toFile(FILE * f){
	editorBrush::toFile(f);

	fwrite(&entclass, sizeof(int), 1, f);
	fwrite(&flags, sizeof(int), 1, f);

	fwrite(&color, sizeof(Vector), 1, f);
	fwrite(&origin, sizeof(Vector), 1, f);

	int attr_c = attr.size();
	fwrite(&attr_c, sizeof(int), 1, f);
	for(std::vector<entKeyValue>::iterator it = attr.begin(); it != attr.end(); ++it){
		fwrite(&(*it), sizeof(entKeyValue), 1, f);
	}
}

editorEntity::editorEntity(FILE * f, char entType) : editorBrush(f){
	fread(&entclass, sizeof(int), 1, f);
	fread(&flags, sizeof(int), 1, f);

	fread(&color, sizeof(Vector), 1, f);
	fread(&origin, sizeof(Vector), 1, f);

	int attr_c;
	fread(&attr_c, sizeof(int), 1, f);
	for(int i = 0; i < attr_c; i++){
		entKeyValue entry;
		fread(&entry, sizeof(entKeyValue), 1, f);
		attr.push_back(entry);
	}

	this->entType = entType;
}

void editorEntity::toMAP(FILE * f){
	fputs("{\n", f);
	for(std::vector<entKeyValue>::iterator it = attr.begin(); it != attr.end(); ++it){
		fprintf(f, "\"%s\" \"%s\"\n", it->key, it->value);
	}
	if(entType == 2)
		editorBrush::toMAP(f);
	fputs("}\n", f);
}