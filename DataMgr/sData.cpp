#include "sData.h"

sDataShape::sDataShape(sCfgObjParmsDef, int sampleLen_, int predictionLen_, int featuresCnt_) : sCfgObj(sCfgObjParmsVal) {
	sampleLen=sampleLen_; predictionLen=predictionLen_; featuresCnt=featuresCnt_;
}
sDataShape::sDataShape(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &predictionLen, "PredictionLen");
	safecall(cfgKey, getParm, &featuresCnt, "FeaturesCount");
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sDataShape::~sDataShape() {
}

sData::sData(sCfgObjParmsDef, sDataShape* shape_, Bool doTrain, Bool doTest, Bool doValidation) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;
	ActionDo[TRAIN]=doTrain; ActionDo[TEST]=doTest; ActionDo[VALID]=doValidation;
}
sData::sData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 0. backup currentKey
	bkpKey=cfg->currentKey;
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &ActionDo[TRAIN], "Train/Do");
	safecall(cfgKey, getParm, &ActionDo[TEST], "Test/Do");
	safecall(cfgKey, getParm, &ActionDo[VALID], "Validation/Do");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(shape, newsname("Shape"), nullptr, cfg, "Shape");
	if (ActionDo[TRAIN]) safespawn(ds[TRAIN], newsname("Train_Data"), nullptr, cfg, "Train/DataSet");
	if (ActionDo[TEST])  safespawn(ds[TEST], newsname("Test_Data"), nullptr, cfg, "Test/DataSet");
	if (ActionDo[VALID]) safespawn(ds[VALID], newsname("Validation_Data"), nullptr, cfg, "Validation/DataSet");	
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sData::~sData() {
	for (int a=0; a<3; a++) {
		if (ActionDo[a]) {
			delete ds[a];
		}
	}
	delete shape;
}
