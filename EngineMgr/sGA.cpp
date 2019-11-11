#include "sGA.h"

sGA::sGA(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sGAparms* GAparms_) : sCore(sObjParmsVal, nullptr, nullptr, layout_, persistor_) {
	parms=GAparms_;
}
sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
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
