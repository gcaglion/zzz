#include "sData.h"

sData::sData(sCfgObjParmsDef, sDataShape* shape_, sDataSet* trainDS_, sDataSet* testDS_, sDataSet* validDS_) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;
	trainDS=trainDS_; testDS=testDS_; validDS=validDS_;
}
sData::sData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &doTraining,	"Train/Do");
	safecall(cfgKey, getParm, &doTest,		"Test/Do");
	safecall(cfgKey, getParm, &doValidation,"Validation/Do");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(false, shape, newsname("Shape"), nullptr, cfg, "Shape");
	if(doTraining) safespawn(false, trainDS, newsname("TrainDataSet"), defaultdbg, cfg, "Train/DataSet");
	if(doTest) safespawn(false, testDS,  newsname("TestData"),  defaultdbg, cfg, "Test/DataSet");
	if(doValidation) safespawn(false, validDS, newsname("ValidDataset"), defaultdbg, cfg, "Validation/DataSet");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sData::~sData() {
	delete trainDS;
	delete testDS;
	delete validDS;
	delete shape;
}
