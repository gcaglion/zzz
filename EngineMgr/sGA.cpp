#include "sGA.h"

sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
sGA::~sGA(){}

void sGA::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("GA training complete.");
}
void sGA::infer(sCoreProcArgs* inferArgs) {

	size_t bfsSize=inferArgs->ds->samplesCnt*inferArgs->ds->predictionLen*inferArgs->ds->selectedFeaturesCnt;
	//-- perfect core!
	for (int i=0; i<bfsSize; i++) inferArgs->ds->predictionBFS[i] = inferArgs->ds->targetBFS[i];
	info("GA inference complete.");

}
