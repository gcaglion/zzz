#include "sFile.h"

sFile::sFile(char* path_, char* name_, char* fullName_) {
	if (path==nullptr) {
		if (name_==nullptr && fullName_==nullptr) {}
		if (name_==nullptr && fullName_!=nullptr) {}
		if (name_!=nullptr && fullName_==nullptr) {}
		if (name_!=nullptr && fullName_!=nullptr) {}
	} else {
		if (name_==nullptr && fullName_==nullptr) {}
		if (name_==nullptr && fullName_!=nullptr) {}
		if (name_!=nullptr && fullName_==nullptr) {}
		if (name_!=nullptr && fullName_!=nullptr) {}
	}
}

bool fexist(const char* fname_) {
	FILE* f;
	bool success;

	success = (fopen_s(&f, fname_, "r")==0);
	if (success) fclose(f);

	return success;
}
bool getFullPath(const char* iName, char* oName, char* startPath) {

	char ret[MAX_PATH];

	//-- if input is nullptr, just set output to current path
	if (iName==nullptr) {
		if (!getCurrentPath(oName)) return false;
		return true;
	}
	//-- if input contains drive name (C:, D:, ...), then it must be considered absolute, and nothing else needs to be done
	if (iName[1]==':') {
		strcpy_s(oName, MAX_PATH, iName);
		return true;
	} else {
		//-- otherwise, start from current path
		if (startPath==nullptr) {
			if (!getCurrentPath(ret)) return false;
		} else {
			strcpy_s(ret, MAX_PATH, startPath);
		}
	}

	//-- '../' , '..\'
	if (iName[0]=='.' && iName[1]=='.'&&(iName[2]=='\\'||iName[2]=='/')) {
		ret[max(instr('/', ret, true), instr('\\', ret, true))]='\0';
		return (getFullPath(&iName[3], oName, ret));
	}
	//-- './'
	if (iName[0]=='.'&&(iName[1]=='/'||iName[1]=='\\')) {
		return (getFullPath(&iName[2], oName));
	}
	//-- '/'
	if (iName[0]=='/'||iName[0]=='\\') {
		sprintf_s(oName, MAX_PATH, "%c%c/%s", ret[0], ret[1], iName);
		return true;
	}
	//-- no modifiers. just append iName to current path
	sprintf_s(oName, MAX_PATH, "%s/%s", ret, iName);
	return true;
}
bool getCurrentPath(char* oPath) {
	TCHAR Buffer[MAX_PATH];
	char  RetBuf[MAX_PATH];
	DWORD dwRet;
	size_t convcharsn;

	dwRet = GetCurrentDirectory(MAX_PATH, Buffer);
	if (dwRet==0) {
		printf("GetCurrentDirectory failed (%d)\n", GetLastError());
		return false;
	}
	wcstombs_s(&convcharsn, RetBuf, Buffer, MAX_PATH-1);
	strcpy_s(oPath, MAX_PATH, RetBuf);
	return true;
}
