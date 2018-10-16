#include "sGAparms.h"

sGAparms::sGAparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sGAparms::sGAparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {}
sGAparms::~sGAparms() {}

void sGAparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
