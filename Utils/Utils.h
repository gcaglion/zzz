#pragma once

#include "../common.h"
#include <time.h>
#include <vector>
using namespace std;

EXPORT void P2V(int Len, numtype* P, std::vector<double> &V);
EXPORT void V2P(int Len, numtype* P, std::vector<double> &V);
EXPORT bool dumpArrayH(int vlen, numtype* v, const char* fname);

EXPORT int MyRndInt(int rmin, int rmax);
EXPORT bool getFullPath(const char* iName, char* oName, char* startPath=nullptr);
EXPORT bool getCurrentPath(char* oPath);
EXPORT int getCharCnt(const char* str, char c);
EXPORT void getCharPos(const char* str, char c, int charCnt, int** oCharPos);
EXPORT int cslToArray(const char* csl, char Separator, char** StrList);
EXPORT int countChar(const char* instr_, char c, int skipFirstN, int skipLastN);
EXPORT void splitFullFileName(const char* iFullName, char* oPath, char* oName);
EXPORT char* substr(char* str, int start, int len);
EXPORT char* right(char* str, int len);
EXPORT char* left(char* str, int len);
EXPORT int  instr(char soughtChar, const char* intoStr, bool fromRight);
EXPORT void replace(char* str, char origc, char newc);
EXPORT int argcnt(const char* mask);
//EXPORT void UpperCase(const char* istr, char* ostr);
EXPORT void removeQuotes(char* istr, char* ostr);
EXPORT void stripChar(char* istr, char c);
EXPORT bool getValuePair(char* istr, char* oName, char* oVal, char eqSign);
EXPORT bool isnumber(char* str);
EXPORT bool isInList(int soughtVal, int listLen, int* list);
EXPORT void gettimestamp(char* timeS, size_t timeSsize);
EXPORT void MT4time2str(long iTime, int iTimeSsize, char* oTimeS);

EXPORT void gotoxy(int x, int y);
EXPORT void cls(HANDLE hConsole);
