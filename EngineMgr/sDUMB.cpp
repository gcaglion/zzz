#include "sDUMB.h"

sDUMB::sDUMB(sCfgObjParmsDef, sCoreLayout* layout_, sDUMBparms* DUMBparms_): sCore(sCfgObjParmsVal, layout_) {
	safecall(cfgKey, getParm, &fixedTRSerror, "Parameters/FixedTRSerror");
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
void sDUMB::singleInfer(int sampleLen_, int sampleFeaturesCnt_, int batchSamplesCnt_, numtype* singleSampleBF, numtype* singleTargetBF, numtype** singlePredictionBF) {

	//-- 1. load input neurons. Need to MAKE SURE incoming array len is the same as inputcount!!!

	//-- 2. forward pass

	//-- 3. copy last layer neurons (on dev) to prediction (on host)

	//-- 3.1. perfect core!
	for (int i=0; i<layout->outputCnt; i++) (*singlePredictionBF)[i]=singleTargetBF[i] +fixedTRSerror;

}
void sDUMB::saveImage(int pid, int tid, int epoch) {
	safecall(persistor, saveCoreDUMBImage, pid, tid, epoch, 0, (numtype*)nullptr);
}
void sDUMB::loadImage(int pid, int tid, int epoch) {
	safecall(persistor, loadCoreDUMBImage, pid, tid, epoch, 0, (numtype*)nullptr);
}
