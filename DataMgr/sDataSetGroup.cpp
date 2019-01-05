#include "sDataSetGroup.h"

void sDataSetGroup::sDataSetGroup_pre() {
	inputCnt=0; outputCnt=0;
	ds=(sDataSet**)malloc(dataSetsCnt*sizeof(sDataSet*));
}

sDataSetGroup::sDataSetGroup(sObjParmsDef, int dataSetsCnt_, sDataSet** ds_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	dataSetsCnt=dataSetsCnt_;	
	sDataSetGroup_pre();
	for (int i=0; i<dataSetsCnt; i++) {
		ds[i]=ds_[i];
	}
	sDataSetGroup_post();
}
void sDataSetGroup::sDataSetGroup_post() {
	//-- sum sampleLen*selFcnt, predictionLen*selFcnt of each dataset into group shape samplelen
	for (int i=0; i<dataSetsCnt; i++) {
		inputCnt+=ds[i]->shape->sampleLen*ds[i]->shape->featuresCnt;
		outputCnt+=ds[i]->shape->predictionLen*ds[i]->shape->featuresCnt;
	}
}
sDataSetGroup::sDataSetGroup(sCfgObjParmsDef, int extraSteps) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &dataSetsCnt, "DataSetsCount");

	//-- 2. do stuff and spawn sub-Keys
	sDataSetGroup_pre();

	for (int i=0; i<dataSetsCnt; i++) {
		safespawn(ds[i], newsname("DataSet%d", i), defaultdbg, cfg, (newsname("/DataSetGroup/DataSet%d", i))->base, extraSteps);
	}
	sDataSetGroup_post();

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sDataSetGroup::~sDataSetGroup() {
	free(ds);
}
