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
	printf("sOraData Destructor called for %s\n", name->full);
	if (isOpen) {
		close();
		isOpen=false;
	}
}

void sOraData::open() {
	try {
		if(env==nullptr) env = Environment::createEnvironment();
		if (conn==nullptr) conn = ((Environment*)env)->createConnection(DBUserName, DBPassword, DBConnString);
		isOpen=true;
	}
	catch (SQLException exc) {
		{ 
			fail("%s FAILURE : %s", name->base, exc.what());
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
	Statement* stmt=nullptr;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	try {
		//-- 1. History: open statement and result set
		sprintf_s(sql, SQL_MAXLEN, "select to_char(newdatetime,'YYYYMMDDHH24MI'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime<=to_date('%s','YYYYMMDDHH24MI') order by 1 desc", pSymbol, pTF, date0_);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = stmt->executeQuery();
		//-- 2. History: get all records
		i=stepsCnt-1;
		while (rset->next()&&i>=0) {
			strcpy_s(oBarTime[i], DATE_FORMAT_LEN, rset->getString(1).c_str());
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
		stmt->closeResultSet(rset);
		((Connection*)conn)->terminateStatement(stmt);
	} catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}
}

void sOraData::getFlatOHLCV(char* pSymbol, char* pTF, char* pDate0, int pastStepsCnt, char** oBarTimeH, float* oBarDataH, int futureStepsCnt, char** oBarTimeF, float* oBarDataF, char* oBarTime0, float* oBaseBar) {
	int i;
	Statement* stmt=nullptr;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	try {
		//-- 1. History: open statement and result set
		sprintf_s(sql, SQL_MAXLEN, "select to_char(newdatetime,'YYYYMMDDHH24MI'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime<=to_date('%s','YYYYMMDDHH24MI') order by 1 desc", pSymbol, pTF, pDate0);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = stmt->executeQuery();
		//-- 2. History: get all records
		i=pastStepsCnt-1;
		while (rset->next()&&i>=0) {
			strcpy_s(oBarTimeH[i], DATE_FORMAT_LEN, rset->getString(1).c_str());
			oBarDataH[5*i+0] = rset->getFloat(2);
			oBarDataH[5*i+1] = rset->getFloat(3);
			oBarDataH[5*i+2] = rset->getFloat(4);
			oBarDataH[5*i+3] = rset->getFloat(5);
			oBarDataH[5*i+4] = rset->getFloat(6);
			i--;
		}
		//-- 3. History: one more fetch to get baseBar
		strcpy_s(oBarTime0, DATE_FORMAT_LEN, rset->getString(1).c_str());
		for (int f=0; f<5; f++)	oBaseBar[f] = rset->getFloat(f+2);
		//-- 4. History: close result set and statement
		stmt->closeResultSet(rset);
		((Connection*)conn)->terminateStatement(stmt);

		//-- 1. Future: open statement and result set
		sprintf_s(sql, SQL_MAXLEN, "select to_char(newdatetime,'YYYYMMDDHH24MI'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime>to_date('%s','YYYYMMDDHH24MI') order by 1", pSymbol, pTF, pDate0);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = stmt->executeQuery();
		//-- 2. Future: get all records
		i=0;
		while (rset->next()&&i<futureStepsCnt) {
			strcpy_s(oBarTimeF[i], DATE_FORMAT_LEN, rset->getString(1).c_str());
			oBarDataF[5*i+0] = rset->getFloat(2);
			oBarDataF[5*i+1] = rset->getFloat(3);
			oBarDataF[5*i+2] = rset->getFloat(4);
			oBarDataF[5*i+3] = rset->getFloat(5);
			oBarDataF[5*i+4] = rset->getFloat(6);
			i++;
		}
		//-- 4. Future: close result set and statement
		stmt->closeResultSet(rset);
		((Connection*)conn)->terminateStatement(stmt);

	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}


}
void sOraData::loadW(int pid, int tid, int epoch, int Wcnt, numtype* W) {
	char sql[SQL_MAXLEN];
	Statement* stmt=nullptr;
	ResultSet *rset;

	//-- if a specific epoch is not provided, we first need to find the last epoch
	if (epoch==-1) {
		sprintf_s(sql, SQL_MAXLEN, "select max(epoch) from CoreImage_NN where processId = %d and ThreadId = %d", pid, tid);
		try {
			stmt = ((Connection*)conn)->createStatement(sql);
			rset = stmt->executeQuery();
			if (rset->next() && !rset->isNull(1)) {
				epoch=rset->getInt(1);
			} else {
				fail("Could not find max epoch for processId=%d, ThreadId=%d", pid, tid);
			}
		} catch (SQLException ex) {
			fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
		}
		stmt->closeResultSet(rset);
		((Connection*)conn)->terminateStatement(stmt);
	}

	//-- once we have the epoch, we load Ws for that pid, tid, epoch
	int i=0;
	sprintf_s(sql, SQL_MAXLEN, "select WId, W from CoreImage_NN where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1,2", pid, tid, epoch);
	try{
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = stmt->executeQuery();
		while (rset->next()&&i<Wcnt) {
			W[i] = rset->getFloat(2);
			i++;
		}
	} catch(SQLException ex){
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}

	//-- close result set and terminate statement before exiting
	stmt->closeResultSet(rset);
	((Connection*)conn)->terminateStatement(stmt);

}
void sOraData::getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char** oDate) {
	// Retrieves plain ordered list of NewDateTime starting from StartDate onwards for <DatesCount> records
	int i;
	Statement* stmt=nullptr;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	if (conn==nullptr) fail("DB Connection is closed. cannot continue.");

	try {
		sprintf_s(sql, SQL_MAXLEN, "select to_char(NewDateTime, 'YYYYMMDDHH24MI') from History.%s_%s%s where NewDateTime>=to_date('%s','YYYYMMDDHH24MI') order by 1", symbol_, timeframe_, (isFilled_)?"_FILLED ":"", StartDate);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = stmt->executeQuery();

		i=0;
		while (rset->next()&&i<DatesCount) {
			strcpy_s(oDate[i], DATE_FORMAT_LEN, rset->getString(1).c_str());
			i++;
		}

		stmt->closeResultSet(rset);
		((Connection*)conn)->terminateStatement(stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}
}
//-- Write
void sOraData::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {

	Statement* stmt = ((Connection*)conn)->createStatement("insert into TrainLog(ProcessId, ThreadId, Epoch, MSE_T, MSE_V) values(:P01, :P02, :P03, :P04, :P05)");

	stmt->setMaxIterations(mseCnt);
	for (int epoch=0; epoch<mseCnt; epoch++) {
		stmt->setInt(1, pid);
		stmt->setInt(2, tid);
		stmt->setInt(3, epoch);
		stmt->setFloat(4, mseT[epoch]);
		stmt->setFloat(5, mseV[epoch]);
		if(epoch<(mseCnt-1)) stmt->addIteration();
	}

	try {
		stmt->executeUpdate();
	} catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}

	//stmt->setDataBuffer(4, mseT, OCCIFLOAT, sizeof(numtype), ntl);
	//stmt->setDataBuffer(5, mseV, OCCIFLOAT, sizeof(numtype), ntl);
	
}
void sOraData::saveRun(int pid, int tid, int npid, int ntid, int barsCnt, int featuresCnt, int* feature, int predictionLen, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted) {

	int runCnt=barsCnt*featuresCnt;
	Statement* stmt = ((Connection*)conn)->createStatement("insert into RunLog (ProcessId, ThreadId, NetProcessId, NetThreadId, Pos, Feature, StepAhead, PredictedTRS, ActualTRS, ErrorTRS, PredictedTR, ActualTR, ErrorTR, Predicted, Actual, Error) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10, :P11, :P12, :P13, :P14, :P15, :P16)");
	stmt->setMaxIterations(runCnt);

	int i=0; int p=0;
	for (int b=0; b<barsCnt; b++) {
		for (int f=0; f<featuresCnt; f++) {
			//for (int p=0; p<predictionLen; p++) {
				stmt->setInt(1, pid);
				stmt->setInt(2, tid);
				stmt->setInt(3, npid);
				stmt->setInt(4, ntid);
				stmt->setInt(5, b);
				stmt->setInt(6, feature[f]);
				stmt->setInt(7, p);
				stmt->setFloat(8, predictedTRS[i]);
				stmt->setFloat(9, actualTRS[i]);
				stmt->setFloat(10, fabs(actualTRS[i]-predictedTRS[i]));
				stmt->setFloat(11, predictedTR[i]);
				stmt->setFloat(12, actualTR[i]);
				stmt->setFloat(13, fabs(actualTR[i]-predictedTR[i]));
				stmt->setFloat(14, predicted[i]);
				stmt->setFloat(15, actual[i]);
				stmt->setFloat(16, fabs(actual[i]-predicted[i]));
				i++;
				if (i<(runCnt-1)) stmt->addIteration();
			//}
		}
	}

	try {
		stmt->executeUpdate();
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}

}
void sOraData::saveW(int pid, int tid, int epoch, int Wcnt, numtype* W) {

	Statement* stmt = ((Connection*)conn)->createStatement("insert into CoreImage_NN (ProcessId, ThreadId, Epoch, WId, W) values(:P01, :P02, :P03, :P04, :P05)");

	stmt->setMaxIterations(Wcnt);
	for (int i=0; i<Wcnt; i++) {
		stmt->setInt(1, pid);
		stmt->setInt(2, tid);
		stmt->setInt(3, epoch);
		stmt->setInt(4, i);
		stmt->setFloat(5, W[i]);
		if (i<(Wcnt-1)) stmt->addIteration();
	}

	try {
		stmt->executeUpdate();
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}
}
void sOraData::saveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {
	char stmtS[SQL_MAXLEN]; sprintf_s(stmtS, SQL_MAXLEN, "insert into ClientInfo(ProcessId, ClientName, ClientStart, SimulationLen, Duration, SimulationStart, DoTraining, DoTrainRun, DoTestRun) values(%d, '%s', sysdate, %d, %ld, to_date('%s','YYYYMMDDHH24MI'), %d, %d, %d)", pid, clientName, simulLen, (DWORD)(duration/1000), simulStart, (doTrain) ? 1 : 0, (doTrainRun) ? 1 : 0, (doTestRun) ? 1 : 0);
	Statement* stmt = ((Connection*)conn)->createStatement(stmtS);
	try {
		stmt->executeUpdate();
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}
}
