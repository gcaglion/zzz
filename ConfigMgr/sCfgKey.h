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

	sCfgKey(sObjParmsDef, int linesCnt_, sCfgLine** cfgLine_, int startLine_);
	~sCfgKey();

	template<typename T> EXPORT void getParm(T* pvar, const char* parmDesc, int* oValsCnt_=nullptr, bool ignoreError=false) {
		sObj* parmObj=nullptr;

		//-- call sObj findChild on parmDesc
		findChild(parmDesc, &parmObj);
		safecall(this, findChild, parmDesc, &parmObj);
		if (parmObj==nullptr) {
			if (!ignoreError) fail("XML parameter not found. keyDesc=%s, parmDesc=%s", name->full, parmDesc);
		} else {
			if (!((sCfgParm*)parmObj)->getVal(pvar, oValsCnt_)) {
				if (!ignoreError) fail("Invalid XML parameter value. keyDesc=%s, parmDesc=%s, value string= \"%s\"", name->full, parmDesc, ((sCfgParm*)parmObj)->valS);
			}
		}

	}

private:
	void setDbg();

};

