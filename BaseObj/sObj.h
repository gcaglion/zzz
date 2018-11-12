#pragma once

#include "../common.h"
#include "sName.h"
#include "svard.h"
#include "sDbg.h"
#include <typeinfo>
#include <vector>
using namespace std;

#define CmdMaxLen	4096
#define sObjParmsDef sObj* parent_, sName* sname_, sDbg* dbg_
#define sObjParmsVal parent_, sname_, dbg_

struct sObj {
	sName* name;
	sDbg* dbg;
	int depth;
	sObj* parent;
	vector<sObj*> child;
	//--
	svard* cmdSvard;
	char cmd[CmdMaxLen];

	EXPORT sObj(sObjParmsDef);
	EXPORT virtual ~sObj();

	EXPORT void findChild(const char* relChildName, sObj** retObj);

	template<typename objT, class ...constructorArgs> EXPORT void _spawn(const char* callerFunc_, objT** childVar_, sName* childSname_, sDbg* childDbg_, constructorArgs... childCargs) {

		//-- build command svard
		cmdSvard=new svard(childCargs...);
		sprintf_s(cmd, CmdMaxLen, "%s = new %s(%s)", childSname_->base, typeid(objT).name(), cmdSvard->fullval);

		try {
			info("%s TRYING  : %s", name->base, cmd);
			(*childVar_) = new objT(this, childSname_, childDbg_, childCargs...);
			info("%s SUCCESS : %s", name->base, cmd);
		}
		catch (std::exception exc) {
			fail("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what());
		}
	}
};

#define safespawn(childVar_, childSname_, childDbg_, ...) _spawn(__func__, &childVar_, childSname_, childDbg_, __VA_ARGS__)
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

//-- the following call a bool function not belonging to an sObj object
#define safecallB(bfunc_, ...) { \
	cmdSvard=new svard(__VA_ARGS__); \
	sprintf_s(cmd, CmdMaxLen, "%s(%s)", #bfunc_, cmdSvard->fullval); \
	info("%s TRYING  : %s", name->base, cmd); \
	if((bfunc_(__VA_ARGS__))) { \
		info("%s SUCCESS : %s", name->base, cmd); \
	} else { \
		fail("%s FAILURE : %s", name->base, cmd); \
	} \
}

#define silentcallB(bfunc_, ...) { \
	cmdSvard=new svard(__VA_ARGS__); \
	sprintf_s(cmd, CmdMaxLen, "%s(%s)", #bfunc_, cmdSvard->fullval); \
	if(!(bfunc_(__VA_ARGS__))) { \
		fail("%s FAILURE : %s", name->base, cmd); \
	} \
}



