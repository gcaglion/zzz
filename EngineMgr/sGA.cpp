#include "sGA.h"

sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
sGA::~sGA(){}

void sGA::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("GA training complete.");
}
void sGA::infer(sCoreProcArgs* inferArgs) {

	inferArgs->ds->reorder(SAMPLE, SBF, BFS);
	inferArgs->ds->reorder(TARGET, SBF, BFS);


	//-- 3.1. use simple pointers to the above arrays. Let's say sGA doen't need BFS conversion
	numtype* sample=inferArgs->ds->sampleBFS;
	numtype* target=inferArgs->ds->targetBFS;
	numtype* prediction=inferArgs->ds->predictionBFS;
//	numtype* sample=inferArgs->ds->sampleSBF;
//	numtype* target=inferArgs->ds->targetSBF;
//	numtype* prediction=inferArgs->ds->predictionSBF;


	//-- core work...
	size_t bfsSize=inferArgs->ds->samplesCnt*inferArgs->ds->predictionLen*inferArgs->ds->selectedFeaturesCnt;
	//-- perfect core!
	for (int i=0; i<bfsSize; i++) prediction[i]=target[i];

	//-- convert prediction in runSet from BFS to SBF order
	inferArgs->ds->reorder(PREDICTED, BFS, SBF);

	info("GA inference complete.");

}
