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

void sDataSet::build(int valStatus) {
	FILE* dumpFile=nullptr;
	int dsidx, tsidx;

	if (doDump) dumpPre(valStatus, &dumpFile);

	for (int s=0; s<samplesCnt; s++) {
		fprintf(dumpFile, "%d,", s);

		//-- 1. samples
		for (int b=0; b<sampleLen; b++) {
			for (int df=0; df<selectedFeaturesCnt; df++) {
				for (int tf=0; tf<sourceTS->sourceData->featuresCnt; tf++) {
					if (selectedFeature[df]==tf) {
						//dsidx = s * sampleLen * selectedFeaturesCnt+b * selectedFeaturesCnt+df;
						dsidx = s * sampleLen * selectedFeaturesCnt+b * selectedFeaturesCnt+df;
						tsidx = s *    1      * sourceTS->sourceData->featuresCnt + b * sourceTS->sourceData->featuresCnt + tf;

						if (isCloned) tsidx+=predictionLen*sourceTS->sourceData->featuresCnt;

						sampleSBF[valStatus][dsidx] = sourceTS->val[valStatus][TARGET][tsidx];
						if(doDump) fprintf(dumpFile, "%f,", sampleSBF[valStatus][dsidx]);

					}
				}
			}
		}
		if(doDump) fprintf(dumpFile, "|,");

		//-- 2. targets
		for (int b=0; b<predictionLen; b++) {
			for (int df=0; df<selectedFeaturesCnt; df++) {
				for (int tf=0; tf<sourceTS->sourceData->featuresCnt; tf++) {
					if (selectedFeature[df]==tf) {
						dsidx = s * predictionLen * selectedFeaturesCnt+b * selectedFeaturesCnt+df;
						tsidx = sampleLen*sourceTS->sourceData->featuresCnt + s*1*sourceTS->sourceData->featuresCnt + b * sourceTS->sourceData->featuresCnt+tf;
						if (isCloned) tsidx+=predictionLen*sourceTS->sourceData->featuresCnt;
						targetSBF[valStatus][dsidx] = sourceTS->val[valStatus][TARGET][tsidx];
						if (doDump) fprintf(dumpFile, "%f,", targetSBF[valStatus][dsidx]);
					}
				}
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
void sDataSet::mallocs() {

}
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
	sampleSBF=(numtype**)malloc(3*sizeof(numtype*)); for(int i=0; i<3; i++) sampleSBF[i]=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetSBF=(numtype**)malloc(3*sizeof(numtype*)); for (int i=0; i<3; i++) targetSBF[i]=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionSBF=(numtype**)malloc(3*sizeof(numtype*)); for (int i=0; i<3; i++) predictionSBF[i]=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	//--
	sampleBFS=(numtype**)malloc(3*sizeof(numtype*)); for (int i=0; i<3; i++) sampleBFS[i]=(numtype*)malloc(samplesCnt*sampleLen*selectedFeaturesCnt*sizeof(numtype));
	targetBFS=(numtype**)malloc(3*sizeof(numtype*)); for (int i=0; i<3; i++) targetBFS[i]=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));
	predictionBFS=(numtype**)malloc(3*sizeof(numtype*)); for (int i=0; i<3; i++) predictionBFS[i]=(numtype*)malloc(samplesCnt*predictionLen*selectedFeaturesCnt*sizeof(numtype));

	//-- generic pointers
	_data[DSsample][SBForder]=sampleSBF;
	_data[DSsample][BFSorder]=sampleBFS;
	_data[DStarget][SBForder]=targetSBF;
	_data[DStarget][BFSorder]=targetBFS;
	_data[DSprediction][SBForder]=predictionSBF;
	_data[DSprediction][BFSorder]=predictionBFS;
}
void sDataSet::frees() {
	return;
	for (int i=0; i<3; i++) {
		free(sampleSBF[i]);
		free(targetSBF[i]);
		free(predictionSBF[i]);
		free(sampleBFS[i]);
		free(targetBFS[i]);
		free(predictionBFS[i]);
	}
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
