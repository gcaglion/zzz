#include "sData.h"

sData::sData(sObjParmsDef, sDataShape* shape_, sDataSet* trainDS_, sDataSet* testDS_, sDataSet* validDS_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	shape=shape_;
	trainDS=trainDS_; testDS=testDS_; validDS=validDS_;
}
sData::sData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &doTraining,	"Train/Do");
	safecall(cfgKey, getParm, &doInference,		"Test/Do");
	safecall(cfgKey, getParm, &doValidation,"Validation/Do");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(shape, newsname("Shape"), nullptr, cfg, "Shape");
	if(doTraining) safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, cfg, "Train/DataSet", shape->sampleLen, shape->predictionLen);
	if(doInference) safespawn(testDS,  newsname("TestData"),  defaultdbg, cfg, "Test/DataSet", shape->sampleLen, shape->predictionLen);
	if(doValidation) safespawn(validDS, newsname("ValidDataset"), defaultdbg, cfg, "Validation/DataSet", shape->sampleLen, shape->predictionLen);
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sData::~sData() {
	delete trainDS;
	delete testDS;
	delete validDS;
	delete shape;
}
