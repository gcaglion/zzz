#include "sGA.h"

sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
sGA::~sGA(){}

//-- abstract methods implementations
void sGA::setLayout(int batchSize_) {

}
void sGA::mallocLayout() {
	//-- malloc + init neurons ...
}
void sGA::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("GA training complete.");
}
void sGA::singleInfer(int sampleLen_, int sampleFeaturesCnt_, int batchSamplesCnt_, numtype* singleSampleBF, numtype* singleTargetBF, numtype** singlePredictionBF) {

	//-- 1. load input neurons. Need to MAKE SURE incoming array len is the same as inputcount!!!

	//-- 2. forward pass

	//-- 3. copy last layer neurons (on dev) to prediction (on host)

	//-- 3.1. perfect core!
	for (int i=0; i<layout->outputCnt; i++) (*singlePredictionBF)[i]=singleTargetBF[i];

}
void sGA::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sGA::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
