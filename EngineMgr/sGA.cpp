#include "sGA.h"

sGA::sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_): sCore(sCfgObjParmsVal, layout_){}
sGA::~sGA(){}

void sGA::train(sCoreProcArgs* trainArgs) {}
void sGA::infer(sCoreProcArgs* inferArgs) {}
