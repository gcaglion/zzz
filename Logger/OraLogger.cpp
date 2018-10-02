#include "OraLogger.h"

sOraLogger::sOraLogger(sObjParmsDef, sOraConnection* db_, bool saveNothing_, bool saveClient_, bool saveMSE_, bool saveRun_, bool saveInternals_, bool saveImage_) : sLogger(sObjParmsVal, LOG_TO_ORCL, saveNothing_, saveClient_, saveMSE_, saveRun_, saveInternals_, saveImage_) {
	db=db_;
}
sOraLogger::sOraLogger(sCfgObjParmsDef) : sLogger(sObjParmsVal, LOG_TO_ORCL) {

}

void sOraLogger::SaveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
	int i;
	Statement* stmt=nullptr;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	try {
	}
	catch (SQLException ex) {
	}
}
void sOraLogger::SaveRun(int pid, int tid, int setid, int npid, int ntid, int runCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual) {}
void sOraLogger::SaveW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
void sOraLogger::LoadW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
void sOraLogger::SaveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {}
void sOraLogger::Commit() {}
