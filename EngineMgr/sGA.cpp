#include "sGA.h"

sGA::sGA(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sGAparms* GAparms_) : sCore(sObjParmsVal, nullptr, nullptr, Alg_, layout_, persistor_) {
	parms=GAparms_;
}
sGA::sGA(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, Alg_, layout_){}
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
void sGA::infer(sCoreProcArgs* inferArgs) {
	fail("Not implemented.");
}
void sGA::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sGA::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
