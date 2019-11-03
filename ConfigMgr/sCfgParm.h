#pragma once
#include "../BaseObj/sObj.h"
#include "XMLdefs.h"

struct sCfgParm : sObj {

	EXPORT sCfgParm(sObjParmsDef, char* parmName_, char* parmValS_);
	EXPORT ~sCfgParm();

	//-- id props
	int valScnt;
	char  valcsl[XMLKEY_PARM_VAL_MAXCNT*XMLKEY_PARM_VAL_MAXLEN];
	char** valS;

	EXPORT bool getVal(int* oVal, int* oValsCnt);
	EXPORT bool getVal(char** oVal, int* oValsCnt);
	EXPORT bool getVal(numtype* oVal, int* oValsCnt);
	EXPORT bool getVal(altnumtype* oVal, int* oValsCnt);
	EXPORT bool getVal(bool* oVal, int* oValsCnt);
	//--
	EXPORT bool getVal(int** oVal, int* oValsCnt=nullptr);
	EXPORT bool getVal(char*** oVal, int* oValsCnt=nullptr);
	EXPORT bool getVal(numtype** oVal, int* oValsCnt=nullptr);
	EXPORT bool getVal(altnumtype** oVal, int* oValsCnt=nullptr);
	EXPORT bool getVal(bool** oVal, int* oValsCnt=nullptr);

	EXPORT bool setValS(const char* parmValS_);

	void sCfgParm::decode(int elementId, int* oVal);

private:
	int intVar;
	char* charPVar;
	numtype numtypeVar;
	bool boolVar;
	int* intArrVar;
	char** charPArrVar;
	numtype* numtypeArrVar;
	bool* boolArrVar;
};
