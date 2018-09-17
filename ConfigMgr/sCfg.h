#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "sCfgLine.h"
#include "sCfgKey.h"

struct sCfg : sObj {
	FILE* cfgFile;
	int linesCnt;
	sCfgLine* cfgLine[XMLLINE_MAXCNT];
	sCfgKey* rootKey;
	sCfgKey* currentKey;

	EXPORT sCfg(sObjParmsDef, const char* cfgFileFullName, int overridesCnt=0, char** overrideName=nullptr, char** overrideValS=nullptr);
	EXPORT ~sCfg();

	EXPORT void setKey(const char* keyDesc_, bool ignoreError=false, bool* oKeyFound_=nullptr);

/*	template<typename T> EXPORT void getParm(T* pvar, const char* parmDesc, int* oValsCnt_=nullptr, bool ignoreError=false) {
		sCfgKey* bkpKey=currentKey;
		safecall(currentKey, getParm, pvar, parmDesc, oValsCnt_, ignoreError);
		currentKey=bkpKey;
	}
*/
	/*	template<typename T> EXPORT void getParm(T* pvar, const char* parmDesc, int* oValsCnt_=nullptr, bool ignoreError=false) {

		sObj* parmObj=nullptr;
		sCfgKey* bkpKey=currentKey;

		//-- call sObj findChild on parmDesc
		safecall(currentKey, findChild, parmDesc, &parmObj);
		if (parmObj==nullptr) {
			if (!ignoreError) fail("XML parameter not found. keyDesc=%s, parmDesc=%s", currentKey->name->full, parmDesc);
		} else {
			if(!((sCfgParm*)parmObj)->getVal(pvar, oValsCnt_)) {
				if (!ignoreError) fail("Invalid XML parameter value. keyDesc=%s, parmDesc=%s, value string= \"%s\"", currentKey->name->full, parmDesc, ((sCfgParm*)parmObj)->valS);
			}
		}
		currentKey=bkpKey;
	}
*/

private:
	char _line[XMLLINE_MAXLEN];
	void parse();

	char keyDesc[XMLKEY_PATH_MAXLEN];	//-- actual keyDesc after modifiers handling
	void setActualKeyDesc(const char* keyDesc_);
};
