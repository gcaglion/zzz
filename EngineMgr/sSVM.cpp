#include "sSVM.h"

sSVM::sSVM(sObjParmsDef, int inputCnt_, int outputCnt_, sCoreLogger* persistor_, sSVMparms* SVMparms_) : sCore(sObjParmsVal, nullptr, nullptr, inputCnt_, outputCnt_, persistor_) {
	parms=SVMparms_;
}
sSVM::sSVM(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sSVMparms* SVMparms_) : sCore(sCfgObjParmsVal, inputCnt_, outputCnt_) {}
sSVM::~sSVM() {}

//-- abstract methods implementations
void sSVM::setLayout() {

}
void sSVM::mallocLayout() {
	//-- malloc + init neurons ...
}
void sSVM::train() {
	procArgs->mseCnt=10;
	info("GA training complete.");
}
void sSVM::infer() {
	fail("Not implemented.");
}
void sSVM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSVM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
