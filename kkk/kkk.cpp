
#include <occi.h> 
using namespace oracle::occi;
#include <string>

int main() {

/*	const std::string userName = "History";
	const std::string password = "HistoryPwd";
	const std::string connectString = "Algo";
*/	char* userName = "History";
	char* password = "HistoryPwd";
	char* connectString = "Algo";

	Environment *env = Environment::createEnvironment();
	Connection *conn = env->createConnection(userName, password, connectString);

	//	#if (OCCI_MAJOR_VERSION > 9)
	//		env->setCacheSortedFlush(true);  // benefit of performance, if available
	//	#endif

	char* pDate0 = "201710010000";
	char* pSymbol="EURUSD";
	char* pTF = "H1";
	char* tname="EURUSD_H1";

	Statement* stmt = conn->createStatement("select to_char(newdatetime,'YYYYMMDDHH24MI'), open, high, low, close, nvl(volume,0) from :2 :3 where NewDateTime<=to_date(':1','YYYYMMDDHH24MI') order by 1 desc");
	stmt->setString(1, std::string(pDate0));
	//stmt->setString(2, std::string(tname));
	stmt->setString(2, std::string(pSymbol));
	stmt->setString(3, std::string(pTF));

	ResultSet* rs=nullptr;
	for (int i=1; i<=1; i++) 		printf("SQL parameter %d = \"%s\" \n", i, stmt->getString(i).c_str());
	try{
		rs = stmt->executeQuery();
	}
	catch (SQLException ex) {
		printf("SQL error: %d ; statement: %s", ex.getErrorCode(), stmt->getSQL().c_str());
	}


	rs->next();
	int cnt=rs->getInt(1);
	stmt->closeResultSet(rs);
	conn->terminateStatement(stmt);
	env->terminateConnection(conn);
	Environment::terminateEnvironment(env);

	return 0;
}
