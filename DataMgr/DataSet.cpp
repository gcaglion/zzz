#include "DataSet.h"

//-- sDataSet, constructors  /destructor
void sDataSet::sDataSet_pre() {
	BWFeature=(int*)malloc(2*sizeof(int));
	selectedFeature=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
}
void sDataSet::sDataSet_post() {

	samplesCnt=sourceTS->steps-sampleLen-targetLen;// +1;
	if (samplesCnt<1) fail("Not Enough Data. samplesCnt=%d", samplesCnt);
	batchCnt=samplesCnt/batchSamplesCnt;
	if ((batchCnt*batchSamplesCnt)!=samplesCnt) fail("Wrong Batch Size. samplesCnt=%d, batchSamplesCnt=%d", samplesCnt, batchSamplesCnt);

	sample=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	target=(numtype*)malloc(samplesCnt*targetLen*selectedFeaturesCnt*sizeof(numtype));
	prediction=(numtype*)malloc(samplesCnt*targetLen*selectedFeaturesCnt*sizeof(numtype));
	sampleBFS=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*targetLen*selectedFeaturesCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*targetLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	targetSFB=(numtype*)malloc(samplesCnt*targetLen*selectedFeaturesCnt*sizeof(numtype));
	predictionSFB=(numtype*)malloc(samplesCnt*targetLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	target0=(numtype*)malloc(samplesCnt*selectedFeaturesCnt*sizeof(numtype));
	prediction0=(numtype*)malloc(samplesCnt*selectedFeaturesCnt*sizeof(numtype));

	//-- fill sample/target data right at creation time. TS has data in SBF format
	buildFromTS();

	for (int b=0; b<batchCnt; b++) {
		//-- populate BFS sample/target for every batch
		SBF2BFS(b, sampleLen, sample, sampleBFS);
		SBF2BFS(b, targetLen, target, targetBFS);
		//-- populate SFB targets, too
		BFS2SFB(b, targetLen, targetBFS, targetSFB);
	}
}

sDataSet::sDataSet(sCfgObjParmsDef, int sampleLen_, int targetLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, int* BWFeature_) : sCfgObj(sCfgObjParmsVal) {

	sDataSet_pre();

	batchSamplesCnt=batchSamplesCnt_;
	selectedFeaturesCnt=selectedFeaturesCnt_;
	for (int f=0; f<selectedFeaturesCnt; f++) selectedFeature[f]=selectedFeature_[f];
	BWFeature[0]=BWFeature_[0]; BWFeature[1]=BWFeature_[1];
	//--
	
	sampleLen=sampleLen_;
	targetLen=targetLen_;

	sDataSet_post();

}
sDataSet::sDataSet(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	sDataSet_pre();

	//-- 0. Backup currentKey
	sCfgKey* bkpKey=cfg->currentKey;
	//-- 1. set Key 
 safecall(cfg, setKey, keyDesc_);

	//-- 2. common parameters
	safecall(cfg->currentKey, getParm, &batchSamplesCnt, "BatchSamplesCount");
	safecall(cfg->currentKey, getParm, &selectedFeature, "SelectedFeatures", &selectedFeaturesCnt);
	safecall(cfg->currentKey, getParm, &BWFeature, "BWFeatures", new int);
	//-- 3. spawn sub-Keys
	safespawn(sourceTS, newsname("%s_TimeSerie", name->base), nullptr, cfg, "TimeSerie");
	//-- 2.1. DataShape parameters, retrieved from object hierarchy
	safecall(cfg, setKey, "../../../Shape");
	safecall(cfg->currentKey, getParm, &sampleLen, "SampleLen");
	safecall(cfg->currentKey, getParm, &targetLen, "PredictionLen");

	//-- 4. Restore currentKey
	cfg->currentKey=bkpKey;

	sDataSet_post();

}

sDataSet::~sDataSet() {
	free(selectedFeature);
	free(BWFeature);
	free(sample);
	if (target!=nullptr) free(target);
	free(prediction);
	free(sampleBFS);
	free(targetBFS);
	free(predictionBFS);
	free(target0);
	free(prediction0);

	delete sourceTS;
}
//-- sDataSet, other methods
void sDataSet::dump(char* dumpFileName_) {
	int s, i, b, f;
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, MAX_PATH, ((dumpFileName==nullptr) ? "C:/temp/DataSet.log" : dumpFileName));

	FILE* dumpFile;
	fopen_s(&dumpFile, dumpFileName, "w");
	if (errno!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "SampleId\t");
	for (b=0; b<(sampleLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf(dumpFile, "  Bar%dF%d\t", b, selectedFeature[f]);
		}
	}
	fprintf(dumpFile, "\t");
	for (b=0; b<(targetLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf(dumpFile, "  Prd%dF%d\t", b, selectedFeature[f]);
		}
	}
	fprintf(dumpFile, "\n");
	for (i=0; i<(1+(sampleLen*selectedFeaturesCnt)); i++) fprintf(dumpFile, "---------\t");
	fprintf(dumpFile, "\t");
	for (i=0; i<(targetLen*selectedFeaturesCnt); i++) fprintf(dumpFile, "---------\t");
	fprintf(dumpFile, "\n");

	int si, ti, sidx, tidx;
	si=0; ti=0;
	for (s=0; s<samplesCnt; s++) {
		//-- samples
		sidx=s*sourceTS->featuresCnt;
		fprintf(dumpFile, "%d\t\t\t", s);
		for (b=0; b<sampleLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					fprintf(dumpFile, "%f\t", sample[si]);
					si++;
				}
				sidx++;
			}
		}
		fprintf(dumpFile, "|\t");

		//-- targets
		tidx=sidx;
		for (b=0; b<targetLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					if (tidx==sourceTS->len) {
						tidx-=sourceTS->featuresCnt;
					}
					fprintf(dumpFile, "%f\t", target[ti]);
					ti++;
				}
				tidx++;
			}
		}
		fprintf(dumpFile, "\n");
	}
	fclose(dumpFile);
}
Bool sDataSet::isSelected(int ts_f) {
	for (int ds_f=0; ds_f<selectedFeaturesCnt; ds_f++) {
		if (selectedFeature[ds_f]==ts_f) return true;
	}
	return false;
}
void sDataSet::buildFromTS() {

	int s, b, f;

	int si, ti, sidx, tidx;
	si=0; ti=0;
	for (s=0; s<samplesCnt; s++) {
		//-- samples
		sidx=s*sourceTS->featuresCnt;
		for (b=0; b<sampleLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					sample[si]=sourceTS->d_trs[sidx];
					si++;
				}
				sidx++;
			}
		}
		//-- targets
		tidx=sidx;
		for (b=0; b<targetLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					target[ti]=sourceTS->d_trs[tidx];
					ti++;
				}
				tidx++;
			}
		}
	}

}
void sDataSet::SBF2BFS(int batchId, int barCnt, numtype* fromSBF, numtype* toBFS) {
	int S=batchSamplesCnt;
	int F=selectedFeaturesCnt;
	int B=barCnt;
	int idx;
	int idx0=batchId*B*F*S;
	int i=idx0;
	for (int bar=0; bar<B; bar++) {												// i1=bar	l1=B
		for (int f=0; f<F; f++) {										// i2=f		l2=F
			for (int s=0; s<S; s++) {										// i3=s		l3=S
				idx=idx0+s*F*B+bar*F+f;
				toBFS[i]=fromSBF[idx];
				i++;
			}
		}
	}
}
void sDataSet::BFS2SBF(int batchId, int barCnt, numtype* fromBFS, numtype* toSBF) {
	int S=batchSamplesCnt;
	int F=selectedFeaturesCnt;
	int B=barCnt;
	int idx;
	int idx0=batchId*B*F*S;
	int i=idx0;
	for (int s=0; s<S; s++) {												// i1=s		l1=S
		for (int bar=0; bar<B; bar++) {											// i2=bar	l1=B
			for (int f=0; f<F; f++) {									// i3=f		l3=F
				idx=idx0+bar*F*S+f*S+s;
				toSBF[i]=fromBFS[idx];
				i++;
			}
		}
	}

}
void sDataSet::BFS2SFBfull(int barCnt, numtype* fromBFS, numtype* toSFB) {
	int S=batchSamplesCnt;
	int F=selectedFeaturesCnt;
	int B=barCnt;
	int i, idx, idx0;
	for (int batchId=0; batchId<batchCnt; batchId++) {
		idx0=batchId*B*F*S;
		i=idx0;
		for (int s=0; s<S; s++) {												// i1=s		l1=S
			for (int f=0; f<F; f++) {										// i2=f		l2=F
				for (int bar=0; bar<B; bar++) {										// i3=bar	l3=B
					idx=idx0+bar*F*S+f*S+s;
					toSFB[i]=fromBFS[idx];
					i++;
				}
			}
		}
	}

}
void sDataSet::BFS2SFB(int batchId, int barCnt, numtype* fromBFS, numtype* toSFB) {
	int S=batchSamplesCnt;
	int F=selectedFeaturesCnt;
	int B=barCnt;
	int idx;
	int idx0=batchId*B*F*S;
	int i=idx0;
	for (int s=0; s<S; s++) {												// i1=s		l1=S
		for (int f=0; f<F; f++) {										// i2=f		l2=F
			for (int bar=0; bar<B; bar++) {										// i3=bar	l3=B
				idx=idx0+bar*F*S+f*S+s;
				toSFB[i]=fromBFS[idx];
				i++;
			}
		}
	}

}
