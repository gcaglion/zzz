#include "sSOM.h"

sSOM::sSOM(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sSOMparms* SOMparms_) : sCore(sObjParmsVal, nullptr, nullptr, layout_, persistor_) {
	parms=SOMparms_;
}
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
void sSOM::infer(sCoreProcArgs* inferArgs) {
	fail("Not implemented.");
}
void sSOM::saveImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
void sSOM::loadImage(int pid, int tid, int epoch) {
	fail("Not implemented.");
}
