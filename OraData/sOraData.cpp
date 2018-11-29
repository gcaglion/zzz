#include "sOraData.h"
#include "sOraDBcommon.h"
#include <iostream>

sOraData::sOraData(sObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	//-- 1. get Parameters
	strcpy_s(DBUserName, DBUSERNAME_MAXLEN, DBUserName_);
	strcpy_s(DBPassword, DBPASSWORD_MAXLEN, DBPassword_);
	strcpy_s(DBConnString, DBCONNSTRING_MAXLEN, DBConnString_);
	//-- 2. open connection
	//if(autoOpen) safecall(this, open);
}
sOraData::sOraData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &DBUserName, "UserName");
	safecall(cfgKey, getParm, &DBPassword, "Password");
	safecall(cfgKey, getParm, &DBConnString, "ConnString");
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. Restore currentKey
	cfg->currentKey=bkpKey;

}
sOraData::~sOraData() {
	if (isOpen) {
		close();
	}
}

void sOraData::open() {
	try {
		if (env==nullptr) env = Environment::createEnvironment();
		if (conn==nullptr) conn = ((Environment*)env)->createConnection(DBUserName, DBPassword, DBConnString);
		isOpen=true;
	}
	catch (SQLException exc) {
		{
			fail("Exception: %s while trying to connect with %s/%s@%s", exc.what(), DBUserName, DBPassword, DBConnString);
		}
	}

}
void sOraData::close() {
	if (env!=nullptr) {
		if (conn!=nullptr) ((Environment*)env)->terminateConnection(((Connection*)conn));
		Environment::terminateEnvironment(((Environment*)env));
	}
}
void sOraData::commit() {
	if (conn!=nullptr) ((Connection*)conn)->commit();
}
//-- Read
void sOraData::getFlatOHLCV2(char* pSymbol, char* pTF, char* date0_, int stepsCnt, char** oBarTime, numtype* oBarData, char* oBarTime0, numtype* oBaseBar, numtype* oBarWidth) {
	int i;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		//-- 1. History: open statement and result set
		sprintf_s(sqlS, SQL_MAXLEN, "select to_char(newdatetime,'%s'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime<=to_date('%s','%s') order by 1 desc", DATE_FORMAT, pSymbol, pTF, date0_, DATE_FORMAT);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		//-- 2. History: get all records
		i=stepsCnt-1;
		while (((ResultSet*)rset)->next()&&i>=0) {
			strcpy_s(oBarTime[i], DATE_FORMAT_LEN, ((ResultSet*)rset)->getString(1).c_str());
			oBarData[5*i+0] = ((ResultSet*)rset)->getFloat(2);
			oBarData[5*i+1] = ((ResultSet*)rset)->getFloat(3);
			oBarData[5*i+2] = ((ResultSet*)rset)->getFloat(4);
			oBarData[5*i+3] = ((ResultSet*)rset)->getFloat(5);
			oBarData[5*i+4] = ((ResultSet*)rset)->getFloat(6);
			//--
			oBarWidth[i]	= oBarData[5*i+1]-oBarData[5*i+2];

			i--;
		}
		//-- 3. History: one more fetch to get baseBar
		strcpy_s(oBarTime0, DATE_FORMAT_LEN, ((ResultSet*)rset)->getString(1).c_str());
		for (int f=0; f<5; f++)	oBaseBar[f] = ((ResultSet*)rset)->getFloat(f+2);
		//-- 4. History: close result set and statement
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char*** oDate) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	// Retrieves plain ordered list of NewDateTime starting from StartDate onwards for <DatesCount> records
	int i;

	if (conn==nullptr) fail("DB Connection is closed. cannot continue.");

	try {
		sprintf_s(sqlS, SQL_MAXLEN, "select to_char(NewDateTime, '%s') from History.%s_%s%s where NewDateTime>=to_date('%s','%s') order by 1", DATE_FORMAT, symbol_, timeframe_, (isFilled_) ? "_FILLED " : "", StartDate, DATE_FORMAT);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();

		i=0;
		while (((ResultSet*)rset)->next()&&i<DatesCount) {
			strcpy_s((*oDate)[i], DATE_FORMAT_LEN, ((ResultSet*)rset)->getString(1).c_str());
			i++;
		}

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
//-- Write
void sOraData::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into TrainLog(ProcessId, ThreadId, Epoch, MSE_T, MSE_V) values(:P01, :P02, :P03, :P04, :P05)");
		((Statement*)stmt)->setMaxIterations(mseCnt);
		for (int epoch=0; epoch<mseCnt; epoch++) {
			((Statement*)stmt)->setInt(1, pid);
			((Statement*)stmt)->setInt(2, tid);
			((Statement*)stmt)->setInt(3, epoch);
			((Statement*)stmt)->setFloat(4, mseT[epoch]);
			((Statement*)stmt)->setFloat(5, mseV[epoch]);
			if (epoch<(mseCnt-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

}
void sOraData::saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted, numtype* barWidth) {

	int runCnt=runStepsCnt*selectedFeaturesCnt;
	int tsidx=0, runidx=0;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, Pos, PosLabel, Feature, StepAhead, PredictedTRS, ActualTRS, ErrorTRS, PredictedTR, ActualTR, ErrorTR, Predicted, Actual, Error, BarWidth, ErrorP) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16, :P17, :P18, :P19)");
		((Statement*)stmt)->setMaxIterations(runCnt);

		for (int s=0; s<runStepsCnt; s++) {
			for (int df=0; df<selectedFeaturesCnt; df++) {
				for (int tf=0; tf<tsFeaturesCnt_; tf++) {
					if (selectedFeature[df]==tf) {
						tsidx = s*tsFeaturesCnt_+tf;
						((Statement*)stmt)->setInt(1, pid);
						((Statement*)stmt)->setInt(2, tid);
						((Statement*)stmt)->setInt(3, npid);
						((Statement*)stmt)->setInt(4, ntid);
						((Statement*)stmt)->setInt(5, s);
						std::string str(posLabel[s]); ((Statement*)stmt)->setMaxParamSize(6, 64); ((Statement*)stmt)->setString(6, str);
						((Statement*)stmt)->setInt(7, tf);
						((Statement*)stmt)->setInt(8, 1);

						//-- for every Actual/Predicted/Error triplet, we need to handle NULL values

						((Statement*)stmt)->setFloat(10, actualTRS[tsidx]); //-- this can never be EMPTY_VALUE
						if (predictedTRS[tsidx]==EMPTY_VALUE) {
							((Statement*)stmt)->setNull(9, OCCIFLOAT);
							((Statement*)stmt)->setNull(11, OCCIFLOAT);
						} else {
							((Statement*)stmt)->setFloat(9, predictedTRS[tsidx]);
							((Statement*)stmt)->setFloat(11, fabs(actualTRS[tsidx]-predictedTRS[tsidx]));
						}
						//--
						((Statement*)stmt)->setFloat(13, actualTR[tsidx]); //-- this can never be EMPTY_VALUE
						if (predictedTR[tsidx]==EMPTY_VALUE) {
							((Statement*)stmt)->setNull(12, OCCIFLOAT);
							((Statement*)stmt)->setNull(14, OCCIFLOAT);
						} else {
							((Statement*)stmt)->setFloat(12, predictedTR[tsidx]);
							((Statement*)stmt)->setFloat(14, fabs(actualTR[tsidx]-predictedTR[tsidx]));
						}
						//--
						((Statement*)stmt)->setFloat(18, barWidth[s]);
						//--
						((Statement*)stmt)->setFloat(16, actual[tsidx]); //-- this can never be EMPTY_VALUE
						if (predicted[tsidx]==EMPTY_VALUE) {
							((Statement*)stmt)->setNull(15, OCCIFLOAT);
							((Statement*)stmt)->setNull(17, OCCIFLOAT);
							((Statement*)stmt)->setNull(19, OCCIFLOAT);
						} else {
							((Statement*)stmt)->setFloat(15, predicted[tsidx]);
							((Statement*)stmt)->setFloat(17, fabs(actual[tsidx]-predicted[tsidx]));
							if (barWidth[s]==0) {
								((Statement*)stmt)->setNull(19, OCCIFLOAT);
							} else {
								((Statement*)stmt)->setFloat(19, fabs(actual[tsidx]-predicted[tsidx])/barWidth[s]);
							}
						}

						if (runidx<(runCnt-1)) ((Statement*)stmt)->addIteration();
						runidx++;
					}
				}
			}
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		//-- insert spacers (one for every feature)
		stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, Pos, PosLabel, Feature, StepAhead, PredictedTRS, ActualTRS, ErrorTRS, PredictedTR, ActualTR, ErrorTR, Predicted, Actual, Error) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16, :P17)");
		((Statement*)stmt)->setMaxIterations(selectedFeaturesCnt);
		for (int f=0; f<selectedFeaturesCnt; f++) {
			((Statement*)stmt)->setInt(1, pid);
			((Statement*)stmt)->setInt(2, tid);
			((Statement*)stmt)->setInt(3, npid);
			((Statement*)stmt)->setInt(4, ntid);
			((Statement*)stmt)->setFloat(5, runStepsCnt-predictionLen-0.5f);
			((Statement*)stmt)->setMaxParamSize(6, 64); ((Statement*)stmt)->setNull(6, OCCISTRING);
			((Statement*)stmt)->setInt(7, selectedFeature[f]);
			((Statement*)stmt)->setInt(8, 1);
			((Statement*)stmt)->setNull(9, OCCIFLOAT);
			((Statement*)stmt)->setNull(10, OCCIFLOAT);
			((Statement*)stmt)->setNull(11, OCCIFLOAT);
			((Statement*)stmt)->setNull(12, OCCIFLOAT);
			((Statement*)stmt)->setNull(13, OCCIFLOAT);
			((Statement*)stmt)->setNull(14, OCCIFLOAT);
			((Statement*)stmt)->setNull(15, OCCIFLOAT);
			((Statement*)stmt)->setNull(16, OCCIFLOAT);
			((Statement*)stmt)->setNull(17, OCCIFLOAT);
			if (f<(selectedFeaturesCnt-1))((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);


	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

}

//-- Save Client Info
void sOraData::saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "insert into ClientInfo(ProcessId, SimulationId, ClientName, ClientStart, Duration, SimulationStartTrain, SimulationStartInfer, SimulationStartValid, DoTraining, DoTestRun, clientXMLFile, shapeXMLFile, actionXMLFile, engineXMLFile) values(%d, %d, '%s', sysdate, %f, '%s','%s', to_date('%s','%s'), %d, %d, '%s', '%s', '%s', '%s')", pid, simulationId, clientName, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, DATE_FORMAT, (doTrain ? 1 : 0), (doInfer ? 1 : 0), clientXMLfile_, shapeXMLfile_, actionXMLfile_, engineXMLfile_);
	safecall(this, sqlExec, sqlS);

}

