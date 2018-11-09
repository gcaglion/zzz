#include "sSVM.h"

sSVM::sSVM(sCfgObjParmsDef, sCoreLayout* layout_, sSVMparms* SVMparms_) : sCore(sCfgObjParmsVal, layout_) {}
sSVM::~sSVM() {}

void sSVM::train(sCoreProcArgs* trainArgs) {}
void sSVM::infer(sCoreProcArgs* inferArgs) {}
