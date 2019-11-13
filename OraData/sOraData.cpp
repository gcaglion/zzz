#include "sOraData.h"
#include "sOraDBcommon.h"
#include <iostream>

sOraData::sOraData(sObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_) : sCfgObj(sObjParmsVal, nullptr, "") {
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
	if (isOpen)	close();
	delete DBUserName; delete DBPassword; delete DBConnString;
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
void sOraData::getFutureBar(char* iSymbol_, char* iTF_, char* iDate0_, char* oDate1_, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV) {
	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		//-- 1. History: open statement and result set
		sprintf_s(sqlS, SQL_MAXLEN, "select to_char(newdatetime,'%s'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime >= to_date('%s','%s') order by 1", DATE_FORMAT, iSymbol_, iTF_, iDate0_, DATE_FORMAT);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		//-- 2. History: get all records
		int i=0;
		while (((ResultSet*)rset)->next()&&i<1) {
			strcpy_s(oDate1_, DATE_FORMAT_LEN, ((ResultSet*)rset)->getString(1).c_str());
			(*oBarO) = ((ResultSet*)rset)->getFloat(2);
			(*oBarH) = ((ResultSet*)rset)->getFloat(3);
			(*oBarL) = ((ResultSet*)rset)->getFloat(4);
			(*oBarC) = ((ResultSet*)rset)->getFloat(5);
			(*oBarV) = ((ResultSet*)rset)->getFloat(6);

			i++;
		}
		if (i==0) fail("not enough records.");

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::getFlatOHLCV2(char* pSymbol, char* pTF, const char* date0_, int stepsCnt, char** oBarTime, numtype* oBarData, char* oBarTime0, numtype* oBaseBar, numtype* oBarWidth) {
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
		if (i>=0) fail("not enough records. stepsCnt=%d ; missing=%d", stepsCnt, i);

		//-- 3. History: one more fetch to get baseBar
		strcpy_s(oBarTime0, DATE_FORMAT_LEN, ((ResultSet*)rset)->getString(1).c_str());
		for (int f=0; f<5; f++)	oBaseBar[f] = ((ResultSet*)rset)->getFloat(f+2);
		//-- 4. History: close result set and statement
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
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
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}
}
//-- Write
void sOraData::saveMSE(int pid, int tid, int mseCnt, int* duration, numtype* mseT, numtype* mseV) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into TrainLog(ProcessId, ThreadId, Epoch, Duration, MSE_T, MSE_V) values(:P01, :P02, :P03, :P04, :P05, :P06)");
		((Statement*)stmt)->setMaxIterations(mseCnt);
		for (int epoch=0; epoch<mseCnt; epoch++) {
			((Statement*)stmt)->setInt(1, pid);
			((Statement*)stmt)->setInt(2, tid);
			((Statement*)stmt)->setInt(3, epoch);
			((Statement*)stmt)->setInt(4, duration[epoch]);
			#ifdef DOUBLE_NUMTYPE
			((Statement*)stmt)->setDouble(5, mseT[epoch]);
			((Statement*)stmt)->setDouble(6, mseV[epoch]);
			#else
			((Statement*)stmt)->setFloat(5, mseT[epoch]);
			((Statement*)stmt)->setFloat(6, mseV[epoch]);
			#endif
			if (epoch<(mseCnt-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

}
void sOraData::saveRun2(int pid, int tid, int npid, int ntid, int seqId, numtype mseR, int runStepsCnt, char** posLabel, int i, int d, int f, int l, numtype***** actualTRS, numtype***** predictedTRS, numtype***** actualTR, numtype***** predictedTR, numtype***** actualBASE, numtype***** predictedBASE) {
	int runidx=0;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, SequenceId, mseR, Pos, PosLabel, DataSource, Feature, WTlevel, ActualTRS, PredictedTRS, ActualTR, PredictedTR, ActualBASE, PredictedBASE) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16, :P17)");
		((Statement*)stmt)->setMaxIterations(runStepsCnt);
		for (int step=0; step<runStepsCnt; step++) {
			((Statement*)stmt)->setInt(1, pid);
			((Statement*)stmt)->setInt(2, tid);
			((Statement*)stmt)->setInt(3, npid);
			((Statement*)stmt)->setInt(4, ntid);
			((Statement*)stmt)->setInt(5, seqId);
			#ifdef DOUBLE_NUMTYPE
			((Statement*)stmt)->setDouble(6, mseR);
			#else
			((Statement*)stmt)->setFloat(6, mseR);
			#endif
			((Statement*)stmt)->setInt(7, step);
			std::string str(posLabel[step]); ((Statement*)stmt)->setMaxParamSize(8, 64); ((Statement*)stmt)->setString(8, str);
			((Statement*)stmt)->setInt(9, d);
			((Statement*)stmt)->setInt(10, f);
			((Statement*)stmt)->setInt(11, l);
			if (actualTRS[step][i][d][f][l]==EMPTY_VALUE) {
				((Statement*)stmt)->setNull(12, OCCIFLOAT);
			} else {
				#ifdef DOUBLE_NUMTYPE
				((Statement*)stmt)->setDouble(12, actualTRS[step][i][d][f][l]);
				#else
				((Statement*)stmt)->setFloat(12, actualTRS[step][i][d][f][l]);
				#endif
			}
			if (predictedTRS[step][i][d][f][l]==EMPTY_VALUE) {
				((Statement*)stmt)->setNull(13, OCCIFLOAT);
			} else {
				#ifdef DOUBLE_NUMTYPE
				((Statement*)stmt)->setDouble(13, predictedTRS[step][i][d][f][l]);
				#else
				((Statement*)stmt)->setFloat(13, predictedTRS[step][i][d][f][l]);
				#endif
			}
			if (actualTR[step][i][d][f][l]==EMPTY_VALUE) {
				((Statement*)stmt)->setNull(14, OCCIFLOAT);
			} else {
				#ifdef DOUBLE_NUMTYPE
				((Statement*)stmt)->setDouble(14, actualTR[step][i][d][f][l]);
				#else
				((Statement*)stmt)->setFloat(14, actualTR[step][i][d][f][l]);
				#endif
			}
			if (predictedTR[step][i][d][f][l]==EMPTY_VALUE) {
				((Statement*)stmt)->setNull(15, OCCIFLOAT);
			} else {
				#ifdef DOUBLE_NUMTYPE
				((Statement*)stmt)->setDouble(15, predictedTR[step][i][d][f][l]);
				#else
				((Statement*)stmt)->setFloat(15, predictedTR[step][i][d][f][l]);
				#endif
			}
			if (actualBASE[step][i][d][f][l]==EMPTY_VALUE) {
				((Statement*)stmt)->setNull(16, OCCIFLOAT);
			} else {
				#ifdef DOUBLE_NUMTYPE
				((Statement*)stmt)->setDouble(16, actualBASE[step][i][d][f][l]);
				#else
				((Statement*)stmt)->setFloat(16, actualBASE[step][i][d][f][l]);
				#endif
			}
			if (predictedBASE[step][i][d][f][l]==EMPTY_VALUE) {
				((Statement*)stmt)->setNull(17, OCCIFLOAT);
			} else {
				#ifdef DOUBLE_NUMTYPE
				((Statement*)stmt)->setDouble(17, predictedBASE[step][i][d][f][l]);
				#else
				((Statement*)stmt)->setFloat(17, predictedBASE[step][i][d][f][l]);
				#endif
			}

			if (runidx<(runStepsCnt-1)) ((Statement*)stmt)->addIteration();
			runidx++;
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

	}
void sOraData::saveRun(int pid, int tid, int npid, int ntid, int seqId, numtype mseR, int runStepsCnt, char** posLabel, int featuresCnt_, int WTlevel_, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actualBASE, numtype* predictedBASE) {

	int runCnt=runStepsCnt*featuresCnt_*(WTlevel_+1);
	int runidx=0;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, mseR, Pos, PosLabel, Feature, WTlevel, ActualTRS, PredictedTRS, ActualTR, PredictedTR, ActualBASE, PredictedBASE, SequenceId) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16)");
		((Statement*)stmt)->setMaxIterations(runCnt);
		for (int step=0; step<runStepsCnt; step++) {
			for (int f=0; f<featuresCnt_; f++) {
				for (int l=0; l<(WTlevel_+1); l++) {
					((Statement*)stmt)->setInt(1, pid);
					((Statement*)stmt)->setInt(2, tid);
					((Statement*)stmt)->setInt(3, npid);
					((Statement*)stmt)->setInt(4, ntid);
					#ifdef DOUBLE_NUMTYPE
					((Statement*)stmt)->setDouble(5, mseR);
					#else
					((Statement*)stmt)->setFloat(5, mseR);
					#endif
					((Statement*)stmt)->setInt(6, step);
					std::string str(posLabel[step]); ((Statement*)stmt)->setMaxParamSize(7, 64); ((Statement*)stmt)->setString(7, str);
					((Statement*)stmt)->setInt(8, f);
					((Statement*)stmt)->setInt(9, l);

					if (actualTRS[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]==EMPTY_VALUE) {
						((Statement*)stmt)->setNull(10, OCCIFLOAT);
					} else {
					#ifdef DOUBLE_NUMTYPE
						((Statement*)stmt)->setDouble(10, actualTRS[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
					#else
						((Statement*)stmt)->setFloat(10, actualTRS[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
					#endif
					}
					if (predictedTRS[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]==EMPTY_VALUE) {
						((Statement*)stmt)->setNull(11, OCCIFLOAT);
					} else {
						#ifdef DOUBLE_NUMTYPE
						((Statement*)stmt)->setDouble(11, predictedTRS[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#else
						((Statement*)stmt)->setFloat(11, predictedTRS[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#endif
					}

					if (actualTR[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]==EMPTY_VALUE) {
						((Statement*)stmt)->setNull(12, OCCIFLOAT);
					} else {
						#ifdef DOUBLE_NUMTYPE
						((Statement*)stmt)->setDouble(12, actualTR[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#else
						((Statement*)stmt)->setFloat(12, actualTR[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#endif
					}
					if (predictedTR[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]==EMPTY_VALUE) {
						((Statement*)stmt)->setNull(13, OCCIFLOAT);
					} else {
						#ifdef DOUBLE_NUMTYPE
						((Statement*)stmt)->setDouble(13, predictedTR[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#else
						((Statement*)stmt)->setFloat(13, predictedTR[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#endif
					}

					if (actualBASE[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]==EMPTY_VALUE) {
						((Statement*)stmt)->setNull(14, OCCIFLOAT);
					} else {
						#ifdef DOUBLE_NUMTYPE
						((Statement*)stmt)->setDouble(14, actualBASE[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#else
						((Statement*)stmt)->setFloat(14, actualBASE[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#endif
					}
					if (predictedBASE[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]==EMPTY_VALUE) {
						((Statement*)stmt)->setNull(15, OCCIFLOAT);
					} else {
						#ifdef DOUBLE_NUMTYPE
						((Statement*)stmt)->setDouble(15, predictedBASE[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#else
						((Statement*)stmt)->setFloat(15, predictedBASE[step*featuresCnt_*(WTlevel_+1)+f*(WTlevel_+1)+l]);
						#endif
					}
					((Statement*)stmt)->setInt(16, seqId);

					if (runidx<(runCnt-1)) ((Statement*)stmt)->addIteration();
					runidx++;
				}
			}
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

}
/*void sOraData::saveRun(int pid, int tid, int npid, int ntid, numtype mseR, int runStepsCnt, int featuresCnt_, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actualBASE, numtype* predictedBASE) {

	int runCnt=runStepsCnt*featuresCnt_;
	int runidx=0;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, mseR, Pos, PosLabel, Feature, ActualTRS, PredictedTRS, ErrorTRS, ActualTR, PredictedTR, ErrorTR, ActualBASE, PredictedBASE, ErrorBASE) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16, :P17)");
		((Statement*)stmt)->setMaxIterations(runCnt);
		for (int step=0; step<runStepsCnt; step++) {
			for (int f=0; f<featuresCnt_; f++) {
				((Statement*)stmt)->setInt(1, pid);
				((Statement*)stmt)->setInt(2, tid);
				((Statement*)stmt)->setInt(3, npid);
				((Statement*)stmt)->setInt(4, ntid);
				((Statement*)stmt)->setFloat(5, mseR);
				((Statement*)stmt)->setInt(6, step);
				std::string str(posLabel[step]); ((Statement*)stmt)->setMaxParamSize(7, 64); ((Statement*)stmt)->setString(7, str);
				((Statement*)stmt)->setInt(8, f);

				if (actualTRS[step*featuresCnt_+f]==EMPTY_VALUE) {
					((Statement*)stmt)->setNull(9, OCCIFLOAT);
					((Statement*)stmt)->setNull(11, OCCIFLOAT);
				} else {
					((Statement*)stmt)->setFloat(9, actualTRS[step*featuresCnt_+f]);
					((Statement*)stmt)->setFloat(11, fabs(actualTRS[step*featuresCnt_+f]-predictedTRS[step*featuresCnt_+f]));
				}

				((Statement*)stmt)->setFloat(10, predictedTRS[step*featuresCnt_+f]);

				if (actualTR[step*featuresCnt_+f]==EMPTY_VALUE) {
					((Statement*)stmt)->setNull(12, OCCIFLOAT);
					((Statement*)stmt)->setNull(14, OCCIFLOAT);
				} else {
					((Statement*)stmt)->setFloat(12, actualTR[step*featuresCnt_+f]);
					((Statement*)stmt)->setFloat(14, fabs(actualTR[step*featuresCnt_+f]-predictedTR[step*featuresCnt_+f]));
				}

				((Statement*)stmt)->setFloat(13, predictedTR[step*featuresCnt_+f]);

				if (actualBASE[step*featuresCnt_+f]==EMPTY_VALUE) {
					((Statement*)stmt)->setNull(15, OCCIFLOAT);
					((Statement*)stmt)->setNull(17, OCCIFLOAT);
				} else {
					((Statement*)stmt)->setFloat(15, actualBASE[step*featuresCnt_+f]);
					((Statement*)stmt)->setFloat(17, fabs(actualBASE[step*featuresCnt_+f]-predictedBASE[step*featuresCnt_+f]));
				}

				((Statement*)stmt)->setFloat(16, predictedBASE[step*featuresCnt_+f]);


				if (runidx<(runCnt-1)) ((Statement*)stmt)->addIteration();
				runidx++;
			}
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), ((Statement*)stmt)->getSQL().c_str());
	}

}
*/
//-- Save Client Info
void sOraData::findPid(int pid_, bool* found_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "select ProcessId from ClientInfo where ProcessId = %d", pid_);

	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		
		int i=0;
		while (((ResultSet*)rset)->next()) i++;
		(*found_)=(i>0);

	} catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}


}
void sOraData::saveClientInfo(int pid, int sequenceId, int simulationId, int npid, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "insert into ClientInfo(ProcessId, SequenceId, SimulationId, NetProcessId, ClientName, ClientStart, Duration, SimulationStartTrain, SimulationStartInfer, SimulationStartValid, DoTraining, DoTestRun, clientXMLFile, shapeXMLFile, actionXMLFile, engineXMLFile) values(%d, %d, %d, %d, '%s', sysdate, %f, '%s','%s', to_date('%s','%s'), %d, %d, '%s', '%s', '%s', '%s')", pid, sequenceId, simulationId, npid, clientName, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, DATE_FORMAT, (doTrain ? 1 : 0), (doInfer ? 1 : 0), clientXMLfile_, shapeXMLfile_, actionXMLfile_, engineXMLfile_);
	safecall(this, sqlExec, sqlS);

}

void sOraData::saveCoreInfo(int pid, int tid, int coreType_, int sampleLen_, int inputCnt_, int targetLen_, int outputCnt_, int batchSize_, numtype* trMin_, numtype* trMax_) {
	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "insert into CoreInfo(ProcessId, ThreadId, CoreType, sampleLen, targetLen, inputCnt, outputCnt, batchSize) values (%d, %d, %d, %d, %d, %d, %d, %d)", pid, tid, coreType_, sampleLen_, targetLen_, inputCnt_, outputCnt_, batchSize_);
	safecall(this, sqlExec, sqlS);

	for (int i=0; i<inputCnt_; i++) {
		sprintf_s(sqlS, SQL_MAXLEN, "insert into CoreScalingInfo(ProcessId, ThreadId, InputId, TRmin, TRmax) values(%d, %d, %d, %f, %f)", pid, tid, i, trMin_[i], trMax_[i]);
		safecall(this, sqlExec, sqlS);
	}
}
void sOraData::loadCoreInfo(int pid, int tid, int* coreType_, int* sampleLen_, int* inputCnt_, int* targetLen_, int* outputCnt_, int* batchSize_, numtype* trMin_, numtype* trMax_) {
	
	//-- always check this, first!
	if (!isOpen) safecall(this, open) {}

	sprintf_s(sqlS, SQL_MAXLEN, "select CoreType, SampleLen, TargetLen, InputCnt, OutputCnt, BatchSize from CoreInfo where ProcessId=%d and ThreadId=%d", pid, tid);
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();

		int i=0;
		while (((ResultSet*)rset)->next()) {
			(*coreType_)=((ResultSet*)rset)->getInt(1);
			(*sampleLen_)=((ResultSet*)rset)->getInt(2);
			(*inputCnt_)=((ResultSet*)rset)->getInt(3);
			(*outputCnt_)=((ResultSet*)rset)->getInt(4);
			(*batchSize_)=((ResultSet*)rset)->getInt(5);
			i++;
		}
		if (i==0) fail("Core Info not found for ProcessId=%d, ThreadId=%d", pid, tid);

		((Connection*)conn)->terminateStatement((Statement*)stmt);
		sprintf_s(sqlS, SQL_MAXLEN, "select InputId, TRmin, TRmax from CoreScalingInfo where ProcessId=%d and ThreadId=%d order by InputId", pid, tid);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		i=0;
		while (((ResultSet*)rset)->next()) {
			trMin_[i]=((ResultSet*)rset)->getFloat(2);
			trMax_[i]=((ResultSet*)rset)->getFloat(3);
			i++;
		}
		if (i==0) fail("Core Scaling Info not found for ProcessId=%d, ThreadId=%d", pid, tid);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

}
//--
void sOraData::saveCoreLayout(int pid, int tid, int parentCoresCnt, int* parentCore, int* parentConnType) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	for (int cp=0; cp<parentCoresCnt; cp++) {
		sprintf_s(sqlS, SQL_MAXLEN, "insert into CoreLayouts(EnginePid, CoreId, ParentCoreId, ParentConnType) values(%d, %d, %d, %d)", pid, 0, parentCore[cp], parentConnType[cp]);
		safecall(this, sqlExec, sqlS);
	}
}
void sOraData::loadCoreLayout(int pid, int tid, int* parentCoresCnt, int** parentCore, int** parentConnType) {

}

//-- Save/Load core images
void sOraData::saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F) {
	ub2* intLen; ub2* floatLen;
	int* pidArr; int* tidArr; int* epochArr; 
	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {

		//-- 1. Weights
		stmt = ((Connection*)conn)->createStatement("insert into CoreImage_NN_W (ProcessId, ThreadId, Epoch, WId, W) values(:P01, :P02, :P03, :P04, :P05)");
		//-- this version uses arrayUpdate()
		intLen = (ub2*)malloc(Wcnt*sizeof(int));
		floatLen = (ub2*)malloc(Wcnt*sizeof(numtype));
		for (int i=0; i<Wcnt; i++) {
			intLen[i]=sizeof(int);
			floatLen[i]=sizeof(numtype);
		}
		//-- first, need to malloc and init arrays for constant pid and tid. Cannot use Vinit, as I don't have an Alg, here
		pidArr=(int*)malloc(Wcnt*sizeof(int));
		tidArr=(int*)malloc(Wcnt*sizeof(int));
		epochArr=(int*)malloc(Wcnt*sizeof(int));
		int* WidArr=(int*)malloc(Wcnt*sizeof(int));
		for (int i=0; i<Wcnt; i++) {
			pidArr[i]=pid;
			tidArr[i]=tid;
			epochArr[i]=epoch;
			WidArr[i]=i;
		}
		//-- set data buffers
		((Statement*)stmt)->setDataBuffer(1, pidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(2, tidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(3, epochArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(4, WidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(5, W, OCCIFLOAT, sizeof(numtype), floatLen);
		//-- execute
		((Statement*)stmt)->executeArrayUpdate(Wcnt);
		//-- free(s)
		free(WidArr); free(epochArr); free(tidArr); free(pidArr); free(intLen); free(floatLen);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		//-- 2. Neurons
		stmt = ((Connection*)conn)->createStatement("insert into CoreImage_NN_N (ProcessId, ThreadId, Epoch, NId, F) values(:P01, :P02, :P03, :P04, :P05)");
		//-- this version uses arrayUpdate()
		intLen = (ub2*)malloc(Fcnt*sizeof(int));
		floatLen = (ub2*)malloc(Fcnt*sizeof(numtype));
		for (int i=0; i<Fcnt; i++) {
			intLen[i]=sizeof(int);
			floatLen[i]=sizeof(numtype);
		}
		//-- first, need to malloc and init arrays for constant pid and tid. Cannot use Vinit, as I don't have an Alg, here
		pidArr=(int*)malloc(Fcnt*sizeof(int));
		tidArr=(int*)malloc(Fcnt*sizeof(int));
		epochArr=(int*)malloc(Fcnt*sizeof(int));
		int* FidArr=(int*)malloc(Fcnt*sizeof(int));
		for (int i=0; i<Fcnt; i++) {
			pidArr[i]=pid;
			tidArr[i]=tid;
			epochArr[i]=epoch;
			FidArr[i]=i;
		}
		//-- set data buffers
		((Statement*)stmt)->setDataBuffer(1, pidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(2, tidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(3, epochArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(4, FidArr, OCCIINT, sizeof(int), intLen);
		((Statement*)stmt)->setDataBuffer(5, F, OCCIFLOAT, sizeof(numtype), floatLen);
		//-- execute
		((Statement*)stmt)->executeArrayUpdate(Fcnt);
		//-- free(s)
		free(FidArr); free(epochArr); free(tidArr); free(pidArr); free(intLen); free(floatLen);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
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
void sOraData::loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F) {

	ub2* intLen;
	ub2* floatLen;

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- if a specific epoch is not provided, we first need to find the last epoch
	if (epoch==-1) {
		sprintf_s(sqlS, SQL_MAXLEN, "select max(epoch) from TrainLog where processId = %d and ThreadId = %d", pid, tid);
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
			fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
		}
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}

	//-- once we have the epoch, we load Ws for that pid, tid, epoch
	try {

		//-- 1. Weights
		//sprintf_s(sqlS, SQL_MAXLEN, "select /*+INDEX_FFS(CoreImage_NN_W CoreImage_NN_W_PK) Parallel_Index(20)*/ WId, W from CoreImage_NN_W where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1", pid, tid, epoch);
		sprintf_s(sqlS, SQL_MAXLEN, "select WId, W from CoreImage_NN_W where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1", pid, tid, epoch);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		//-- this version uses arrayUpdate()
		intLen = (ub2*)malloc(Wcnt*sizeof(int));
		floatLen = (ub2*)malloc(Wcnt*sizeof(numtype));
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
		//--
		((ResultSet*)rset)->next(Wcnt);
		if (((ResultSet*)rset)->status()!=ResultSet::DATA_AVAILABLE) {
			fail("Could not find Core Image for ProcessId=%d , ThreadId=%d , Epoch=%d", pid, tid, epoch);
		}
		//-- free(s)
		free(WidArr); free(intLen); free(floatLen);
		//-- close result set and terminate statement before exiting
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		//-- 2. Neurons
		sprintf_s(sqlS, SQL_MAXLEN, "select NId, F from CoreImage_NN_N where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1,2", pid, tid, epoch);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		//-- this version uses arrayUpdate()
		intLen = (ub2*)malloc(Fcnt*sizeof(int));
		floatLen = (ub2*)malloc(Fcnt*sizeof(numtype));
		int* FidArr=(int*)malloc(Fcnt*sizeof(int));
		//-- first, need to malloc and init arrays for constant pid and tid. Cannot use Vinit, as I don't have an Alg, here
		for (int i=0; i<Fcnt; i++) {
			intLen[i]=sizeof(int);
			floatLen[i]=sizeof(numtype);
			FidArr[i]=i;
		}
		rset = ((Statement*)stmt)->executeQuery();
		((ResultSet*)rset)->setDataBuffer(1, FidArr, OCCIINT, sizeof(int), intLen);
		((ResultSet*)rset)->setDataBuffer(2, F, OCCIFLOAT, sizeof(numtype), floatLen);
		//--
		((ResultSet*)rset)->next(Fcnt);
		if (((ResultSet*)rset)->status()!=ResultSet::DATA_AVAILABLE) {
			fail("Could not find Core Image for ProcessId=%d , ThreadId=%d , Epoch=%d", pid, tid, epoch);
		}
		//-- free(s)
		free(FidArr); free(intLen); free(floatLen);
		//-- close result set and terminate statement before exiting
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}


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
//-- Save/Load Core Logger image
void sOraData::saveCoreLoggerParms(int pid_, int tid_, int readFrom, bool saveToDB, bool saveToFile, bool saveMSEFlag, bool saveRunFlag, bool saveInternalsFlag, bool saveImageFlag) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "insert into coreLoggerParms(ProcessId, ThreadId, ReadFrom, SaveToDB, SaveToFile, SaveMSEFlag, SaveRunFlag, SaveInternalsFlag, SaveImageFlag) values (%d, %d, %d, %d, %d, %d, %d, %d, %d)", pid_, tid_, readFrom, (saveToDB)?1:0, (saveToFile)?1:0, (saveMSEFlag)?1:0, (saveRunFlag)?1:0, (saveInternalsFlag)?1:0, (saveImageFlag)?1:0);

	safecall(this, sqlExec, sqlS);

}
void sOraData::loadCoreLoggerParms(int pid_, int tid_, int* readFrom, bool* saveToDB, bool* saveToFile, bool* saveMSEFlag, bool* saveRunFlag, bool* saveInternalsFlag, bool* saveImageFlag) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open) {}

	sprintf_s(sqlS, SQL_MAXLEN, "select ReadFrom, SaveToDB, SaveToFile, SaveMSEFlag, SaveRunFlag, SaveInternalsFlag, SaveImageFlag from CoreLoggerParms where ProcessId=%d and ThreadId=%d", pid_, tid_);
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();

		int i=0;
		while (((ResultSet*)rset)->next()) {
			(*readFrom)=((ResultSet*)rset)->getInt(1);
			(*saveToDB)=(((ResultSet*)rset)->getInt(2)==1);
			(*saveToFile)=(((ResultSet*)rset)->getInt(3)==1);
			(*saveRunFlag)=(((ResultSet*)rset)->getInt(4)==1);
			(*saveMSEFlag)=(((ResultSet*)rset)->getInt(5)==1);
			(*saveInternalsFlag)=(((ResultSet*)rset)->getInt(6)==1);
			(*saveImageFlag)=(((ResultSet*)rset)->getInt(7)==1);
			i++;
		}
		if (i==0) fail("Core Logger Parameters not found for ProcessId=%d, ThreadId=%d", pid_, tid_);

	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}


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
//--
void sOraData::loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, float* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_) {

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

	} catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
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

//-- Save Core<XXX>Internals
void sOraData::saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* comp_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	try {
		stmt = ((Connection*)conn)->createStatement("insert into CoreNNInternalsSCGD(ProcessId, ThreadId, Iteration, delta, mu, alpha, beta, lambda, lambdau, Gtse_old, Gtse_new, comp, pnorm, rnorm, dW) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15)");
		((Statement*)stmt)->setMaxIterations(iterationsCnt_);
		for (int i=0; i<iterationsCnt_; i++) {
			((Statement*)stmt)->setInt(1, pid_);
			((Statement*)stmt)->setInt(2, tid_);
			((Statement*)stmt)->setInt(3, i);
#ifdef DOUBLE_NUMTYPE
			((Statement*)stmt)->setDouble(4, delta_[i]);
			((Statement*)stmt)->setDouble(5, mu_[i]);
			((Statement*)stmt)->setDouble(6, alpha_[i]);
			((Statement*)stmt)->setDouble(7, beta_[i]);
			((Statement*)stmt)->setDouble(8, lambda_[i]);
			((Statement*)stmt)->setDouble(9, lambdau_[i]);
			((Statement*)stmt)->setDouble(10, Gtse_old_[i]);
			((Statement*)stmt)->setDouble(11, Gtse_new_[i]);
			((Statement*)stmt)->setDouble(12, comp_[i]);
			((Statement*)stmt)->setDouble(13, pnorm_[i]);
			((Statement*)stmt)->setDouble(14, rnorm_[i]);
			((Statement*)stmt)->setDouble(15, dwnorm_[i]);
#else
			((Statement*)stmt)->setFloat(4, delta_[i]);
			((Statement*)stmt)->setFloat(5, mu_[i]);
			((Statement*)stmt)->setFloat(6, alpha_[i]);
			((Statement*)stmt)->setFloat(7, beta_[i]);
			((Statement*)stmt)->setFloat(8, lambda_[i]);
			((Statement*)stmt)->setFloat(9, lambdau_[i]);
			((Statement*)stmt)->setFloat(10, Gtse_old_[i]);
			((Statement*)stmt)->setFloat(11, Gtse_new_[i]);
			((Statement*)stmt)->setFloat(12, comp_[i]);
			((Statement*)stmt)->setFloat(13, pnorm_[i]);
			((Statement*)stmt)->setFloat(14, rnorm_[i]);
			((Statement*)stmt)->setFloat(15, dwnorm_[i]);
#endif
			if (i<(iterationsCnt_-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

}

//-- Save/Load engine info
void sOraData::saveEngineInfo(int pid, int engineType, int coresCnt, int sampleLen_, int predictionLen_, int featuresCnt_, int batchSize_, int WNNdecompLevel_, int WNNwaveletType_, bool saveToDB_, bool saveToFile_, sOraData* dbconn_, int* coreId, int* coreLayer, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- 1. ENGINES
	sprintf_s(sqlS, SQL_MAXLEN, "insert into Engines(ProcessId, EngineType, DataSampleLen, DataPredictionLen, DataFeaturesCnt, DataBatchSize, WNNdecompLevel, WNNwaveletType, SaveToDB, SaveToFile, Orausername, Orapassword, Oraconnstring) values(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s', '%s', '%s')", pid, engineType, sampleLen_, predictionLen_, featuresCnt_, batchSize_, WNNdecompLevel_, WNNwaveletType_, (saveToDB_) ? 1 : 0, (saveToFile_) ? 1 : 0, dbconn_->DBUserName, dbconn_->DBPassword, dbconn_->DBConnString);
	safecall(this, sqlExec, sqlS);
	//-- 1.1. ENGINES SCALING PARMS
	for (int f=0; f<featuresCnt_; f++) {
		sprintf_s(sqlS, SQL_MAXLEN, "insert into EngineScalingParms(ProcessId, DecompLevel, Feature, trMin, trMax) values(%d, %d, %d, %f, %f)", pid, -1, f, trMin_[f], trMax_[f]);
		safecall(this, sqlExec, sqlS);
	}
	for (int l=0; l<WNNdecompLevel_; l++) {
		for (int f=0; f<featuresCnt_; f++) {
			sprintf_s(sqlS, SQL_MAXLEN, "insert into EngineScalingParms(ProcessId, DecompLevel, Feature, trMin, trMax) values(%d, %d, %d, %f, %f)", pid, l, f, fftMin_[l][f], fftMax_[l][f]);
			safecall(this, sqlExec, sqlS);
		}
	}

	//-- 2. ENGINECORES
	for (int c=0; c<coresCnt; c++) {
		sprintf_s(sqlS, SQL_MAXLEN, "insert into EngineCores(EnginePid, CoreId, Layer, CoreThreadId, CoreType) values(%d, %d, %d, %d, %d)", pid, coreId[c], coreLayer[c], tid[c], coreType[c]);
		safecall(this, sqlExec, sqlS);
		//-- 3. CORELAYOUTS
		for (int cp=0; cp<parentCoresCnt[c]; cp++) {
			sprintf_s(sqlS, SQL_MAXLEN, "insert into CoreLayouts(EnginePid, CoreId, ParentCoreId, ParentConnType) values(%d, %d, %d, %d)", pid, c, parentCore[c][cp], parentConnType[c][cp]);
			safecall(this, sqlExec, sqlS);
		}
	}

}
void sOraData::loadEngineInfo(int pid, int* engineType_, int* coresCnt, int* sampleLen_, int* predictionLen_, int* featuresCnt_, int* batchSize_, int* WNNdecompLevel_, int* WNNwaveletType_, bool* saveToDB_, bool* saveToFile_, sOraData* dbconn_, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- nested statement and result set
	char nsqlS[SQL_MAXLEN]; Statement* nstmt; ResultSet* nrset;

	try {
		//-- 0. engine type, data shape and persistor
		sprintf_s(sqlS, SQL_MAXLEN, "select EngineType, DataSampleLen, DataPredictionLen, DataFeaturesCnt, DataBatchSize, WNNdecompLevel, WNNwaveletType, saveToDB, saveToFile, OraUserName, OraPassword, OraConnstring from Engines where ProcessId= %d", pid);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		int i=0;
		while (((ResultSet*)rset)->next()) {
			(*engineType_)=((ResultSet*)rset)->getInt(1);
			(*sampleLen_)=((ResultSet*)rset)->getInt(2);
			(*predictionLen_)=((ResultSet*)rset)->getInt(3);
			(*featuresCnt_)=((ResultSet*)rset)->getInt(4);
			(*batchSize_)=((ResultSet*)rset)->getInt(5);
			(*WNNdecompLevel_)=((ResultSet*)rset)->getInt(6);
			(*WNNwaveletType_)=((ResultSet*)rset)->getInt(7);
			(*saveToDB_)=(((ResultSet*)rset)->getInt(8)==1);
			(*saveToFile_)=(((ResultSet*)rset)->getInt(9)==1);
			strcpy_s(dbconn_->DBUserName, DBUSERNAME_MAXLEN, ((ResultSet*)rset)->getString(10).c_str());
			strcpy_s(dbconn_->DBPassword, DBPASSWORD_MAXLEN, ((ResultSet*)rset)->getString(11).c_str());
			strcpy_s(dbconn_->DBConnString, DBCONNSTRING_MAXLEN, ((ResultSet*)rset)->getString(12).c_str());
			i++;
		}
		if (i==0) fail("Engine pid %d not found.", pid);

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		//-- 1. scaling parameters (base)
		sprintf_s(sqlS, SQL_MAXLEN, "select trMin, trMax from EngineScalingParms where Processid= %d and DecompLevel=-1", pid);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		int ftrCnt=0;
		while (((ResultSet*)rset)->next()) {
			trMin_[ftrCnt]=((ResultSet*)rset)->getFloat(1);
			trMax_[ftrCnt]=((ResultSet*)rset)->getFloat(2);
			ftrCnt++;
		}
		if (ftrCnt==0) fail("Engine Scaling Parms for Engine pid %d not found.", pid);
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		//-- 1.1. scaling parameters (fft)
		for (int l=0; l<(*WNNdecompLevel_); l++) {
			sprintf_s(sqlS, SQL_MAXLEN, "select trMin, trMax from EngineScalingParms where Processid= %d and DecompLevel=%d", pid, l);
			stmt = ((Connection*)conn)->createStatement(sqlS);
			rset = ((Statement*)stmt)->executeQuery();
			ftrCnt=0;
			while (((ResultSet*)rset)->next()) {
				fftMin_[l][ftrCnt]=((ResultSet*)rset)->getFloat(1);
				fftMax_[l][ftrCnt]=((ResultSet*)rset)->getFloat(2);
				ftrCnt++;
			}
			if (ftrCnt==0) fail("Engine Scaling Parms for Engine pid %d and DecompLevel=%d not found.", pid, l);
			((Statement*)stmt)->closeResultSet((ResultSet*)rset);
			((Connection*)conn)->terminateStatement((Statement*)stmt);
		}
		//-- 2. coresCnt, coreId, coreType
		sprintf_s(sqlS, SQL_MAXLEN, "select CoreId, CoreType, CoreThreadId from EngineCores where EnginePid= %d", pid);
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		i=0;
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

			((Statement*)nstmt)->closeResultSet((ResultSet*)nrset);
			((Connection*)conn)->terminateStatement((Statement*)nstmt);
		}
		(*coresCnt)=i;

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

	//--
}
int sOraData::getSavedEnginePids(int maxPids_, int* oPid) {
	
	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	//-- build statement
	sprintf_s(sqlS, SQL_MAXLEN, "select ProcessId from Engines order by 1");

	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();
		int i=0;
		while (((ResultSet*)rset)->next() && i<maxPids_) {
			oPid[i]=((ResultSet*)rset)->getInt(1);
			i++;
		}
		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		return i-1;
	} catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}



}

//--
void sOraData::loadDBConnInfo(int pid_, int tid_, char** oDBusername, char** oDBpassword, char** oDBconnstring) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open) {}

	sprintf_s(sqlS, SQL_MAXLEN, "select UserName, Password, ConnString from DBConnections where ProcessId=%d and ThreadId=%d", pid_, tid_);
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		rset = ((Statement*)stmt)->executeQuery();

		int i=0;
		while (((ResultSet*)rset)->next()) {
			strcpy_s((*oDBusername), DBUSERNAME_MAXLEN, ((ResultSet*)rset)->getString(1).c_str());
			strcpy_s((*oDBpassword), DBUSERNAME_MAXLEN, ((ResultSet*)rset)->getString(2).c_str());
			strcpy_s((*oDBconnstring), DBUSERNAME_MAXLEN, ((ResultSet*)rset)->getString(3).c_str());
			i++;
		}

		((Statement*)stmt)->closeResultSet((ResultSet*)rset);
		((Connection*)conn)->terminateStatement((Statement*)stmt);

		if (i==0) fail("DB Connection not found for ProcessId=%d, ThreadId=%d", pid_, tid_);

	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}
}
void sOraData::saveDBConnInfo(int pid_, int tid_, char* oDBusername, char* oDBpassword, char* oDBconnstring){	//-- always check this, first!
	
	if (!isOpen) safecall(this, open);
	sprintf_s(sqlS, SQL_MAXLEN, "insert into DBconnections(ProcessId, ThreadId, DBConnId, Username, Password, ConnString) values(%d, %d, %d, '%s','%s','%s')", pid_, tid_, 0, oDBusername, oDBpassword, oDBconnstring);
	safecall(this, sqlExec, sqlS);

}

//-- private stuff
void sOraData::sqlExec(char* sqlS) {
	try {
		stmt = ((Connection*)conn)->createStatement(sqlS);
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
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
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}
}


//-- C# interface stuff
extern "C" __declspec(dllexport) int _getSavedEnginePids(const char* DBusername, const char* DBpassword, const char* DBconnstring, int maxPids_, int* oPid) {
	try {
		sOraData* db=new sOraData(nullptr, newsname("GUIdb"), defaultdbg, nullptr, DBusername, DBpassword, DBconnstring);
		return(db->getSavedEnginePids(maxPids_, oPid));
	}
	catch (std::exception exc) {
		//fail("Exception=%s", exc.what());
	}
	return 0;
}

//-- MetaTrader stuff
void sOraData::saveTradeInfo(int MT4clientPid, int MT4sessionId, int MT4accountId, int MT4enginePid, int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	sprintf_s(sqlS, SQL_MAXLEN, "insert into TradeInfo(ClientPid, SessionId, AccountId, TicketId, EnginePid, OpenTime, LastBarT, LastBarO, LastBarH, LastBarL, LastBarC, LastBarV, LastForecastO, LastForecastH, LastForecastL, LastForecastC, LastForecastV, ForecastO, ForecastH, ForecastL, ForecastC, ForecastV, TradeScenario, TradeResult, TPhit, SLhit) values(%d, %d, %d, %d, %d, to_date('%s','%s'), to_date('%s','%s'), %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %d)", MT4clientPid, MT4sessionId, MT4accountId, iPositionTicket, MT4enginePid, iPositionOpenTime, DATE_FORMAT, iLastBarT, DATE_FORMAT, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iLastForecastO, iLastForecastH, iLastForecastL, iLastForecastC, iLastForecastV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTPhit, iSLhit);
	safecall(this, sqlExec, sqlS);

}

//--
void sOraData::saveXMLconfig(int simulationId_, int pid_, int tid_, int fileId_, int parmsCnt_, char** parmDesc_, char** parmVal_) {

	//-- always check this, first!
	if (!isOpen) safecall(this, open);

	string tmpS;
	try {
		stmt = ((Connection*)conn)->createStatement("insert into XMLConfigs(SimulationId, ProcessId, ThreadId, FileId, ParmId, ParmDesc, ParmVal) values (:P01, :P02, :P03, :P04, :P05, :P06, :P07)");
		((Statement*)stmt)->setMaxIterations(parmsCnt_);
		for (int i=0; i<parmsCnt_; i++) {
			((Statement*)stmt)->setInt(1, simulationId_);
			((Statement*)stmt)->setInt(2, pid_);
			((Statement*)stmt)->setInt(3, tid_);
			((Statement*)stmt)->setInt(4, fileId_);
			((Statement*)stmt)->setInt(5, i);

			((Statement*)stmt)->setMaxParamSize(6, 4000);
			tmpS.assign(parmDesc_[i], 4000);
			((Statement*)stmt)->setString(6, tmpS);

			((Statement*)stmt)->setMaxParamSize(7, 4000);
			tmpS.assign(parmVal_[i], 4000);
			((Statement*)stmt)->setString(7, tmpS);
			if (i<(parmsCnt_-1)) ((Statement*)stmt)->addIteration();
		}
		((Statement*)stmt)->executeUpdate();
		((Connection*)conn)->terminateStatement((Statement*)stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d (%s); statement: %s", ex.getErrorCode(), ex.getMessage().c_str(), ((Statement*)stmt)->getSQL().c_str());
	}

}