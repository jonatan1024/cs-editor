#ifndef __EDITOR_FGDCLASSES__
#define __EDITOR_FGDCLASSES__

#include "sdk/amxxmodule.h"

#include "entity.h"

#ifndef _DEF_FGDCLASSTYPE_
#define _DEF_FGDCLASSTYPE_
enum fgdClassType{
	BaseClass = 0,
	PointClass,
	SolidClass
};
#endif

class fgdClasses{
private:
	void handleGET(char * req, editorEntity * ent, int uid);
public:
	fgdClasses();
	~fgdClasses();

	int getNextClass(int index, fgdClassType type, char ** name, char ** desc);
	bool getSizeColor(int index, Vector * mins, Vector * maxs, Vector * color);
	char * craftResponse(char * req, int * len, char * buffer, int pos);
	int getClassIndex(char * name);
	char * getClassName(int index);
};

#endif