#include "sGAparms.h"

sGAparms::sGAparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sGAparms::sGAparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {}
sGAparms::~sGAparms() {}
