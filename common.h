#pragma once

#define _CRT_RAND_S
#include <stdlib.h>

#include <Windows.h>
#include <stdio.h>
#include <stdexcept>

//#define DOUBLE_NUMTYPE
#ifdef DOUBLE_NUMTYPE
typedef double numtype;
typedef float altnumtype;
#else
typedef float numtype;
typedef double altnumtype;
#endif

#include "ConfigMgr/XMLdefs.h"
#define DATE_FORMAT_LEN XMLKEY_PARM_VAL_MAXLEN
#define DATE_FORMAT   "YYYY-MM-DD-HH24:MI"
#define DATE_FORMAT_C "%Y-%m-%d-%H:%M"

#define USE_ORCL
//#define USE_GPU	//-- this was moved to Configuration Manager

#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif

#define Quote(var_) #var_
#define EMPTY_VALUE	-9999.0f

//-- GUI callback
typedef void(__stdcall *NativeReportProgress) (int, void*);
