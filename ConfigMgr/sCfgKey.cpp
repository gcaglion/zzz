#include "sCfgKey.h"

sCfgKey::sCfgKey(sObjParmsDef, int linesCnt_, sCfgLine** cfgLine_, int startLine_) : sObj(sObjParmsVal) {

	startLine=startLine_;
	endLine=(startLine<0)?linesCnt_:cfgLine_[startLine]->partner;
	keysCnt=0; parmsCnt=0;

	//-- scan key entries
	for (int l=startLine+1; l<endLine; l++) {

		//-- check for subkeys
		if (cfgLine_[l]->type==cfgLine_KeyStart) {

			//-- spawn subKey with updated debugger
			safespawn(key[keysCnt], newsname("%s", cfgLine_[l]->naked), dbg, linesCnt_, cfgLine_, l);
			
			//-- check for key-specific Debugger for the current subKey
			setDbg();

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

void sCfgKey::setDbg() {

	//-- key-specific debugger parameters are initialized to defaults
	Bool dbg_verbose_=DEFAULT_DBG_VERBOSITY; Bool dbg_dbgtoscreen_=DEFAULT_DBG_TO_SCREEN; Bool dbg_dbgtofile_=DEFAULT_DBG_TO_FILE;
	char* dbg_outfilepath_ = new char[MAX_PATH]; dbg_outfilepath_[0]='\0';

	sObj* dbgKey=nullptr;

	safecall(this, findChild, "Debugger", &dbgKey);
	if (dbgKey!=nullptr) {
		//-- if <Debugger> sub-key is found, override default parameters

		safecall(this, getParm, &dbg_verbose_, "Debugger/Verbose", (int*)nullptr, true);
		safecall(this, getParm, &dbg_dbgtoscreen_, "Debugger/ScreenOutput", (int*)nullptr, true);
		safecall(this, getParm, &dbg_dbgtofile_, "Debugger/FileOutput", (int*)nullptr, true);
		safecall(this, getParm, &dbg_outfilepath_, "Debugger/OutFilePath", (int*)nullptr, true);
	}

	//-- sObj always sets dbg (to default, parent's, or the one passed)
	if (dbg!=parent->dbg) delete dbg;	//-- delete current dbg, unless it's inherited by parent
	dbg = new sDbg(dbg_verbose_, dbg_dbgtoscreen_, dbg_dbgtofile_, dbg_outfilepath_);
	dbg->createOutFile(name->base, this, depth);
	//-- 

}
