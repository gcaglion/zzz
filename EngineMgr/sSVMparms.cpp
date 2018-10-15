#include "sSVMparms.h"

sSVMparms::sSVMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSVMparms::sSVMparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {}
sSVMparms::~sSVMparms(){}
