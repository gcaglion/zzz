#pragma once
#include "../BaseObj/sObj.h"
#include "XMLdefs.h"

#define cfgLine_Comment		-1
#define cfgLine_KeyStart	0
#define cfgLine_KeyEnd		1
#define cfgLine_Parm		2
#define cfgLine_Include		3

struct sCfgLine : sObj {
	int type;
	int depth;
	int parent;
	int partner;
	char raw[XMLLINE_MAXLEN];
	char clean[XMLLINE_MAXLEN];
	char naked[XMLLINE_MAXLEN];
	//--
	char pname[XMLKEY_PARM_NAME_MAXLEN];
	char pval[XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

	sCfgLine(sObjParmsDef, char* rawLine_, int overridesCnt, char** overrideName, char** overrideValS);
	~sCfgLine();

private:
	bool isComment() {
		if (strlen(clean)==0) return true;
		if (clean[0]=='#') return true;
		return false;
	}
};
