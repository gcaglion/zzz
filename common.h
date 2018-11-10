#pragma once

#define _CRT_RAND_S
#include <stdlib.h>

#include <Windows.h>
#include <stdio.h>
#include <stdexcept>

typedef float numtype;
typedef double altnumtype;

#define DATE_FORMAT "YYYYMMDDHHMI"
#define DATE_FORMAT_LEN 12+1

#define USE_ORCL
#define USE_GPU

#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif

#define Quote(var_) #var_
#define EMPTY_VALUE	-9999.0f