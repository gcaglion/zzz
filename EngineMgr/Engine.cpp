#include "Engine.h"

//-- Engine stuff

/*
void sEngine::addCore(int coreId) {

	switch (coreLayout[coreId]->type) {
	case CORE_NN:
		core[coreId]=new sNN()
		safespawn(false, core[coreId], newsname("Core%d-NN", coreId), defaultdbg,  tNN, parms, coreLayout[coreId]);
		break;
	case CORE_GA:
		sprintf_s(cdesc, CORE_MAX_DESC_LEN, "Core%d(GA)", coreId);
		//safespawn(core[coreId], tGA, parms, coreLayout[coreId]);
		break;
	case CORE_SVM: 
		break;
	case CORE_SOM: 
		break;
	default:
		fail("coreId %d : invalid coreType: %d", coreLayout[coreId]->Id, coreLayout[coreId]->type);
		break;
	}
}
*/
sEngine::sEngine(sCfgObjParmsDef, sDataShape* dataShape_) : sCfgObj(sCfgObjParmsVal) {

	dataShape=dataShape_;
	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;

	//-- 1. get Parameters
	//-- 2. do stuff and spawn sub-Keys
	int c;

	safecall(cfgKey, getParm, &type, "Type");

	switch (type) {
	case ENGINE_CUSTOM:
		//-- 0. coresCnt
		safecall(cfgKey, getParm, &coresCnt, "Custom/CoresCount");
		//-- 1. malloc one core and one coreLayout for each core
		core=(sCore**)malloc(coresCnt*sizeof(sCore*));
		//-- 2. create layout, set base coreLayout properties for each Core (type, desc, connType, outputCnt)
		for (c=0; c<coresCnt; c++) {
			safespawn(false, core[c], newsname("Core%d",c), defaultdbg, cfg, (newsname("Core%d", c))->base, c, dataShape);
		}
		break;
	case ENGINE_WNN:
		//safecall(parms->setKey("WNN"));
		//... get() ...
		break;
	case ENGINE_XIE:
		//safecall(parms->setKey("XIE"));
		//... get() ...
		break;
	default:
		fail("Invalid Engine Type: %d", type);
		break;
	}

	//-- 3. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
	for (c=0; c<coresCnt; c++) {
		setCoreLayer(core[c]);
		layerCoresCnt[core[c]->layer]++;
	}
	//-- 4. determine layersCnt, and InputCnt for each Core
	for (int l=0; l<MAX_ENGINE_LAYERS; l++) {
		for (c=0; c<layerCoresCnt[l]; c++) {
			if (l==0) {
				//-- do nothing. keep core shape same as engine shape
			} else {
				//-- change sampleLen
				core[c]->baseDataShape->sampleLen=layerCoresCnt[l-1]*core[c]->baseDataShape->predictionLen;
			}
		}
		if (c==0) break;
		layersCnt++;
	}

	//-- 5. init each core
	for (c=0; c<coresCnt; c++) {
		core[c]->init(c, dataShape, nullptr);
	}
	
	//-- 6. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}

sEngine::~sEngine() {
	free(core);
	//free(coreLayout);
	free(layerCoresCnt);
}

void sEngine::setCoreLayer(sCore* c) {
	int ret=0;
	int maxParentLayer=-1;
	for (int p=0; p<c->parentsCnt; p++) {
		sCore* parent=core[c->parentId[p]];
		setCoreLayer(parent);
		if (parent->layer>maxParentLayer) {
			maxParentLayer=parent->layer;
		}
		ret=maxParentLayer+1;
	}
	c->layer=ret;
}
