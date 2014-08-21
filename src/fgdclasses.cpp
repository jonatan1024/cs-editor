#include "fgdclasses.h"
#include "fgdparser.h"
#include "defs.h"

#include "world.h"
extern editorWorld * g_world;
#include "httpserver.h"

extern std::vector<fgdClass> classes;

fgdClasses::fgdClasses(){
	char filename[64];
	strcpy(filename,g_gamedir);
	strcat(filename,EDITORFGD_FILE);
	fgdParse(filename);
}

fgdClasses::~fgdClasses(){
	classes.clear();
}

int fgdClasses::getClassIndex(char * name){
	for(std::vector<fgdClass>::iterator it = classes.begin(); it != classes.end(); ++it) {
		if(!strcmp(it->name, name)){
			return it - classes.begin();
		}
	}
	return -1;
}

int fgdClasses::getNextClass(int index, fgdClassType type, char ** name, char ** desc){
	for(std::vector<fgdClass>::iterator it = classes.begin() + index; it != classes.end(); ++it) {
		if(it->type == type){
			*name = it->name;
			*desc = it->desc;
			return it-classes.begin();
		}
	}
	return -1;
}

char * fgdClasses::getClassName(int index){
	if(index < 0 || (unsigned int)index >= classes.size())
		return "";
	return classes[index].name;
}

bool fgdClasses::getSizeColor(int index, Vector * mins, Vector * maxs, Vector * color){
	//keep calm, casting just to supress warnings :)
	if(index < 0 || (unsigned int)index >= classes.size())
		return false;
	std::vector<unsigned int> queue;
	queue.push_back(index);
	bool size_found = false, color_found = false;
	while(!queue.empty()){
		fgdClass * cur = &classes[queue.front()];
		queue.erase(queue.begin());
		if(!size_found && cur->size[0][0] < cur->size[1][0]){
			for(int i = 0; i < 3; i++){
				(*mins)[i] = cur->size[0][i];
				(*maxs)[i] = cur->size[1][i];
				classes[index].size[0][i] = cur->size[0][i];
				classes[index].size[1][i] = cur->size[1][i];
			}
			size_found = true;
		}
		if(!color_found && (cur->color[0] || cur->color[1] || cur->color[2])){
			for(int i = 0; i < 3; i++){
				(*color)[i] = cur->color[i];
				classes[index].color[i] = cur->color[i];
			}
			color_found = true;
		}

		if(size_found && color_found)
			break;
		for(std::vector<unsigned int>::reverse_iterator it = cur->base.rbegin(); it != cur->base.rend(); ++it) {
			queue.push_back(*it);
		}
	}
	return (size_found && color_found);
}

void fgdClasses::handleGET(char * req, editorEntity * ent, int uid){
	ent->flags = 0;
	ent->attr.clear();
	
	entKeyValue entry;
	bool firstAttrs = true;

	char * next = NULL;
	do{
		next = strchr(req, '&');
		if(next){
			next[0] = 0;
			next++;
		}
		char * value = strchr(req, '=');
		if(value){
			value[0] = 0;
			value++;
			char * key = req;

			if(!strncmp(key, "flags_",6)){
				ent->flags |= atoi(key+6);
			}else{
				if(firstAttrs){
					strcpy(entry.key, "classname");
					strcpy(entry.value, classes[ent->entclass].name);
					ent->attr.push_back(entry);
					if(ent->flags){
						strcpy(entry.key, "spawnflags");
						sprintf(entry.value, "%d", ent->flags);
						ent->attr.push_back(entry);
					}
					firstAttrs = false;
				}
				urlDecode(key, entry.key, sizeof(entry.key));
				urlDecode(value, entry.value, sizeof(entry.value));
				ent->attr.push_back(entry);
			}
		}
		req = next;
	}while(next);

	if(ent->entType == 1){
		strcpy(entry.key, "origin");
		sprintf(entry.value, "%.0f %.0f %.0f", ent->origin[0], ent->origin[1], ent->origin[2]+VERTICAL_OFFSET);
		ent->attr.push_back(entry);
	}
}

