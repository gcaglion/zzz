#include "sDS.h"
//#include <vld.h>

void sDS::mallocs1() {
	sample=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*(WTlevel+1)*sizeof(numtype));
	target=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*(WTlevel+1)*sizeof(numtype));
	prediction=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*(WTlevel+1)*sizeof(numtype));
	TRmin=(numtype*)malloc(featuresCnt*sizeof(numtype));
	TRmax=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
}
void sDS::buildFromTS(sTS* ts_) {
	
	//-- check that ts historyLen is greater than ds sampleLen
	if (!(ts_->stepsCnt>=(sampleLen+targetLen))) fail("not enough history in timeserie (%d) to build one sample/target (%d/%d)", ts_->stepsCnt, sampleLen, targetLen);

	//-- build samples/targets
	int dsidxS=0, tsidxS=0, dsidxT=0, tsidxT=0;
	for (int s=0; s<samplesCnt; s++) {
		//-- sample
		for (int bar=0; bar<sampleLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				tsidxS=(s+bar)*featuresCnt+f;
				for (int l=0; l<(WTlevel+1); l++) {
					sample[dsidxS] = (WTlevel>0) ? ts_->FFTval[l][tsidxS] : ts_->valTR[tsidxS];
					dsidxS++;
				}
			}
		}
		//-- target
		for (int bar=0; bar<targetLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				tsidxT=featuresCnt*sampleLen+(s+bar)*featuresCnt+f;
				for (int l=0; l<(WTlevel+1); l++) {
					target[dsidxT] = (WTlevel>0) ? ts_->FFTval[l][tsidxT] : ts_->valTR[tsidxT];
					dsidxT++;
				}
			}
		}
	}
}

