#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_;
	persistor=persistor_;

}
sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_;

	//-- 1. get Parameters
	safespawn(persistor, newsname("%s_Persistor", name->base), defaultdbg, cfg, "Persistor");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sCore::~sCore() {}

void sCore::infer(int samplesCnt_, int sampleLen_, int predictionLen_, int featuresCnt_, numtype* INsampleSBF, numtype* INtargetSBF, numtype* OUTpredictionSBF) {
	int sFromIdx, sToIdx, tFromIdx, tToIdx;
	int slen=sampleLen_*featuresCnt_;
	int plen=predictionLen_*featuresCnt_;
	numtype* singleSample=(numtype*)malloc(slen*sizeof(numtype));
	numtype* singleTarget=(numtype*)malloc(plen*sizeof(numtype));
	numtype* singlePrediction=(numtype*)malloc(plen*sizeof(numtype));

	//-- set private _batchCnt and _batchSize for the network from dataset
	_batchCnt=samplesCnt_;
	_batchSize=1;

	//-- set Layout. This should not change weightsCnt[] at all, just nodesCnt[]
	setLayout(_batchSize);
	//-- malloc + init neurons
	mallocLayout();

	for (int s=0; s<samplesCnt_; s++) {

		//-- build single sample
		for (int b=0; b<sampleLen_; b++) {
			for (int f=0; f<featuresCnt_; f++) {
				sFromIdx=s*sampleLen_*featuresCnt_+b*featuresCnt_+f;
				sToIdx=b*featuresCnt_+f;
				singleSample[sToIdx]=INsampleSBF[sFromIdx];
			}
		}

		//-- build single target
		for (int b=0; b<predictionLen_; b++) {
			for (int f=0; f<featuresCnt_; f++) {
				tFromIdx=s*predictionLen_*featuresCnt_+b*featuresCnt_+f;
				tToIdx=b*featuresCnt_+f;
				singleTarget[tToIdx]=INtargetSBF[tFromIdx];
			}
		}

		//-- infer prediction for single sample
		singleInfer(singleSample, singleTarget, &singlePrediction);

		//-- copy prediction back to OUTpredictionSBF
		for (int b=0; b<predictionLen_; b++) {
			for (int f=0; f<featuresCnt_; f++) {
				tToIdx=s*predictionLen_*featuresCnt_+b*featuresCnt_+f;
				tFromIdx=b*featuresCnt_+f;
				OUTpredictionSBF[tToIdx]=singlePrediction[tFromIdx];
			}
		}

	}

	free(singleSample); free(singleTarget); free(singlePrediction);
}
