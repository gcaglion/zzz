#include "sGAparms.h"

sGAparms::sGAparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	levelsCnt=1;
}
sGAparms::sGAparms(sObjParmsDef, sLogger* persistor_, int loadingPid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_) {
	levelsCnt=1;
}
sGAparms::~sGAparms() {}

//-- local implementations of virtual functions defined in sCoreParms
void sGAparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sGAparms::save(int pid, int tid) {}
void sGAparms::load(int pid, int tid) {}
