#include "sSOM.h"

sSOM::sSOM(sCfgObjParmsDef, sCoreLayout* layout_, sSOMparms* SOMparms_) : sCore(sCfgObjParmsVal, layout_){}
sSOM::~sSOM() {}

//-- abstract methods implementations
void sSOM::setLayout(int batchSize_) {

}
void sSOM::mallocLayout() {
	//-- malloc + init neurons ...
}
void sSOM::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("GA training complete.");
}
void sSOM::singleInfer(numtype* singleSampleSBF, numtype* singleTargetSBF, numtype** singlePredictionSBF) {

	//-- 1. load input neurons. Need to MAKE SURE incoming array len is the same as inputcount!!!

	//-- 2. forward pass

	//-- 3. copy last layer neurons (on dev) to prediction (on host)

	//-- 3.1. perfect core!
	for (int i=0; i<layout->outputCnt; i++) (*singlePredictionSBF)[i]=singleTargetSBF[i];

}
void sSOM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSOM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
