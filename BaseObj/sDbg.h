#pragma once
#include "../common.h"
#include <stdio.h>
#include "../Utils/Utils.h"
#include "ObjDefs.h"

#define DEFAULT_DBG_TO_SCREEN	true
#define DEFAULT_DBG_TO_FILE		true
#define DEFAULT_DBG_FPATH		"C:/temp/logs"
#define DEFAULT_DBG_FNAME		"Debugger"
#define DEFAULT_DBG_VERBOSITY	true
#define DEFAULT_DBG_TIMING		false
#define DEFAULT_DBG_PAUSERR		true
#define DBG_MSG_MAXLEN			2048
#define DBG_STACK_MAXLEN		32768*1024	// 32MB
//--
#define DBG_MSG_INFO 0
#define DBG_MSG_ERR  1
#define DBG_MSG_FAIL  2
//--
#define DBG_ORIGIN_NEW 0
#define DBG_ORIGIN_PARENT 1
#define DBG_ORIGIN_CFG 2

#include "sDbgMacros.h"

struct sDbg {
	bool verbose;
	bool dbgtofile;
	bool dbgtoscreen;
	bool timing;
	bool pauseOnError;
	char* outfilepath;
	char* outfilename;
	char* outfilefullname;
	FILE* outfile;
	char msg[DBG_MSG_MAXLEN];
	char stack[DBG_STACK_MAXLEN];

	EXPORT void out(int msgtype, const char* callerFunc_, int stackLevel_, char* msgMask_, ...);
	EXPORT sDbg(bool verbose_, bool dbgtoscreen_, bool dbgtofile_, char* outfilepath_);
	EXPORT ~sDbg();	
	EXPORT void createOutFile(char* objName, void* objAddr, int objDepth);

};

#define defaultdbg new sDbg(DEFAULT_DBG_VERBOSITY, DEFAULT_DBG_TO_SCREEN, DEFAULT_DBG_TO_FILE, DEFAULT_DBG_FPATH)
#define clonedbg(fromDbg) new sDbg(fromDbg->verbose, fromDbg->dbgtoscreen, fromDbg->dbgtofile, fromDbg->outfilepath)
