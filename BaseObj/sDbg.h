#pragma once
#include "../common.h"
#include <stdio.h>
#include "ObjDefs.h"
#include "svard.h"
#include "../Utils/Utils.h"

#define DEFAULT_DBG_TO_SCREEN	true
#define DEFAULT_DBG_TO_FILE		true
#define DEFAULT_DBG_FPATH		"C:/temp/logs"
#define DEFAULT_DBG_FNAME		"Debugger"
#define DEFAULT_DBG_VERBOSITY	false
#define DEFAULT_DBG_TIMING		true
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
	//--
	char msg[DBG_MSG_MAXLEN];
	char stack[DBG_STACK_MAXLEN];

	EXPORT sDbg(bool verbose_, bool timing_, bool dbgtoscreen_, bool dbgtofile_, char* outfilepath_);
	EXPORT ~sDbg();	
	EXPORT void createOutFile(char* objName, void* objAddr, int objDepth);
	//-- local copy of stripChar(), to avoid linkng Utils.lib to all modules that use sDbg
	EXPORT int _argcnt(const char* mask) {
		int cnt=0;
		for (int i=0; i<(int)strlen(mask); i++) {
			if (mask[i]==37) cnt++;
		}
		return cnt;
	}


	//EXPORT void out(int msgtype, const char* callerFunc_, int stackLevel_, char* msgMask_, ...);
	template<class ...Args> EXPORT void out(int msgtype, const char* callerFunc_, int stackLevel_, NativeReportProgress* GUIreporter_, char* msgMask_, Args... msgArgs) {

		if (!verbose&&msgtype==DBG_MSG_INFO) return;

		char tmpmsg[DBG_MSG_MAXLEN];
		char indent[ObjMaxDepth]=""; for (int t=0; t<stackLevel_; t++) strcat_s(indent, ObjMaxDepth, "\t");

		//-- build tmpmsg
		sprintf_s(tmpmsg, DBG_MSG_MAXLEN, msgMask_, msgArgs...);
		stripChar(tmpmsg, '\n');
		//-- build msg from tmpmsg
		sprintf_s(msg, DBG_MSG_MAXLEN, "%s\n", tmpmsg);
		strcat_s(stack, DBG_STACK_MAXLEN, msg);

		//-- finally, out to selected destinations
		if (dbgtoscreen) {
			if (GUIreporter_!=nullptr) {
				(*GUIreporter_)(10, msg);
			} else {
				printf("%s%s", indent, msg);
			}
		}
		if (dbgtofile && outfile!=nullptr) fprintf(outfile, "%s%s", indent, msg);
	
	}

private:
	FILE* outfile;
	svard* msgSvard;
	

};

#define defaultdbg new sDbg(DEFAULT_DBG_VERBOSITY, DEFAULT_DBG_TIMING, DEFAULT_DBG_TO_SCREEN, DEFAULT_DBG_TO_FILE, DEFAULT_DBG_FPATH)
#define erronlydbg new sDbg(false, DEFAULT_DBG_TIMING, DEFAULT_DBG_TO_SCREEN, DEFAULT_DBG_TO_FILE, DEFAULT_DBG_FPATH)
#define clonedbg(fromDbg) new sDbg(fromDbg->verbose, fromDbg->timing, fromDbg->dbgtoscreen, fromDbg->dbgtofile, fromDbg->outfilepath)
