#include "FXData.h"

//=== sFXData
sFXData::sFXData(sCfgObjParmsDef, sOraConnection* db_, char* symbol_, char* tf_, Bool isFilled_) : sDataSource(sCfgObjParmsVal, FXDB_SOURCE, FXDataFeaturesCnt, true, FXHIGH, FXLOW) {
	db=db_;
	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXData::sFXData(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal, FXDB_SOURCE, FXDataFeaturesCnt, true, FXHIGH, FXLOW) {

	//-- 0. Backup currentKey
	sCfgKey* bkpKey=cfg->currentKey;
	//-- 1. set Key 
 safecall(cfg, setKey, keyDesc_);
	//-- 2. get Parameters
	safecall(cfg->currentKey, getParm, &Symbol, "Symbol");
	safecall(cfg->currentKey, getParm, &TimeFrame, "TimeFrame");
	safecall(cfg->currentKey, getParm, &IsFilled, "IsFilled");
	//-- 3. spawn sub-Keys
	safespawn(db, newsname("DBConnection"), nullptr, cfg, "DBConnection");
	//-- 4. Restore currentKey
	cfg->currentKey=bkpKey;
}
sFXData::~sFXData() {
	free(Symbol);
	free(TimeFrame);
}

void sFXData::load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {

	//-- we could also retrieve FXData from file...

	safecall(db, getFlatOHLCV, Symbol, TimeFrame, pDate0, pRecCount, oBarTime, oBarData, oBaseTime, oBaseBar);


}
