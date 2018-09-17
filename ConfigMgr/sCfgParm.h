#pragma once
#include "../BaseObj/sObj.h"
#include "XMLdefs.h"

struct sCfgParm : sObj {

	sCfgParm(sObjParmsDef, char* parmName_, char* parmValS_);
	~sCfgParm();

	//-- id props
	int valScnt;
	char** valS;

	EXPORT Bool getVal(int* oVal, int* oValsCnt);
	EXPORT Bool getVal(char** oVal, int* oValsCnt);
	EXPORT Bool getVal(numtype* oVal, int* oValsCnt);
	EXPORT Bool getVal(bool* oVal, int* oValsCnt);
	//--
	EXPORT Bool getVal(int** oVal, int* oValsCnt=nullptr);
	EXPORT Bool getVal(char*** oVal, int* oValsCnt=nullptr);
	EXPORT Bool getVal(numtype** oVal, int* oValsCnt=nullptr);
	EXPORT Bool getVal(bool** oVal, int* oValsCnt=nullptr);

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
