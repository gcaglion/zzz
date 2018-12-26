#include "sNNparms.h"

void arr2csl(int vlen, float* elem, int cslLen, char** oCSL) {
	char elemS[XMLKEY_PARM_VAL_MAXLEN];

	if (vlen>0)	sprintf_s((*oCSL), cslLen, "%f,", elem[0]);
	if (vlen>1) {
		for (int i=1; i<vlen; i++) {
			sprintf_s(elemS, XMLKEY_PARM_VAL_MAXLEN, "%f,", elem[i]);
			strcat_s((*oCSL), cslLen, elemS);
		}
	}
	if (vlen>0) (*oCSL)[strlen((*oCSL))-2]='\0';
}
void arr2csl(int vlen, int* elem, int cslLen, char** oCSL) {
	char elemS[XMLKEY_PARM_VAL_MAXLEN];

	if (vlen>0)	sprintf_s((*oCSL), cslLen, "%d,", elem[0]);
	if (vlen>1) {
		for (int i=1; i<vlen; i++) {
			sprintf_s(elemS, XMLKEY_PARM_VAL_MAXLEN, "%d,", elem[i]);
			strcat_s((*oCSL), cslLen, elemS);
		}
	}
	if (vlen>0) (*oCSL)[strlen((*oCSL))-1]='\0';
}

sNNparms::sNNparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	mallocs();
	int levelActsCnt;
	safecall(cfgKey, getParm, &levelRatio, "Topology/LevelRatio", false, &levelsCnt); levelsCnt+=2;
	safecall(cfgKey, getParm, &ActivationFunction, "Topology/LevelActivation", false, &levelActsCnt);
	//-- need to rebuild array parameters original strings, to be saved later by saveCoreNNparms()
	arr2csl(levelsCnt-2, levelRatio, XMLKEY_PARM_VAL_MAXLEN, &levelRatioS);
	arr2csl(levelsCnt, ActivationFunction, XMLKEY_PARM_VAL_MAXLEN, &levelActivationS);
	//--
	if (levelActsCnt!=levelsCnt) fail("Level Activations count mismatch (%d provided vs. %d required)", levelActsCnt, levelsCnt);
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
	case BP_SCGD: 
		//safecall(cfgKey, getParm, &SCGDmaxK, "Training/BP_Scgd/maxK");
		SCGDmaxK=MaxEpochs;
		break;
	case BP_LM: break;
		//--... TO DO ...
	default:
		fail("invalid BP_Algo: %d", BP_Algo);
	}

}
sNNparms::sNNparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_, loadingTid_) {
	mallocs();
	safecall(persistor_, loadCoreNNparms, loadingPid_, loadingTid_, &levelRatioS, &levelActivationS, &useContext, &useBias, &MaxEpochs, &TargetMSE, &NetSaveFreq, &StopOnDivergence, &BP_Algo, &LearningRate, &LearningMomentum);
	//-- need to find levelsCnt and build levelRatio array from levelRatioS and levelActivationS
	char* tmpS1="tmpParm1";
	sCfgParm* tmpParm1= new sCfgParm(this, newsname("tmpParm1"), defaultdbg, GUIreporter, tmpS1, levelRatioS);
	tmpParm1->getVal(&levelRatio, &levelsCnt);
	levelsCnt+=2;
	delete tmpParm1;
	char* tmpS2="tmpParm2";
	sCfgParm* tmpParm2= new sCfgParm(this, newsname("tmpParm2"), defaultdbg, GUIreporter, tmpS2, levelActivationS);
	int tmpInt=0;
	tmpParm2->getVal(&ActivationFunction, &tmpInt);

}
sNNparms::~sNNparms() {
	free(levelRatio);
	free(ActivationFunction);
	free(levelRatioS);
	free(levelActivationS);
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
	safecall(persistor_, saveCoreNNparms, pid_, tid_, levelRatioS, levelActivationS, useContext, useBias, MaxEpochs, TargetMSE, NetSaveFreq, StopOnDivergence, BP_Algo, LearningRate, LearningMomentum, SCGDmaxK);
}

//-- private stuff
void sNNparms::mallocs() {
	levelRatioS=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	levelActivationS=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);

	levelRatio=(float*)malloc((CORE_MAX_INTERNAL_LEVELS-2)*sizeof(float));
	ActivationFunction=(int*)malloc(CORE_MAX_INTERNAL_LEVELS*sizeof(int));
	//levelRatioS[0]='\0'; levelActivationS[0]='\0';
}
