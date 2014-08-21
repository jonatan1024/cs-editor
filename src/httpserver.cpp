#include "httpserver.h"
#include "textures.h"
#include "fgdclasses.h"

extern textureList * g_textures;
extern fgdClasses * g_fgdclasses;

void urlDecode(char * source, char * destination, int length){
	while(*source && --length){
		if(*source != '+' && *source != '%'){
			*destination = *source;
		} else if(*source == '+'){
			*destination = ' ';
		} else {
			source++;
			sscanf(source,"%2hhx",destination);
			source++;
			if(*destination == ','){
				*destination = ' ';
			}
		}
		source++;
		destination++;
	}
	*destination = 0;
}

char * httpServer::craftResponse(char * req, char * version, int * len, bool cached){
	const char texbrowser_style[] = "<style>body{background:#000;}p{background:#00F;color:#FFF;font-family:Courier New;font-size:9px;margin-top:1px;}img{border:0;}table{display:inline;}td{padding:0;}</style>\n";
	const char texbrowser_texpattern[] = "<table><tr><td><a href=\"set=%d,%d\"><img src=\"files/%d\"></a><tr><td><p>%s</p></table>\n";


	char * buffer = (char*)calloc(256, 1);
	int pos = sprintf(buffer, "%s 200 OK\n"
		"Content-Type: text/html\n"	//quick hack for the new Chrome-based MOTD
		"ETag: \"\"\n"
		"\n", version);
	req++;
	if(!strncmp(req, "textures/", 9)){
		req += 9;
		if(!strncmp(req, "files/", 6)){
			req += 6;
			if(cached){
				pos = sprintf(buffer,"%s 304 Not Modified\n\n", version);
				//buffer[pos]=0;
			}else{
				int size;
				byte * bmp = g_textures->getBMP(atoi(req), &size);
				buffer = (char*)realloc(buffer, pos+size);
				memcpy(buffer+pos, bmp, size);
				free(bmp);//todo: vysrat se na to tady, protoze to pak stejne vsechno bude v cache, kde si to samo bude freeovat!
				pos+=size;
			}
		}
		else if(!strncmp(req, "filter=", 7)){
			req += 7;
			int uid = 0;
			char * uid_s = strchr(req,',');
			if(uid_s){
				uid_s[0]=0;
				uid_s++;
				uid = atoi(uid_s);
			}

			filteredtex_t filtered[MAX_FILTERED_TEX];
			int count = g_textures->getFiltered(req, filtered);
			if(count == -1){
				printf("WARNING: too common filter, listing only %d textures!\n", MAX_FILTERED_TEX);
				count = MAX_FILTERED_TEX;
			}
			int bufsize = pos + sizeof(texbrowser_style) + (sizeof(texbrowser_texpattern)+64)*count;
			buffer = (char*)realloc(buffer, bufsize);
			strcpy(buffer + pos, texbrowser_style);
			pos += sizeof(texbrowser_style);
			for(int i = 0; i < count; i++){
				filteredtex_t * tex = &filtered[i];
				pos += sprintf(buffer + pos, texbrowser_texpattern, tex->id, uid, tex->id, tex->texname);
			}
		}
		else if(!strncmp(req, "set=", 4)){
			req += 4;
			int uid = 0;
			char * uid_s = strchr(req, ',');
			if(uid_s){
				uid_s[0] = 0;
				uid_s++;
				uid = atoi(uid_s);
			}
			int texture = atoi(req);
			wadlist_t * wad;
			texlist_t * tex;
			if(g_textures->IDWadTex(texture, &wad, &tex)){
				MF_ExecuteForward(forward, texture, uid);
				//long way to 256 bytes, so I think we can stay calm and not realloc
				const char style[] = "<style>body{background:#000;}h3{color:#FFF;}</style>\n";
				const char pattern[] = "<h3>Selected texture: %s (%s/%s)</h3>\n<img src=\"files/%d\">\n";
				strcpy(buffer + pos, style);
				pos += sizeof(style);
				pos += sprintf(buffer + pos, pattern, tex->texname, wad->gamedir?g_gamedir:"valve", wad->wadname, texture);
			}
		}
	}else if(!strncmp(req, "entities/", 9)){
		req += 9;
		return g_fgdclasses->craftResponse(req, len, buffer, pos);
	}

	*len = pos;
	return buffer;
}

SOCKET httpServer::getSSocket(){
	return ssocket;
}

#define MAX_REQUEST_LENGTH 2500
#define MAX_ADDR_LENGTH 2048

DWORD WINAPI thread(LPVOID lpParam){
	httpServer * server = (httpServer*)lpParam;

	SOCKET csocket;

	csocket = accept(server->getSSocket(), NULL, NULL);
	if(csocket == INVALID_SOCKET)
		goto end;

	char buffer[MAX_REQUEST_LENGTH];
	memset(buffer, 0, MAX_REQUEST_LENGTH);
	if(recv(csocket, buffer, MAX_REQUEST_LENGTH, 0) <= 0)
		goto end;
	char addr[MAX_ADDR_LENGTH], version[9];
	if(sscanf_s(buffer, "GET %[^ ] %s", addr, MAX_ADDR_LENGTH, version, 9) != 2 || strlen(version) != 8)
		goto end;
	bool cached = (strstr(buffer,"If-None-Match") != NULL);
	
	//printf("DEBUG: %s request: %s\n",version,addr);
	int len;
	char * resbuffer = server->craftResponse(addr, version, &len, cached);
	char * res = resbuffer;
	bool first = true;
	do{
		int chunk = min(len,first?MAX_FIRSTPACKET_LEN:MAX_NEXTPACKET_LEN);
		if(send(csocket, res, chunk, 0 ) == SOCKET_ERROR)
			goto end;
		len-=chunk;
		res+=chunk;
		first = false;
	}while(len);
	free(resbuffer);
	shutdown(csocket, SD_SEND);

	end:
    closesocket(csocket);
	return 0;
}

void httpServer::tick(){
	if(cthread && WaitForSingleObject(cthread,0) != WAIT_TIMEOUT){
		CloseHandle(cthread);
		cthread = NULL;
	}

	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(ssocket, &readSet);
	timeval timeout;
	memset(&timeout,0,sizeof(timeval));
	if(select(ssocket,&readSet,NULL,NULL,&timeout) != 1)
		return;
	if(!cthread){
		cthread = CreateThread(NULL,0,thread,this,0,NULL);
	}
	//thread(NULL);
}

bool httpServer::start(){
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
		ED_LOG("[CS-EDITOR] WSAStartup failed!");
		return false;
	}

	ssocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(ssocket == INVALID_SOCKET){
		ED_LOG("[CS-EDITOR] socket failed!");
		return false;
	}
	//u_long nonblock = 1;
	//ioctlsocket(ssocket,FIONBIO,&nonblock);

	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;

	short port = atoi(strchr(CVAR_GET_STRING("net_address"), ':') + 1);
	addr.sin_port=_byteswap_ushort(port);

	if(bind(ssocket, (sockaddr *)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR){
		ED_LOG("[CS-EDITOR] bind failed!");
		return false;
	}

    if(listen(ssocket, SOMAXCONN) == SOCKET_ERROR){
        ED_LOG("[CS-EDITOR] listen failed!");
        return false;
    }
	return true;
}

void httpServer::setForward(int forward){
	this->forward = forward;
}

httpServer::httpServer(){
	ssocket = INVALID_SOCKET;
	cthread = NULL;
	forward = 0;
}

httpServer::~httpServer(){
	if(ssocket != INVALID_SOCKET)
		closesocket(ssocket);
	WSACleanup();
}