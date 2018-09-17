#pragma once

#include <Windows.h>
#include <stdio.h>

typedef float numtype;
#define DATE_FORMAT "YYYYMMDDHHMI"
#define DATE_FORMAT_LEN 12

#define USE_ORCL
#define USE_GPU

#ifdef __cplusplus
#define Bool bool
#else
typedef int Bool;
#define true 1
#define false 0
#define nullptr 0
#endif

#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif

#define Quote(var_) #var_
