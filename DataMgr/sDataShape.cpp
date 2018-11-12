#include "sDataShape.h"

sDataShape::sDataShape(sObjParmsDef, int sampleLen_, int predictionLen_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sampleLen=sampleLen_; predictionLen=predictionLen_;
}
sDataShape::sDataShape(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &predictionLen, "PredictionLen");
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sDataShape::~sDataShape() {
}

