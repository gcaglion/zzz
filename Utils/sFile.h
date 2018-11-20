#pragma once
#include "../common.h"
#include "Utils.h"

struct sFile {
	char path[MAX_PATH];
	char name[MAX_PATH];
	char fullName[MAX_PATH];

	sFile(char* path_, char* name_, char* fullName_);

};
bool fexist(const char* fname_);
bool getFullPath(const char* iName, char* oName, char* startPath);
bool getCurrentPath(char* oPath);
