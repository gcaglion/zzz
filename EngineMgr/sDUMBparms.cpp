#include "sDUMBparms.h"

sDUMBparms::sDUMBparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	levelsCnt=1;
}
sDUMBparms::sDUMBparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {
	levelsCnt=1;
}
sDUMBparms::~sDUMBparms() {}

void sDUMBparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
