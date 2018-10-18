#include "sFXDataSource.h"

//=== sFXDataSource
sFXDataSource::sFXDataSource(sCfgObjParmsDef, sOraData* db_, char* symbol_, char* tf_, bool isFilled_, bool autoOpen) : sDataSource(sCfgObjParmsVal, DB_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	db=db_;
	file=nullptr;
	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXDataSource::sFXDataSource(sCfgObjParmsDef, sFileData* file_, char* symbol_, char* tf_, bool isFilled_, bool autoOpen) : sDataSource(sCfgObjParmsVal, DB_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	db=nullptr;
	file=file_;
	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXDataSource::sFXDataSource(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal, DB_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {

	//-- 1. get Parameters
	safecall(cfg->currentKey, getParm, &Symbol, "Symbol");
	safecall(cfg->currentKey, getParm, &TimeFrame, "TimeFrame");
	safecall(cfg->currentKey, getParm, &IsFilled, "IsFilled");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(db, newsname("DBConnection"), defaultdbg, cfg, "DBConnection", autoOpen);
	//-- 3. Restore currentKey
	cfg->currentKey=bkpKey;

}
sFXDataSource::~sFXDataSource() {
	free(Symbol);
	free(TimeFrame);
}

void sFXDataSource::load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {

	//-- we could also retrieve FXData from file...

	safecall(db, getFlatOHLCV, Symbol, TimeFrame, pDate0, pRecCount, oBarTime, oBarData, oBaseTime, oBaseBar);


}
void sFXDataSource::open() {
	safecall(db, open);
}
void sFXDataSource::getStartDates(char* date0_, int datesCnt_, char** oStartDates_) {
	if (type==DB_SOURCE) {
		db->getStartDates(Symbol, TimeFrame, IsFilled, date0_, datesCnt_, oStartDates_);
	} else {
		file->getStartDates(file, date0_, datesCnt_, oStartDates_);
	}
}