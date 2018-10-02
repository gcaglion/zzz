#include "Logger.h"

sLogger::sLogger(sObjParmsDef, int dest_, bool saveNothing_, bool saveClient_, bool saveMSE_, bool saveRun_, bool saveInternals_, bool saveImage_) : sObj(sObjParmsVal) {
	dest=dest_;  saveNothing=saveNothing_; saveClient=saveClient_; saveMSE=saveMSE_; saveRun=saveRun_; saveInternals=saveInternals_; saveImage=saveImage_;
}
sLogger::~sLogger() { }
