#ifndef __EDITOR_WORLD__
#define __EDITOR_WORLD__

#include "brush.h"
#include "entity.h"
#include <stdlib.h>
#include <time.h>

class editorWorld{
private:
	editorBrush**list;
	int list_c;

	inline bool brushOK(int brush);
	void loadFromFile(char * filename);
	void saveToFile(char * filename);
	void saveToMAP(char * filename);
public:
	editorWorld();
	int addBrush(editorBrush* brush);
	int cloneBrush(int brush);
	editorBrush * getBrush(int brush);
	editorEntity * getEntity(int entity);
	bool removeBrush(int brush);
	bool checkBrushOwner(int owner, int brush);
	int getBrushOwner(int brush);
	int rayCast(Vector origin, Vector direction, int * face);
	bool getBrushBBox(int brush, Vector * mins, Vector * maxs);
	bool moveBrush(int brush, Vector offset);
	void compile();
	void reloadModels();
	~editorWorld();
};
#endif