#include "brush.h"

/*
new box
*/

editorBrush::editorBrush(){
	/*faces = NULL;
	faces_c = 0;*/
	faces.resize(0);
}

editorBrush::editorBrush(int owner){
	setOwner(owner);
	/*faces = NULL;
	faces_c = 0;*/
	faces.resize(0);
}

editorBrush::editorBrush(int owner, Vector mins, Vector maxs, int texture){
	this->isBox = true;
	this->mins = mins;
	this->maxs = maxs;
	this->entType = 0;
	setOwner(owner);

	Vector v[8];
	v[0][0] = mins[0];
	v[0][1] = mins[1];
	v[1][0] = maxs[0];
	v[1][1] = mins[1];
	v[2][0] = maxs[0];
	v[2][1] = maxs[1];
	v[3][0] = mins[0];
	v[3][1] = maxs[1];
	for(int i = 0; i < 4; i++){
		v[i + 4][0] = v[i][0];
		v[i + 4][1] = v[i][1];

		v[i][2] = mins[2];
		v[i + 4][2] = maxs[2];
	}
	faces.resize(6);
	//faces = (editorFace**)malloc(6*sizeof(editorFace*));
	faces[0] = editorFace(v[0],v[1],v[2],v[3]);
	faces[1] = editorFace(v[7],v[6],v[5],v[4]);
	faces[2] = editorFace(v[1],v[0],v[4],v[5]);
	faces[3] = editorFace(v[3],v[2],v[6],v[7]);
	faces[4] = editorFace(v[2],v[1],v[5],v[6]);
	faces[5] = editorFace(v[0],v[3],v[7],v[4]);
	for(int i = 0; i < 6; i++)
		faces[i].texture = texture;
	//faces_c = 6;

	newModel();
}

/*
copy brush
*/
editorBrush::editorBrush(const editorBrush& brush){
	isBox = brush.isBox;
	mins = brush.mins;
	maxs = brush.maxs;
	entType = 0;
	strncpy(steamid, brush.steamid, 32);
	if(brush.entType == 0){
		faces = brush.faces;

		reloadModel();
	}
}


void editorBrush::setOwner(int owner){
	if(owner)
		strncpy(this->steamid, GETPLAYERAUTHID(INDEXENT(owner)), 32);
	else
		this->steamid[0] = 0;
}

int editorBrush::getOwner(){
	for(int i = 1; i < 33; i++){
		if(!strcmp(steamid, GETPLAYERAUTHID(INDEXENT(i))))
			return i;
	}
	return 0;
}

/*
create new model entity
*/
void editorBrush::newModel(){
	model = CREATE_NAMED_ENTITY(ALLOC_STRING(BRUSH_CLASSNAME));
	SET_MODEL(model,BRUSH_MODELNAME);
	model->v.origin = mins;
	Vector size = (maxs-mins)/4;
	for(int i = 0; i < 3; i++){
		model->v.controller[i] = size[i];
	}
	model->v.skin = g_textures->getSkin(faces[0].texture);	//faces[0]->texture should do the trick
	MDLL_Spawn(model);
}
/*
ray-brush intersection
*/
float editorBrush::intersect(Vector org, Vector dir, int * face){
	float distance = intersectBBox(org,dir);
	if(!isBox && distance > 0){
		//todo: intersect with all feces		
		*face = 0;
	}
	return distance;
}

/*
ray-bbox intersection
*/
float editorBrush::intersectBBox(Vector org, Vector dir){
	Vector v1,v2;
	for(int i = 0; i < 3; i++){
		v1[i] = (mins[i] - org[i])/dir[i];
		v2[i] = (maxs[i] - org[i])/dir[i];
	}

	float tmin = max(max(min(v1[0], v2[0]), min(v1[1], v2[1])), min(v1[2], v2[2]));
	float tmax = min(min(max(v1[0], v2[0]), max(v1[1], v2[1])), max(v1[2], v2[2]));
	if (tmax < 0 || tmin < 0)
		return 0;
	if (tmin > tmax)
		return 0;

	return tmin;
}

/*
destroy model entity
*/

void editorBrush::deleteModel(){
	if(!model)
		return;
	REMOVE_ENTITY(model);
	model = NULL;
}

bool editorBrush::checkOwner(int owner){
	return !strcmp(steamid,GETPLAYERAUTHID(INDEXENT(owner)));
}

void editorBrush::getBBox(Vector * mins, Vector * maxs){
	*mins = this->mins;
	*maxs = this->maxs;
}

void editorBrush::move(Vector offset){
	mins = mins + offset;
	maxs = maxs + offset;
	Vector * origin = &model->v.origin;
	*origin = *origin + offset;
	for(unsigned int i = 0; i < faces.size(); i++){
		faces[i].move(offset);
	}
}

void editorBrush::toFile(FILE * f){
	fwrite(&mins, sizeof(Vector), 1, f);
	fwrite(&maxs, sizeof(Vector), 1, f);
	fwrite(&isBox, sizeof(bool), 1, f);
	fwrite(steamid, sizeof(char), 32, f);
	int faces_c = faces.size();
	fwrite(&faces_c, sizeof(int), 1, f);
	for(int i = 0; i < faces_c; i++)
		faces[i].toFile(f);
}

//load from file
editorBrush::editorBrush(FILE * f){
	fread(&mins, sizeof(Vector), 1, f);
	fread(&maxs, sizeof(Vector), 1, f);
	fread(&isBox, sizeof(bool), 1, f);
	fread(steamid, sizeof(char), 32, f);
	int faces_c;
	fread(&faces_c, sizeof(int), 1, f);
	/*faces = (editorFace**)malloc(sizeof(editorFace*)*faces_c);
	for(int i = 0; i < faces_c; i++)
		faces[i] = new editorFace(f);*/
	faces.resize(faces_c);
	for(int i = 0; i < faces_c; i++)
		faces[i] = editorFace(f);

	this->entType = 0;
}

void editorBrush::toMAP(FILE * f){
	fputs("{\n", f);
	for(unsigned int i = 0; i < faces.size(); i++){
		faces[i].toMAP(f);
	}
	fputs("}\n", f);
}

void editorBrush::reloadModel(){
	if(isBox)
		newModel();
	//todo: "else" - reload models on faces
	
}

editorBrush::~editorBrush(){
	deleteModel();
	/*for(unsigned int i = 0; i < faces.size(); i++)
		delete faces[i];*/
	//free(faces);
	faces.clear();
}