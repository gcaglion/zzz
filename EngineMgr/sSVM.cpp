#include "sSVM.h"

sSVM::sSVM(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sSVMparms* SVMparms_) : sCore(sObjParmsVal, nullptr, nullptr, Alg_, layout_, persistor_) {
	parms=SVMparms_;
}
sSVM::sSVM(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sSVMparms* SVMparms_) : sCore(sCfgObjParmsVal, Alg_, layout_) {}
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
void sSVM::infer(sCoreProcArgs* inferArgs) {
	fail("Not implemented.");
}
void sSVM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSVM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
