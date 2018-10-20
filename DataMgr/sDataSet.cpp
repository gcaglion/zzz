#include "sDataSet.h"

//-- sDataSet, constructors  /destructor
void sDataSet::sDataSet_pre() {
	BWFeature=(int*)malloc(2*sizeof(int));
	selectedFeature=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
}
void sDataSet::sDataSet_post() {

	samplesCnt=sourceTS->steps-shape->sampleLen-shape->predictionLen;// +1;
	if (samplesCnt<1) fail("Not Enough Data. samplesCnt=%d", samplesCnt);
	float batchCntF=((float)samplesCnt/(float)batchSamplesCnt);
	if((samplesCnt%batchSamplesCnt)!=0) {
		fail("Wrong Batch Size. HistoryLen=%d, SampleLen=%d, PredictionLen=%d => samplesCnt=%d, batchSamplesCnt=%d => batchCnt=%.2f", sourceTS->steps, shape->sampleLen, shape->predictionLen, samplesCnt, batchSamplesCnt, batchCntF);
	} else {
		batchCnt=(int)floor(batchCntF);
	}
	sample=(numtype*)malloc(samplesCnt*shape->sampleLen*selectedFeaturesCnt*sizeof(numtype));
	target=(numtype*)malloc(samplesCnt*shape->predictionLen*selectedFeaturesCnt*sizeof(numtype));
	prediction=(numtype*)malloc(samplesCnt*shape->predictionLen*selectedFeaturesCnt*sizeof(numtype));
	sampleBFS=(numtype*)malloc(samplesCnt*shape->sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*shape->predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*shape->predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	targetSFB=(numtype*)malloc(samplesCnt*shape->predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionSFB=(numtype*)malloc(samplesCnt*shape->predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	target0=(numtype*)malloc(samplesCnt*selectedFeaturesCnt*sizeof(numtype));
	prediction0=(numtype*)malloc(samplesCnt*selectedFeaturesCnt*sizeof(numtype));

}

sDataSet::sDataSet(sCfgObjParmsDef, sDataShape* shape_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, int* BWFeature_) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;

	sDataSet_pre();

	batchSamplesCnt=batchSamplesCnt_;
	selectedFeaturesCnt=selectedFeaturesCnt_;
	for (int f=0; f<selectedFeaturesCnt; f++) selectedFeature[f]=selectedFeature_[f];
	BWFeature[0]=BWFeature_[0]; BWFeature[1]=BWFeature_[1];

	sDataSet_post();

}
sDataSet::sDataSet(sCfgObjParmsDef, sDataShape* shape_) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;

	sDataSet_pre();

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &batchSamplesCnt, "BatchSamplesCount");
	safecall(cfgKey, getParm, &selectedFeature, "SelectedFeatures", false, &selectedFeaturesCnt);
	safecall(cfgKey, getParm, &BWFeature, "BWFeatures", false, new int);
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(sourceTS, newsname("%s_TimeSerie", name->base), nullptr, cfg, "TimeSerie");
	
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
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
void sDataSet::dump() {
	int s, i, b, f;
	
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_dump.csv", dbg->outfilepath, name->base, sourceTS->date0);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "SampleId,");
	for (b=0; b<(shape->sampleLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf(dumpFile, "  Bar%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf(dumpFile, ",");
	for (b=0; b<(shape->predictionLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf(dumpFile, "  Prd%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf(dumpFile, "\n");
	for (i=0; i<(1+(shape->sampleLen*selectedFeaturesCnt)); i++) fprintf(dumpFile, "---------,");
	fprintf(dumpFile, ",");
	for (i=0; i<(shape->predictionLen*selectedFeaturesCnt); i++) fprintf(dumpFile, "---------,");
	fprintf(dumpFile, "\n");

	int si, ti, sidx, tidx;
	si=0; ti=0;
	for (s=0; s<samplesCnt; s++) {
		//-- samples
		sidx=s*sourceTS->featuresCnt;
		fprintf(dumpFile, "%d,,,", s);
		for (b=0; b<shape->sampleLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					fprintf(dumpFile, "%f,", sample[si]);
					si++;
				}
				sidx++;
			}
		}
		fprintf(dumpFile, "|,");

		//-- targets
		tidx=sidx;
		for (b=0; b<shape->predictionLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					if (tidx==sourceTS->len) {
						tidx-=sourceTS->featuresCnt;
					}
					fprintf(dumpFile, "%f,", target[ti]);
					ti++;
				}
				tidx++;
			}
		}
		fprintf(dumpFile, "\n");
	}
	fclose(dumpFile);
}
bool sDataSet::isSelected(int ts_f) {
	for (int ds_f=0; ds_f<selectedFeaturesCnt; ds_f++) {
		if (selectedFeature[ds_f]==ts_f) return true;
	}
	return false;
}
void sDataSet::buildFromTS(float scaleMin_, float scaleMax_) {

	int s, b, f;
	int si, ti, sidx, tidx;
	si=0; ti=0;
	
	//-- 1. scale transformed data in Timeserie. This populates sourceTS->d_trs
	sourceTS->scale(scaleMin_, scaleMax_);

	//-- 2. populate sample[], target[] from sourceTS->d_trs
	for (s=0; s<samplesCnt; s++) {
		//-- samples
		sidx=s*sourceTS->featuresCnt;
		for (b=0; b<shape->sampleLen; b++) {
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
		for (b=0; b<shape->predictionLen; b++) {
			for (f=0; f<sourceTS->featuresCnt; f++) {
				if (isSelected(f)) {
					target[ti]=sourceTS->d_trs[tidx];
					ti++;
				}
				tidx++;
			}
		}
	}
	
	//-- 3. populate SFB targets and BFS sample/target for every batch
	for (b=0; b<batchCnt; b++) {
		SBF2BFS(b, shape->sampleLen, sample, sampleBFS);
		SBF2BFS(b, shape->predictionLen, target, targetBFS);
		BFS2SFB(b, shape->predictionLen, targetBFS, targetSFB);
	}

	//-- 4. if a DumpFileName was specified, then dump
	if (doDump) dump();

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
