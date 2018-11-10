#include "sDataSet.h"

sDataSet::sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_, bool doDump_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sourceTS=sourceTS_; sampleLen=sampleLen_; predictionLen=predictionLen_; batchSamplesCnt=batchSamplesCnt_; selectedFeaturesCnt=selectedFeaturesCnt_; BWcalc=BWcalc_;
	isCloned=false;

	mallocs1();

	for (int f=0; f<selectedFeaturesCnt; f++) selectedFeature[f]=selectedFeature_[f];
	if (BWcalc_) {
		BWfeature[0]=BWfeature_[0]; BWfeature[1]=BWfeature_[1];
	}
	doDump=doDump_;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	mallocs2();
}
sDataSet::sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_) : sCfgObj(sCfgObjParmsVal) {
	sampleLen=sampleLen_; predictionLen=predictionLen_;
	isCloned=false;

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

sDataSet::sDataSet(sObjParmsDef, sDataSet* trainDS_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	sourceTS=trainDS_->sourceTS; sampleLen=trainDS_->sampleLen; predictionLen=trainDS_->predictionLen; batchSamplesCnt=trainDS_->batchSamplesCnt; selectedFeaturesCnt=trainDS_->selectedFeaturesCnt; BWcalc=trainDS_->BWcalc;
	isCloned=true;

	mallocs1();

	for (int f=0; f<selectedFeaturesCnt; f++) selectedFeature[f]=trainDS_->selectedFeature[f];
	if (trainDS_->BWcalc) {
		BWfeature[0]=trainDS_->BWfeature[0]; BWfeature[1]=trainDS_->BWfeature[1];
	}
	doDump=trainDS_->doDump;
	if (trainDS_->dumpPath!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, trainDS_->dumpPath);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	mallocs2();
	samplesCnt-=predictionLen;
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
		for (int bar=0; bar<sampleLen; bar++) {
			for (int dsf=0; dsf<selectedFeaturesCnt; dsf++) {
				tsidxS=sample*tsfcnt+bar*tsfcnt+selectedFeature[dsf];
				sampleSBF[dsidxS] = sourceTS->val[fromValSource][fromValStatus][tsidxS]; 
				if (doDump) fprintf(dumpFile, "%f,", sampleSBF[dsidxS]);
				dsidxS++;
			}
		}
		if (doDump) fprintf(dumpFile, "|,");

		//-- target
		for (int bar=0; bar<predictionLen; bar++) {
			for (int dsf=0; dsf<selectedFeaturesCnt; dsf++) {
				tsidxT=sample*tsfcnt+bar*tsfcnt+selectedFeature[dsf];
				tsidxT+=tsfcnt*sampleLen;
				targetSBF[dsidxT] = sourceTS->val[fromValSource][fromValStatus][tsidxT];
				if (doDump) fprintf(dumpFile, "%f,", targetSBF[dsidxT]);
				dsidxT++;
			}
		}
		if (doDump) fprintf(dumpFile, "\n");
	}
	if (doDump)	fclose(dumpFile);
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

				idx[SBF]=s*Bcnt*Fcnt+b*Fcnt+f;
				idx[BFS]=b*Fcnt*Scnt+f*Scnt+s;
				//idx[SFBorder]=s*Bcnt*Fcnt+f*Bcnt+b;

				i=idx[FROMorderId]; o=idx[TOorderId];
				_data[section][TOorderId][o] = _data[section][FROMorderId][i];

			}
		}
	}
}
void sDataSet::unbuild(int fromValSource, int toValSource, int toValStatus) {

	int tsfcnt=sourceTS->sourceData->featuresCnt;
	int dsidxS=0;
	int tsidxS=0;
	int dsidxT=0;
	int tsidxT=0;
	for (int sample=0; sample<samplesCnt; sample++) {

		//-- sample
		for (int bar=0; bar<sampleLen; bar++) {
			for (int dsf=0; dsf<selectedFeaturesCnt; dsf++) {
				tsidxS=sample*tsfcnt+bar*tsfcnt+selectedFeature[dsf];
				sourceTS->val[toValSource][toValStatus][tsidxS] = sampleSBF[dsidxS];
				dsidxS++;
			}
		}

		//-- target
		for (int bar=0; bar<predictionLen; bar++) {
			for (int dsf=0; dsf<selectedFeaturesCnt; dsf++) {
				tsidxT=sample*tsfcnt+bar*tsfcnt+selectedFeature[dsf];
				tsidxT+=tsfcnt*sampleLen;
				sourceTS->val[toValSource][toValStatus][tsidxT] = targetSBF[dsidxT];
				dsidxT++;
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
	samplesCnt=sourceTS->stepsCnt-sampleLen+1 -predictionLen;
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
	for (int ds_f=0; ds_f<selectedFeaturesCnt; ds_f++) {
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
	if (isCloned) {
		strcpy_s(suffix3, 16, "SHIFTEDFROM");
	} else {
		strcpy_s(suffix3, 16, "INDIPENDENT");
	}

	//-- open dumpFile
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_%s-%s_dump_%p.csv", dumpPath, name->base, sourceTS->date0, suffix1, suffix3, this);
	if (fopen_s(dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	//-- print headers
	fprintf((*dumpFile), "SampleId,");
	for (b=0; b<(sampleLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf((*dumpFile), "Bar%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(predictionLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf((*dumpFile), "  Target%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(predictionLen); b++) {
		for (f=0; f<selectedFeaturesCnt; f++) {
			fprintf((*dumpFile), "  Prediction%dF%d,", b, selectedFeature[f]);
		}
	}
	fprintf((*dumpFile), "\n");
	for (i=0; i<(1+(sampleLen*selectedFeaturesCnt)); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), ",");
	for (i=0; i<(predictionLen*selectedFeaturesCnt); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), ",");
	for (i=0; i<(predictionLen*selectedFeaturesCnt); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), "\n");
}
