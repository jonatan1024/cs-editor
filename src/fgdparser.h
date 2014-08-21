#ifndef __EDITOR_FGDPARSER__
#define __EDITOR_FGDPARSER__


#include <vector>

typedef struct {
	int val;
	char desc[32];
	bool def;
} fgdFlags;

typedef struct {
	char val[64];
	char desc[64];
} fgdChoices;

typedef struct {
	char key[32];
	char desc[64];
	char val[64];
	std::vector<fgdChoices> choices;
} fgdKeyValue;

#ifndef _DEF_FGDCLASSTYPE_
#define _DEF_FGDCLASSTYPE_
enum fgdClassType{
	BaseClass = 0,
	PointClass,
	SolidClass
};
#endif

#ifndef _DEF_BYTE_
typedef unsigned char byte;
#endif

typedef struct {
	fgdClassType type;

	std::vector<unsigned int> base;
	signed char size[2][3];
	byte color[3];

	char name[32];
	char desc[64];

	std::vector<fgdFlags> spawnflags;
	std::vector<fgdKeyValue> properties;
} fgdClass;

void fgdParse(char * filename);

#endif