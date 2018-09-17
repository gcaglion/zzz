#include "sData.h"

sDataShape::sDataShape(sCfgObjParmsDef, int sampleLen_, int predictionLen_, int featuresCnt_) : sCfgObj(sCfgObjParmsVal) {
	sampleLen=sampleLen_; predictionLen=predictionLen_; featuresCnt=featuresCnt_;
}
sDataShape::sDataShape(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 0. Backup currentKey
	sCfgKey* bkpKey=cfg->currentKey;
	//-- 1. set Key 
 safecall(cfg, setKey, keyDesc_);
	//-- 2. get Parameters
	safecall(cfg->currentKey, getParm, &sampleLen, "SampleLen");
	safecall(cfg->currentKey, getParm, &predictionLen, "PredictionLen");
	safecall(cfg->currentKey, getParm, &featuresCnt, "FeaturesCount");
	//-- 3. spawn sub-Keys
	//-- 4. Restore currentKey
	cfg->currentKey=bkpKey;

}
sDataShape::~sDataShape() {
}

sData::sData(sCfgObjParmsDef, sDataShape* shape_, Bool doTrain, Bool doTest, Bool doValidation) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;
	ActionDo[TRAIN]=doTrain; ActionDo[TEST]=doTest; ActionDo[VALID]=doValidation;
}
sData::sData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 0. Backup currentKey
	sCfgKey* bkpKey=cfg->currentKey;
	//-- 1. set Key 
 safecall(cfg, setKey, keyDesc_);
	//-- 2. get Parameters
	safecall(cfg->currentKey, getParm, &ActionDo[TRAIN], "Train/Do");
	safecall(cfg->currentKey, getParm, &ActionDo[TEST], "Test/Do");
	safecall(cfg->currentKey, getParm, &ActionDo[VALID], "Validation/Do");
	//-- 3. spawn sub-Keys
	safespawn(shape, newsname("Shape"), nullptr, cfg, "Shape");
	
	if (ActionDo[TRAIN]) safespawn(ds[TRAIN], newsname("Train_Data"), nullptr, cfg, "Train/DataSet");
	if (ActionDo[TEST])  safespawn(ds[TEST], newsname("Test_Data"), nullptr, cfg, "Test/DataSet");
	if (ActionDo[VALID]) safespawn(ds[VALID], newsname("Validation_Data"), nullptr, cfg, "Validation/DataSet");
	
	//-- 4. Restore currentKey
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
