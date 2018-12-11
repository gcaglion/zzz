#include "../common.h"
#include "../Forecaster/sRoot.h"

#define clifail { usage(); return -1;}

void usage() {
	printf("Usage:\n------\n");
	printf("zzz Train <Simulation Id> <Client XML file> <DataShape XML file> <trainDataSet XML file> <Engine XML file> \n");
	printf("zzz Infer <Simulation Id> <Client XML file> <DataShape XML file> <inferDataSet XML file> <Engine XML file> <Saved Engine pid> \n");
	printf("zzz Both  <Simulation Id> <Client XML file> <DataShape XML file> <inferDataSet XML file> <Engine XML file> <Saved Engine pid> \n");
	system("pause");
}
int main(int argc, char* argv[]) {

	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(nullptr);	//-- nullptr says there's no GUI to call back with dbg messages
		//-----------
		//root->kaz(); return -1;
		//-----------

		if (argc<3) clifail;

		if (_stricmp(argv[1], "Train")==0) {
			if (argc!=7) { clifail; } else {
				root->trainClient(atoi(argv[2]), argv[3], argv[4], argv[5], argv[6]);
			}

		} else if (_stricmp(argv[1], "Infer")==0) {
			if (argc!=8) { clifail; } else {
				root->inferClient(atoi(argv[2]), argv[3], argv[4], argv[5], argv[6], atoi(argv[7]));
			}

		} else if (_stricmp(argv[1], "Both")==0) {
			if (argc!=7) { clifail; } else {
				root->bothClient(atoi(argv[2]), argv[3], argv[4], argv[5], argv[6]);
			}
		} else { clifail; }
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true, "");

}