char * fgdClasses::craftResponse(char * req, int * len, char * buffer, int pos){
	const char entpage_layout[] = "<style>body{background:#E2E2E2;}*{font-size:7pt;font-family:MS Sans Serif;}select,.t{width:125px;}</style><form><b>Class:</b> %s<hr><b>Flags:</b><br>%s<hr><b>Attributes:</b><br>(use commas ',' instead of spaces ' ')<table>%s</table><hr><input type=\"submit\" value=\"Apply changes\"></form>";
	const char entpage_flags[] = "<input type=\"checkbox\" name=\"flags_%d\" value=\"\" %s>%s<br>";
	const char entpage_inputrow[] = "<tr><td>%s<td><input class=\"t\" type=\"text\" name=\"%s\" value=\"%s\">";

	if(!strncmp(req, "entity=", 7)){
		req += 7;
		int entindex = -1;
		int uid = 0;
		char * uid_s = strchr(req,',');
		if(!uid_s)
			goto endcrafting;
		uid_s[0] = 0;
		uid_s++;
		entindex = atoi(req);
		char * get_s = strchr(uid_s,'?');
		if(get_s)
			get_s[0] = 0;
		uid = atoi(uid_s);

		//FIXME: we can't do this since we want some admin rights
		/*if(!g_world->checkBrushOwner(uid, entindex))
			goto endcrafting;*/
		editorEntity * ent = g_world->getEntity(entindex);
		if(!ent)
			goto endcrafting;
		
		if(get_s)
			handleGET(get_s+1, ent, uid);

		//Let's generate page content!
		char flags[32*(sizeof(entpage_flags)+48)] = "";
		char * pflags = flags;
		char * attrs = NULL;
		int attrs_o = 0;

		//KURVA KURVA my tu delame frontu ale potrebujeme neco na prochazeni L-P-K
		std::vector<unsigned int> queue;		
		queue.push_back(ent->entclass);
		while(!queue.empty()){
			fgdClass * cur = &classes[queue.front()];
			queue.erase(queue.begin());
			//manage Flags
			for(std::vector<fgdFlags>::iterator it = cur->spawnflags.begin(); it != cur->spawnflags.end(); ++it){
				bool checked = false;
				if(ent->flags == -1)
					checked = it->def;
				else if(ent->flags & it->val)
					checked = true;
				pflags += sprintf(pflags, entpage_flags, it->val, checked ? "checked" : "", it->desc);
			}

			//manage Attributtes
			for(std::vector<fgdKeyValue>::iterator it = cur->properties.begin(); it != cur->properties.end(); ++it){
				//lazy as fuck
				if(cur->properties.empty())
					break;
				//find last inserted value
				char * val = NULL;
				for(std::vector<entKeyValue>::iterator it2 = ent->attr.begin(); it2 != ent->attr.end(); ++it2){
					if(!strcmp(it2->key,it->key)){
						val = it2->value;
						break;
					}
				}
				
				//realloc to: written bytes + max lenght of input row + max size of choices
				attrs = (char *)realloc(attrs,attrs_o + (sizeof(entpage_inputrow) + 160) + (it->choices.size() * 150) );

				if(it->choices.empty()){
					attrs_o += sprintf(attrs + attrs_o, entpage_inputrow, it->desc, it->key, val ? val : it->val);
				} else {
					attrs_o += sprintf(attrs + attrs_o, "<tr><td>%s<td><select name=\"%s\"><option selected>", it->desc, it->key);
					for(std::vector<fgdChoices>::iterator it2 = it->choices.begin(); it2 != it->choices.end(); ++it2){
						bool selected = !strcmp(it->val,it2->val) || (val && !strcmp(val,it2->val));
						attrs_o += sprintf(attrs + attrs_o, "<option value=\"%s\" %s>%s", it2->val, selected ? "selected" : "", it2->desc);
					}
					attrs_o += sprintf(attrs + attrs_o, "</select>");
				}
			}

			for(std::vector<unsigned int>::reverse_iterator it = cur->base.rbegin(); it != cur->base.rend(); ++it) {
				queue.push_back(*it);
			}
		}

		buffer = (char *)realloc(buffer, pos + sizeof(entpage_layout)+32 + strlen(flags) + strlen(attrs));
		pos += sprintf(buffer + pos, entpage_layout, classes[ent->entclass].name, flags, attrs);
		free(attrs);
	}
	endcrafting:
	*len = pos;
	return buffer;
}