#include "dbg.h"

sDbg::sDbg(Bool verbose_, Bool dbgtoscreen_, Bool dbgtofile_, char* outfilepath_) {
	verbose=verbose_; dbgtoscreen=dbgtoscreen_, dbgtofile=dbgtofile_;
	strcpy_s(outfilepath, MAX_PATH, outfilepath_);
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
}

void sDbg::createOutFile(char* parentName, void* parentAddr) {
	sprintf_s(outfilename, MAX_PATH, "%s(%p)_Dbg.%s", parentName, parentAddr, (verbose) ? "log" : "err");
	sprintf_s(outfilefullname, MAX_PATH, "%s/%s", outfilepath, outfilename);
	fopen_s(&outfile, outfilefullname, "w");
	if (errno!=0) out(DBG_MSG_FAIL, __func__, "Error %d", errno);
}
void sDbg::out(int msgtype, const char* callerFunc_, char* msgMask_, ...) {
	if (msgtype==DBG_MSG_INFO&&!verbose) return;

	char indent[16]=""; for (int t=0; t<stackLevel; t++) strcat_s(indent, 16, "\t");
	char timestamp[50]; gettimestamp(timestamp, 50);
	char tmpmsg[DBG_MSG_MAXLEN];

	va_list va_args;
	va_start(va_args, msgMask_);
	vsprintf_s(tmpmsg, DBG_MSG_MAXLEN, msgMask_, va_args);
	sprintf_s(msg, DBG_MSG_MAXLEN, "%s%s\n", indent, tmpmsg);
	strcat_s(stack, DBG_STACK_MAXLEN, msg);
	va_end(va_args);

	if (dbgtoscreen) printf("%s", msg);
	if (dbgtofile && outfile!=nullptr) fprintf(outfile, "%s", msg);

}

