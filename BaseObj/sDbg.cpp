#include "sDbg.h"

sDbg::sDbg(bool verbose_, bool timing_, bool dbgtoscreen_, bool dbgtofile_, char* outfilepath_) {
	outfilepath=(char*)malloc(MAX_PATH);
	outfilename=(char*)malloc(MAX_PATH);
	outfilefullname=(char*)malloc(MAX_PATH);
	
	verbose=verbose_; timing=timing_; dbgtoscreen=dbgtoscreen_, dbgtofile=dbgtofile_;
	getFullPath(outfilepath_, outfilepath);

	FILE* dbgPropFile; int linesCnt; 
	char _line[XMLLINE_MAXLEN]; char _pname[XMLKEY_PARM_NAME_MAXLEN]; char _pval[XMLKEY_PARM_VAL_MAXLEN];
	if (fopen_s(&dbgPropFile, "debug.properties", "r")==0) {
		linesCnt=0;
		while (fgets(_line, XMLLINE_MAXLEN, dbgPropFile)!=NULL) {
			stripChar(_line, ' '); stripChar(_line, '\t'); stripChar(_line, '\n');
			if (getValuePair(_line, _pname, _pval, '=')) {
				if (_stricmp(_pname, "DEFAULT_VERBOSE")==0) verbose=(_stricmp(_pval, "TRUE")==0);
				if (_stricmp(_pname, "DEFAULT_TIMING")==0) timing=(_stricmp(_pval, "TRUE")==0);
				if (_stricmp(_pname, "DEFAULT_TOSCREEN")==0) dbgtoscreen=(_stricmp(_pval, "TRUE")==0);
				if (_stricmp(_pname, "DEFAULT_TOFILE")==0) dbgtofile=(_stricmp(_pval, "TRUE")==0);
				if (_stricmp(_pname, "DEFAULT_FPATH")==0) getFullPath(_pval, outfilepath);

				if (_stricmp(_pname, "DEFAULT_PAUSEERR")==0) pauseOnError=(_stricmp(_pval, "TRUE")==0);
			}
			linesCnt++;

		}
		fclose(dbgPropFile);
	}
	

	stack[0]='\0';
	outfile=nullptr;
}
sDbg::~sDbg() {
	if (outfile!=nullptr) {
		fflush(outfile);
		fseek(outfile, 0, SEEK_END); // seek to end of file
		size_t fsize = ftell(outfile); // get current file pointer
		fclose(outfile);
		if (fsize==0) remove(outfilefullname);
	}
	free(outfilepath);
	free(outfilename);
	free(outfilefullname);
}

void sDbg::createOutFile(char* objName, void* objAddr, int objDepth) {
	if (dbgtofile) {
		
		sprintf_s(outfilename, MAX_PATH, "%s(%p)_Dbg.%s", objName, objAddr, (verbose) ? "log" : "err");
		getFullPath(outfilename, outfilefullname, outfilepath);
		if (fopen_s(&outfile, outfilefullname, "w")!=0) {
			out(DBG_MSG_FAIL, __func__, objDepth, nullptr, "Error %d creating file %s", errno, outfilefullname);
			throw std::exception(msg);
		}
	}
}