//-- Save/Load core images
void sOraData::saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into CoreImage_NN (ProcessId, ThreadId, Epoch, WId, W) values(:P01, :P02, :P03, :P04, :P05)");

		//-- this version uses arrayUpdate()
		ub2* intLen = (ub2*)malloc(Wcnt*sizeof(int));
		ub2* floatLen = (ub2*)malloc(Wcnt*sizeof(numtype));
		for (int i=0; i<Wcnt; i++) {
			intLen[i]=sizeof(int);
			floatLen[i]=sizeof(numtype);
		}

		//-- first, need to malloc and init arrays for constant pid and tid. Cannot use Vinit, as I don't have an Alg, here
		int* pidArr=(int*)malloc(Wcnt*sizeof(int));
		int* tidArr=(int*)malloc(Wcnt*sizeof(int));
		int* epochArr=(int*)malloc(Wcnt*sizeof(int));
		int* WidArr=(int*)malloc(Wcnt*sizeof(int));

		for (int i=0; i<Wcnt; i++) {
			pidArr[i]=pid;
			tidArr[i]=tid;
			epochArr[i]=epoch;
			WidArr[i]=i;
		}

		((Statement*)stmt)->setDataBuffer(1, pidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(2, tidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(3, epochArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(4, WidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(5, W, OCCIFLOAT, sizeof(numtype), floatLen);

		((Statement*)stmt)->executeArrayUpdate(Wcnt);

		free(WidArr); free(epochArr); free(tidArr); free(pidArr); free(intLen); free(floatLen);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::saveCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sOraData::saveCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sOraData::saveCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sOraData::saveCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	//fail("Not implemented.");
}
void sOraData::loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- if a specific epoch is not provided, we first need to find the last epoch
	if (epoch==-1) {
		sprintf_s(sqlS, SQL_MAXLEN, "select max(epoch) from CoreImage_NN where processId = %d and ThreadId = %d", pid, tid);
		try {
			stmt = ((Connection*)conn)->createStatement(sqlS);
			rset = ((Statement*)stmt)->executeQuery();
			if (((ResultSet*)rset)->next()&&!((ResultSet*)rset)->isNull(1)) {
				epoch=((ResultSet*)rset)->getInt(1);
			} else {
				fail("Could not find max epoch for processId=%d, ThreadId=%d", pid, tid);
			}
		}
		catch (SQLException ex) {
			fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
		}
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}

	//-- once we have the epoch, we load Ws for that pid, tid, epoch
	try {
		sprintf_s(sqlS, SQL_MAXLEN, "select WId, W from CoreImage_NN where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1,2", pid, tid, epoch);
		stmt = ((Connection*)conn)->createStatement(sqlS);

		//-- this version uses arrayUpdate()
		ub2* intLen = (ub2*)malloc(Wcnt*sizeof(int));
		ub2* floatLen = (ub2*)malloc(Wcnt*sizeof(numtype));
		int* WidArr=(int*)malloc(Wcnt*sizeof(int));

		//-- first, need to malloc and init arrays for constant pid and tid. Cannot use Vinit, as I don't have an Alg, here
		for (int i=0; i<Wcnt; i++) {
			intLen[i]=sizeof(int);
			floatLen[i]=sizeof(numtype);
			WidArr[i]=i;
		}


		rset = ((Statement*)stmt)->executeQuery();
		((ResultSet*)rset)->setDataBuffer(1, WidArr, OCCIINT, sizeof(int), intLen);
		((ResultSet*)rset)->setDataBuffer(2, W, OCCIFLOAT, sizeof(numtype), floatLen);

		((ResultSet*)rset)->next(Wcnt);

		free(WidArr); free(intLen); free(floatLen);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

	//-- close result set and terminate statement before exiting
	((Statement*)stmt)->closeResultSet((ResultSet*)rset);
	((Connection*)conn)->terminateStatement((Statement*)stmt);

}
void sOraData::loadCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sOraData::loadCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sOraData::loadCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	fail("Not implemented.");
}
void sOraData::loadCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	info("Done nothing.");
}

//-- Save/Load Core<XXX>Paameters
void sOraData::saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_, int SCGDmaxK_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "insert into CoreNNparms(ProcessId, ThreadId, LevelRatioS, LevelActivationS, UseContext, UseBias, MaxEpochs, TargetMSE, NetSaveFrequency, StopOnDivergence, BPAlgo, BPStd_LearningRate, BPStd_LearningMomentum, BPscgd_maxK) \
														 values(%d, %d, '%s', '%s', %d, %d, %d, %f, %d, %d, %d, %f, %f, %d)", \
		pid, tid, levelRatioS_, levelActivationS_, (useContext_) ? 1 : 0, (useBias_) ? 1 : 0, maxEpochs_, targetMSE_, netSaveFrequency_, stopOnDivergence_, BPalgo_, learningRate_, learningMomentum_, SCGDmaxK_ \
	);
	safecall(this, sqlExec, sqlS);

}
void sOraData::saveCoreGAparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sOraData::saveCoreSVMparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sOraData::saveCoreSOMparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sOraData::saveCoreDUMBparms(int pid, int tid, int p1, numtype p2) {
	fail("Not implemented.");
}
void sOraData::loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open) {}

	sprintf_s(sqlS, SQL_MAXLEN, "select LevelRatioS, LevelActivationS, UseContext, UseBias, MaxEpochs, TargetMSE, NetSaveFrequency, StopOnDivergence, BPAlgo, BPStd_LearningRate, BPStd_LearningMomentum \
								from CoreNNparms where ProcessId=%d and ThreadId=%d", pid, tid);
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();

		int i=0;
		while (((ResultSet*)rset)->next()) {
			strcpy_s((*levelRatioS_), XMLKEY_PARM_VAL_MAXLEN, ((ResultSet*)rset)->getString(1).c_str());
			strcpy_s((*levelActivationS_), XMLKEY_PARM_VAL_MAXLEN, ((ResultSet*)rset)->getString(2).c_str());
			(*useContext_)=(((ResultSet*)rset)->getInt(3)==1);
			(*useBias_)=(((ResultSet*)rset)->getInt(4)==1);
			(*maxEpochs_)=((ResultSet*)rset)->getInt(5);
			(*targetMSE_)=((ResultSet*)rset)->getFloat(6);
			(*netSaveFrequency_)=((ResultSet*)rset)->getInt(7);
			(*stopOnDivergence_)=(((ResultSet*)rset)->getInt(8)==1);
			(*BPalgo_)=((ResultSet*)rset)->getInt(9);
			(*learningRate_)=((ResultSet*)rset)->getFloat(10);
			(*learningMomentum_)=((ResultSet*)rset)->getFloat(11);
			i++;
		}

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		if (i==0) fail("Core Parameters not found for ProcessId=%d, ThreadId=%d", pid, tid);

	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::loadCoreGAparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
