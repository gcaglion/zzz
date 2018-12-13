#include "sDataShape.h"

sDataShape::sDataShape(sObjParmsDef, int sampleLen_, int predictionLen_, int featuresCnt_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sampleLen=sampleLen_; predictionLen=predictionLen_; featuresCnt=featuresCnt_;
}
sDataShape::sDataShape(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &predictionLen, "PredictionLen");
	featuresCnt=0;
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sDataShape::~sDataShape() {
}

