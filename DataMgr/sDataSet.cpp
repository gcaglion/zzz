#include "sDataSet.h"

sDataSet::sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sourceTS=sourceTS_; sampleLen=sampleLen_; predictionLen=predictionLen_; batchSamplesCnt=batchSamplesCnt_; selectedFeaturesCnt=selectedFeaturesCnt_; BWcalc=BWcalc_;
	mallocs1();
	for (int f=0; f<selectedFeaturesCnt; f++) selectedFeature[f]=selectedFeature_[f];
	if (BWcalc_) {
		BWfeature[0]=BWfeature_[0]; BWfeature[1]=BWfeature_[1];
	}
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
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
	//-- 0. default dump path is dbg outfilepath
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	safecall(cfgKey, getParm, &dumpPath, "DumpPath", true);
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
	case VAL:	v=sourceTS->valA; break;
	case TRVAL: v=sourceTS->trvalA; break;
	case TRSVAL:  v=sourceTS->trsvalA; break;
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
					sampleSBF[si]=v[sidx];
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
					targetSBF[ti]=v[tidx];
					ti++;
				}
				tidx++;
			}
		}

	}

	//-- 3. convert SBF to BFS samples and targets
	reorder(DSsample, SBForder, BFSorder);
	reorder(DStarget, SBForder, BFSorder);

	if (doDump)	dump();
}
void sDataSet::dump(int type, bool prediction_) {
	int s, i, b, f;

	char suffix1[10];
	if (type==VAL) strcpy_s(suffix1, 10, "BASE");
	if (type==TRVAL) strcpy_s(suffix1, 10, "TR");
	if (type==TRSVAL) strcpy_s(suffix1, 10, "TRS");
	char suffix2[12];
	if (prediction_) {
		strcpy_s(suffix2, 12, "PRD");
	} else {
		strcpy_s(suffix2, 12, "ACT");
	}

	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_%s-%s_dump_%p.csv", dumpPath, name->base, sourceTS->date0, suffix1, suffix2, this);
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
					fprintf(dumpFile, "%f,", sampleSBF[si]);
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
					fprintf(dumpFile, "%f,", targetSBF[ti]);
					ti++;
				}
				tidx++;
			}
		}
		fprintf(dumpFile, "\n");
	}
	fclose(dumpFile);
}
void sDataSet::reorder(int section, int FROMorderId, int TOorderId) {

	int Bcnt=(section==DSsample)?sampleLen:predictionLen;
	int Fcnt=selectedFeaturesCnt;
	int Scnt=samplesCnt;

	int idx[3];
	int i, o;

	for (int b=0; b<Bcnt; b++) {
		for (int f=0; f<Fcnt; f++) {
			for (int s=0; s<Scnt; s++) {

				idx[SBForder]=s*Bcnt*Fcnt+b*Fcnt+f;
				idx[BFSorder]=b*Fcnt*Scnt+f*Scnt+s;
				idx[SFBorder]=s*Bcnt*Fcnt+f*Bcnt+b;

				i=idx[FROMorderId]; o=idx[TOorderId];
				_data[section][TOorderId][o] = _data[section][FROMorderId][i];

			}
		}
	}
}

//-- private stuff
void sDataSet::mallocs1() {
	selectedFeature=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
	BWfeature=(int*)malloc(BWfeaturesCnt*sizeof(int));
	dumpPath=(char*)malloc(MAX_PATH);
}
void sDataSet::mallocs2() {
	samplesCnt=sourceTS->stepsCnt-sampleLen-predictionLen+1;
	if ((samplesCnt%batchSamplesCnt)!=0) {
		fail("Wrong Batch Size. samplesCnt=%d , batchSamplesCnt=%d", samplesCnt, batchSamplesCnt)
	} else {
		batchCnt = samplesCnt/batchSamplesCnt;
	}
	//--
	sampleSBF=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetSBF=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionSBF=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	sampleBFS=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//-- generic pointers
	_data[DSsample][SBForder]=sampleSBF;
	_data[DSsample][BFSorder]=sampleBFS;
	_data[DStarget][SBForder]=targetSBF;
	_data[DStarget][BFSorder]=targetBFS;
	_data[DSprediction][SBForder]=predictionSBF;
	_data[DSprediction][BFSorder]=predictionBFS;
}
void sDataSet::frees() {
	free(sampleSBF); free(targetSBF); free(predictionSBF);
	free(sampleBFS); free(targetBFS); free(predictionBFS);
	free(selectedFeature);
	free(dumpPath);
}
bool sDataSet::isSelected(int ts_f) {
	for (int ds_f=0; ds_f<selectedFeaturesCnt; ds_f++) {
		if (selectedFeature[ds_f]==ts_f) return true;
	}
	return false;
}
