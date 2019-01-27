#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "sCfgParm.h"
#include "sCfgLine.h"

struct sCfgKey : sObj {
	int startLine, endLine;
	int parmsCnt;
	sCfgParm* parm[XMLKEY_PARM_MAXCNT];
	int keysCnt;
	sCfgKey* key[XMLKEY_MAXCNT];

	sDbg* overrideDbg=nullptr;

	sCfgKey(sObjParmsDef, int linesCnt_, sCfgLine** cfgLine_, int startLine_);
	~sCfgKey();

	template<typename T> EXPORT void getParm(T* pvar, const char* parmDesc, bool ignoreError=false, int* oValsCnt_=nullptr) {
		sObj* parmObj=nullptr;

		//-- call sObj findChild on parmDesc
		safecallSilent(this, findChild, parmDesc, &parmObj);
		if (parmObj==nullptr) {
			if (!ignoreError) fail("XML parameter not found. keyDesc=%s, parmDesc=%s", name->full, parmDesc);
		} else {
			if (!((sCfgParm*)parmObj)->getVal(pvar, oValsCnt_)) {
				if (!ignoreError) fail("Invalid XML parameter value. keyDesc=%s, parmDesc=%s, value string= \"%s\"", name->full, parmDesc, ((sCfgParm*)parmObj)->valS);
			}
		}

	}

	EXPORT int getParmsCntTot();
	EXPORT void getAllParms(int* oParmsCnt, char** oParmDesc, char** oParmValS);

private:
	void setDbg();

};

