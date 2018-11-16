#include "sSVM.h"

sSVM::sSVM(sCfgObjParmsDef, sCoreLayout* layout_, sSVMparms* SVMparms_) : sCore(sCfgObjParmsVal, layout_) {}
sSVM::~sSVM() {}

//-- abstract methods implementations
void sSVM::setLayout(int batchSize_) {

}
void sSVM::mallocLayout() {
	//-- malloc + init neurons ...
}
void sSVM::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("GA training complete.");
}
void sSVM::singleInfer(numtype* singleSampleSBF, numtype* singleTargetSBF, numtype** singlePredictionSBF) {

	//-- 1. load input neurons. Need to MAKE SURE incoming array len is the same as inputcount!!!

	//-- 2. forward pass

	//-- 3. copy last layer neurons (on dev) to prediction (on host)

	//-- 3.1. perfect core!
	for (int i=0; i<layout->outputCnt; i++) (*singlePredictionSBF)[i]=singleTargetSBF[i];

}
void sSVM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSVM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
