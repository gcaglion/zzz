#include "sDUMB.h"

sDUMB::sDUMB(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sDUMBparms* DUMBparms_) : sCore(sObjParmsVal, nullptr, nullptr, Alg_, layout_, persistor_) {
	parms=DUMBparms_;
}
sDUMB::sDUMB(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sDUMBparms* DUMBparms_): sCore(sCfgObjParmsVal, Alg_, layout_) {
	safecall(cfgKey, getParm, &fixedTRSerror, "Parameters/FixedTRSerror");
}
sDUMB::~sDUMB(){}

//-- abstract methods implementations
void sDUMB::setLayout(int batchSize_) {

}
void sDUMB::mallocLayout() {
	//-- malloc + init neurons ...
}
void sDUMB::train(sCoreProcArgs* trainArgs) {
	trainArgs->mseCnt=10;
	//-- malloc mse[maxepochs], always host-side.
	trainArgs->duration=(int*)malloc(trainArgs->mseCnt*sizeof(int));
	trainArgs->mseT=(numtype*)malloc(trainArgs->mseCnt*sizeof(numtype));
	trainArgs->mseV=(numtype*)malloc(trainArgs->mseCnt*sizeof(numtype));

	for (int e=0; e<trainArgs->mseCnt; e++) {
		trainArgs->mseT[e]=1;
		trainArgs->mseV[e]=0;
	}

	info("DUMB training complete.");
}
void sDUMB::infer(sCoreProcArgs* inferArgs) {
	int tlen=inferArgs->ds->samplesCnt*inferArgs->ds->targetLen*inferArgs->ds->featuresCnt;
	for (int i=0; i<tlen; i++) inferArgs->ds->predictionSBF[i]=inferArgs->ds->targetSBF[i];
}

void sDUMB::saveImage(int pid, int tid, int epoch) {
	safecall(persistor, saveCoreDUMBImage, pid, tid, epoch, 0, (numtype*)nullptr);
}
void sDUMB::loadImage(int pid, int tid, int epoch) {
	safecall(persistor, loadCoreDUMBImage, pid, tid, epoch, 0, (numtype*)nullptr);
}
