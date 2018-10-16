#include "sSOMparms.h"

sSOMparms::sSOMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSOMparms::sSOMparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {}
sSOMparms::~sSOMparms() {}

void sSOMparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
