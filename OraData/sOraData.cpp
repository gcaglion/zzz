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
		if(env==nullptr) env = Environment::createEnvironment();
		if (conn==nullptr) conn = ((Environment*)env)->createConnection(DBUserName, DBPassword, DBConnString);
		isOpen=true;
	} catch (SQLException exc) {
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
	((Connection*)conn)->commit();
}
//-- Read
void sOraData::getFlatOHLCV2(char* pSymbol, char* pTF, char* date0_, int stepsCnt, char** oBarTime, float* oBarData, char* oBarTime0, float* oBaseBar) {
	int i;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		//-- 1. History: open statement and result set
		sprintf_s(sql, SQL_MAXLEN, "select to_char(newdatetime,'%s'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime<=to_date('%s','%s') order by 1 desc", DATE_FORMAT, pSymbol, pTF, date0_, DATE_FORMAT);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = ((Statement*)stmt)->executeQuery();
		//-- 2. History: get all records
		i=stepsCnt-1;
		std::string dateS;
		while (rset->next()&&i>=0) {
			dateS=rset->getString(1);
			strcpy_s(oBarTime[i], DATE_FORMAT_LEN, dateS.c_str());
//			strcpy_s(oBarTime[i], DATE_FORMAT_LEN, rset->getString(1).c_str());
			oBarData[5*i+0] = rset->getFloat(2);
			oBarData[5*i+1] = rset->getFloat(3);
			oBarData[5*i+2] = rset->getFloat(4);
			oBarData[5*i+3] = rset->getFloat(5);
			oBarData[5*i+4] = rset->getFloat(6);
			i--;
		}
		//-- 3. History: one more fetch to get baseBar
		strcpy_s(oBarTime0, DATE_FORMAT_LEN, rset->getString(1).c_str());
		for (int f=0; f<5; f++)	oBaseBar[f] = rset->getFloat(f+2);
		//-- 4. History: close result set and statement
		((Statement*)stmt)->closeResultSet(rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	} catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char*** oDate) {
	
	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	// Retrieves plain ordered list of NewDateTime starting from StartDate onwards for <DatesCount> records
	int i;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	if (conn==nullptr) fail("DB Connection is closed. cannot continue.");

	try {
		sprintf_s(sql, SQL_MAXLEN, "select to_char(NewDateTime, '%s') from History.%s_%s%s where NewDateTime>=to_date('%s','%s') order by 1", DATE_FORMAT, symbol_, timeframe_, (isFilled_)?"_FILLED ":"", StartDate, DATE_FORMAT);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = ((Statement*)stmt)->executeQuery();

		i=0;
		while (rset->next()&&i<DatesCount) {
			strcpy_s((*oDate)[i], DATE_FORMAT_LEN, rset->getString(1).c_str());
			i++;
		}

		((Statement*)stmt)->closeResultSet(rset);
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
			if(epoch<(mseCnt-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	} catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
	
}
void sOraData::saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted) {

	int runCnt=runStepsCnt*selectedFeaturesCnt;
	int tsidx=0, runidx=0;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, Pos, PosLabel, Feature, StepAhead, PredictedTRS, ActualTRS, ErrorTRS, PredictedTR, ActualTR, ErrorTR, Predicted, Actual, Error) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16, :P17)");
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
						((Statement*)stmt)->setFloat(16, actual[tsidx]); //-- this can never be EMPTY_VALUE
						if (predicted[tsidx]==EMPTY_VALUE) {
							((Statement*)stmt)->setNull(15, OCCIFLOAT);
							((Statement*)stmt)->setNull(17, OCCIFLOAT);
						} else {
							((Statement*)stmt)->setFloat(15, predicted[tsidx]);
							((Statement*)stmt)->setFloat(17, fabs(actual[tsidx]-predicted[tsidx]));
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
			if(f<(selectedFeaturesCnt-1))((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);


	} catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

}
void sOraData::saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	char stmtS[SQL_MAXLEN]; sprintf_s(stmtS, SQL_MAXLEN, "insert into ClientInfo(ProcessId, SimulationId, ClientName, ClientStart, Duration, SimulationStartTrain, SimulationStartInfer, SimulationStartValid, DoTraining, DoTrainRun) values(%d, %d, '%s', sysdate, %f, to_date('%s','%s'), to_date('%s','%s'), to_date('%s','%s'), %d, %d)",	pid, simulationId, clientName, elapsedSecs, simulStartTrain, DATE_FORMAT, simulStartInfer, DATE_FORMAT, simulStartValid, DATE_FORMAT, (doTrain?1:0), (doTestRun?1:0) );

	try {
		stmt = ((Connection*)conn)->createStatement(stmtS);
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	} catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}

//-- Save/Load core images
void sOraData::coreNNsaveImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into CoreImage_NN (ProcessId, ThreadId, Epoch, WId, W) values(:P01, :P02, :P03, :P04, :P05)");
		((Statement*)stmt)->setMaxIterations(Wcnt);
		for (int i=0; i<Wcnt; i++) {
			((Statement*)stmt)->setInt(1, pid);
			((Statement*)stmt)->setInt(2, tid);
			((Statement*)stmt)->setInt(3, epoch);
			((Statement*)stmt)->setInt(4, i);
			((Statement*)stmt)->setFloat(5, W[i]);
			if (i<(Wcnt-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::coreNNloadImage(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	char sql[SQL_MAXLEN];
	ResultSet *rset;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- if a specific epoch is not provided, we first need to find the last epoch
	if (epoch==-1) {
		sprintf_s(sql, SQL_MAXLEN, "select max(epoch) from CoreImage_NN where processId = %d and ThreadId = %d", pid, tid);
		try {
			stmt = ((Connection*)conn)->createStatement(sql);
			rset = ((Statement*)stmt)->executeQuery();
			if (rset->next()&&!rset->isNull(1)) {
				epoch=rset->getInt(1);
			} else {
				fail("Could not find max epoch for processId=%d, ThreadId=%d", pid, tid);
			}
		}
		catch (SQLException ex) {
			fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
		}
		((Statement*)stmt)->closeResultSet(rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}

	//-- once we have the epoch, we load Ws for that pid, tid, epoch
	int i=0;
	sprintf_s(sql, SQL_MAXLEN, "select WId, W from CoreImage_NN where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1,2", pid, tid, epoch);
	try {
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = ((Statement*)stmt)->executeQuery();
		while (rset->next()&&i<Wcnt) {
			W[i] = rset->getFloat(2);
			i++;
		}
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

	//-- close result set and terminate statement before exiting
	((Statement*)stmt)->closeResultSet(rset);
	((Connection*)conn)->terminateStatement((Statement*)stmt);

}

//-- Save/Load engine info
void sOraData::saveEngineImage(){

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- 1. ENGINES
	//-- 2. ENGINECORES
	//-- 3. CORELAYOUTS

	try {
		stmt = ((Connection*)conn)->createStatement("insert into CoreImage_NN (ProcessId, ThreadId, Epoch, WId, W) values(:P01, :P02, :P03, :P04, :P05)");
		((Statement*)stmt)->setMaxIterations(Wcnt);
		for (int i=0; i<Wcnt; i++) {
			((Statement*)stmt)->setInt(1, pid);
			((Statement*)stmt)->setInt(2, tid);
			((Statement*)stmt)->setInt(3, epoch);
			((Statement*)stmt)->setInt(4, i);
			((Statement*)stmt)->setFloat(5, W[i]);
			if (i<(Wcnt-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::loadEngineImage() {}
