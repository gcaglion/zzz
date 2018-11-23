#include "sGA.h"

sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
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
