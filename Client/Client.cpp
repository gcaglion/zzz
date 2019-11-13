#include "../common.h"
#include "../Forecaster/sRoot.h"
#include "../ConfigMgr/XMLdefs.h"

#define clifail { usage(); return -1;}

void usage() {
	printf("Usage:\n------\n");
	printf("zzz Train <Simulation Id> <Client XML file> <trainDataSet XML file> <Engine XML file> [--parameter=value]\n");
	printf("zzz Infer <Simulation Id> <Client XML file> <inferDataSet XML file> <Saved Engine pid> [--parameter=value]\n");
	system("pause");
}
int main(int argc, char* argv[]) {
	//BOOL f = HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(nullptr);	//-- nullptr says there's no GUI to call back with dbg messages
		//-----------
		root->kaz(); terminate(true, "");
		//-----------

		if (argc<6) clifail;

		//-- override parameters management
		int overridesCnt=argc-6;
		char** overridePname=(char**)malloc(overridesCnt*sizeof(char*));
		char** overridePval=(char**)malloc(overridesCnt*sizeof(char*));
		for (int op=6; op<(6+overridesCnt); op++) {
			overridePname[op-6]=(char*)malloc(XMLKEY_PARM_NAME_MAXLEN);
			overridePval[op-6]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
			if (strlen(argv[op])<5) clifail;
			if (!(argv[op][0]=='-'&&argv[op][1]=='-')) clifail;
			if (!getValuePair(&argv[op][2], overridePname[op-6], overridePval[op-6], '=')) clifail;
		}

		if (_stricmp(argv[1], "Train")==0) {
			//root->trainClient(atoi(argv[2]), argv[3], argv[4], argv[5], nullptr, overridesCnt, overridePname, overridePval);
		} else if (_stricmp(argv[1], "Infer")==0) {
			//root->inferClient(atoi(argv[2]), argv[3], argv[4], atoi(argv[5]), nullptr, overridesCnt, overridePname, overridePval);
		} else { clifail; }
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true, "");

}
