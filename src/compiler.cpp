#include "compiler.h"
#include "defs.h"

HANDLE g_compilerthread = NULL;

int g_compilerstatus = -1;

char lfilename[64];
int timestamp = 0;

DWORD WINAPI compilerthread(LPVOID lpParam);

void editor_compile(char * filename, int stamp){
	if(!g_compilerthread){
		strcpy(lfilename, filename);
		timestamp = stamp;
		g_compilerthread = CreateThread(NULL, 0, compilerthread, lfilename, 0, NULL);
	}else{
		ED_LOG("I'm still compiling!");
	}
}

DWORD WINAPI compilerthread(LPVOID lpParam){
	const char exes[4][4] = {"csg", "bsp", "vis", "rad"};

	char * filename = (char*)lpParam;

	const int exelen = sizeof(g_gamedir) + sizeof(COMPILERS_PATT) + 3 + 64;
	char exe[exelen];
	
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);

	for(int i = 0; i < 4; i++){
		g_compilerstatus = i;

		sprintf(exe, COMPILERS_PATT, g_gamedir, exes[i], filename);
		memset(&pinfo, 0, sizeof(pinfo));
		CreateProcess(NULL, exe, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &sinfo, &pinfo);
		WaitForSingleObject(pinfo.hProcess, INFINITE);
	}
	g_compilerstatus = -1;
	return 0;
}

int editor_compiled(){
	if(g_compilerthread && WaitForSingleObject(g_compilerthread, 0) != WAIT_TIMEOUT){
		CloseHandle(g_compilerthread);
		g_compilerthread = NULL;
		return timestamp;
	}
	return 0;
}