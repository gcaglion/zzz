#include "sFXDataSource.h"

//=== sFXDataSource
sFXDataSource::sFXDataSource(sObjParmsDef, sOraData* db_, char* symbol_, char* tf_, bool isFilled_, bool autoOpen) : sDataSource(sObjParmsVal, db_, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXDataSource::sFXDataSource(sObjParmsDef, sFileData* file_, char* symbol_, char* tf_, bool isFilled_, bool autoOpen) : sDataSource(sObjParmsVal, file_, -99, false, NULL, NULL) {
	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXDataSource::sFXDataSource(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfg->currentKey, getParm, &Symbol, "Symbol");
	safecall(cfg->currentKey, getParm, &TimeFrame, "TimeFrame");
	safecall(cfg->currentKey, getParm, &IsFilled, "IsFilled");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(oradb, newsname("DBConnection"), defaultdbg, cfg, "DBConnection", autoOpen);
	//-- 3. Restore currentKey
	cfg->currentKey=bkpKey;

}
sFXDataSource::~sFXDataSource() {
	free(Symbol);
	free(TimeFrame);
}

void sFXDataSource::load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {
	//-- we could also retrieve FXData from file...
	safecall(oradb, getFlatOHLCV2, Symbol, TimeFrame, pDate0, pRecCount, oBarTime, oBarData, oBaseTime, oBaseBar);
}
void sFXDataSource::open() {
	safecall(oradb, open);
}
void sFXDataSource::getStartDates(char* date0_, int datesCnt_, char** oStartDates_) {
	if (type==DB_SOURCE) {
		safecall(oradb, getStartDates, Symbol, TimeFrame, IsFilled, date0_, datesCnt_, oStartDates_);
	} else {
		//safecall(filedb, getStartDates, file, date0_, datesCnt_, oStartDates_);
	}
}