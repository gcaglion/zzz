#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"

//-- limits
#define DBUSERNAME_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define DBPASSWORD_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define DBCONNSTRING_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define SQL_MAXLEN	4096

struct sOraDB : sCfgObj {

	EXPORT sOraDB(sCfgObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_, bool autoOpen=false);
	EXPORT sOraDB(sCfgObjParmsDef, bool autoOpen=true);
	EXPORT ~sOraDB();

	EXPORT void getFlatOHLCV(char* pSymbol, char* pTF, char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);


private:
	void* env;
	void* conn;

	char* DBUserName = new char[DBUSERNAME_MAXLEN];
	char* DBPassword = new char[DBPASSWORD_MAXLEN];
	char* DBConnString = new char[DBCONNSTRING_MAXLEN];

	void open();
	void close();
	void commit();

};
