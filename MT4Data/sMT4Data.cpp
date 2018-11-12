#include "sMT4Data.h"

sMT4Data::sMT4Data(sObjParmsDef, int openMode_, bool autoOpen_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	mallocs();

	if (autoOpen_) open(openMode_);
}
sMT4Data::sMT4Data(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}
sMT4Data::~sMT4Data() {}

void sMT4Data::open(int mode_) {
	fail("not implemented.");
}
void sMT4Data::close() {
	fail("not implemented.");
}
void sMT4Data::getStartDates(sMT4Data* dateSource_, char* startDate_, int datesCnt_, char** oDate_) {}
void sMT4Data::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
	fail("not implemented.");
}
void sMT4Data::saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted) {
	fail("not implemented.");
}
void sMT4Data::commit() {
	fail("not implemented.");
}
void sMT4Data::mallocs() {
}