sDS::sDS(sObjParmsDef, sTS* fromTS_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, "") {
	sampleLen=sampleLen_; targetLen=targetLen_; batchSize=batchSize_; doDump=doDump_;
	featuresCnt=fromTS_->featuresCnt;
	WTlevel=fromTS_->WTlevel;
	samplesCnt=fromTS_->stepsCnt-sampleLen-targetLen+1;
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath); if (dumpPath_!=nullptr) strcpy_s(dumpPath, MAX_PATH, dumpPath_);

	mallocs1();

	safecall(this, buildFromTS, fromTS_);

	//-- dump
	if (doDump) dump();

}
sDS::sDS(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	safecall(cfgKey, getParm, &sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &targetLen, "TargetLen");
	safecall(cfgKey, getParm, &batchSize, "BatchSize");
	//--
	safecall(cfgKey, getParm, &doDump, "Dump");
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	char* _dumpPath=&dumpPath[0];
	safecall(cfgKey, getParm, &_dumpPath, "DumpPath", true);
	//--
	sTS* _ts; safespawn(_ts, newsname("%s_TimeSerie", name->base), defaultdbg, cfg, "TimeSerie");
	if (_ts->doDump) _ts->dump();

	featuresCnt=_ts->featuresCnt;
	WTlevel=_ts->WTlevel;
	samplesCnt=_ts->stepsCnt-sampleLen-targetLen+1;

	mallocs1();

	safecall(this, buildFromTS, _ts);

	//-- dump
	if (doDump) dump();

	cfg->currentKey=bkpKey;
}
sDS::sDS(sObjParmsDef, sDS* copyFromDS_) : sCfgObj(sObjParmsVal, nullptr, "") {

	sampleLen=copyFromDS_->sampleLen;
	targetLen=copyFromDS_->targetLen;
	featuresCnt=copyFromDS_->featuresCnt;
	samplesCnt=copyFromDS_->samplesCnt;
	batchSize=copyFromDS_->batchSize;
	doDump=copyFromDS_->doDump;
	strcpy_s(dumpPath, MAX_PATH, copyFromDS_->dumpPath);

	//-- mallocs
	mallocs1();

	//-- copy sample  from original DS
	memcpy_s(sample, samplesCnt*sampleLen*featuresCnt*sizeof(numtype), copyFromDS_->sample, samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	memcpy_s(target, samplesCnt*targetLen*featuresCnt*sizeof(numtype), copyFromDS_->target, samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	//-- dump
	if (doDump) dump();

}
sDS::sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_) : sCfgObj(sObjParmsVal, nullptr, "") {

	//-- sampleLen
	sampleLen=parentDScnt_*parentDS_[0]->targetLen;

	//--the rest is taken from parentDS[0]
	targetLen=parentDS_[0]->targetLen;
	featuresCnt=parentDS_[0]->featuresCnt;
	samplesCnt=parentDS_[0]->samplesCnt;// +parentDS_[0]->sampleLen-sampleLen;
	batchSize=parentDS_[0]->batchSize;
	doDump=parentDS_[0]->doDump;
	strcpy_s(dumpPath, MAX_PATH, parentDS_[0]->dumpPath);

	//-- consistency checks for all the above
	for (int d=1; d<parentDScnt_; d++) {
		if (parentDS_[d]->targetLen!=parentDS_[0]->targetLen) fail("parentDS[%d]->targetLen (%d) differs from parentDS[0]->targetLen (%d)", d, parentDS_[d]->targetLen, parentDS_[0]->targetLen);
		if (parentDS_[d]->featuresCnt!=parentDS_[0]->featuresCnt) fail("parentDS[%d]->featuresCnt (%d) differs from parentDS[0]->featuresCnt (%d)", d, parentDS_[d]->featuresCnt, parentDS_[0]->featuresCnt);
//		if (parentDS_[d]->samplesCnt!=parentDS_[0]->samplesCnt) fail("parentDS[%d]->samplesCnt (%d) differs from parentDS[0]->samplesCnt (%d)", d, parentDS_[d]->samplesCnt, parentDS_[0]->samplesCnt);
	}

	mallocs1();

	//-- build sample  from parentDSs' prediction
	int sbfi=0; int i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
				for (int d=0; d<parentDScnt_; d++) {
					for (int f=0; f<featuresCnt; f++) {
						sbfi=s*targetLen*featuresCnt+b*featuresCnt+f;
						sample[i]=parentDS_[d]->prediction[sbfi];
					i++;
				}				
			}
		}
	}
	//-- build target  from parentDS[0] target
	sbfi=0; i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				target[i]=parentDS_[0]->target[i];
				i++;
			}
		}
	}

	//-- TRmin/TRmax
	for (int f=0; f<featuresCnt; f++) {
		TRmin[f]=1e9; TRmax[f]=-1e9;
		for (int d=0; d<parentDScnt_; d++) {
			if (parentDS_[d]->TRmin[f]<TRmin[f]) TRmin[f]=parentDS_[d]->TRmin[f];
			if (parentDS_[d]->TRmax[f]>TRmax[f]) TRmax[f]=parentDS_[d]->TRmax[f];
		}
	}

	//-- dump
	if (doDump) dump();

}
sDS::sDS(sObjParmsDef, const char* srcFileName_) : sCfgObj(sObjParmsVal, nullptr, "") {

	sampleLen=-1;
	targetLen=-1;
	featuresCnt=-1;
	samplesCnt=-1;
	batchSize=1;

	char srcFileName[MAX_PATH];
	FILE* fs; 
	getFullPath(srcFileName_, srcFileName);
	char kaz[MAX_PATH]; splitFullFileName(srcFileName, dumpPath, kaz); //-- we set dumpPath to srcFile path
	if (fopen_s(&fs, srcFileName, "r")!=0) fail("Could not open data file %s", srcFileName);

	const int linesize=32768;
	char line[linesize]; char* pline=&line[0];
	int sampleId, barId, featureId;
	int offset;

	//-- count number of rows
	while (fgets(pline, linesize, fs)!=NULL) samplesCnt++;
	fseek(fs, 0, SEEK_SET);
	//--
	if (fgets(pline, linesize, fs)==NULL) {
		return;
	} else {
		pline+=8;	//-- skip "SampleId"
		while (sscanf_s(pline, ",Bar%dF%d%n", &barId, &featureId, &offset)!=0) {
			if (featureId>featuresCnt) featuresCnt++;
			if (barId>sampleLen) sampleLen++;
			pline+=offset;
		}
		featuresCnt++;
		sampleLen++;

		sscanf_s(pline, ",%n", &offset);
		pline+=offset;
		while (sscanf_s(pline, ", Target%dF%d%n", &barId, &featureId, &offset)!=0) {
			if (barId>targetLen) targetLen++;
			pline+=offset;
		}
		targetLen++;	
		pline=&line[0];
	}
	//--
	mallocs1();
	//--
	int si=0, ti=0;
	while (fgets(pline, linesize, fs)!=NULL) {
		sscanf_s(pline, "%d%n", &sampleId, &offset);
		pline+=offset;
		#ifdef DOUBLE_NUMTYPE
		while (sscanf_s(pline, ",%lf%n", &sample[si], &offset)!=0) {
		#else
		while (sscanf_s(pline, ",%f%n", &sample[si], &offset)!=0) {
		#endif
			si++;
			pline+=offset;
		}
		pline++; pline++;	// skips "|" between sample and target
		#ifdef DOUBLE_NUMTYPE
		while (sscanf_s(pline, ",%lf%n", &target[ti], &offset)!=0) {
		#else
		while (sscanf_s(pline, ",%f%n", &target[ti], &offset)!=0) {
		#endif
			ti++;
			pline+=offset;
		}
		pline=&line[0];
	}
}
sDS::~sDS(){
	free(sample); free(target); free(prediction);
	//free(TRmin); free(TRmax); 
	free(scaleM); free(scaleP);
}

void sDS::invertSequence() {
	numtype* tmpSample=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	numtype* tmpTarget=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	numtype* tmpPrediction=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	for (int s=0; s<samplesCnt; s++) {
		for(int b=0; b<sampleLen; b++){
			for (int f=0; f<featuresCnt; f++) {
				tmpSample[(samplesCnt-s-1)*sampleLen*featuresCnt+b*featuresCnt+f]=sample[s*sampleLen*featuresCnt+b*featuresCnt+f];
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				tmpTarget[(samplesCnt-s-1)*targetLen*featuresCnt+b*featuresCnt+f]=target[s*targetLen*featuresCnt+b*featuresCnt+f];
				tmpPrediction[(samplesCnt-s-1)*targetLen*featuresCnt+b*featuresCnt+f]=prediction[s*targetLen*featuresCnt+b*featuresCnt+f];
			}
		}
	}
	memcpy_s(sample, samplesCnt*sampleLen*featuresCnt*sizeof(numtype), tmpSample, samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	memcpy_s(target, samplesCnt*targetLen*featuresCnt*sizeof(numtype), tmpTarget, samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	memcpy_s(prediction, samplesCnt*targetLen*featuresCnt*sizeof(numtype), tmpPrediction, samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	free(tmpSample); free(tmpTarget); free(tmpPrediction);
}
void sDS::slideSequence(int steps) {
	numtype* tmpSample=(numtype*)malloc(1*sampleLen*featuresCnt*sizeof(numtype));
	numtype* tmpTarget=(numtype*)malloc(1*targetLen*featuresCnt*sizeof(numtype));
	numtype* tmpPrediction=(numtype*)malloc(1*targetLen*featuresCnt*sizeof(numtype));

	if (steps>0) {
		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				tmpSample[b*featuresCnt+f]=sample[0*sampleLen*featuresCnt+b*featuresCnt+f];
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				tmpTarget[b*featuresCnt+f]=target[0*targetLen*featuresCnt+b*featuresCnt+f];
				tmpPrediction[b*featuresCnt+f]=prediction[0*targetLen*featuresCnt+b*featuresCnt+f];
			}
		}

		for (int s=0; s<(samplesCnt-1); s++) {
			for (int b=0; b<sampleLen; b++) {
				for (int f=0; f<featuresCnt; f++) {
					sample[s*sampleLen*featuresCnt+b*featuresCnt+f]=sample[(s+1)*sampleLen*featuresCnt+b*featuresCnt+f];
				}
			}
			for (int b=0; b<targetLen; b++) {
				for (int f=0; f<featuresCnt; f++) {
					target[s*targetLen*featuresCnt+b*featuresCnt+f]=target[(s+1)*targetLen*featuresCnt+b*featuresCnt+f];
					prediction[s*targetLen*featuresCnt+b*featuresCnt+f]=prediction[(s+1)*targetLen*featuresCnt+b*featuresCnt+f];
				}
			}
		}

		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				sample[(samplesCnt-1)*sampleLen*featuresCnt+b*featuresCnt+f]=tmpSample[b*featuresCnt+f];
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				target[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f]=tmpTarget[b*featuresCnt+f];
				prediction[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f]=tmpPrediction[b*featuresCnt+f];
			}
		}
	} else {
		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				tmpSample[b*featuresCnt+f]=sample[(samplesCnt-1)*sampleLen*featuresCnt+b*featuresCnt+f];
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				tmpTarget[b*featuresCnt+f]=target[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f];
				tmpPrediction[b*featuresCnt+f]=prediction[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f];
			}
		}

		for (int s=(samplesCnt-2); s>=0; s--) {
			for (int b=0; b<sampleLen; b++) {
				for (int f=0; f<featuresCnt; f++) {
					sample[(s+1)*sampleLen*featuresCnt+b*featuresCnt+f]=sample[s*sampleLen*featuresCnt+b*featuresCnt+f];
				}
			}
			for (int b=0; b<targetLen; b++) {
				for (int f=0; f<featuresCnt; f++) {
					target[(s+1)*targetLen*featuresCnt+b*featuresCnt+f]=target[s*targetLen*featuresCnt+b*featuresCnt+f];
					prediction[(s+1)*targetLen*featuresCnt+b*featuresCnt+f]=prediction[s*targetLen*featuresCnt+b*featuresCnt+f];
				}
			}
		}

		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				sample[0*sampleLen*featuresCnt+b*featuresCnt+f]=tmpSample[b*featuresCnt+f];
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				target[0*targetLen*featuresCnt+b*featuresCnt+f]=tmpTarget[b*featuresCnt+f];
				prediction[0*targetLen*featuresCnt+b*featuresCnt+f]=tmpPrediction[b*featuresCnt+f];
			}
		}
	}
	free(tmpSample); free(tmpTarget); free(tmpPrediction);
}
void sDS::duplicateSequence() {
	numtype* samplebkp=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	numtype* targetbkp=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	numtype* predictionbkp=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	memcpy_s(samplebkp, samplesCnt*sampleLen*featuresCnt*sizeof(numtype), sample, samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	memcpy_s(targetbkp, samplesCnt*targetLen*featuresCnt*sizeof(numtype), target, samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	memcpy_s(predictionbkp, samplesCnt*targetLen*featuresCnt*sizeof(numtype), prediction, samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	samplesCnt*=2;
	free(sample); sample=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	free(target); target=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	free(prediction); prediction=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	memcpy_s(&sample[0], samplesCnt/2*sampleLen*featuresCnt*sizeof(numtype), samplebkp, samplesCnt/2*sampleLen*featuresCnt*sizeof(numtype));
	memcpy_s(&sample[samplesCnt/2*sampleLen*featuresCnt], samplesCnt/2*sampleLen*featuresCnt*sizeof(numtype), samplebkp, samplesCnt/2*sampleLen*featuresCnt*sizeof(numtype));
	memcpy_s(&target[0], samplesCnt/2*targetLen*featuresCnt*sizeof(numtype), targetbkp, samplesCnt/2*targetLen*featuresCnt*sizeof(numtype));
	memcpy_s(&target[samplesCnt/2*targetLen*featuresCnt], samplesCnt/2*targetLen*featuresCnt*sizeof(numtype), targetbkp, samplesCnt/2*targetLen*featuresCnt*sizeof(numtype));
	memcpy_s(&prediction[0], samplesCnt/2*targetLen*featuresCnt*sizeof(numtype), predictionbkp, samplesCnt/2*targetLen*featuresCnt*sizeof(numtype));
	memcpy_s(&prediction[samplesCnt/2*targetLen*featuresCnt], samplesCnt/2*targetLen*featuresCnt*sizeof(numtype), predictionbkp, samplesCnt/2*targetLen*featuresCnt*sizeof(numtype));

	free(samplebkp); free(targetbkp); free(predictionbkp);
}
void sDS::halveSequence() {
	samplesCnt/=2;
}
void sDS::swapFirstLast() {
	int b, f;
	numtype* tmpSample=(numtype*)malloc(1*sampleLen*featuresCnt*sizeof(numtype));
	numtype* tmpTarget=(numtype*)malloc(1*targetLen*featuresCnt*sizeof(numtype));
	numtype* tmpPrediction=(numtype*)malloc(1*targetLen*featuresCnt*sizeof(numtype));

	for (b=0; b<sampleLen; b++) {
		for (f=0; f<featuresCnt; f++) {
			tmpSample[b*featuresCnt+f]=sample[0*sampleLen*featuresCnt+b*featuresCnt+f];
			sample[0*sampleLen*featuresCnt+b*featuresCnt+f]=sample[(samplesCnt-1)*sampleLen*featuresCnt+b*featuresCnt+f];
			sample[(samplesCnt-1)*sampleLen*featuresCnt+b*featuresCnt+f]=tmpSample[b*featuresCnt+f];
		}
	}
	for (b=0; b<targetLen; b++) {
		for (f=0; f<featuresCnt; f++) {
			tmpTarget[b*featuresCnt+f]=target[0*targetLen*featuresCnt+b*featuresCnt+f];
			target[0*targetLen*featuresCnt+b*featuresCnt+f]=target[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f];
			target[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f]=tmpTarget[b*featuresCnt+f];
			tmpPrediction[b*featuresCnt+f]=prediction[0*targetLen*featuresCnt+b*featuresCnt+f];
			prediction[0*targetLen*featuresCnt+b*featuresCnt+f]=prediction[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f];
			prediction[(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f]=tmpPrediction[b*featuresCnt+f];
		}
	}

	free(tmpSample); free(tmpTarget); free(tmpPrediction);
}

void sDS::dump(bool isScaled) {
	FILE* dumpFile=nullptr;
	dumpPre(isScaled, &dumpFile);

	int dsidxS=0, dsidxT=0, dsidxP=0;
	for (int s=0; s<samplesCnt; s++) {
		//-- sample
		fprintf(dumpFile, "%d,", s);
		for (int bar=0; bar<sampleLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				for (int l=0; l<WTlevel; l++) {
					fprintf(dumpFile, "%f,", sample[dsidxS]);
					dsidxS++;
				}
			}
		}
		fprintf(dumpFile, "|,");
		//-- target
		for (int bar=0; bar<targetLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				for (int l=0; l<WTlevel; l++) {
					fprintf(dumpFile, "%f,", target[dsidxT]);
					dsidxT++;
				}
			}
		}
		fprintf(dumpFile, "|,");
		//-- prediction
		for (int bar=0; bar<targetLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				for (int l=0; l<WTlevel; l++) {
					fprintf(dumpFile, "%f,", prediction[dsidxP]);
					dsidxP++;
				}
			}
		}
		fprintf(dumpFile, "\n");
	}

	fclose(dumpFile);
}
void sDS::dumpPre(bool isScaled, FILE** dumpFile) {
	int b, f;

	//-- open dumpFile
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%sdump_p%d_t%d_%p.csv", dumpPath, name->base, (isScaled)?"SCALED_":"_", GetCurrentProcessId(), GetCurrentThreadId(), this);
	if (fopen_s(dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	//-- print headers
	fprintf((*dumpFile), "SampleId,");
	for (b=0; b<(sampleLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			for (int l=0; l<WTlevel; l++) {
				fprintf((*dumpFile), "Bar%dF%dL%d,", b, f, l);
			}
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(targetLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			for (int l=0; l<WTlevel; l++) {
				fprintf((*dumpFile), "  Target%dF%dL%d,", b, f, l);
			}
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(targetLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			for (int l=0; l<WTlevel; l++) {
				fprintf((*dumpFile), "  Prediction%dF%dL%d,", b, f, l);
			}
		}
	}
	fprintf((*dumpFile), "\n");
}

void sDS::getSeq(int trg_vs_prd, numtype* oVal) {
	int si=0, ti=0;

	for (int b=0; b<sampleLen; b++) {
		for (int f=0; f<featuresCnt; f++) {
			for (int l=0; l<(WTlevel+1); l++) {
				si=b*featuresCnt+f*(WTlevel+1)+l;
				oVal[ti]=(trg_vs_prd==TARGET) ? sample[si] : EMPTY_VALUE;
				ti++;
			}
		}
	}

	for (int s=0; s<samplesCnt; s++) {
		for (int f=0; f<featuresCnt; f++) {
			for (int l=0; l<(WTlevel+1); l++) {
				si=s*targetLen*featuresCnt+f*(WTlevel+1)+l;
				oVal[ti]=(trg_vs_prd==TARGET) ? target[si] : prediction[si];
				ti++;
			}
		}
	}

	for (int b=1; b<targetLen; b++) {
		for (int f=0; f<featuresCnt; f++) {
			for (int l=0; l<(WTlevel+1); l++) {
				si=(samplesCnt-1)*targetLen*featuresCnt*(WTlevel+1)+b*featuresCnt*(WTlevel+1)+f*(WTlevel+1)+l;
				oVal[ti]=(trg_vs_prd==TARGET) ? target[si] : prediction[si];
				ti++;
			}
		}
	}
}

void sDS::target2prediction() {
	int i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				prediction[i]=target[i];
				i++;
			}
		}
	}
}

void sDS::scale(float scaleMin_, float scaleMax_) {

	for (int f=0; f<featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(TRmax[f]-TRmin[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*TRmax[f]);
	}

	int si=0, ti=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				sample[si]=sample[si]*scaleM[f]+scaleP[f];
				si++;
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				target[ti]=(target[ti]==EMPTY_VALUE) ? EMPTY_VALUE : target[ti]*scaleM[f]+scaleP[f];
				prediction[ti]=prediction[ti]*scaleM[f]+scaleP[f];
				ti++;
			}
		}
	}
	if (doDump) dump(true);
}
void sDS::unscale() {
	int si=0, ti=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				sample[si]=(sample[si]==EMPTY_VALUE) ? EMPTY_VALUE : (sample[si]-scaleP[f])/scaleM[f];
				si++;
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				target[ti]=(target[ti]==EMPTY_VALUE) ? EMPTY_VALUE : (target[ti]-scaleP[f])/scaleM[f];
				prediction[ti]=(prediction[ti]-scaleP[f])/scaleM[f];
				ti++;
			}
		}
	}
}
void sDS::untransformSeq(int seqDT_, numtype* seqBase_, numtype* iTRval, numtype* iActualVal, numtype* oBASEval) {
	int curr, prev;
	int stepsCnt=samplesCnt+sampleLen+targetLen-1;
	for (int s=0; s<stepsCnt; s++) {
		for (int f=0; f<featuresCnt; f++) {
			curr=s*featuresCnt+f;
			prev=(s-1)*featuresCnt+f;
			if (seqDT_==DT_NONE) {
				oBASEval[curr]=iTRval[curr];
			}
			if (seqDT_==DT_DELTA) {
				if (s>0) {
					if (iTRval[curr]==EMPTY_VALUE) {
						oBASEval[curr]=EMPTY_VALUE;
					} else {
						oBASEval[curr]=iTRval[curr]+iActualVal[prev];
						if (iActualVal[curr]==EMPTY_VALUE) iActualVal[curr]=oBASEval[curr];
					}
				} else {
					if (iTRval[curr]==EMPTY_VALUE) {
						oBASEval[curr]=EMPTY_VALUE;
					} else {
						oBASEval[curr]=iTRval[curr]+seqBase_[f];
					}
				}
			}
			if (seqDT_==DT_LOG) {
				if (iTRval[curr]==EMPTY_VALUE) {
					oBASEval[curr]=EMPTY_VALUE;
				} else {
					oBASEval[curr]=exp(iTRval[curr]);
				}
			}
		}
	}
}
