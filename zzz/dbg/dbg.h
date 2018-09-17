#pragma once
#include "../CommonEnv.h"
#include <stdio.h>
#include "../Utils/Utils.h"

#define DEFAULT_DBG_TO_SCREEN	true
#define DEFAULT_DBG_TO_FILE	true
#define DEFAULT_DBG_FPATH		"C:/temp/logs"
#define DEFAULT_DBG_FNAME		"Debugger"
#define DEFAULT_DBG_VERBOSITY	true
#define DEFAULT_DBG_TIMING		false
#define DEFAULT_DBG_PAUSERR		true
#define DBG_MSG_MAXLEN			1024
#define DBG_STACK_MAXLEN		32768
//--
#define DBG_MSG_INFO 0
#define DBG_MSG_ERR  1
#define DBG_MSG_FAIL  2
//--

#ifdef __cplusplus
#define info(msgMask_, ...) dbg->out(DBG_MSG_INFO, __func__, msgMask_, __VA_ARGS__)
#define err(msgMask_, ...) dbg->out(DBG_MSG_ERR, __func__, msgMask_, __VA_ARGS__)
#define fail(msgMask_, ...) { dbg->out(DBG_MSG_ERR, __func__, msgMask_, __VA_ARGS__); throw std::exception(dbg->msg);}
#else
#define info(msgMask_, ...) out(dbg, DBG_MSG_INFO, __func__, msgMask_, __VA_ARGS__)
#define err(msgMask_, ...) out(dbg, DBG_MSG_ERR, __func__, msgMask_, __VA_ARGS__)
#define fail(msgMask_, ...) { out(dbg, DBG_MSG_ERR, __func__, msgMask_, __VA_ARGS__); throw std::exception(dbg->msg);}
#endif

typedef struct sDbg {
	int stackLevel;
	Bool verbose;
	Bool dbgtofile;
	Bool dbgtoscreen;
	Bool timing;
	Bool pauseOnError;
	char outfilepath[MAX_PATH];
	char outfilename[MAX_PATH];
	char outfilefullname[MAX_PATH];
	FILE* outfile;
	char msg[DBG_MSG_MAXLEN];
	char stack[DBG_STACK_MAXLEN];

#ifdef __cplusplus
	EXPORT void out(int msgtype, const char* callerFunc_, char* msgMask_, ...);
	EXPORT sDbg(Bool verbose_=DEFAULT_DBG_VERBOSITY, Bool dbgtoscreen_=DEFAULT_DBG_TO_SCREEN, Bool dbgtofile_=DEFAULT_DBG_TO_FILE, char* outfilepath_=DEFAULT_DBG_FPATH);
	EXPORT ~sDbg();	
	EXPORT void createOutFile(char* parentName, void* parentAddr);
#endif
} tDbg;

#define defaultdbg new sDbg(DEFAULT_DBG_VERBOSITY, DEFAULT_DBG_TO_SCREEN, DEFAULT_DBG_TO_FILE, DEFAULT_DBG_FPATH)
