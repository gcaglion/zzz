#pragma once
#include "../common.h"
#include "sObj.h"

#define XMLFILE_LINE_MAXLEN	2048
//--
#define XMLKEY_MAXCNT		1024
#define XMLKEY_NAME_MAXLEN	OBJ_NAME_MAXLEN
#define XMLKEY_MAXDEPTH		OBJ_MAX_CHILDREN+XMLKEY_NAME_MAXLEN
#define XMLKEY_PATH_MAXLEN	XMLKEY_MAXDEPTH*(XMLKEY_NAME_MAXLEN+1)
#define XMLKEY_PARM_MAXCNT	OBJ_MAX_CHILDREN
#define XMLKEY_PARM_NAME_MAXLEN XMLKEY_NAME_MAXLEN
#define XMLKEY_PARM_VAL_MAXCNT	OBJ_MAX_CHILDREN
#define XMLKEY_PARM_VAL_MAXLEN	256
//--

void stripChar(char* istr, char c) {
	size_t ilen=strlen(istr);
	char* ostr=(char*)malloc(ilen+1);
	int ri=0;
	for (unsigned int si=0; si<ilen; si++) {
		if (istr[si]!=c) {
			ostr[ri]=istr[si];
			ri++;
		}
	}
	ostr[ri]='\0';
	memcpy_s(istr, ri, ostr, ri);
	istr[ri]='\0';
	free(ostr);
}
bool skipLine(char* line) {
	if (strlen(line)==0) return true;
	if (line[0]=='#') return true;
	return false;
}
void cleanLine(char* line) {
	stripChar(line, ' ');
	stripChar(line, '\t');
	stripChar(line, '\n');
}
bool isKeyStart(char* line) {
	return (line[0]=='<' && line[1]!='/' && line[strlen(line)-1]=='>');
}
bool isKeyEnd(char* line) {
	return (line[0]=='<' && line[1]=='/' && line[strlen(line)-1]=='>');
}

struct sCfgKey :sObj {
	FILE* cfgFile=nullptr;

	sCfgKey(sObjParmsDef, const char* callerFunc_, char* keyLine_) : sObj(sObjParmsVal) {
	}

	void parse() {

	}

};

struct sCfgFile : sObj {
	FILE* fHandle;
	sObj* currentKey=this;

	sCfgFile(sObjParmsDef, const char* cfgFileFullName_) : sObj(sObjParmsVal) {

		//-- open file
		fopen_s(&fHandle, cfgFileFullName_, "r");
		if (errno!=0) fail("Could not open configuration file %s . Error %d", cfgFileFullName_, errno);

		//-- load 
		while (fgets(line, XMLFILE_LINE_MAXLEN, fHandle)!=NULL) {
			cleanLine(line);				//-- strip spaces & tabs
			if (skipLine(line)) continue;	// empty line or comment

			if (isKeyStart(line)) {
				//-- new sKey
				//				L0key[L0keysCnt] = new sCfgFileKey(this, childsname("cfgKey_0-%d", L0keysCnt), newdbg(1), __func__, line);

			} else 	if (isKeyEnd(line)) {
				//-- current key becomes current key's parent
				//			currentChild=currentChild->parent;
			} else {
				//-- new sParm
				//			safespawn(currentChild, sCfgFileParm, newsname("key[%d]->parm[%d]"), dbg, line);
				//		_spawnV<sCfgFileParm>(__func__, (sCfgFileParm*)currentChild, "currentChild-Parm", newsname("key[%d]->parm[%d]"), dbg, line);
				//			currentChild = (s0*)_spawn<sCfgFileParm>(__func__, "currentChild", newsname("key[%d]->parm[%d]"), dbg, line);
			}
		}
	}

	void parse() {
		while (fgets(line, XMLFILE_LINE_MAXLEN, fHandle)!=NULL) {
			cleanLine(line);				//-- strip spaces & tabs
			if (skipLine(line)) continue;	// empty line or comment

			if (isKeyStart(line)) {
				//-- new sKey
				//safespawn(currentChild, sCfgFileKey, newsname("tempKeyName"), dbg, line);
				//_spawnV<sCfgFileKey>(__func__, (sCfgFileKey*)currentChild, "currentChild-Key", newsname("tempKeyName_%d", 76), dbg, line);


				//currentChild = (s0*)_spawn<sCfgFileKey>(__func__, "currentChild", newsname("tempKeyName"), dbg, line);

			} else 	if (isKeyEnd(line)) {
				//-- current key becomes current key's parent
				//currentChild=currentChild->parent;
			} else {
				//-- new sParm
				//			safespawn(currentChild, sCfgFileParm, newsname("key[%d]->parm[%d]"), dbg, line);
				//_spawnV<sCfgFileParm>(__func__, (sCfgFileParm*)currentChild, "currentChild-Parm", newsname("key[%d]->parm[%d]"), dbg, line);
				//			currentChild = (s0*)_spawn<sCfgFileParm>(__func__, "currentChild", newsname("key[%d]->parm[%d]"), dbg, line);
			}
		}

	}
	~sCfgFile();


private:
	char line[XMLFILE_LINE_MAXLEN];
};

