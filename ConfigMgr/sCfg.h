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
	//sCfgKey* bkpKey;

	EXPORT sCfg(sObjParmsDef, const char* cfgFileFullName, int overridesCnt=0, char** overrideName=nullptr, char** overrideValS=nullptr);
	EXPORT ~sCfg();

	EXPORT void setKey(const char* keyDesc_, bool ignoreError=false, bool* oKeyFound_=nullptr);

private:
	char _line[XMLLINE_MAXLEN];
	void parse();

	char keyDesc[XMLKEY_PATH_MAXLEN];	//-- actual keyDesc after modifiers handling
	void setActualKeyDesc(const char* keyDesc_);
};
