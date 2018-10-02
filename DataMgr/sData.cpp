#include "sData.h"

sData::sData(sCfgObjParmsDef, sDataShape* shape_, sDataSet* trainDS_, sDataSet* testDS_, sDataSet* validDS_) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;
	trainDS=trainDS_; testDS=testDS_; validDS=validDS_;
}
sData::sData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	//-- 2. do stuff and spawn sub-Keys
	safespawn(false, shape, newsname("Shape"), nullptr, cfg, "Shape");
	doTraining		= safespawn(true, trainDS, newsname("TrainDataSet"), defaultdbg, cfg, "Train/DataSet");
	doTest			= safespawn(true, testDS,  newsname("TestData"),  defaultdbg, cfg, "Test/DataSet");
	doValidation	= safespawn(true, validDS, newsname("ValidDataset"), defaultdbg, cfg, "Validation/DataSet");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sData::~sData() {
	delete trainDS;
	delete testDS;
	delete validDS;
	delete shape;
}
