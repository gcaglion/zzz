#include "sFXDataSource.h"

//=== sFXDataSource
sFXDataSource::sFXDataSource(sObjParmsDef, sOraData* db_, const char* symbol_, const char* tf_, bool isFilled_) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	type=DB_SOURCE; oradb=db_;

	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXDataSource::sFXDataSource(sObjParmsDef, sFileData* file_, const char* symbol_, const char* tf_, bool isFilled_) : sDataSource(sObjParmsVal, -99, false, NULL, NULL) {
	type=FILE_SOURCE; filedb=file_;

	strcpy_s(Symbol, FX_SYMBOL_MAXLEN, symbol_);
	strcpy_s(TimeFrame, FX_TIMEFRAME_MAXLEN, tf_);
	IsFilled=isFilled_;
}
sFXDataSource::sFXDataSource(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfg->currentKey, getParm, &Symbol, "Symbol");
	safecall(cfg->currentKey, getParm, &TimeFrame, "TimeFrame");
	safecall(cfg->currentKey, getParm, &IsFilled, "IsFilled");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(oradb, newsname("DBConnection"), defaultdbg, cfg, "DBConnection");
	type=DB_SOURCE; featuresCnt=FXDATA_FEATURESCNT;
	//-- 3. Restore currentKey
	cfg->currentKey=bkpKey;

}
sFXDataSource::~sFXDataSource() {
	free(Symbol);
	free(TimeFrame);
}

void sFXDataSource::load(char* pDate0, int pDate0Lag, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar) {
	//-- we could also retrieve FXData from file...
	safecall(oradb, getFlatOHLCV2, Symbol, TimeFrame, pDate0, pDate0Lag, pRecCount, oBarTime, oBarData, oBaseTime, oBaseBar);
}
void sFXDataSource::open() {
	safecall(oradb, open);
}
void sFXDataSource::getStartDates(char* date0_, int datesCnt_, char*** oStartDates_) {
	if (type==DB_SOURCE) {
		safecall(oradb, getStartDates, Symbol, TimeFrame, IsFilled, date0_, datesCnt_, oStartDates_);
	} else {
		safecall(filedb, getStartDates, date0_, datesCnt_, oStartDates_);
	}
}
void sFXDataSource::loadFuture(char* iSymbol_, char* iTF_, char* iDate0_, char* oDate1_, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV) {
	safecall(oradb, getFutureBar, iSymbol_, iTF_, iDate0_, oDate1_, oBarO, oBarH, oBarL, oBarC, oBarV);
}
