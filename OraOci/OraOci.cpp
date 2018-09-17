#include "OraOci.h"
#undef fail
#include <occi.h> 
using namespace oracle::occi;
#include <string>
#include "../BaseObj/sDbgMacros.h"

sOraConnection::sOraConnection(sCfgObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_) : sCfgObj(sCfgObjParmsVal) {
	//-- 1. get Parameters
	strcpy_s(DBUserName, DBUSERNAME_MAXLEN, DBUserName_);
	strcpy_s(DBPassword, DBPASSWORD_MAXLEN, DBPassword_);
	strcpy_s(DBConnString, DBCONNSTRING_MAXLEN, DBConnString_);
	//-- 2. open connection
	safecall(this, open);
}
sOraConnection::sOraConnection(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &DBUserName, "UserName");
	safecall(cfgKey, getParm, &DBPassword, "Password");
	safecall(cfgKey, getParm, &DBConnString, "ConnString");
	//-- 2. do stuff and spawn sub-Keys
	safecall(this, open);	//-- open connection
	//-- 3. Restore currentKey
	cfg->currentKey=cfg->bkpKey;

}
sOraConnection::~sOraConnection() {
	close();
}

void sOraConnection::open() {
	try {
		env = Environment::createEnvironment();
		conn = ((Environment*)env)->createConnection(DBUserName, DBPassword, DBConnString);
	} catch (SQLException exc) {
		fail("%s FAILURE : %s . SQL Exception: %s", name->base, cmd, exc.what());
	}

}
void sOraConnection::close() {
	((Environment*)env)->terminateConnection(((Connection*)conn));
	Environment::terminateEnvironment(((Environment*)env));
}
void sOraConnection::commit() {
	((Connection*)conn)->commit();
}
void sOraConnection::getFlatOHLCV(char* pSymbol, char* pTF, char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {
	int i;
	Statement* stmt=nullptr;
	ResultSet *rset;
	char sql[SQL_MAXLEN];

	try {
		sprintf_s(sql, SQL_MAXLEN, "select to_char(newdatetime,'YYYYMMDDHH24MI'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime<=to_date('%s','YYYYMMDDHH24MI') order by 1 desc", pSymbol, pTF, pDate0);
		stmt = ((Connection*)conn)->createStatement(sql);
		rset = stmt->executeQuery();

		i=pRecCount-1;
		while (rset->next()&&i>=0) {
			strcpy_s(oBarTime[i], 12+1, rset->getString(1).c_str());
			oBarData[5*i+0] = rset->getFloat(2);
			oBarData[5*i+1] = rset->getFloat(3);
			oBarData[5*i+2] = rset->getFloat(4);
			oBarData[5*i+3] = rset->getFloat(5);
			oBarData[5*i+4] = rset->getFloat(6);
			i--;
		}
		//-- one more fetch to get baseBar
		if (rset->next()) {
			strcpy_s(oBaseTime, 12+1, rset->getString(1).c_str());
			for (int f=0; f<5; f++) {
				oBaseBar[f] = rset->getFloat(f+2);
			}
			oBaseBar[0] = rset->getFloat(2);
		}

		stmt->closeResultSet(rset);
		((Connection*)conn)->terminateStatement(stmt);
	}
	catch (SQLException ex) {
		fail("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}


}

/*
int main() {

	const std::string userName = "History";
	const std::string password = "HistoryPwd";
	const std::string connectString = "Algo";
	
	Environment *env = Environment::createEnvironment();

//	#if (OCCI_MAJOR_VERSION > 9)
//		env->setCacheSortedFlush(true);  // benefit of performance, if available
//	#endif

	Connection *conn = env->createConnection(userName, password, connectString);
	Statement *stmt = conn->createStatement("SELECT * from history.eurusd_h1");
	ResultSet *rs = stmt->executeQuery();
	rs->next();
	int cnt=rs->getInt(1);
	stmt->closeResultSet(rs);
	conn->terminateStatement(stmt);
	env->terminateConnection(conn);
	Environment::terminateEnvironment(env);

	return 0;
}
*/