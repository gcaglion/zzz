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
void sFileData::saveMSE(int pid, int tid, int mseCnt, int* duration, numtype* mseT, numtype* mseV) {
	fail("not implemented.");
}
void sFileData::saveRun(int pid, int tid, int npid, int ntid, numtype mseR, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted, numtype* barWidth_) {
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
//--
void sFileData::findPid(int pid_, bool* found_) {
	fail("Not implemented.");
}

void sFileData::saveClientInfo(int pid, int simulationId, int npid, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_) {
	fail("not implemented.");
}
//-- Save/Load core images
void sFileData::saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F) {
	fail("Not implemented.");
}
void sFileData::saveCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::saveCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::saveCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::saveCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F) {
	fail("Not implemented.");
}
void sFileData::loadCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::loadCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::loadCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sFileData::loadCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
//-- Save/Load Core Logger image
void sFileData::saveCoreLoggerParms(int pid_, int tid_, int readFrom, bool saveToDB, bool saveToFile, bool saveMSEFlag, bool saveRunFlag, bool saveInternalsFlag, bool saveImageFlag) {
	fail("Not implemented.");
}
void sFileData::loadCoreLoggerParms(int pid_, int tid_, int* readFrom, bool* saveToDB, bool* saveToFile, bool* saveMSEFlag, bool* saveRunFlag, bool* saveInternalsFlag, bool* saveImageFlag) {
	fail("Not implemented.");
}
//-- Save/Load core parms
void sFileData::saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_, int SCGDmaxK_) {
	fail("Not implemented.");
}
void sFileData::saveCoreGAparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sFileData::saveCoreSVMparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sFileData::saveCoreSOMparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sFileData::saveCoreDUMBparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sFileData::loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_) {
	fail("Not implemented.");
}
void sFileData::loadCoreGAparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
void sFileData::loadCoreSVMparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
void sFileData::loadCoreSOMparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
void sFileData::loadCoreDUMBparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
//-- Save Core<XXX>Internals
void sFileData::saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* comp_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_) {
	fail("Not implemented.");
}

void sFileData::loadEngineInfo(int pid, int* engineType, int* coresCnt, int* sampleLen_, int* predictionLen_, int* featuresCnt_, bool* saveToDB_, bool* saveToFile_, const char* dbconn_, int* coreId, int* coreType, int* coreThreadId, int* parentCoresCnt, int** parentCore, int** parentConnType) {
	fail("Not implemented.");
}
void sFileData::saveEngineInfo(int pid, int engineType, int coresCnt, int sampleLen_, int predictionLen_, int featuresCnt_, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType) {
	fail("Not implemented.");
}

void sFileData::saveXMLconfig(int simulationId_, int pid_, int tid_, int fileId_, int parmsCnt_, char** parmDesc_, char** parmVal_) {
	fail("Not implemented.");
}