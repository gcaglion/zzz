#pragma once

#include "../common.h"

#define info(...) { printf(__VA_ARGS__); printf("\n"); }
#define fail(...) { printf(__VA_ARGS__); printf("\n"); system("pause"); }
#define defaultDbgLevel 1

struct sDbg {
	int level;
	//-- blah...blah...blah...
	sDbg(int level_ /*,...* blah...blah...*/) {
		level=level_;
		//-- blah...blah...blah...
	}
};
#define newdbg(level_) new sDbg(level_)

