#include "../common.h"
#include "../Forecaster/sRoot.h"

#define clifail { usage(); return -1;}

void usage() {
	printf("Usage:\n------\n");
	printf("zzz Train <Config path> \n");
	printf("zzz Infer <Config path>  <Saved Engine pid> \n");
	printf("zzz Train <Client XML file> <DataShape XML file> <trainDataSet XML file> <Engine XML file> \n");
	printf("zzz Infer <Client XML file> <DataShape XML file> <inferDataSet XML file> <Engine XML file> <Saved Engine pid> \n");
	system("pause");
}
int main(int argc, char* argv[]) {

	if (argc<3) clifail;

	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(argc, argv);	//-- always takes default debugger settings
		//-----------
//		root->kaz();
//		return -1;
		//-----------
		if (_stricmp(argv[1], "Train")==0) {
			if (argc==3) {
				root->trainClient(argv[2]);
			} else {
				root->trainClient(argv[2], argv[3], argv[4], argv[5]);
			}
		} else if (_stricmp(argv[1], "Infer")==0) {
			if (argc==4) {
				root->inferClient(argv[2], atoi(argv[3]));
			} else {
				if (argc<7) {
					clifail;
				} else {
					root->inferClient(argv[2], argv[3], argv[4], argv[5], atoi(argv[6]));
				}
			}
		} else { clifail; }
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true, "");

}