void sOraData::loadCoreSVMparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
void sOraData::loadCoreSOMparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}
void sOraData::loadCoreDUMBparms(int pid, int tid, int* p1, numtype* p2) {
	fail("Not implemented.");
}

//-- Save/Load engine info
void sOraData::saveEngineInfo(int pid, int engineType, int coresCnt, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- 1. ENGINES
	sprintf_s(sqlS, SQL_MAXLEN, "insert into Engines(ProcessId, EngineType) values(%d, %d)", pid, engineType);
	safecall(this, sqlExec, sqlS);

	//-- 2. ENGINECORES
	for (int c=0; c<coresCnt; c++) {
		sprintf_s(sqlS, SQL_MAXLEN, "insert into EngineCores(EnginePid, CoreId, CoreThreadId, CoreType) values(%d, %d, %d, %d)", pid, coreId[c], tid[c], coreType[c]);
		safecall(this, sqlExec, sqlS);
		//-- 3. CORELAYOUTS
		for (int cp=0; cp<parentCoresCnt[c]; cp++) {
			sprintf_s(sqlS, SQL_MAXLEN, "insert into CoreLayouts(EnginePid, CoreId, ParentCoreId, ParentConnType) values(%d, %d, %d, %d)", pid, c, parentCore[c][cp], parentConnType[c][cp]);
			safecall(this, sqlExec, sqlS);
		}
	}

}
void sOraData::loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- nested statement and result set
	char nsqlS[SQL_MAXLEN]; Statement* nstmt; ResultSet* nrset;

	//-- 1. coresCnt, coreId, coreType
	sprintf_s(sqlS, SQL_MAXLEN, "select CoreId, CoreType, CoreThreadId from EngineCores where EnginePid= %d", pid);
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		int i=0;
		while (((ResultSet*)rset)->next()) {
			coreId[i]=((ResultSet*)rset)->getInt(1);
			coreType[i]=((ResultSet*)rset)->getInt(2);
			tid[i]=((ResultSet*)rset)->getInt(3);

			sprintf_s(nsqlS, SQL_MAXLEN, "select ParentCoreId, ParentConnType from CoreLayouts where EnginePid= %d and CoreId= %d", pid, coreId[i]);
			nstmt = ((Connection*)conn)->createStatement(nsqlS);
			nrset = ((Statement*)nstmt)->executeQuery();
			int ni=0;
			while (nrset->next()) {
				parentCore[i][ni]=nrset->getInt(1);
				parentConnType[i][ni]=nrset->getInt(2);

				ni++;
			}
			parentCoresCnt[i]=ni;

			i++;
		}
		(*coresCnt)=i;

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

	//--
}


//-- private stuff
void sOraData::sqlExec(char* sqlS) {
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::sqlGet(int len, int** valP, const char* sqlMask, ...) {
	try {

		va_list va_args;
		va_start(va_args, sqlMask);
		vsprintf_s(sqlS, ObjNameMaxLen, sqlMask, va_args);
		va_end(va_args);

		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();

		int i=0;
		while (((ResultSet*)rset)->next()&&i<len) {
			(*valP)[i]=((ResultSet*)rset)->getInt(1);
			i++;
		}

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}