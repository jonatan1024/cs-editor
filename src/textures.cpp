#include "textures.h"

//SINCE THERE IS NO MOTHERFUCKING FUNCTION FOR CASE INSENSITIVE SUBSTRING SEARCH IN WINAPI
char *strcasestr(const char *arg1, const char *arg2)
{                  
   const char *a, *b;
                   
   for(;*arg1;*arg1++) {
                   
     a = arg1;
     b = arg2;
     
     while((*a++ | 32) == (*b++ | 32))
       if(!*b) 
         return (char*)(arg1);
     
   }
     
   return(NULL);
}

textureList::textureList(){
	wadlist = (wadlist_t*)malloc(MAX_WADS*sizeof(wadlist_t));
	wadlist_i = 1;
	wadcount = 0;
	strcpy(tl_file,g_gamedir);
	strcat(tl_file,TEXTURELIST_FILE);

	if(!fromFile()){
		ED_LOG("[CS-EDITOR] texture list not found, creating one");
		findWADs("valve",false);
		findWADs(g_gamedir,true);
		toFile();
	}
}

textureList::~textureList(){
	for(int i = 0; i < wadcount; i++){
		free(wadlist[i].texlist);
	}
	free(wadlist);
}

bool textureList::fromFile(){
	FILE * f = fopen(tl_file,"r");
	if(!f)
		return false;
	fscanf(f,"%d:\n",&wadcount);
	if(!wadcount){
		fclose(f);
		return false;
	}
	//i suppose we're not going to add any wads on the fly
	wadlist = (wadlist_t*)malloc(wadcount*sizeof(wadlist_t));
	for(int i = 0; i < wadcount; i++){
		wadlist_t * wad = &wadlist[i];
		int gamedir_;
		fscanf(f,"[%[^]]] (%d, %d)\n",wad->wadname,&gamedir_,&wad->texcount);
		wad->gamedir = (gamedir_ > 0);
		wad->texlist = (texlist_t*)malloc(wad->texcount*sizeof(texlist_t));
		for(int i2 = 0; i2 < wad->texcount; i2++){
			texlist_t * tex = &wad->texlist[i2];
			fscanf(f,"\"%[^\"]\" (%f, %f, %f) [%d]\n",tex->texname,&tex->color[0],&tex->color[1],&tex->color[2],&tex->miptexpos);
		}
	}
	fclose(f);
	return true;
}

void textureList::toFile(){
	FILE * f = fopen(tl_file,"w");
	fprintf(f,"%d:\n",wadcount);
	for(int i = 0; i < wadcount; i++){
		wadlist_t * wad = &wadlist[i];
		fprintf(f,"[%s] (%d, %d)\n",wad->wadname,wad->gamedir,wad->texcount);
		for(int i2 = 0; i2 < wad->texcount; i2++){
			texlist_t * tex = &wad->texlist[i2];
			fprintf(f,"\"%s\" (%f, %f, %f) [%d]\n",tex->texname,tex->color[0],tex->color[1],tex->color[2],tex->miptexpos);
		}
	}
	fclose(f);
}

inline bool textureList::validateID(int id){
	if(id==-1)
		return false;
	if(LOWORD(id) >= wadcount)
		return false;
	if(HIWORD(id) >= wadlist[LOWORD(id)].texcount)
		return false;
	return true;
}

inline wadlist_t * textureList::IDWad(int texture){
	return &wadlist[LOWORD(texture)];
}
inline texlist_t * textureList::IDTex(int texture){
	return &wadlist[LOWORD(texture)].texlist[HIWORD(texture)];
}

byte * textureList::getBMP(int id, int * size){
	*size = 0;
	if(!validateID(id))
		return NULL;
	wadlist_t * wad = IDWad(id);
	texlist_t * tex = IDTex(id);
	char filename[64];
	if(wad->gamedir)
		strcpy(filename, g_gamedir);
	else
		strcpy(filename, "valve");
	strcat(filename, "/");
	strcat(filename, wad->wadname);
	FILE * f = fopen(filename, "rb");
	if(!f)
		return NULL;
	fseek(f, tex->miptexpos, FILE_BEGIN);
	bspmiptex_t miptex;
	fread(&miptex, sizeof(bspmiptex_t), 1, f);
	
	//computing total bmp size and allocating
	int bmpcont = miptex.nWidth * miptex.nHeight;
	int bmpsize = bmpcont + 256*4 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); //rozmery+paleta+header1+header2
	byte * bmp = (byte*)malloc(bmpsize);

	//clearing and setting bitmap headers
	memset(bmp, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	BITMAPFILEHEADER * bmp_header1 = (BITMAPFILEHEADER*)bmp;
	BITMAPINFOHEADER * bmp_header2 = (BITMAPINFOHEADER*)(bmp + sizeof(BITMAPFILEHEADER));
	bmp_header1->bfType = 'MB';//"BM"
	bmp_header1->bfSize = bmpsize;
	bmp_header1->bfOffBits = bmpsize - bmpcont;
	bmp_header2->biSize = sizeof(BITMAPINFOHEADER);
	bmp_header2->biWidth = miptex.nWidth;
	bmp_header2->biHeight = miptex.nHeight;
	bmp_header2->biPlanes = 1;
	bmp_header2->biBitCount = 8;
	bmp_header2->biSizeImage = bmpcont;


	//read&write img
	fseek(f, miptex.nOffsets[0] - sizeof(bspmiptex_t), FILE_CURRENT);
	for(int i = 1; i <= miptex.nHeight; i++){
		fread(bmp + bmpsize - i*miptex.nWidth, 1, miptex.nWidth, f);
	}

	//TODO: write this in much cleaner, faster way!
	//read&write palette
	fseek(f, (tex->miptexpos + miptex.nOffsets[3] + (bmpcont>>6) + 2), FILE_BEGIN);
	memset(bmp + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),0,256*4);
	for(int i = 0; i < 256; i++){
		byte * fuckpos = bmp + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + i*4;
		fread(fuckpos+1,1,3,f);
		int * fpos = (int*)fuckpos;
		*fpos = _byteswap_ulong(*fpos);
	}

	fclose(f);
	/*f = fopen(miptex.szName, "wb");
	fwrite(bmp, 1, bmpsize, f);
	fclose(f);*/
	*size = bmpsize;
	return bmp;
}

