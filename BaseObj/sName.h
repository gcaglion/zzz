#pragma once

#include "../common.h"
#include "../Utils/Utils.h"
#include "ObjDefs.h"
#include <string.h>
using namespace std;

struct sName {
	char base[ObjNameMaxLen];
	char full[ObjMaxDepth*ObjNameMaxLen];

	EXPORT sName(const char* mask_, ...);
	EXPORT ~sName();

	EXPORT void update(int objDepth_, sName* parentSname_);


};
#define newsname(nameMask_, ...) new sName(nameMask_, __VA_ARGS__)


EXPORT string strBuild(const char* mask_, ...);