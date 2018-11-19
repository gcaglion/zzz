#include "sDbg.h"

sDbg::sDbg(bool verbose_, bool timing_, bool dbgtoscreen_, bool dbgtofile_, char* outfilepath_) {
	outfilepath=(char*)malloc(MAX_PATH);
	outfilename=(char*)malloc(MAX_PATH);
	outfilefullname=(char*)malloc(MAX_PATH);
	timing=timing_; verbose=verbose_; dbgtoscreen=dbgtoscreen_, dbgtofile=dbgtofile_;
	getFullPath(outfilepath_, outfilepath);
	stack[0]='\0';
	outfile=nullptr;
}
sDbg::~sDbg() {
	free(outfilepath); free(outfilename); free(outfilefullname);
	if (outfile!=nullptr) {
		fflush(outfile);
		fseek(outfile, 0, SEEK_END); // seek to end of file
		size_t fsize = ftell(outfile); // get current file pointer
		fclose(outfile);
		if (fsize==0) remove(outfilefullname);
	}
}

void sDbg::createOutFile(char* objName, void* objAddr, int objDepth) {
	if (dbgtofile) {
		
		sprintf_s(outfilename, MAX_PATH, "%s(%p)_Dbg.%s", objName, objAddr, (verbose) ? "log" : "err");
		getFullPath(outfilename, outfilefullname, outfilepath);
		if (fopen_s(&outfile, outfilefullname, "w")!=0) {
			out(DBG_MSG_FAIL, __func__, objDepth, "Error %d creating file %s", errno, outfilefullname);
			throw std::exception(msg);
		}
	}
}