int textureList::getFiltered(char * filter, filteredtex_t * filtered){
	int count = 0;
	for(int i = 0; i < wadcount; i++){
		wadlist_t * wad = &wadlist[i];
		for(int i2 = 0; i2 < wad->texcount; i2++){
			texlist_t * tex = &wad->texlist[i2];
			if(strcasestr(tex->texname,filter)){
				filtered[count].texname = tex->texname;
				filtered[count].id = MAKELONG(i,i2);
				count++;
				if(count==MAX_FILTERED_TEX)
					return -1;
			}
		}
	}
	return count;
}

int textureList::getSkin(int texture){
	if(!validateID(texture))
		return -1;
	Vector color = IDTex(texture)->color;
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
	return skin;
}

int textureList::textureID(char* texture){
	for(int i = 0; i < wadcount; i++){
		wadlist_t * wad = &wadlist[i];
		for(int i2 = 0; i2 < wad->texcount; i2++){
			texlist_t * tex = &wad->texlist[i2];
			if(!strcmp(tex->texname,texture)){
				return MAKELONG(i,i2);
			}
		}
	}
	return -1;
}

wadlist_t * textureList::addWAD(bool gamedir, char* wadname){
	if(wadcount == wadlist_i*MAX_WADS)
		wadlist = (wadlist_t*)realloc(wadlist, (++wadlist_i)*MAX_WADS*sizeof(wadlist_t));
	wadlist[wadcount].gamedir = gamedir;
	strcpy(wadlist[wadcount].wadname, wadname);
	return &wadlist[wadcount++];
}


void textureList::findWADs(char * dir, bool gamedir){
	HANDLE	hFile;
	WIN32_FIND_DATA	FileInformation;
	char mask[64];
	strcpy(mask,dir);
	strcat(mask,"\\*.wad");
	hFile = FindFirstFile(mask, &FileInformation);
	if(hFile == INVALID_HANDLE_VALUE)
		return;
	do{
		wadlist_t * wad = addWAD(gamedir,FileInformation.cFileName);
		strcpy(mask+strlen(dir)+1, FileInformation.cFileName);
		getWADTextures(mask, wad);
	}while(FindNextFile(hFile, &FileInformation));
}

void textureList::getWADTextures(char * path, wadlist_t * wad){
	char buffer[16];
	memset(buffer,0,16);
	FILE * f;
	f = fopen(path,"rb");
	if(!f)
		return;
	fread(buffer,1,4,f);
	if(!strcmp(buffer,"WAD3")){
		fread(&wad->texcount,4,1,f);
		if(wad->texcount){
			wad->texlist = (texlist_t*)malloc(wad->texcount*sizeof(texlist_t));
			int offset;
			fread(&offset,4,1,f);
			//let's jump just to the gates of hell
			fseek(f,offset,FILE_BEGIN);
			waddirentry_t entry;
			int count = wad->texcount;
			for(int i = 0; i < count; i++){
				//we don't want to have blank entries when we skip texture
				texlist_t * tex = &wad->texlist[i - (count - wad->texcount)];
				fread(&entry,sizeof(entry),1,f);
				//skip bogus textures
				if(!entry.nDiskSize || entry.bCompression || entry.nType!='C'){
					wad->texcount--;
					continue;
				}
				tex->miptexpos = entry.nFilePos;
				strcpy(tex->texname,entry.szName);
			}
			//let's do colors
			bspmiptex_t miptex;
			byte img[256*256];
			pal_t pal[256];
			int color[3];
			for(int i = 0; i < wad->texcount; i++){
				texlist_t * tex = &wad->texlist[i];
				fseek(f,tex->miptexpos,FILE_BEGIN);
				fread(&miptex,sizeof(bspmiptex_t),1,f);
				fseek(f,tex->miptexpos+miptex.nOffsets[3],FILE_BEGIN);
				int size4 = (miptex.nWidth*miptex.nHeight)>>6;
				fread(img,1,size4,f);
				fread(buffer,1,2,f);
				fread(pal,1,sizeof(pal),f);
				memset(color,0,3*sizeof(int));
				for(int i2 = 0; i2 < size4; i2++){
					ADD2COLS(color,pal[img[i2]]);
				}
				for(int i3 = 0; i3 < 3; i3++){
					tex->color[i3] = (float)color[i3]/size4;	//(float) just for lulz
				}
			}
		}
	}
	fclose(f);
}

void textureList::saveMAPwads(FILE * f){
	fputs("\"wad\" \"",f);

	char cwd[MAX_PATH];
	_getcwd(cwd,MAX_PATH);
	//warning: another windows only-thing, discarding drive letter
	char * path = cwd+2;
	for(int i = 0; i < wadcount; i++){
		wadlist_t * wad = &wadlist[i];
		fprintf(f,"%s\\%s\\%s;", path, wad->gamedir?g_gamedir:"valve", wad->wadname);
	}
	fputs("\"\n",f);
}