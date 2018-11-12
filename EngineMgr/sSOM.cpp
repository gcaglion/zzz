#include "sSOM.h"

sSOM::sSOM(sCfgObjParmsDef, sCoreLayout* layout_, sSOMparms* SOMparms_) : sCore(sCfgObjParmsVal, layout_){}
sSOM::~sSOM() {}

void sSOM::train(sCoreProcArgs* trainArgs) {}
void sSOM::infer(sCoreProcArgs* inferArgs) {}
