#include "sCfgKey.h"

sCfgKey::sCfgKey(sObjParmsDef, int linesCnt_, sCfgLine** cfgLine_, int startLine_) : sObj(sObjParmsVal) {

	startLine=startLine_;
	endLine=(startLine<0)?linesCnt_:cfgLine_[startLine]->partner;
	keysCnt=0; parmsCnt=0;

	//bool dbg_verbose_=-1; bool dbg_dbgtoscreen_=-1; bool dbg_dbgtofile_=-1; char* dbg_outfilepath_ = nullptr;
	//sCfgParm* tmpParm;
	sDbg* overrideDbg;

	//-- scan key entries
	for (int l=startLine+1; l<endLine; l++) {

		//-- check for subkeys
		if (cfgLine_[l]->type==cfgLine_KeyStart) {

			//setDbg();

			//-- spawn subKey with updated debugger
			safespawn(false, key[keysCnt], newsname("%s", cfgLine_[l]->naked), dbg, linesCnt_, cfgLine_, l);

			//-- if what we just spawned is a Debugger key, update overrideDbg
			if (_stricmp(key[keysCnt]->name->base, "Debugger")==0) {
				//-- clone current dbg into overrideDbg
				overrideDbg=clonedbg(dbg);
				//-- update overrideDBG parms from xml
				safecall(this, getParm, &overrideDbg->verbose, "Debugger/Verbose", true);
				safecall(this, getParm, &overrideDbg->dbgtoscreen, "Debugger/ScreenOutput", true);
				safecall(this, getParm, &overrideDbg->dbgtofile, "Debugger/FileOutput", true);
				safecall(this, getParm, &overrideDbg->outfilepath, "Debugger/OutFilePath", true);
				//-- delete current dbg, unless it's inherited by parent
				if (dbg!=parent->dbg) delete dbg;
				//-- overrideDbg becomes the actual dbg
				dbg=overrideDbg;
				dbg->createOutFile(name->base, this, depth);
			}

			//-- get out of subKey
			l=key[keysCnt]->endLine;
			keysCnt++;
		}

		//-- check for key parameters
		if (cfgLine_[l]->type==cfgLine_Parm) {
			safespawn(false, parm[parmsCnt], newsname("%s", cfgLine_[l]->pname), dbg, cfgLine_[l]->pname, cfgLine_[l]->pval);
			parmsCnt++;
		}
	}

}
sCfgKey::~sCfgKey(){}

void sCfgKey::setDbg() {

	//-- key-specific debugger parameters are initialized to defaults
	bool dbg_verbose_=DEFAULT_DBG_VERBOSITY; bool dbg_dbgtoscreen_=DEFAULT_DBG_TO_SCREEN; bool dbg_dbgtofile_=DEFAULT_DBG_TO_FILE;
	char* dbg_outfilepath_ = new char[MAX_PATH]; dbg_outfilepath_[0]='\0';

	sObj* dbgKey=nullptr;

	safecall(this, findChild, "Debugger", &dbgKey);
	if (dbgKey!=nullptr) {
		//-- if <Debugger> sub-key is found, override default parameters

		safecall(this, getParm, &dbg_verbose_, "Debugger/Verbose", true);
		safecall(this, getParm, &dbg_dbgtoscreen_, "Debugger/ScreenOutput", true);
		safecall(this, getParm, &dbg_dbgtofile_, "Debugger/FileOutput", true);
		safecall(this, getParm, &dbg_outfilepath_, "Debugger/OutFilePath", true);
	}

	//-- sObj always sets dbg (to default, parent's, or the one passed)
	if (dbg!=parent->dbg) delete dbg;	//-- delete current dbg, unless it's inherited by parent
	dbg = new sDbg(dbg_verbose_, dbg_dbgtoscreen_, dbg_dbgtofile_, dbg_outfilepath_);
	dbg->createOutFile(name->base, this, depth);
	//-- 

}
