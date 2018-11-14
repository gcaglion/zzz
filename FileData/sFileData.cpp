#include "sFileData.h"

sFileData::sFileData(sObjParmsDef, int openMode_, bool autoOpen_, int filesCnt_, char** fileFullName_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	filesCnt=filesCnt_;
	mallocs();
	for (int f=0; f<filesCnt; f++) strcpy_s(fileFullName[f], MAX_PATH, fileFullName_[f]);

	if (autoOpen_) open(openMode_);
}
sFileData::sFileData(sObjParmsDef, int openMode_, bool autoOpen_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {}
sFileData::sFileData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}
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
	for (int f=0; f<filesCnt; f++) {
		if(fileH[f]!=nullptr) fclose(fileH[f]);
	}
}
void sFileData::getStartDates(char* StartDate, int DatesCount, char*** oDate) {
	fail("Not implemented!");
}
void sFileData::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
	fail("not implemented.");
}
void sFileData::saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted) {
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
void sFileData::saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun) {
	fail("not implemented.");
}
void sFileData::saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* parentCoresCnt, int** parentCore, int** parentConnType) {
	fail("Not implemented.");
}
void sFileData::saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_) {
	fail("Not implemented.");
}
void sFileData::loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_) {
	fail("Not implemented.");
}
