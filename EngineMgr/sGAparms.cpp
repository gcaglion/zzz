#include "sGAparms.h"

sGAparms::sGAparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	levelsCnt=1;
}
sGAparms::sGAparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {
	levelsCnt=1;
}
sGAparms::~sGAparms() {}

void sGAparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
