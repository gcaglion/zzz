#pragma once

#include "../common.h"
#include "sName.h"
#include "svard.h"
#include "sDbg.h"
#include <typeinfo>

#define CmdMaxLen	4096
#define sObjParmsDef sObj* parent_, sName* sname_, sDbg* dbg_
#define sObjParmsVal parent_, sname_, dbg_

struct sObj {
	sName* name;
	//--
	sDbg* dbg;
	int depth;
	sObj* parent;
	int childrenCnt;
	sObj* child[ObjMaxChildren];
	//--
	svard* cmdSvard;
	char cmd[CmdMaxLen];

	EXPORT sObj(sObjParmsDef);
	EXPORT ~sObj();

	EXPORT void findChild(const char* relChildName, sObj** retObj);

	template<typename objT, class ...constructorArgs> EXPORT bool _spawn(const char* callerFunc_, objT** childVar_, sName* childSname_, sDbg* childDbg_, bool ignoreError_, constructorArgs... childCargs) {
		bool ret;
		//-- build command svard
		cmdSvard=new svard(childCargs...);
		sprintf_s(cmd, CmdMaxLen, "%s = new %s(%s)", childSname_->base, typeid(objT).name(), cmdSvard->fullval);

		try {
			info("%s TRYING  : %s", name->base, cmd);
				(*childVar_) = new objT(this, childSname_, childDbg_, childCargs...);
				ret=true;
			info("%s SUCCESS : %s", name->base, cmd);
		}
		catch (std::exception exc) {
			ret=false;
			if (ignoreError_) {
				info("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what());
			} else {
				fail("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what());
			}
		}
		return ret;
	}

};

#define safespawn(ignoreError_, childVar_, childSname_, childDbg_, ...) _spawn(__func__, &childVar_, childSname_, childDbg_, ignoreError_, __VA_ARGS__)
#define safecall(obj_, met_, ...){ \
	cmdSvard=new svard(__VA_ARGS__); \
	sprintf_s(cmd, CmdMaxLen, "%s->%s(%s)", obj_->name->base, Quote(met_), cmdSvard->fullval); \
	try{ \
		info("%s TRYING  : %s", name->base, cmd); \
		obj_->met_(__VA_ARGS__); \
		info("%s SUCCESS : %s", name->base, cmd); \
	} catch (std::exception exc) { \
		fail("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what()); \
	}\
}
