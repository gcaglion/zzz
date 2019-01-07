#include "sDataSet.h"

sDataSet::sDataSet(sObjParmsDef, sDataShape* shape_, sMT4DataSource* MTsrc_, int selectedFeaturesCnt_, int* selectedFeature_, int dt_, bool doDump_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {

	//-- single sample, no targets
	hasTargets=false;

	shape=shape_; shape->featuresCnt=selectedFeaturesCnt_;
	mallocs1();
	for (int f=0; f<shape->featuresCnt; f++) selectedFeature[f]=selectedFeature_[f];


	batchSamplesCnt=1;
	doDump=doDump_;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	//-- timeserie?
	sourceTS=new sTimeSerie(this, newsname("%s_TimeSerie", name->base), defaultdbg, GUIreporter, MTsrc_, MTsrc_->lastbartime, MTsrc_->sampleLen, dt_, dumpPath);

	setSamples(); 
	mallocs2();

}

sDataSet::sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, sDataShape* shape_, int selectedFeaturesCnt_, int* selectedFeature_, int batchSamplesCnt_, bool doDump_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sourceTS=sourceTS_; batchSamplesCnt=batchSamplesCnt_; 
	shape=shape_; shape->featuresCnt=selectedFeaturesCnt_;
	hasTargets=true;

	mallocs1();

	for (int f=0; f<shape->featuresCnt; f++) selectedFeature[f]=selectedFeature_[f];
	doDump=doDump_;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	setSamples(); mallocs2();
}
sDataSet::sDataSet(sCfgObjParmsDef, int extraSteps) : sCfgObj(sCfgObjParmsVal) {

	safespawn(shape, newsname("%s_Shape", name->base), defaultdbg, 0, 0, 0);
	safecall(cfgKey, getParm, &shape->sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &shape->predictionLen, "PredictionLen");
	mallocs1();
	safecall(cfgKey, getParm, &selectedFeature, "SelectedFeatures", false, &shape->featuresCnt);
	safecall(cfgKey, getParm, &batchSamplesCnt, "BatchSamplesCount");
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 0. default dump path is dbg outfilepath
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	safecall(cfgKey, getParm, &dumpPath, "DumpPath", true);

	hasTargets=true;

	safespawn(sourceTS, newsname("%s_TimeSerie", name->base), defaultdbg, cfg, "TimeSerie", extraSteps);

	setSamples(); mallocs2();

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}

sDataSet::~sDataSet() {
	frees();
}

void sDataSet::build(int fromValSource, int fromValStatus) {
	FILE* dumpFile=nullptr;
	int tsfcnt=sourceTS->sourceData->featuresCnt;

	if (doDump) dumpPre(fromValStatus, &dumpFile);

	int dsidxS=0;
	int tsidxS=0;
	int dsidxT=0;
	int tsidxT=0;
	for (int sample=0; sample<samplesCnt; sample++) {

		//-- sample
		if (doDump) fprintf(dumpFile, "%d,", sample);
		for (int bar=0; bar<shape->sampleLen; bar++) {
			for (int dsf=0; dsf<shape->featuresCnt; dsf++) {
				tsidxS=sample*tsfcnt+bar*tsfcnt+selectedFeature[dsf];

				sampleSBF[dsidxS] = sourceTS->val[fromValSource][fromValStatus][tsidxS];
				if (doDump) fprintf(dumpFile, "%f,", sampleSBF[dsidxS]);
				dsidxS++;
			}
		}
		if (doDump) fprintf(dumpFile, "|,");

		//-- target
		if (hasTargets) {
			for (int bar=0; bar<shape->predictionLen; bar++) {
				for (int dsf=0; dsf<shape->featuresCnt; dsf++) {
					tsidxT=sample*tsfcnt+bar*tsfcnt+selectedFeature[dsf];
					tsidxT+=tsfcnt*shape->sampleLen;

					targetSBF[dsidxT] = sourceTS->val[fromValSource][fromValStatus][tsidxT];
					if (doDump) fprintf(dumpFile, "%f,", targetSBF[dsidxT]);
					dsidxT++;
				}
			}
		}

		if (doDump) fprintf(dumpFile, "\n");
	}
	if (doDump)	fclose(dumpFile);
}
void sDataSet::unbuild(int fromValSource, int toValSource, int toValStatus) {

	int tsfcnt=sourceTS->sourceData->featuresCnt;
	int dsfcnt=shape->featuresCnt;
	int trowlen=shape->predictionLen*dsfcnt;
	int srowlen=shape->sampleLen*tsfcnt;
	int dsidx=0;
	int tsidx=0;

	//-- for the first sample, scan all the bars in sample
	for (int bar=0; bar<shape->sampleLen; bar++) {
		for (int dsf=0; dsf<dsfcnt; dsf++) {
			tsidx=bar*tsfcnt+selectedFeature[dsf];
			sourceTS->val[toValSource][toValStatus][tsidx] = EMPTY_VALUE;
		}
	}

	//-- for each sample/target row, take first bar from the target section
	for (int s=0; s<samplesCnt; s++) {
		for (int dsf=0; dsf<dsfcnt; dsf++) {
			dsidx=s*trowlen+dsf;
			tsidx=srowlen+s*tsfcnt+selectedFeature[dsf];
			sourceTS->val[toValSource][toValStatus][tsidx] = _data[fromValSource][SBF][dsidx];
		}
	}

	if (hasTargets) {
		//-- now we are on the last row of the target section. need to take all bars>0
		for (int bar=1; bar<shape->predictionLen; bar++) {
			for (int dsf=0; dsf<dsfcnt; dsf++) {
				dsidx=(samplesCnt-1)*trowlen+bar*dsfcnt+dsf;
				tsidx=(samplesCnt+shape->sampleLen)*tsfcnt+(bar-1)*tsfcnt+selectedFeature[dsf];
				sourceTS->val[toValSource][toValStatus][tsidx] = _data[fromValSource][SBF][dsidx];
			}
		}
	}

}

