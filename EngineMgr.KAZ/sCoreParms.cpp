#include "sCoreParms.h"

sCoreParms::sCoreParms(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}
sCoreParms::~sCoreParms() {
	free(scaleMin); 
	free(scaleMax);
}
