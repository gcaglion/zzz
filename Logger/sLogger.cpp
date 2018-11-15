#include "sLogger.h"

sLogger::sLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	saveToDB=saveToDB_;
	saveToFile=saveToFile_;
}
sLogger::sLogger(sObjParmsDef, sOraData* oradb_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	saveToDB=true; oradb=oradb_;
	saveToFile=false;
}
sLogger::sLogger(sObjParmsDef, sFileData* filedb_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	saveToDB=false;
	saveToFile=true; filedb=filedb_;
}
sLogger::sLogger(sObjParmsDef, sOraData* oradb_, sFileData* filedb_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	saveToDB=true; oradb=oradb_;
	saveToFile=true; filedb=filedb_;
}
sLogger::sLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	safecall(cfgKey, getParm, &saveToFile, "saveToFile");

	//-- 2. do stuff and spawn sub-Keys
	if (saveToDB) safespawn(oradb, newsname("Logger_OraData"), defaultdbg, cfg, "OraData");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sLogger::~sLogger() {}

void sLogger::open() {
	if (saveToDB) safecall(oradb, open);
	if (saveToFile) safecall(filedb, open, FILE_MODE_WRITE);
}
void sLogger::close() {
	if (saveToDB) safecall(oradb, close);
	if (saveToFile) safecall(filedb, close);
}
void sLogger::commit() {
	if (saveToDB) safecall(oradb, commit);
	if (saveToFile) safecall(filedb, commit);
}
//--
void sLogger::saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun) {
	if (saveToDB) safecall(oradb, saveClientInfo, pid, simulationId, clientName, startTime, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, doTrain, doTrainRun, doTestRun);
	if (saveToFile) safecall(filedb, saveClientInfo, pid, simulationId, clientName, startTime, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, doTrain, doTrainRun, doTestRun);
}
//--
void sLogger::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
	if (saveToDB) safecall(oradb, saveMSE, pid, tid, mseCnt, mseT, mseV);
	if (saveToFile) safecall(filedb, saveMSE, pid, tid, mseCnt, mseT, mseV);
}
void sLogger::saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted) {
	if (saveToDB) safecall(oradb, saveRun, pid, tid, npid, ntid, runStepsCnt, tsFeaturesCnt_, selectedFeaturesCnt, selectedFeature, predictionLen, posLabel, actualTRS, predictedTRS, actualTR, predictedTR, actual, predicted);
	if (saveToFile) safecall(filedb, saveRun, pid, tid, npid, ntid, runStepsCnt, tsFeaturesCnt_, selectedFeaturesCnt, selectedFeature, predictionLen, posLabel, actualTRS, predictedTRS, actualTR, predictedTR, actual, predicted);
}
//--
void sLogger::saveEngineInfo(int pid, int engineType, int coresCnt, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType) {
	if (saveToDB) safecall(oradb, saveEngineInfo, pid, engineType, coresCnt, coreId, coreType, tid, parentCoresCnt, parentCore, parentConnType);
	if (saveToFile) safecall(filedb, saveEngineInfo, pid, engineType, coresCnt, coreId, coreType, tid, parentCoresCnt, parentCore, parentConnType);
}
void sLogger::loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* coreThreadId, int* parentCoresCnt, int** parentCore, int** parentConnType) {
	if (saveToDB) safecall(oradb, loadEngineInfo, pid, engineType, coresCnt, coreId, coreType, coreThreadId, parentCoresCnt, parentCore, parentConnType);
	if (saveToFile) safecall(filedb, loadEngineInfo, pid, engineType, coresCnt, coreId, coreType, coreThreadId, parentCoresCnt, parentCore, parentConnType);
}
//-- Save/Load Core<XXX>Image
void sLogger::saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, saveCoreNNImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, saveCoreNNImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::saveCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, saveCoreGAImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, saveCoreGAImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::saveCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, saveCoreSOMImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, saveCoreSOMImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::saveCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, saveCoreSVMImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, saveCoreSVMImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::saveCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, saveCoreDUMBImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, saveCoreDUMBImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, loadCoreNNImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, loadCoreNNImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, loadCoreGAImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, loadCoreGAImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, loadCoreSOMImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, loadCoreSOMImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, loadCoreSVMImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, loadCoreSVMImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (saveToDB) safecall(oradb, loadCoreDUMBImage, pid, tid, epoch, Wcnt, W);
	if (saveToFile) safecall(filedb, loadCoreDUMBImage, pid, tid, epoch, Wcnt, W);
}
//-- Save/Load Core<XXX>Paameters
void sLogger::saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_) {
	if (saveToDB) safecall(oradb, saveCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_);
	if (saveToFile) safecall(filedb, saveCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_);
}
void sLogger::saveCoreGAparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::saveCoreSOMparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::saveCoreSVMparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::saveCoreDUMBparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_) {
	if (saveToDB) safecall(oradb, loadCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_);
	if (saveToFile) safecall(filedb, loadCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_);
}
void sLogger::loadCoreGAparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::loadCoreSOMparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::loadCoreSVMparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
void sLogger::loadCoreDUMBparms(int pid, int tid, int p1, int p2) {
	fail("Not implemented.");
}
