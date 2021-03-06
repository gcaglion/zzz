#include "sCfgParm.h"
//#include <vld.h>

sCfgParm::sCfgParm(sObjParmsDef, char* parmName_, char* parmValS_) : sObj(sObjParmsVal) {
	valS=(char**)malloc(XMLKEY_PARM_VAL_MAXCNT*sizeof(char*)); for (int v=0; v<XMLKEY_PARM_VAL_MAXCNT; v++) valS[v]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	setValS(parmValS_);
}
sCfgParm::~sCfgParm() {
	for (int v=0; v<XMLKEY_PARM_VAL_MAXCNT; v++) free(valS[v]);
	free(valS);
}
//--
bool sCfgParm::getVal(int* oVal, int* oValsCnt) { return(getVal(&oVal));}
bool sCfgParm::getVal(char** oVal, int* oValsCnt) { return(getVal(&oVal));}
bool sCfgParm::getVal(numtype* oVal, int* oValsCnt) { return(getVal(&oVal)); }
bool sCfgParm::getVal(altnumtype* oVal, int* oValsCnt) { return(getVal(&oVal)); }
bool sCfgParm::getVal(bool* oVal, int* oValsCnt) { return(getVal(&oVal));}
//--
bool sCfgParm::getVal(int** oVal, int* oValsCnt) {
	if(oValsCnt!=nullptr) (*oValsCnt)=valScnt;
	for (int v=0; v<valScnt; v++) {
		if (isnumber(valS[v])) {
			oVal[0][v]=atoi(valS[v]);
		} else {
			safecallSilent(this, decode, v, &oVal[0][v]);
		}
	}
	return true;
}
bool sCfgParm::getVal(char*** oVal, int* oValsCnt) {
	if (oValsCnt!=nullptr) (*oValsCnt)=valScnt;
	for (int v=0; v<valScnt; v++) {
		strcpy_s((*oVal)[v], XMLKEY_PARM_VAL_MAXLEN, valS[v]);
	}
	return true;
}
bool sCfgParm::getVal(numtype** oVal, int* oValsCnt) {
	if (oValsCnt!=nullptr) (*oValsCnt)=valScnt;
	for (int v=0; v<valScnt; v++) {
		oVal[0][v]=(numtype)atof(valS[v]);
	}
	return true;
}
bool sCfgParm::getVal(altnumtype** oVal, int* oValsCnt) {
	if (oValsCnt!=nullptr) (*oValsCnt)=valScnt;
	for (int v=0; v<valScnt; v++) {
		oVal[0][v]=(altnumtype)atof(valS[v]);
	}
	return true;
}
bool sCfgParm::getVal(bool** oVal, int* oValsCnt) {
	if (oValsCnt!=nullptr) (*oValsCnt)=valScnt;
	for (int v=0; v<valScnt; v++) {
		oVal[0][v] = (_stricmp(valS[v], "TRUE")==0);
	}
	return true;
}

bool sCfgParm::setValS(const char* parmValS_) {
	strcpy_s(valcsl, XMLKEY_PARM_VAL_MAXCNT*XMLKEY_PARM_VAL_MAXLEN, parmValS_);
	valScnt=cslToArray(parmValS_, ',', valS);
	return(valScnt>0);
}