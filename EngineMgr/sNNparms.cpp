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
sNNparms::sNNparms(sObjParmsDef, sLogger* persistor_, int loadingPid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_) {
	mallocs();
}
sNNparms::~sNNparms() {
	free(levelRatio);
	free(ActivationFunction);
}

//-- local implementations of virtual functions defined in sCoreParms
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
void sNNparms::save(sLogger* persistor_, int pid_, int tid_) {
	safecall(persistor_, saveCoreNNparms, pid_, tid_, levelRatioS, levelActivationS, useContext, useBias, MaxEpochs, TargetMSE, NetSaveFreq, StopOnDivergence, BP_Algo, LearningRate, LearningMomentum);
}
void sNNparms::load(sLogger* persistor_, int pid_, int tid_) {}

//-- private stuff
void sNNparms::mallocs() {
	levelRatio=(float*)malloc((CORE_MAX_INTERNAL_LEVELS-2)*sizeof(float));
	ActivationFunction=(int*)malloc(CORE_MAX_INTERNAL_LEVELS*sizeof(int));
	levelRatioS[0]='\0'; levelActivationS[0]='\0';
}
