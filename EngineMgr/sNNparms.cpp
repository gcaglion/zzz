#include "sNNparms.h"

sNNparms::sNNparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	mallocs();
	int levelActsCnt;
	safecall(cfgKey, getParm, &levelRatio, "Topology/LevelRatio", false, &levelsCnt); levelsCnt+=2;
	safecall(cfgKey, getParm, &ActivationFunction, "Topology/LevelActivation", false, &levelActsCnt);
	if (levelActsCnt!=levelsCnt) fail("Too few Level Activations specified (%d vs. %d required)", levelActsCnt, levelsCnt);
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

void sNNparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		switch (ActivationFunction[l]) {
		case NN_ACTIVATION_TANH:
			scaleMin[l] = -1;
			scaleMax[l] = 1;
			break;
		case NN_ACTIVATION_EXP4:
			scaleMin[l] = 0;
			scaleMax[l] = 1;
			break;
		case NN_ACTIVATION_RELU:
			scaleMin[l] = 0;
			scaleMax[l] = 1;
			break;
		case NN_ACTIVATION_SOFTPLUS:
			scaleMin[l] = 0;
			scaleMax[l] = 1;
			break;
		default:
			scaleMin[l] = -1;
			scaleMax[l] = 1;
			break;
		}
	}
}
