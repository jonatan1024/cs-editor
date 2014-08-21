#ifndef __EDITOR_BRUSH__
#define __EDITOR_BRUSH__

#include "sdk/amxxmodule.h"
#include "defs.h"
#include <limits>
#include "face.h"
#include <vector>
#include "textures.h"
extern textureList * g_textures;

class editorBrush{
protected:
	Vector mins;
	Vector maxs;
	char steamid[32];

	bool isBox;

	edict_t * model;

	float intersectBBox(Vector origin, Vector direction);
	virtual void newModel();
	void deleteModel();
	void setOwner(int owner);
private:
	std::vector<editorFace> faces;
	/*editorFace ** faces;
	int faces_c;*/
public:
	char entType;	//0: worldBrsh, 1: pointEnt, 2: brushEnt

	editorBrush();
	editorBrush(const editorBrush& brush);
	editorBrush(int owner);
	editorBrush(int owner, Vector mins, Vector maxs, int texture);
	editorBrush(FILE * f); //load from file
	float intersect(Vector origin, Vector direction, int * face);
	bool checkOwner(int owner);
	int getOwner();
	void getBBox(Vector * mins, Vector * maxs);
	virtual void move(Vector offset);
	
	virtual void toFile(FILE * f);
	virtual void toMAP(FILE * f);
	virtual void reloadModel();
	~editorBrush();
};

#endif
