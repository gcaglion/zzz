#include "Utils.h"

bool fexist(const char* fname_) {
	FILE* f;
	bool success;
	
	success = (fopen_s(&f, fname_, "r") ==0);
	if(success) fclose(f);

	return success;
}

EXPORT bool getFullFileName(const char* iName, char* oName, const char* currPath_) {
	
	char currPath[MAX_PATH];
	//char fname[MAX_PATH];

	//-- 1. get current path
	if (currPath_==nullptr) {
		if (!getCurrentPath(currPath)) return false;
	} else {
		strcpy_s(currPath, MAX_PATH, currPath_);
	}

	//-- 2. check for path modifiers in name
	int plen;
	if (iName[0]=='.' && iName[1]=='.') {
		if (iName[2]=='\\'||iName[2]=='/') {
			plen=max(instr('/', currPath, true), instr('\\', currPath, true));
			currPath[plen]='\0';
			return (getFullFileName(&iName[3], oName, currPath));
		}
	}

	//-- 3. try opening iName with no path
	strcpy_s(oName, MAX_PATH, iName);
	if (fexist(oName)) return true;

	//-- 4. try opening iName in currPath
	sprintf_s(oName, MAX_PATH, "%s\\%s", currPath, iName);
	return (fexist(oName));

}

EXPORT char* MyGetCurrentDirectory() {
	TCHAR Buffer[MAX_PATH];
	char  RetBuf[MAX_PATH];
	DWORD dwRet;
	size_t convcharsn;

	dwRet = GetCurrentDirectory(MAX_PATH, Buffer);
	if (dwRet==0) {
		printf("GetCurrentDirectory failed (%d)\n", GetLastError());
	}
	wcstombs_s(&convcharsn, RetBuf, Buffer, MAX_PATH-1);
	return &RetBuf[0];
}
EXPORT bool getCurrentPath(char* oPath) {
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
/*EXPORT void UpperCase(const char* istr, char* ostr) {
	int pos=0;
	while (istr[pos]!='\0') {
		ostr[pos]=toupper(istr[pos]);
		pos++;
	}
	ostr[pos]='\0';
}
*/
EXPORT int countChar(const char* instr_, char c, int skipFirstN, int skipLastN) {
	int n=0;
	for (int i=skipFirstN; i<(strlen(instr_)-skipLastN); i++) {
		if (instr_[i]==c) n++;
	}
	return n;
}
EXPORT int getCharCnt(const char* str, char c) {
	int cnt=0;
	for (int i=0; i<strlen(str); i++) {
		if (str[i]==c) cnt++;
	}
	return cnt;
}
EXPORT void getCharPos(const char* str, char c, int charCnt, int** oCharPos) {
	int cnt=0;
	for (int i=0; i<strlen(str); i++) {
		if (str[i]==c) (*oCharPos[cnt])=i;
	}
}
EXPORT int cslToArray(const char* csl, char Separator, char** StrList) {
	//-- 1. Put a <separator>-separated list of string values into an array of strings, and returns list length
	int i = 0;
	int prevSep = 0;
	int ListLen = 0;
	int kaz=0;

	while (csl[i]!='\0') {
		kaz = (prevSep==0) ? 0 : 1;
		if (csl[i]==Separator) {
			// separator
			memcpy(StrList[ListLen], &csl[prevSep+kaz], i-prevSep-kaz);
			StrList[ListLen][i-prevSep-kaz] = '\0';	// add null terminator
			stripChar(StrList[ListLen], ' '); stripChar(StrList[ListLen], '\t');
			if(strlen(StrList[ListLen])>0) ListLen++;
			prevSep = i;
		}
		i++;
	}
	//-- portion of pDesc after the last comma
	memcpy(StrList[ListLen], &csl[prevSep+kaz], i-prevSep-kaz);
	StrList[ListLen][i-prevSep-kaz] = '\0';	// add null terminator
	stripChar(StrList[ListLen], ' '); stripChar(StrList[ListLen], '\t');
	if (strlen(StrList[ListLen])==0) ListLen--;

	return (ListLen+1);
}
EXPORT void splitFullFileName(char* iFullName, char* oPath, char* oName) {
	int i;
	int ls=instr('\\', iFullName, true);
	if (ls<0) ls=instr('/', iFullName, true);

	oPath[0]='\0';
	for (i=0; i<ls; i++) oPath[i]=iFullName[i];
	oPath[i]='\0';

	oName[0]='\0';
	for (i=0; i<(strlen(iFullName)-ls); i++) oName[i]=iFullName[ls+i+1];
	oName[i]='\0';
}
EXPORT char* substr(char* str, int start, int len) {
	char ret[1000];
	memcpy(ret, &str[start], len);
	ret[len] = '\0';
	return &ret[0];
}
EXPORT char* right(char* str, int len) {
	return(substr(str, (int)strlen(str)-len, len));
}
EXPORT char* left(char* str, int len) {
	return(substr(str, 0, len));
}
EXPORT int  instr(char soughtChar, const char* intoStr, bool fromRight) {
	int i;
	if (fromRight) {
		for (i=(int)strlen(intoStr)-1; i>=0; i--) {
			if (intoStr[i]==soughtChar) return i;
		}
	} else {
		for (i=0; i<strlen(intoStr); i++) {
			if (intoStr[i]==soughtChar) return i;
		}
	}
	return -1;
}
EXPORT int argcnt(const char* mask) {
	int cnt=0;
	for (int i=0; i<strlen(mask); i++) {
		if (mask[i]==37) cnt++;
	}
	return cnt;
}
EXPORT void removeQuotes(char* istr, char* ostr) {
	size_t slen=strlen(istr);
	size_t rlen=slen;
	int ri=0;
	for (int si=0; si<slen; si++) {
		if (istr[si]!=34) {
			ostr[ri]=istr[si];
			ri++;
		}
	}
	ostr[ri]='\0';
}
EXPORT void stripChar(char* istr, char c) {
	size_t ilen=strlen(istr);
	char* ostr=(char*)malloc(ilen+1);
	int ri=0;
	for (int si=0; si<ilen; si++) {
		if (istr[si]!=c) {
			ostr[ri]=istr[si];
			ri++;
		}
	}
	ostr[ri]='\0';
	memcpy_s(istr, ri, ostr, ri);
	istr[ri]='\0';
	free(ostr);
}
EXPORT bool getValuePair(char* istr, char* oName, char* oVal, char eqSign) {
	int eqpos=instr(eqSign, istr, false); if (eqpos<0) return false;
	memcpy_s(oName, eqpos, istr, eqpos);
	oName[eqpos]='\0';
	memcpy_s(oVal, strlen(istr)-eqpos+1, &istr[eqpos+1], strlen(istr)-eqpos);
	oVal[strlen(istr)-eqpos]='\0';
	return true;
}
EXPORT bool isnumber(char* str) {
	for (int i=0; i<strlen(str); i++) {
		if (str[i]>'9'||str[i]<'0') return false;
	}
	return true;
}
EXPORT bool isInList(int soughtVal, int listLen, int* list) {
	for(int i=0; i<listLen; i++) {
		if (list[i]==soughtVal) return true;
	}
	return false;
}
EXPORT void gettimestamp(char* timeS, size_t timeSsize) {
	time_t ltime; // calendar time 
	HANDLE TimeMutex;

	TimeMutex=CreateMutex(NULL, TRUE, NULL);
	WaitForSingleObject(TimeMutex, INFINITE);
	ltime=time(NULL); // get current cal time 
	tm* ltm=new tm();
	localtime_s(ltm, &ltime);
	sprintf_s(timeS, 18, "%d%02d%02d-%02d:%02d:%02d", 1900+ltm->tm_year, ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	ReleaseMutex(TimeMutex);
}

EXPORT void gotoxy(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
