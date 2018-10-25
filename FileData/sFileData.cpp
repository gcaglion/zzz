#include "sFileData.h"

sFileData::sFileData(sCfgObjParmsDef, int openMode_, bool autoOpen_, int filesCnt_, char** fileFullName_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	filesCnt=filesCnt_;
	mallocs();
	for (int f=0; f<filesCnt; f++) strcpy_s(fileFullName[f], MAX_PATH, fileFullName_[f]);

	if (autoOpen_) open(openMode_);
}
sFileData::sFileData(sCfgObjParmsDef, int openMode_, bool autoOpen_) : sCfgObj(sCfgObjParmsVal) {}
sFileData::~sFileData() {

	for (int f=0; f<filesCnt; f++) {
		free(fileFullName[f]);
		free(fileFullName);
	}
}

void sFileData::open(int mode_) {
	char modeS[3]; char modeDesc[10];
	switch (mode_) {
	case FILE_MODE_READ:
		strcpy_s(modeS, 3, "r"); strcpy_s(modeDesc, 10, "read"); break;
	case FILE_MODE_WRITE:
		strcpy_s(modeS, 3, "w"); strcpy_s(modeDesc, 10, "write"); break;
	case FILE_MODE_APPEND:
		strcpy_s(modeS, 3, "w+"); strcpy_s(modeDesc, 10, "append"); break;
	default:
		fail("Invalid mode: %d", mode_); break;
	}

	for (int f=0; f<filesCnt; f++) {
		if (fopen_s(&fileH[f], fileFullName[f], modeS)!=0) fail("Could not open file %s for %s. Error %d", fileFullName[f], modeDesc, errno);
	}
}
void sFileData::close() {
	for (int f=0; f<filesCnt; f++) fclose(fileH[f]);
}
void sFileData::getStartDates(sFileData* dateSource_, char* startDate_, int datesCnt_, char** oDate_) {}
void sFileData::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
	fail("not implemented.");
}
void sFileData::saveRun(int pid, int tid, int npid, int ntid, int barsCnt, int featuresCnt, int* feature, numtype* actualTRS, numtype* predictedTRS, numtype* actual, numtype* predicted) {
	fail("not implemented.");
}
void sFileData::commit() {
	close();
}
void sFileData::mallocs() {
	fileFullName=(char**)malloc(filesCnt*sizeof(char*));
	fileH=(FILE**)malloc(filesCnt*sizeof(FILE*));
	for (int f=0; f<filesCnt; f++) fileFullName[f]=(char*)malloc(MAX_PATH);
}
