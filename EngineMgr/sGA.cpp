#include "sGA.h"

sGA::sGA(sObjParmsDef, int inputCnt_, int outputCnt_, sCoreLogger* persistor_, sGAparms* GAparms_) : sCore(sObjParmsVal, nullptr, nullptr, inputCnt_, outputCnt_, persistor_) {
	parms=GAparms_;
}
sGA::sGA(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, inputCnt_, outputCnt_){}
sGA::~sGA(){}

//-- abstract methods implementations
void sGA::setLayout() {

}
void sGA::mallocLayout() {
	//-- malloc + init neurons ...
}
void sGA::train() {
	procArgs->mseCnt=10;
	info("GA training complete.");
}
void sGA::infer() {
	fail("Not implemented.");
}
void sGA::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sGA::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
