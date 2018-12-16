#include "sSOM.h"

sSOM::sSOM(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sSOMparms* SOMparms_) : sCore(sObjParmsVal, nullptr, nullptr, Alg_, layout_, persistor_) {
	parms=SOMparms_;
}
sSOM::sSOM(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sSOMparms* SOMparms_) : sCore(sCfgObjParmsVal, Alg_, layout_){}
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
void sSOM::infer(sCoreProcArgs* inferArgs) {
	fail("Not implemented.");
}
void sSOM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSOM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
