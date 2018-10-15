#include "sSOMparms.h"

sSOMparms::sSOMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSOMparms::sSOMparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {}
sSOMparms::~sSOMparms() {}
