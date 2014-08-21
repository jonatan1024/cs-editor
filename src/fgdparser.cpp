#include <stdio.h>
#include <string.h>

#include "fgdparser.h"

std::vector<fgdClass> classes;

void fgdTrim(char * str){
	char * pos = str + strlen(str);
	do{
		pos--;
	}while(*pos == ' ' || *pos == '\t' || *pos == '"' || *pos == '\n' || *pos == '\r');
	*(pos+1) = 0;
	pos = str;
	while(*pos == ' ' || *pos == '\t' || *pos == '"')
		pos++;
	if(pos > str){
		for(; *pos; pos++, str++){
			*str = *pos;
		}
		*str = 0;
	}
}


void fgdParse(char * filename){

	FILE * f = fopen(filename, "r");
	char line[256];
	while(fgets(line, 256, f)){
		//skip to next line if this one doesn't contain class header
		if(line[0] != '@')
			continue;
		
		fgdClass thisclass;
		for(int i = 0; i < 3; i++){
			thisclass.color[i] = 0;
			thisclass.size[0][i] = 0;
			thisclass.size[1][i] = 0;
		}
		char type[16];
		char attr[192];
		int matched = sscanf(line, "@%[^ ]%[^=] = %[^:\n] : %[^\n]", type, attr, thisclass.name, thisclass.desc);
		
		//zero out the description if nothing is present
		if(matched < 3)
			continue;
		if (matched == 3){
			thisclass.desc[0] = 0;
		}

		//convert the string type to enum one
		if(!strncmp(type,"Base",4)){
			thisclass.type = BaseClass;
		}else if(!strncmp(type,"Point",5)){
			thisclass.type = PointClass;
		}else if(!strncmp(type,"Solid",5)){
			thisclass.type = SolidClass;
		}else{
			//skip to next if none of these classes matches
			continue;
		}

		//remove trailing zeros in class name and hande oneliners
		bool oneliner = false;
		fgdTrim(thisclass.name);
		if(thisclass.desc[0])
			fgdTrim(thisclass.desc);
		if(thisclass.name[strlen(thisclass.name)-1] == ']'){
			thisclass.name[strlen(thisclass.name)-2] = 0;
			fgdTrim(thisclass.name);
			oneliner = true;
		}
		if(thisclass.desc[0] && thisclass.desc[strlen(thisclass.desc)-1] == ']'){
			thisclass.desc[strlen(thisclass.desc)-2] = 0;
			fgdTrim(thisclass.desc);
			oneliner = true;
		}

		//parse attributes
		char params[128];
		params[0] = 0;
		while(sscanf(attr, " %[^(](%[^)])%[^\n]", type, params, attr) > 0){
			//rescan because of attributes' void parameters
			if(!params[0]){
				//if rescan fails, jump off this fail train
				if(sscanf(attr, " %[^(]()%[^\n]", type, attr) < 1)
					break;
			}

			if(!strncmp(type,"base", 4)){
				char name[32];
				while(int matched = sscanf(params, "%[^,], %[^\n]", name, params)){
					for(std::vector<fgdClass>::reverse_iterator it = classes.rbegin(); it != classes.rend(); ++it) {
						if(!strcmp(name, it->name)){
							thisclass.base.push_back(classes.rend() - it-1);
							break;
						}
					}
					
					if(matched == 1)
						break;
				}
			}else if(!strncmp(type, "size", 4)){
				int size[6];
				int matches = sscanf(params, "%d %d %d, %d %d %d", &size[0], &size[1], &size[2], &size[3], &size[4], &size[5]);
				if(matches == 3){
					for(int i = 0; i < 3; i++){
						size[3 + i] = size[i];
						size[i] = -size[i];
					}
				}
				for(int i = 0; i < 6; i++)
					thisclass.size[i/3][i%3] = size[i];
			}else if(!strncmp(type,"color", 5)){
				int color[3];
				sscanf(params, "%d %d %d", &color[0], &color[1], &color[2]);
				for(int i = 0; i < 3; i++)
					thisclass.color[i] = color[i];
			}

			params[0] = 0;
		}

		//parse properties
		if(!oneliner){
			while(fgets(line, 256, f)){
				if(line[0] == '[')
					continue;
				if(line[0] == ']')
					break;

				fgdKeyValue keyval;
				char type[24];
				int matched = sscanf(line, "%[^(](%[^:=]:%[^:]:%[^\n]", keyval.key, type, keyval.desc, keyval.val);
				if(matched < 2)
					continue;
				if (matched == 3){
					keyval.val[0] = 0;
				}
				
				fgdTrim(keyval.key);
				//parse flags
				if(!strcmp(keyval.key, "spawnflags") && !strncmp(type+1, "lags", 4)){
					char * pos = line;
					bool oneliner = false;
					//parse oneline flags
					while(*pos){
						if(*pos == '['){
							oneliner = true;
							fgdFlags flags;
							char def[8], val[16];
							int matched = sscanf(pos, "[%[^:]:%[^:]:%[^]]]", val, flags.desc, def);
							if(matched == 2)
								def[0] = 0;
							fgdTrim(val);
							flags.val = atoi(val);
							fgdTrim(flags.desc);
							fgdTrim(def);
							flags.def = (atoi(def) > 0);
							thisclass.spawnflags.push_back(flags);
							break;
						}
						pos++;
					}
					//parse multiline flags
					if(!oneliner){
						while(fgets(line, 256, f)){
							char * pos = line;
							while(*pos == ' ' || *pos == '\t')
								pos++;
							if(*pos == '[')
								continue;
							if(*pos == ']')
								break;

							fgdFlags flags;
							char def[8], val[16];
							int matched = sscanf(line, "%[^:]:%[^:]:%[^\n]", val, flags.desc, def);
							if(matched == 2)
								def[0] = 0;
							fgdTrim(val);
							flags.val = atoi(val);
							fgdTrim(flags.desc);
							fgdTrim(def);
							flags.def = (atoi(def) > 0);
							thisclass.spawnflags.push_back(flags);
						}
					}
				}else{
					bool ischo = false;
					if(!strncmp(type+1, "hoices", 6)){
						sscanf(keyval.val, "%[^=]=", keyval.val);
						ischo = true;
					}
					fgdTrim(keyval.desc);
					if(keyval.val[0]){
						fgdTrim(keyval.val);
					}
					//parse choices
					if(ischo){
						while(fgets(line, 256, f)){
							char * pos = line;
							while(*pos == ' ' || *pos == '\t')
								pos++;
							if(*pos == '[')
								continue;
							if(*pos == ']')
								break;

							fgdChoices choices;
							if(sscanf(line, "%[^:]:%[^\n]", choices.val, choices.desc) < 2)
								continue;
							fgdTrim(choices.val);
							fgdTrim(choices.desc);
							keyval.choices.push_back(choices);
						}
					}
					//printf("#%s#%s#%s#%s#\n", keyval.key, type, keyval.desc, keyval.val);
					thisclass.properties.push_back(keyval);
				}
			}
		}

		classes.push_back(thisclass);

		//printf("TYPE: '%d' ATTR: '%s'; NAME: '%s'; desc: '%s'\n", thisclass.type, attr, thisclass.name, thisclass.desc);
		//getchar();

	}
	fclose(f);
}