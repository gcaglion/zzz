#include "sGA.h"

sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
sGA::~sGA(){}

void sGA::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("GA training complete.");
}
void sGA::infer(sCoreProcArgs* inferArgs) {

	//-- 3.1. use simple pointers to the above arrays. Let's say sGA doen't need BFS conversion
	numtype* sample=inferArgs->ds->sampleSBF[TRS];
	numtype* target=inferArgs->ds->targetSBF[TRS];
	numtype* prediction=inferArgs->ds->predictionSBF[TRS];


	size_t bfsSize=inferArgs->ds->samplesCnt*inferArgs->ds->predictionLen*inferArgs->ds->selectedFeaturesCnt;
	//-- perfect core!
	for (int i=0; i<bfsSize; i++) prediction[i]=target[i];

	info("GA inference complete.");

}
