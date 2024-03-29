#include "sCfg.h"
//#include <vld.h>

sCfg::sCfg(sObjParmsDef, const char* cfgFileFullName_, int currDepth_, int overridesCnt, char** overrideName, char** overrideValS) : sObj(sObjParmsVal) {
	currDepth=currDepth_;
	currParent[currDepth]=-1;
	prevParent[currDepth]=-1;
	subCfgCnt=0;
	strcpy_s(cfgFileFullName, MAX_PATH, cfgFileFullName_);
	splitFullFileName(cfgFileFullName_, cfgFilePath, cfgFileName);

	//-- open file
	if( fopen_s(&cfgFile, cfgFileFullName, "r") !=0) fail("Could not open configuration file %s . Error %d", cfgFileFullName, errno);

	//-- create one cfgLine object for each line in file, including comments
	linesCnt=0;
	while (fgets(_line, XMLLINE_MAXLEN, cfgFile)!=NULL) {
		safespawn(cfgLine[linesCnt], newsname("line_%d", linesCnt), dbg, _line);
		linesCnt++;

		//-- include another xml as subCfg[subCfgCnt]	
		if (cfgLine[linesCnt-1]->type==cfgLine_Include) {
			//-- first, need to set include filename relative to current path
			getFullPath(cfgLine[linesCnt-1]->naked, subCfgFileFullName, cfgFilePath);
			//-- then, open it with fullname
			safespawn(subCfg[subCfgCnt], newsname("subCfg[%d]", subCfgCnt), dbg, subCfgFileFullName);
			subCfgCnt++;
			//-- lines in subCfg become lines in cfg
			for (int i=0; i<subCfg[subCfgCnt-1]->linesCnt; i++) cfgLine[linesCnt+i]=subCfg[subCfgCnt-1]->cfgLine[i];
			linesCnt+=subCfg[subCfgCnt-1]->linesCnt;
		}
	}

	//-- check line pairing (start/end keys)
	parse();

	//-- create root key (just one per file), and set it as current
	safespawn(rootKey, newsname("rootKey"), dbg, linesCnt, cfgLine, -1);
	currentKey=rootKey;

	fclose(cfgFile);

}
sCfg::~sCfg() {
}

void sCfg::setKey(string keyDesc_, bool ignoreError, bool* oKeyFound_) {

	
	sObj* keyObj=nullptr;

	//-- handle modifiers
	setActualKeyDesc(keyDesc_.c_str());

	//-- call sObj findChild on actual relative path
	findChild(keyDesc, &keyObj);
	if (keyObj==nullptr) {
		if (!ignoreError) fail("XML key not found. keyDesc=%s", keyDesc);
		if (oKeyFound_!=nullptr) (*oKeyFound_)=false;
	} else {
		//-- set this Key as current
		currentKey=(sCfgKey*)keyObj;
		if (oKeyFound_!=nullptr) (*oKeyFound_)=true;
	}

}

//-- private
void sCfg::parse() {
	int l;

	//-- 1. set depth for each line, and find maxDepth
	
	for (l=0; l<linesCnt; l++) {

		if (cfgLine[l]->type==cfgLine_KeyStart) {
			cfgLine[l]->parent=currParent[currDepth];
			cfgLine[l]->depth=currDepth;
			currDepth++;
			//--
			prevParent[currDepth]=currParent[currDepth-1];
			currParent[currDepth]=l;
			//--
		} else if (cfgLine[l]->type==cfgLine_Parm||cfgLine[l]->type==cfgLine_Comment) {
			cfgLine[l]->parent=currParent[currDepth];
			cfgLine[l]->depth=currDepth;
		} else if (cfgLine[l]->type==cfgLine_KeyEnd) {
			currDepth--;
			cfgLine[l]->depth=currDepth;
			cfgLine[l]->parent=currParent[currDepth];
			//--
			currParent[currDepth]=prevParent[currDepth+1];
			//--
		}
	}

	//-- 2. pair start/end keys
	int endl;
	for (int startl=0; startl<linesCnt; startl++) {
		if (cfgLine[startl]->type==cfgLine_KeyStart) {
			cfgLine[startl]->partner=-1;
			for (endl=startl+1; endl<linesCnt; endl++) {
				if (_stricmp(cfgLine[startl]->naked, cfgLine[endl]->naked)==0&&cfgLine[startl]->depth==cfgLine[endl]->depth && cfgLine[endl]->type==cfgLine_KeyEnd) {
					cfgLine[startl]->partner=endl;
					cfgLine[endl]->partner=startl;
					break;
				}
			}
			if (cfgLine[startl]->partner==-1) fail("XML error: key %s at line %d not closed", cfgLine[startl]->clean, startl+1);
		}
	}

}
void sCfg::setActualKeyDesc(const char* keyDesc_) {
	int r, p;

	r=0;
	while (keyDesc_[r]=='/') {
		//-- start from this cfg rootKey full name
		strcpy_s(keyDesc, XMLKEY_PATH_MAXLEN, rootKey->name->full);
		//-- append sought keyDesc, without initial '/'
		if (strlen(keyDesc_)>1) {
			strcat_s(keyDesc, XMLKEY_PATH_MAXLEN, "/");
			strcat_s(keyDesc, XMLKEY_PATH_MAXLEN, &keyDesc_[r+1]);
		}
		r++;
	}

	p=0;
	while (keyDesc_[p+0]=='.' &&keyDesc_[p+1]=='.' &&keyDesc_[p+2]=='/') {
		//-- start from currentKey's parent full name
		currentKey=(sCfgKey*)currentKey->parent;
		strcpy_s(keyDesc, XMLKEY_PATH_MAXLEN, currentKey->name->full);
		p+=3;
	}
	//-- append sought keyDesc, without initial '..'
	if (p>0&&strlen(keyDesc_)>p) {
		strcat_s(keyDesc, XMLKEY_PATH_MAXLEN, &keyDesc_[p-1]);
	}

	if (r==0&&p==0) {
		//-- start from this cfg currentKey full name
		strcpy_s(keyDesc, XMLKEY_PATH_MAXLEN, currentKey->name->full);
		//-- append sought keyDesc, without initial '/'
		strcat_s(keyDesc, XMLKEY_PATH_MAXLEN, "/");
		//--check if current path ("." or "./") is specified
		int s=0;
		if (keyDesc_[0]=='.') s++;
		if (keyDesc_[1]=='/') s++;
		strcat_s(keyDesc, XMLKEY_PATH_MAXLEN, &keyDesc_[s]);
	}

	//-- cut out this cfg name from initial part of keyDesc, so to make it relative
	int rootlen=(int)strlen(name->full);
	int desclen=(int)strlen(keyDesc);
	int difflen=desclen-rootlen;
	int i;
	for (i=0; i<difflen; i++) keyDesc[i]=keyDesc[i+rootlen+1];

}
