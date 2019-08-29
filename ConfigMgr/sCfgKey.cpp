#include "sCfgKey.h"

sCfgKey::sCfgKey(sObjParmsDef, int linesCnt_, sCfgLine** cfgLine_, int startLine_) : sObj(sObjParmsVal) {

	startLine=startLine_;
	endLine=(startLine<0)?linesCnt_:cfgLine_[startLine]->partner;
	keysCnt=0; parmsCnt=0;

	//bool dbg_verbose_=-1; bool dbg_dbgtoscreen_=-1; bool dbg_dbgtofile_=-1; char* dbg_outfilepath_ = nullptr;
	//sCfgParm* tmpParm;

	//-- scan key entries
	for (int l=startLine+1; l<endLine; l++) {

		//-- check for subkeys
		if (cfgLine_[l]->type==cfgLine_KeyStart) {

			//setDbg();

			//-- spawn subKey with updated debugger
			safespawn(key[keysCnt], newsname("%s", cfgLine_[l]->naked), dbg, linesCnt_, cfgLine_, l);

			//-- if what we just spawned is a Debugger key, update overrideDbg
			if (_stricmp(key[keysCnt]->name->base, "Debugger")==0) {

				//-- create overrideDbg with default settings
				overrideDbg=defaultdbg;
				//-- update overrideDBG parms from xml
				safecall(this, getParm, &overrideDbg->verbose, "Debugger/Verbose", true);
				safecall(this, getParm, &overrideDbg->dbgtoscreen, "Debugger/ScreenOutput", true);
				safecall(this, getParm, &overrideDbg->dbgtofile, "Debugger/FileOutput", true);
				safecall(this, getParm, &overrideDbg->outfilepath, "Debugger/OutFilePath", true);
				safecall(this, getParm, &overrideDbg->timing, "Debugger/Timing", true);
			}

			//-- get out of subKey
			l=key[keysCnt]->endLine;
			keysCnt++;
		}

		//-- check for key parameters
		if (cfgLine_[l]->type==cfgLine_Parm) {
			safespawn(parm[parmsCnt], newsname("%s", cfgLine_[l]->pname), dbg, cfgLine_[l]->pname, cfgLine_[l]->pval);
			parmsCnt++;
		}
	}

}
sCfgKey::~sCfgKey(){}

void sCfgKey::setParm(const char* parmDesc_, const char* parmValS_) {
	sObj* parmObj=nullptr;

	//-- call sObj findChild on parmDesc
	safecallSilent(this, findChild, parmDesc_, &parmObj);
	if (parmObj==nullptr) {
		//if (!ignoreError) fail("XML parameter not found. keyDesc=%s, parmDesc=%s", name->full, parmDesc);
	} else {
		if (!((sCfgParm*)parmObj)->setValS(parmValS_)) {
			//if (!ignoreError) fail("Invalid XML parameter value. keyDesc=%s, parmDesc=%s, value string= \"%s\"", name->full, parmDesc, ((sCfgParm*)parmObj)->valS);
		}
	}
}
void sCfgKey::setDbg() {

	//-- key-specific debugger parameters are initialized to defaults
	bool dbg_verbose_=false; bool dbg_timing_=false; bool dbg_dbgtoscreen_=true; bool dbg_dbgtofile_=false;
	char* dbg_outfilepath_ = new char[MAX_PATH]; dbg_outfilepath_[0]='\0';

	sObj* dbgKey=nullptr;

	safecall(this, findChild, "Debugger", &dbgKey);
	if (dbgKey!=nullptr) {
		//-- if <Debugger> sub-key is found, override default parameters

		safecall(this, getParm, &dbg_verbose_, "Debugger/Verbose", true);
		safecall(this, getParm, &dbg_dbgtoscreen_, "Debugger/ScreenOutput", true);
		safecall(this, getParm, &dbg_dbgtofile_, "Debugger/FileOutput", true);
		safecall(this, getParm, &dbg_outfilepath_, "Debugger/OutFilePath", true);
		safecall(this, getParm, &dbg_timing_, "Debugger/Timing", true);
	}

	//-- sObj always sets dbg (to default, parent's, or the one passed)
	if (dbg!=parent->dbg) delete dbg;	//-- delete current dbg, unless it's inherited by parent
	dbg = new sDbg(dbg_verbose_, dbg_timing_, dbg_dbgtoscreen_, dbg_dbgtofile_, dbg_outfilepath_);
	dbg->createOutFile(name->base, this, depth);
	//-- 

}

int sCfgKey::getParmsCntTot() {
	int ret=parmsCnt;
	for (int k=0; k<keysCnt; k++) {
		ret+=key[k]->getParmsCntTot();
	}
	return ret;
}
void sCfgKey::getAllParms(int* oParmsCnt, char** oParmDesc, char** oParmValS) {
	for (int k=0; k<keysCnt; k++) {
		key[k]->getAllParms(oParmsCnt, oParmDesc, oParmValS);
	}
	for (int p=0; p<parmsCnt; p++) {
		strcpy_s(oParmDesc[(*oParmsCnt)], ObjMaxDepth*ObjNameMaxLen, parm[p]->name->full);
		strcpy_s(oParmValS[(*oParmsCnt)], XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT, parm[p]->valcsl);
		(*oParmsCnt)++;
	}
	
}