#include "sDUMB.h"

sDUMB::sDUMB(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sDUMBparms* DUMBparms_) : sCore(sObjParmsVal, nullptr, "", layout_, persistor_) {
	parms=DUMBparms_;
}
sDUMB::sDUMB(sCfgObjParmsDef, sCoreLayout* layout_, sDUMBparms* DUMBparms_): sCore(sCfgObjParmsVal, layout_) {
	safecall(cfgKey, getParm, &fixedTRSerror, "Parameters/FixedTRSerror");
}
sDUMB::~sDUMB(){}

//-- abstract methods implementations
void sDUMB::setLayout() {

}
void sDUMB::mallocLayout() {
	//-- malloc + init neurons ...
}
void sDUMB::train() {
	procArgs->mseCnt=10;
	//-- malloc mse[maxepochs], always host-side.
	procArgs->duration=(int*)malloc(procArgs->mseCnt*sizeof(int));
	procArgs->mseT=(numtype*)malloc(procArgs->mseCnt*sizeof(numtype));
	procArgs->mseV=(numtype*)malloc(procArgs->mseCnt*sizeof(numtype));

	for (int e=0; e<procArgs->mseCnt; e++) {
		procArgs->mseT[e]=1;
		procArgs->mseV[e]=0;
	}

	info("DUMB training complete.");
}
void sDUMB::infer() {
	int tlen=procArgs->samplesCnt*procArgs->inputCnt;
	for (int i=0; i<tlen; i++) procArgs->prediction[i]=procArgs->target[i];
}

void sDUMB::saveImage(int pid, int tid, int epoch) {
	safecall(persistor, saveCoreDUMBImage, pid, tid, epoch, 0, (numtype*)nullptr);
}
void sDUMB::loadImage(int pid, int tid, int epoch) {
	safecall(persistor, loadCoreDUMBImage, pid, tid, epoch, 0, (numtype*)nullptr);
}
