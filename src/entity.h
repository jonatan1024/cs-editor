#ifndef __EDITOR_ENTITY__
#define __EDITOR_ENTITY__

#include "sdk/amxxmodule.h"
#include "defs.h"
#include "brush.h"
#include <vector>

typedef struct{
	char key[32];
	char value[64];
}entKeyValue;

class editorEntity: public editorBrush{
private:
	Vector color;
	void newModel();
public:
	Vector origin;

	std::vector<entKeyValue> attr;
	int flags;
	int entclass;

	editorEntity(const editorEntity& entity);
	editorEntity(int owner, Vector origin, int entclass);
	editorEntity(FILE * f, char entType); //load from file

	void move(Vector offset);

	void toFile(FILE * f);
	void toMAP(FILE * f);
	void reloadModel();
};

#endif
