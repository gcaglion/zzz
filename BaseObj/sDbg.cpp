#include "sDbg.h"

sDbg::sDbg(bool verbose_, bool dbgtoscreen_, bool dbgtofile_, char* outfilepath_) {
	outfilepath=(char*)malloc(MAX_PATH);
	outfilename=(char*)malloc(MAX_PATH);
	outfilefullname=(char*)malloc(MAX_PATH);
	verbose=verbose_; dbgtoscreen=dbgtoscreen_, dbgtofile=dbgtofile_;
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
		//getFullFileName(outfilename, outfilefullname, outfilepath);
		getFullPath(outfilename, outfilefullname, outfilepath);
		if (fopen_s(&outfile, outfilefullname, "w")!=0) {
			out(DBG_MSG_FAIL, __func__, objDepth, "Error %d creating file %s", errno, outfilefullname);
			throw std::exception(msg);
		}
	}
}
void sDbg::out(int msgtype, const char* callerFunc_, int stackLevel_, char* msgMask_, ...) {
	if (msgtype==DBG_MSG_INFO&&!verbose) return;

	char indent[ObjMaxDepth]=""; for (int t=0; t<stackLevel_; t++) strcat_s(indent, ObjMaxDepth, "\t");
	//char timestamp[50]; gettimestamp(timestamp, 50);
	char tmpmsg[DBG_MSG_MAXLEN];

	va_list va_args;
	va_start(va_args, msgMask_);
	vsprintf_s(tmpmsg, DBG_MSG_MAXLEN, msgMask_, va_args);
	stripChar(tmpmsg, '\n');
	sprintf_s(msg, DBG_MSG_MAXLEN, "%s\n", tmpmsg);
	strcat_s(stack, DBG_STACK_MAXLEN, msg);
	va_end(va_args);

	if (dbgtoscreen) printf("%s%s", indent, msg);
	if (dbgtofile && outfile!=nullptr) fprintf(outfile, "%s%s", indent, msg);

}

