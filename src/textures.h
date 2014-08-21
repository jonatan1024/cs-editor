#ifndef __EDITOR_TEXTURES__
#define __EDITOR_TEXTURES__

#include "sdk/amxxmodule.h"
#include "defs.h"

typedef struct{
	char * texname;
	int id;
}filteredtex_t;

typedef struct{
	char texname[16];
	int miptexpos;
	Vector color;
}texlist_t;

typedef struct{
	bool gamedir;
	char wadname[32];
	texlist_t * texlist;
	int texcount;
}wadlist_t;

typedef struct{
	char szName[16];
	int nWidth, nHeight;
	int nOffsets[4];
}bspmiptex_t;

typedef struct{
    int nFilePos, nDiskSize, nSize;
    byte nType, bCompression;
    short nDummy;
    char szName[16];
}waddirentry_t;

typedef byte pal_t[3];

class textureList{
private:
	wadlist_t * wadlist;
	int wadlist_i;
	int wadcount;
	char tl_file[64];

	bool fromFile();
	void toFile();
	void findWADs(char * dir, bool gamedir);
	void getWADTextures(char * path, wadlist_t * wad);
	wadlist_t * addWAD(bool gamedir, char * wadname);
	inline bool validateID(int texture);
	inline wadlist_t * IDWad(int texture);
	inline texlist_t * IDTex(int texture);
public:
	textureList();
	~textureList();
	int getSkin(int texture);
	int textureID(char * texture);
	int getFiltered(char * filter, filteredtex_t * filtered);
	byte * getBMP(int texture, int * size);

	inline bool IDWadTex(int texture, wadlist_t ** wad, texlist_t ** tex){
		if(!validateID(texture))
			return false;
		if(wad)
			*wad = IDWad(texture);
		if(tex)
			*tex = IDTex(texture);
		return true;
	}

	void saveMAPwads(FILE * f);

};
#endif