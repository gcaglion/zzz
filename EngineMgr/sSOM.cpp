#include "sSOM.h"

sSOM::sSOM(sObjParmsDef, int inputCnt_, int outputCnt_, sCoreLogger* persistor_, sSOMparms* SOMparms_) : sCore(sObjParmsVal, nullptr, nullptr, inputCnt_, outputCnt_, persistor_) {
	parms=SOMparms_;
}
sSOM::sSOM(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sSOMparms* SOMparms_) : sCore(sCfgObjParmsVal, inputCnt_, outputCnt_){}
sSOM::~sSOM() {}

//-- abstract methods implementations
void sSOM::setLayout() {

}
void sSOM::mallocLayout() {
	//-- malloc + init neurons ...
}
void sSOM::train() {
	procArgs->mseCnt=10;
	info("GA training complete.");
}
void sSOM::infer() {
	fail("Not implemented.");
}
void sSOM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSOM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
