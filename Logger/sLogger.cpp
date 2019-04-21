#include "sLogger.h"

sLogger::sLogger(sObjParmsDef, int readFrom_, bool saveToDB_, bool saveToFile_) : sCfgObj(sObjParmsVal, nullptr, "") {
	source=readFrom_; saveToDB=saveToDB_;
	saveToFile=saveToFile_;
}
sLogger::sLogger(sObjParmsDef, sOraData* oradb_) : sCfgObj(sObjParmsVal, nullptr, "") {
	source=OraData;
	saveToDB=true; oradb=oradb_;
	saveToFile=false;
}
sLogger::sLogger(sObjParmsDef, sFileData* filedb_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	source=FileData;
	saveToDB=false;
	saveToFile=true; filedb=filedb_;
}
sLogger::sLogger(sObjParmsDef, int readFrom_, sOraData* oradb_, sFileData* filedb_) : sCfgObj(sObjParmsVal, nullptr, "") {
	source=readFrom_; 
	saveToDB=true; oradb=oradb_;
	saveToFile=true; filedb=filedb_;
}
sLogger::sLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &source, "ReadFrom");
	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	safecall(cfgKey, getParm, &saveToFile, "saveToFile");

	//-- 2. do stuff and spawn sub-Keys
	if (saveToDB) safespawn(oradb, newsname("Logger_OraData"), defaultdbg, cfg, "OraData");
	if (saveToFile) safespawn(filedb, newsname("Logger_FileData"), defaultdbg, cfg, "FileData");
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
void sLogger::findPid(int pid_, bool* found_) {
	if (saveToDB) safecall(oradb, findPid, pid_, found_);
	if (saveToFile) safecall(filedb, findPid, pid_, found_);
}
void sLogger::saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_) {
	if (saveToDB) safecall(oradb, saveClientInfo, pid, simulationId, clientName, startTime, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, doTrain, doInfer, clientXMLfile_, shapeXMLfile_, actionXMLfile_, engineXMLfile_);
	if (saveToFile) safecall(filedb, saveClientInfo, pid, simulationId, clientName, startTime, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, doTrain, doInfer, clientXMLfile_, shapeXMLfile_, actionXMLfile_, engineXMLfile_);
}
//--
void sLogger::saveMSE(int pid, int tid, int mseCnt, int* duration, numtype* mseT, numtype* mseV) {
	if (saveToDB) safecall(oradb, saveMSE, pid, tid, mseCnt, duration, mseT, mseV);
	if (saveToFile) safecall(filedb, saveMSE, pid, tid, mseCnt, duration, mseT, mseV);
}
void sLogger::saveRun(int pid, int tid, int npid, int ntid, numtype mseR, int runStepsCnt, char** posLabel, int featuresCnt_, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actualBASE, numtype* predictedBASE) {
	if (saveToDB) safecall(oradb, saveRun, pid, tid, npid, ntid, mseR, runStepsCnt, posLabel, featuresCnt_, actualTRS, predictedTRS, actualTR, predictedTR, actualBASE, predictedBASE);
	//if (saveToFile) safecall(filedb, saveRun, pid, tid, npid, ntid, mseR, runStepsCnt, tsFeaturesCnt_, selectedFeaturesCnt, selectedFeature, predictionLen, posLabel, actualTRS, predictedTRS, actualTR, predictedTR, actual, predicted, barWidth_);
}
//--
void sLogger::saveEngineInfo(int pid, int engineType, int coresCnt, int sampleLen_, int predictionLen_, int featuresCnt_, int WNNdecompLevel_, int WNNwaveletType_, bool saveToDB_, bool saveToFile_, sOraData* dbconn_, int* coreId, int* coreLayer, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_) {
	if (saveToDB) safecall(oradb, saveEngineInfo, pid, engineType, coresCnt, sampleLen_, predictionLen_, featuresCnt_, WNNdecompLevel_, WNNwaveletType_, saveToDB_, saveToFile_, dbconn_, coreId, coreLayer, coreType, tid, parentCoresCnt, parentCore, parentConnType, trMin_, trMax_, fftMin_, fftMax_);
	//if (saveToFile) safecall(filedb, saveEngineInfo, pid, engineType, coresCnt, sampleLen_, predictionLen_, featuresCnt_, coreId, coreType, tid, parentCoresCnt, parentCore, parentConnType);
}
void sLogger::loadEngineInfo(int pid, int* engineType_, int* coresCnt, int* sampleLen_, int* predictionLen_, int* featuresCnt_, int* WNNdecompLevel_, int* WNNwaveletType_, bool* saveToDB_, bool* saveToFile_, sOraData* dbconn_, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_) {
	if (source==OraData) safecall(oradb, loadEngineInfo, pid, engineType_, coresCnt, sampleLen_, predictionLen_, featuresCnt_, WNNdecompLevel_, WNNwaveletType_, saveToDB_, saveToFile_, dbconn_, coreId, coreType, tid, parentCoresCnt, parentCore, parentConnType, trMin_, trMax_, fftMin_, fftMax_);
	//if (source==FileData) safecall(filedb, loadEngineInfo, pid, engineType, coresCnt, sampleLen_, predictionLen_, featuresCnt_, saveToDB_, saveToFile_, "DioPorco", coreId, coreType, coreThreadId, parentCoresCnt, parentCore, parentConnType);
}
//-- Save/Load Core<XXX>Image
void sLogger::saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F) {
	if (saveToDB) safecall(oradb, saveCoreNNImage, pid, tid, epoch, Wcnt, W, Fcnt, F);
	if (saveToFile) safecall(filedb, saveCoreNNImage, pid, tid, epoch, Wcnt, W, Fcnt, F);
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
void sLogger::loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F) {
	if (source==OraData) safecall(oradb, loadCoreNNImage, pid, tid, epoch, Wcnt, W, Fcnt, F);
	if (source==FileData) safecall(filedb, loadCoreNNImage, pid, tid, epoch, Wcnt, W, Fcnt, F);
}
void sLogger::loadCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (source==OraData) safecall(oradb, loadCoreGAImage, pid, tid, epoch, Wcnt, W);
	if (source==FileData) safecall(filedb, loadCoreGAImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (source==OraData) safecall(oradb, loadCoreSOMImage, pid, tid, epoch, Wcnt, W);
	if (source==FileData) safecall(filedb, loadCoreSOMImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (source==OraData) safecall(oradb, loadCoreSVMImage, pid, tid, epoch, Wcnt, W);
	if (source==FileData) safecall(filedb, loadCoreSVMImage, pid, tid, epoch, Wcnt, W);
}
void sLogger::loadCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	if (source==OraData) safecall(oradb, loadCoreDUMBImage, pid, tid, epoch, Wcnt, W);
	if (source==FileData) safecall(filedb, loadCoreDUMBImage, pid, tid, epoch, Wcnt, W);
}
//-- Save/Load Core<XXX>Paameters
void sLogger::saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_, int SCGDmaxK_) {
	if (saveToDB) safecall(oradb, saveCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_, SCGDmaxK_);
	if (saveToFile) safecall(filedb, saveCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_, SCGDmaxK_);
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
	//fail("Not implemented.");
}
void sLogger::loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_) {
	if (source==OraData) safecall(oradb, loadCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_);
	if (source==FileData) safecall(filedb, loadCoreNNparms, pid, tid, levelRatioS_, levelActivationS_, useContext_, useBias_, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_);
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
//--
void sLogger::saveCoreLoggerParms(int pid_, int tid_, int readFrom, bool saveToDB, bool saveToFile, bool saveMSEFlag, bool saveRunFlag, bool saveInternalsFlag, bool saveImageFlag){
	if (saveToDB) safecall(oradb, saveCoreLoggerParms, pid_, tid_, readFrom, saveToDB, saveToFile, saveMSEFlag, saveRunFlag, saveInternalsFlag, saveImageFlag);
	if (saveToFile) safecall(filedb, saveCoreLoggerParms, pid_, tid_, readFrom, saveToDB, saveToFile, saveMSEFlag, saveRunFlag, saveInternalsFlag, saveImageFlag);
}
void sLogger::loadCoreLoggerParms(int pid_, int tid_, int* readFrom, bool* saveToDB, bool* saveToFile, bool* saveMSEFlag, bool* saveRunFlag, bool* saveInternalsFlag, bool* saveImageFlag){
	if (source==OraData) safecall(oradb, loadCoreLoggerParms, pid_, tid_, readFrom, saveToDB, saveToFile, saveMSEFlag, saveRunFlag, saveInternalsFlag, saveImageFlag);
	if (source==FileData) safecall(filedb, loadCoreLoggerParms, pid_, tid_, readFrom, saveToDB, saveToFile, saveMSEFlag, saveRunFlag, saveInternalsFlag, saveImageFlag);
}
//--
void sLogger::saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* comp_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_){
	if (saveToDB) safecall(oradb, saveCoreNNInternalsSCGD, pid_, tid_, iterationsCnt_, delta_, mu_, alpha_, beta_, lambda_, lambdau_, Gtse_old_, Gtse_new_, comp_, pnorm_, rnorm_, dwnorm_);
	if (saveToFile) safecall(filedb, saveCoreNNInternalsSCGD, pid_, tid_, iterationsCnt_, delta_, mu_, alpha_, beta_, lambda_, lambdau_, Gtse_old_, Gtse_new_, comp_, pnorm_, rnorm_, dwnorm_);
}
//--
void sLogger::loadDBConnInfo(int pid_, int tid_, char** oDBusername, char** oDBpassword, char** oDBconnstring) {
	if (source==OraData) safecall(oradb, loadDBConnInfo, pid_, tid_, oDBusername, oDBpassword, oDBconnstring);
	//if (source==FileData) safecall(filedb, loadDBConnInfo, pid_, tid_, oDBusername, oDBpassword, oDBconnstring);
}
void sLogger::saveDBConnInfo(int pid_, int tid_, char* oDBusername, char* oDBpassword, char* oDBconnstring) {
	if (source==OraData) safecall(oradb, saveDBConnInfo, pid_, tid_, oDBusername, oDBpassword, oDBconnstring);
	//if (source==FileData) safecall(filedb, saveDBConnInfo, pid_, tid_, oDBusername, oDBpassword, oDBconnstring);
}
//--
void sLogger::saveTradeInfo(int MT4clientPid, int MT4sessionId, int MT4accountId, int MT4enginePid, int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit) {
	//-- this should be treated as atomic (i.e. automatic commit)
	if (source==OraData) {
		safecall(oradb, saveTradeInfo, MT4clientPid, MT4sessionId, MT4accountId, MT4enginePid, iPositionTicket, iPositionOpenTime, iLastBarT, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTPhit, iSLhit);
	}
	//if (source==FileData) .....
}
//--
void sLogger::saveXMLconfig(int simulationId_, int pid_, int tid_, int fileId_, sCfg* cfg_) {

	//-- because sCfg does not know of sLogger, we need to extract all parameters from sCfg here, then call oradb/filedb routines
	
	int parmsCnt=cfg_->rootKey->getParmsCntTot();
	
	char** parmDesc=(char**)malloc(parmsCnt*sizeof(char*));
	char** parmVal=(char**)malloc(parmsCnt*sizeof(char*));
	for (int p=0; p<parmsCnt; p++) {
		parmDesc[p]=(char*)malloc(ObjMaxDepth*ObjNameMaxLen);
		parmVal[p]=(char*)malloc(XMLKEY_PARM_VAL_MAXCNT*XMLKEY_PARM_VAL_MAXLEN);
	}

	parmsCnt=0;
	cfg_->rootKey->getAllParms(&parmsCnt, parmDesc, parmVal);


	if (saveToDB) safecall(oradb, saveXMLconfig, simulationId_, pid_, tid_, fileId_, parmsCnt, parmDesc, parmVal);
	if (saveToFile) safecall(filedb, saveXMLconfig, simulationId_, pid_, tid_, fileId_, parmsCnt, parmDesc, parmVal);

	for (int p=0; p<parmsCnt; p++) {
		free(parmDesc[p]);
		free(parmVal[p]);
	}
	free(parmDesc);
	free(parmVal);
}