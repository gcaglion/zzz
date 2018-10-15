#include "sNNparms.h"

sNNparms::sNNparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	mallocs();

	safecall(cfgKey, getParm, &levelRatio, "Topology/LevelRatio", false, &levelsCnt); levelsCnt++;
	safecall(cfgKey, getParm, &ActivationFunction, "Topology/LevelActivation", false, new int);
	safecall(cfgKey, getParm, &useContext, "Topology/UseContext");
	safecall(cfgKey, getParm, &useBias, "Topology/UseBias");

	safecall(cfgKey, getParm, &MaxEpochs, "Training/MaxEpochs");
	safecall(cfgKey, getParm, &TargetMSE, "Training/TargetMSE");
	safecall(cfgKey, getParm, &NetSaveFreq, "Training/NetSaveFrequency");
	safecall(cfgKey, getParm, &StopOnDivergence, "Training/StopOnDivergence");
	safecall(cfgKey, getParm, &BP_Algo, "Training/BP_Algo");

	switch (BP_Algo) {
	case BP_STD:
		safecall(cfgKey, getParm, &LearningRate, "Training/BP_Std/LearningRate");
		safecall(cfgKey, getParm, &LearningMomentum, "Training/BP_Std/LearningMomentum");
		break;
	case BP_QUICKPROP:
		//--... TO DO ...
		break;
	case BP_RPROP:
		break;
		//--... TO DO ...
	case BP_QING:
		//--... TO DO ...
		break;
	case BP_SCGD: break;
		//--... TO DO ...
	case BP_LM: break;
		//--... TO DO ...
	default:
		fail("invalid BP_Algo: %d", BP_Algo);
	}

}
sNNparms::sNNparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {
	mallocs();
}
sNNparms::~sNNparms() {
	free(levelRatio);
	free(ActivationFunction);
}
