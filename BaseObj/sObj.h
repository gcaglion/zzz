#pragma once

#include "../common.h"
#include "sName.h"
#include "svard.h"
#include "timer.h"
#include "sDbg.h"
#include <typeinfo>
#include <vector>
using namespace std;


#define sObjParmsDef sObj* parent_, sName* sname_, sDbg* dbg_, NativeReportProgress* GUIreporter_
#define sObjParmsVal parent_, sname_, dbg_, GUIreporter_

struct sObj {
	sName* name;
	sDbg* dbg;
	//--
	NativeReportProgress* GUIreporter;	//-- this is set in Root constructor, and is simply copied by every sObj to its children on spawn

	int depth;
	sObj* parent;
	vector<sObj*> child;
	svard* cmdSvard=nullptr;
	sTimer* timer=new sTimer();
	char cmd[CmdMaxLen];
	char cmdElapsed[TIMER_ELAPSED_FORMAT_LEN];

	EXPORT sObj(sObjParmsDef);
	EXPORT virtual ~sObj();

	EXPORT void findChild(const char* relChildName, sObj** retObj);

	template<typename objT, class ...constructorArgs> EXPORT void _spawn(const char* callerFunc_, objT** childVar_, sName* childSname_, sDbg* childDbg_, constructorArgs... childCargs) {

		//-- build command svard
		if (cmdSvard!=nullptr) delete cmdSvard;
		cmdSvard=new svard(childCargs...);
		sprintf_s(cmd, CmdMaxLen, "%s = new %s(%s)", childSname_->base, typeid(objT).name(), cmdSvard->fullval);

		try {
			if (dbg->timing) {
				timer->start();
				info("[%s] %s TRYING  : %s", timer->startTimeS, name->base, cmd);
			} else {
				info("%s TRYING  : %s", name->base, cmd);
			}
			(*childVar_) = new objT(this, childSname_, childDbg_, GUIreporter, childCargs...);
			if (dbg->timing) {
				timer->stop(cmdElapsed);
				info("[%s] %s SUCCESS : %s . Elapsed time: %s", timer->stopTimeS, name->base, cmd, cmdElapsed);
			} else {
				info("%s SUCCESS : %s", name->base, cmd);
			}
		}
		catch (std::exception exc) {
			if (dbg->timing) {
				fail("[%s] %s FAILURE : %s . Exception: %s", timer->startTimeS, name->base, cmd, exc.what());
			} else {
				fail("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what());
			}
		}
	}
};

#define safespawn(childVar_, childSname_, childDbg_, ...) _spawn(__func__, &childVar_, childSname_, childDbg_, __VA_ARGS__);
#define safecall(obj_, met_, ...){ \
	cmdSvard=new svard(__VA_ARGS__); \
	sprintf_s(cmd, CmdMaxLen, "%s->%s(%s)", obj_->name->base, Quote(met_), cmdSvard->fullval); \
	try{ \
		if (dbg->timing) { \
			timer->start(); \
			info("[%s] %s TRYING  : %s", timer->startTimeS, name->base, cmd); \
		} else { \
			info("%s TRYING  : %s", name->base, cmd); \
		} \
		obj_->met_(__VA_ARGS__); \
			if (dbg->timing) { \
				timer->stop(cmdElapsed); \
				info("[%s] %s SUCCESS : %s . Elapsed time: %s", timer->stopTimeS, name->base, cmd, cmdElapsed); \
			} else { \
				info("%s SUCCESS : %s", name->base, cmd); \
			} \
	} catch (std::exception exc) { \
		if (dbg->timing) { \
			fail("[%s] %s FAILURE : %s . Exception: %s", timer->startTimeS, name->base, cmd, exc.what()); \
		} else { \
			fail("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what()); \
		} \
	}\
}

/*
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
*/
#define safecallSilent(obj_, met_, ...){ \
	try{ \
		obj_->met_(__VA_ARGS__); \
	} catch (std::exception exc) { \
		fail("%s->%s() FAILURE at line %d: Exception: %s", name->base, __func__, __LINE__, exc.what()); \
	}\
}

/*
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
*/


