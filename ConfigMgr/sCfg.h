#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "sCfgLine.h"
#include "sCfgKey.h"

struct sCfg : sObj {
	FILE* cfgFile;
	int linesCnt;
	sCfgLine* cfgLine[XMLLINE_MAXCNT];
	int subCfgCnt;
	sCfg* subCfg[XMLLINE_MAXCNT];
	sCfgKey* rootKey;
	sCfgKey* currentKey;

	EXPORT sCfg(sObjParmsDef, const char* cfgFileFullName_, int currDepth_=0, int overridesCnt=0, char** overrideName=nullptr, char** overrideValS=nullptr);
	EXPORT ~sCfg();

	EXPORT void setKey(const char* keyDesc_, bool ignoreError=false, bool* oKeyFound_=nullptr);

private:
	int currDepth;
	int currParent[XMLKEY_MAXDEPTH]; 
	int prevParent[XMLKEY_MAXDEPTH]; 

	char cfgFileName[MAX_PATH];
	char cfgFilePath[MAX_PATH];
	char cfgFileFullName[MAX_PATH];
	char subCfgFileFullName[MAX_PATH];

	char _line[XMLLINE_MAXLEN];
	void parse();

	char keyDesc[XMLKEY_PATH_MAXLEN];	//-- actual keyDesc after modifiers handling
	void setActualKeyDesc(const char* keyDesc_);
};
