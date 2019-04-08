#include "sCoreParms.h"

sCoreParms::sCoreParms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_) : sCfgObj(sObjParmsVal, nullptr, "") {}
sCoreParms::sCoreParms(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}
sCoreParms::~sCoreParms() {}
