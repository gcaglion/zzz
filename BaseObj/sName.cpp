#include "sName.h"


sName::sName(const char* mask_, ...) {
	//-- constructor only sets base. 
	va_list va_args;
	va_start(va_args, mask_);
	vsprintf_s(base, ObjNameMaxLen, mask_, va_args);
	va_end(va_args);
}

sName::~sName() {}

void sName::update(int objDepth_, sName* parentSname_) {

	
	//depth += (parentSname_==nullptr) ? 0 : parentSname_->depth;

	if (parentSname_==nullptr) {
		full[0]='\0';
	} else {
		sprintf_s(full, ObjMaxDepth*ObjNameMaxLen, "%s/", parentSname_->full);
	}
	strcat_s(full, ObjMaxDepth*ObjNameMaxLen, base);

}

EXPORT string strBuild(const char* mask_, ...) {
	char buf[ObjNameMaxLen];
	va_list va_args;
	va_start(va_args, mask_);
	vsprintf_s(buf, ObjNameMaxLen, mask_, va_args);
	va_end(va_args);
	std::string ret=buf;
	return ret;
}
