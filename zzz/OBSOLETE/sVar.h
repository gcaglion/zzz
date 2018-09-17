#pragma once
#include "../common.h"
#include <stdarg.h>
#include <typeinfo>

#define VarNameMaxLen 256

template<typename T> struct sVar {
	char name[VarNameMaxLen];
	char mask[VarNameMaxLen];
	T* val;

	sVar() {}
	sVar(T* val_, char* nameMask_, ...) {
		//--
		va_list va_args;
		va_start(va_args, nameMask_);
		vsprintf_s(name, VarNameMaxLen, nameMask_, va_args);
		va_end(va_args);
		//--
		val = val_;
	}

	void setName(char* nameMask_, ...) {
		va_list va_args;
		va_start(va_args, nameMask_);
		vsprintf_s(name, VarNameMaxLen, nameMask_, va_args);
		va_end(va_args);
	}
	void setVal(T* val_) {
		//if (typeid(val_).hash_code()==typeid(name).hash_code()) {
		//	strcpy_s(val, VarNameMaxLen, val_);
		//} else {
			val=val_;
		//}
	}

	T* spawn() {
		return val;
	}


};
#define newsvar()
