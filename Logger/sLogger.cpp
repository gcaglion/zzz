#include "sLogger.h"

//sLogger::sLogger(sObjParmsDef) : sCfgObj(sObjParmsVal, nullptr, nullptr) {}
sLogger::sLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}
sLogger::~sLogger() {}

void sLogger::open() {
	if (saveToDB) safecall(oradb, open);
	if (saveToFile) safecall(filedb, open, FILE_MODE_WRITE);
	isOpen=true;
}