#include "../common.h"
#include "../Forecaster/sRoot.h"

#define clifail { usage(); return -1;}

void usage() {
	printf("Usage:\n------\n");
	printf("zzz Train <Simulation Id> <Client XML file> <trainDataSet XML file> <Engine XML file> \n");
	printf("zzz Infer <Simulation Id> <Client XML file> <inferDataSet XML file> <Saved Engine pid> \n");
	system("pause");
}
int main(int argc, char* argv[]) {
	//BOOL f = HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(nullptr);	//-- nullptr says there's no GUI to call back with dbg messages
		//-----------
		//root->kaz(); terminate(true, "");
		//-----------

		if (argc<6) clifail;

		if (_stricmp(argv[1], "Train")==0) {
			root->trainClient(atoi(argv[2]), argv[3], argv[4], argv[5], nullptr);
		} else if (_stricmp(argv[1], "Infer")==0) {
			root->inferClient(atoi(argv[2]), argv[3], argv[4], atoi(argv[5]), nullptr);
		} else { clifail; }
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true, "");

}
