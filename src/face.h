#ifndef __EDITOR_FACE__
#define __EDITOR_FACE__

#include "sdk/amxxmodule.h"
#include "defs.h"
#include <vector>

#include "textures.h"
extern textureList * g_textures;

class editorFace{
private:
	std::vector<Vector> vertices;
	/*Vector * vertices;
	int vertices_c;*/
	Vector normal;
	Vector axis_u;
	Vector axis_v;
	float offset_u;
	float offset_v;
	float rotation;
	float scale_u;
	float scale_v;

	void calcUV(bool);
public:
	int texture;
	editorFace();
	editorFace(Vector,Vector,Vector,Vector);
	editorFace(FILE * f); //load from file
	bool intersect(Vector origin, Vector direction);
	void move(Vector offset);
	void toFile(FILE * f);
	void toMAP(FILE * f);
	~editorFace();
};

#endif
