#include "sDataSet.h"

sDataSet::sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sourceTS=sourceTS_; sampleLen=sampleLen_; predictionLen=predictionLen_; batchSamplesCnt=batchSamplesCnt_; selectedFeaturesCnt=selectedFeaturesCnt_; BWcalc=BWcalc_;
	mallocs1();
	for (int f=0; f<selectedFeaturesCnt; f++) selectedFeature[f]=selectedFeature_[f];
	if (BWcalc_) {
		BWfeature[0]=BWfeature_[0]; BWfeature[1]=BWfeature_[1];
	}

	mallocs2();
}
sDataSet::sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_) : sCfgObj(sCfgObjParmsVal) {
	sampleLen=sampleLen_; predictionLen=predictionLen_;

	mallocs1();

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &batchSamplesCnt, "BatchSamplesCount");
	safecall(cfgKey, getParm, &selectedFeature, "SelectedFeatures", false, &selectedFeaturesCnt);
	safecall(cfgKey, getParm, &BWcalc, "BarWidthCalc");
	safecall(cfgKey, getParm, &BWfeature, "BarWidthFeatures", false, &BWfeaturesCnt);
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(sourceTS, newsname("%s_TimeSerie", name->base), defaultdbg, cfg, "TimeSerie");

	mallocs2();

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
sDataSet::~sDataSet() {
	frees();
}
void sDataSet::build(float scaleMin_, float scaleMax_, int type) {
	numtype* v=nullptr;
	switch (type) {
	case VAL:	v=sourceTS->val; break;
	case TRVAL: v=sourceTS->trval; break;
	case TRSVAL:  v=sourceTS->trsval; break;
	}
	if (v==nullptr) fail("invalid data type: %d", type);

	int s, b, f;
	int si, ti, sidx, tidx;
	si=0; ti=0;
	tidx=0;

	//-- scale timeserie data
	sourceTS->scale(scaleMin_, scaleMax_);

	//-- 2. populate sample[], target[] from sourceTS->d_tr
	for (s=0; s<samplesCnt; s++) {
		//-- samples
		sidx=s*sourceTS->sourceData->featuresCnt;
		for (b=0; b<sampleLen; b++) {
			for (f=0; f<sourceTS->sourceData->featuresCnt; f++) {
				if (isSelected(f)) {
					sample[si]=v[sidx];
					si++;
				}
				sidx++;
			}
		}

		//-- targets
		tidx=sidx;
		for (b=0; b<predictionLen; b++) {
			for (f=0; f<sourceTS->sourceData->featuresCnt; f++) {
				if (isSelected(f)) {
					target[ti]=v[tidx];
					ti++;
				}
				tidx++;
			}
		}

	}

	//-- 3. populate SFB targets and BFS sample/target for every batch
	for (b=0; b<batchCnt; b++) {
		SBF2BFS(b, sampleLen, sample, sampleBFS);
		SBF2BFS(b, predictionLen, target, targetBFS);
		BFS2SFB(b, predictionLen, targetBFS, targetSFB);
	}

	if (doDump) dump();

}
void sDataSet::dump(int type) {
	int s, i, b, f;

	char suffix[10];
	if (type==VAL) strcpy_s(suffix, 10, "BASE");
	if (type==TRVAL) strcpy_s(suffix, 10, "TR");
	if (type==TRSVAL) strcpy_s(suffix, 10, "TRS");
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_%s_dump_%p.csv", dbg->outfilepath, name->base, sourceTS->date0, suffix, this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "SampleId,");
	for (b=0; b<(sampleLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf(dumpFile, "Bar%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf(dumpFile, ",");
	for (b=0; b<(predictionLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf(dumpFile, "  Prd%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf(dumpFile, "\n");
	for (i=0; i<(1+(sampleLen*selectedFeaturesCnt)); i++) fprintf(dumpFile, "---------,");
	fprintf(dumpFile, ",");
	for (i=0; i<(predictionLen*selectedFeaturesCnt); i++) fprintf(dumpFile, "---------,");
	fprintf(dumpFile, "\n");

	int si, ti, sidx, tidx;
	si=0; ti=0;
	for (s=0; s<samplesCnt; s++) {
		//-- samples
		sidx=s*sourceTS->sourceData->featuresCnt;
		fprintf(dumpFile, "%d,", s);
		for (b=0; b<sampleLen; b++) {
			for (f=0; f<sourceTS->sourceData->featuresCnt; f++) {
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
		for (b=0; b<predictionLen; b++) {
			for (f=0; f<sourceTS->sourceData->featuresCnt; f++) {
				if (isSelected(f)) {
					if (tidx==sourceTS->len) {
						tidx-=sourceTS->sourceData->featuresCnt;
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

//-- private stuff
void sDataSet::mallocs1() {
	selectedFeature=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
	BWfeature=(int*)malloc(BWfeaturesCnt*sizeof(int));
}
void sDataSet::mallocs2() {
	samplesCnt=sourceTS->stepsCnt-sampleLen-predictionLen+1;
	if ((samplesCnt%batchSamplesCnt)!=0) {
		fail("Wrong Batch Size. samplesCnt=%d , batchSamplesCnt=%d", samplesCnt, batchSamplesCnt)
	} else {
		batchCnt = samplesCnt/batchSamplesCnt;
	}
	//--
	sample=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	target=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	prediction=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	sampleBFS=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	targetSFB=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionSFB=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	target0=(numtype*)malloc(samplesCnt*selectedFeaturesCnt*sizeof(numtype));
	prediction0=(numtype*)malloc(samplesCnt*selectedFeaturesCnt*sizeof(numtype));
}
void sDataSet::frees() {
	free(sample); free(target); free(prediction);
	free(sampleBFS); free(targetBFS); free(predictionBFS);
	free(targetSFB); free(predictionSFB);
	free(selectedFeature);
}
bool sDataSet::isSelected(int ts_f) {
	for (int ds_f=0; ds_f<selectedFeaturesCnt; ds_f++) {
		if (selectedFeature[ds_f]==ts_f) return true;
	}
	return false;
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