void sDataSet::setBFS() {
	for (int b=0; b<batchCnt; b++) {
		//-- populate BFS sample/target for every batch
		SBF2BFS(b, shape->sampleLen, sampleSBF, sampleBFS);
		if(hasTargets) SBF2BFS(b, shape->predictionLen, targetSBF, targetBFS);
	}
}
void sDataSet::setSBF() {
	for (int b=0; b<batchCnt; b++) {
		//-- populate SBF predictionfor every batch
		BFS2SBF(b, shape->predictionLen, predictionBFS, predictionSBF);
	}
}
void sDataSet::SBF2BFS(int batchId, int barCnt, numtype* fromSBF, numtype* toBFS) {
	int S=batchSamplesCnt;
	int F=shape->featuresCnt;
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
	int F=shape->featuresCnt;
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


//-- private stuff
void sDataSet::mallocs1() {
	selectedFeature=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
	dumpPath=(char*)malloc(MAX_PATH);
}
void sDataSet::setSamples() {
	samplesCnt=sourceTS->stepsCnt-shape->sampleLen+1;
	if(hasTargets) samplesCnt-=shape->predictionLen;
	if ((samplesCnt%batchSamplesCnt)!=0) {
		fail("Wrong Batch Size. samplesCnt=%d , batchSamplesCnt=%d", samplesCnt, batchSamplesCnt)
	} else {
		batchCnt = samplesCnt/batchSamplesCnt;
	}
}
void sDataSet::mallocs2() {
	//--
	sampleSBF=(numtype*)malloc(samplesCnt*shape->sampleLen*shape->featuresCnt*sizeof(numtype));
	targetSBF=(numtype*)malloc(samplesCnt*shape->predictionLen*shape->featuresCnt*sizeof(numtype));
	predictionSBF=(numtype*)malloc(samplesCnt*shape->predictionLen*shape->featuresCnt*sizeof(numtype));
	//--
	sampleBFS=(numtype*)malloc(samplesCnt*shape->sampleLen*shape->featuresCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*shape->predictionLen*shape->featuresCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*shape->predictionLen*shape->featuresCnt*sizeof(numtype));

	//-- generic pointers
	_data[SAMPLE][SBF]=sampleSBF;
	_data[SAMPLE][BFS]=sampleBFS;
	_data[TARGET][SBF]=targetSBF;
	_data[TARGET][BFS]=targetBFS;
	_data[PREDICTED][SBF]=predictionSBF;
	_data[PREDICTED][BFS]=predictionBFS;

}
void sDataSet::frees() {
	free(sampleSBF);
	free(targetSBF);
	free(predictionSBF);
	free(sampleBFS);
	free(targetBFS);
	free(predictionBFS);
}
bool sDataSet::isSelected(int ts_f) {
	for (int ds_f=0; ds_f<shape->featuresCnt; ds_f++) {
		if (selectedFeature[ds_f]==ts_f) return true;
	}
	return false;
}
void sDataSet::dumpPre(int valStatus, FILE** dumpFile) {
	int b, f, i;

	//-- set dumpFile name
	char suffix1[10];
	if (valStatus==BASE) strcpy_s(suffix1, 10, "BASE");
	if (valStatus==TR) strcpy_s(suffix1, 10, "TR");
	if (valStatus==TRS) strcpy_s(suffix1, 10, "TRS");
	char suffix3[16];
	strcpy_s(suffix3, 16, "INDIPENDENT");

	//-- open dumpFile
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s-%s_dump_p%d_t%d_%p.csv", dumpPath, name->base, suffix1, suffix3, GetCurrentProcessId(), GetCurrentThreadId(), this);
	if (fopen_s(dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	//-- print headers
	fprintf((*dumpFile), "SampleId,");
	for (b=0; b<(shape->sampleLen); b++) {
		for (f=0; f<shape->featuresCnt; f++) {
			fprintf((*dumpFile), "Bar%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(shape->predictionLen); b++) {
		for (f=0; f<shape->featuresCnt; f++) {
			fprintf((*dumpFile), "  Target%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(shape->predictionLen); b++) {
		for (f=0; f<shape->featuresCnt; f++) {
			fprintf((*dumpFile), "  Prediction%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf((*dumpFile), "\n");
	for (i=0; i<(1+(shape->sampleLen*shape->featuresCnt)); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), ",");
	for (i=0; i<(shape->predictionLen*shape->featuresCnt); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), ",");
	for (i=0; i<(shape->predictionLen*shape->featuresCnt); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), "\n");
}
