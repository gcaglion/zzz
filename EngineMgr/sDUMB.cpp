#include "sDUMB.h"

sDUMB::sDUMB(sCfgObjParmsDef, sCoreLayout* layout_, sDUMBparms* DUMBparms_): sCore(sCfgObjParmsVal, layout_) {
	safecall(cfgKey, getParm, &scalingFactor, "Parameters/ScalingFactor");
}
sDUMB::~sDUMB(){}

//-- abstract methods implementations
void sDUMB::setLayout(int batchSize_) {

}
void sDUMB::mallocLayout() {
	//-- malloc + init neurons ...
}
void sDUMB::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	info("DUMB training complete.");
}
void sDUMB::singleInfer(numtype* singleSampleSBF, numtype* singleTargetSBF, numtype** singlePredictionSBF) {

	//-- 1. load input neurons. Need to MAKE SURE incoming array len is the same as inputcount!!!

	//-- 2. forward pass

	//-- 3. copy last layer neurons (on dev) to prediction (on host)

	//-- 3.1. perfect core!
	for (int i=0; i<layout->outputCnt; i++) (*singlePredictionSBF)[i]=singleTargetSBF[i] * scalingFactor;

}
