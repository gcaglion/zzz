#include "sDS.h"
//#include <vld.h>

void sDS::mallocs1() {
	sampleSBF=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	targetSBF=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	predictionSBF=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	sampleBFS=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
}
void sDS::buildFromTS(sTS* ts_, int WNNsrc_) {
	//-- build samples/targets
	int dsidxS=0, tsidxS=0, dsidxT=0, tsidxT=0;
	for (int sample=0; sample<samplesCnt; sample++) {
		//-- sample
		for (int bar=0; bar<sampleLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				tsidxS=(sample+bar)*featuresCnt+f;
				if (WNNsrc_==0) {
					sampleSBF[dsidxS] = ts_->valTR[tsidxS];
				} else {
					sampleSBF[dsidxS] = ts_->FFTval[WNNsrc_-1][tsidxS];
				}
				dsidxS++;
			}
		}
		//-- target
		for (int bar=0; bar<targetLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				tsidxT=(sample+bar)*featuresCnt+f;
				tsidxT+=featuresCnt*sampleLen;
				if (WNNsrc_==0) {
					targetSBF[dsidxT] = ts_->valTR[tsidxT];
				} else {
					targetSBF[dsidxT] = ts_->FFTval[WNNsrc_-1][tsidxT];
				}
				dsidxT++;
			}
		}
	}
}

sDS::sDS(sObjParmsDef, sTS* fromTS_, int WNNsrc_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, "") {
	sampleLen=sampleLen_; targetLen=targetLen_; batchSize=batchSize_;
	featuresCnt=fromTS_->featuresCnt;
	samplesCnt=fromTS_->stepsCnt-sampleLen-targetLen+1;
	doDump=doDump_;
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath); if (dumpPath_!=nullptr) strcpy_s(dumpPath, MAX_PATH, dumpPath_);

	mallocs1();

	buildFromTS(fromTS_, WNNsrc_);

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
	samplesCnt=_ts->stepsCnt-sampleLen-targetLen+1;

	mallocs1();

	buildFromTS(_ts, 0);

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

	//-- copy sample SBF from original DS
	memcpy_s(sampleSBF, samplesCnt*sampleLen*featuresCnt*sizeof(numtype), copyFromDS_->sampleSBF, samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	memcpy_s(targetSBF, samplesCnt*targetLen*featuresCnt*sizeof(numtype), copyFromDS_->targetSBF, samplesCnt*targetLen*featuresCnt*sizeof(numtype));

	//-- dump
	if (doDump) dump();

}
sDS::sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_) : sCfgObj(sObjParmsVal, nullptr, "") {

	//-- sampleLen
	sampleLen=parentDScnt_*parentDS_[0]->targetLen;

	//--the rest is taken from parentDS[0]
	targetLen=parentDS_[0]->targetLen;
	featuresCnt=parentDS_[0]->featuresCnt;
	samplesCnt=parentDS_[0]->samplesCnt+parentDS_[0]->sampleLen-sampleLen;
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

	//-- build sample SBF from parentDSs' predictionSBF
	int sbfi=0; int i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				for (int d=0; d<parentDScnt_; d++) {
					sampleSBF[i]=parentDS_[d]->predictionSBF[sbfi];
					i++;
				}
				sbfi++;
			}
		}
	}
	//-- build target SBF from parentDS[0] targetSBF
	sbfi=0; i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				targetSBF[i]=parentDS_[0]->targetSBF[i];
				i++;
			}
		}
	}

	//-- dump
	if (doDump) dump();

}
sDS::~sDS(){
	free(sampleSBF); free(targetSBF); free(predictionSBF);
	free(sampleBFS); free(targetBFS); free(predictionBFS);
}

void sDS::dump() {
	FILE* dumpFile=nullptr;
	dumpPre(&dumpFile);

	int dsidxS=0, dsidxT=0, dsidxP=0;
	for (int sample=0; sample<samplesCnt; sample++) {
		//-- sample
		fprintf(dumpFile, "%d,", sample);
		for (int bar=0; bar<sampleLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				fprintf(dumpFile, "%f,", sampleSBF[dsidxS]);
				dsidxS++;
			}
		}
		fprintf(dumpFile, "|,");
		//-- target
		for (int bar=0; bar<targetLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				fprintf(dumpFile, "%f,", targetSBF[dsidxT]);
				dsidxT++;
			}
		}
		fprintf(dumpFile, "|,");
		//-- prediction
		for (int bar=0; bar<targetLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				fprintf(dumpFile, "%f,", predictionSBF[dsidxP]);
				dsidxP++;
			}
		}
		fprintf(dumpFile, "\n");
	}

	fclose(dumpFile);
}
void sDS::dumpPre(FILE** dumpFile) {
	int b, f;

	//-- open dumpFile
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s__dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	if (fopen_s(dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	//-- print headers
	fprintf((*dumpFile), "SampleId,");
	for (b=0; b<(sampleLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			fprintf((*dumpFile), "Bar%dF%d,", b, f);
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(targetLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			fprintf((*dumpFile), "  Target%dF%d,", b, f);
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(targetLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			fprintf((*dumpFile), "  Prediction%dF%d,", b, f);
		}
	}
	fprintf((*dumpFile), "\n");
}

void sDS::getSeq(int trg_vs_prd, numtype* oVal) {
	int si=0, ti=0;

	for (int b=0; b<sampleLen; b++) {
		for (int f=0; f<featuresCnt; f++) {
			si=b*featuresCnt+f;
			oVal[ti]=sampleSBF[si];
			ti++;
		}
	}
	
	for (int s=0; s<samplesCnt; s++) {
		for (int f=0; f<featuresCnt; f++) {
			si=s*targetLen*featuresCnt+f;
			oVal[ti]=(trg_vs_prd==TARGET) ? targetSBF[si] : predictionSBF[si];
			ti++;
		}
	}

	for (int b=1; b<targetLen; b++) {
		for (int f=0; f<featuresCnt; f++) {
			si=(samplesCnt-1)*targetLen*featuresCnt+b*featuresCnt+f;
			oVal[ti]=(trg_vs_prd==TARGET) ? targetSBF[si] : predictionSBF[si];
			ti++;
		}
	}
}

void sDS::setBFS(int batchCnt, int batchSize) {
	for (int b=0; b<batchCnt; b++) {
		//-- populate BFS sample/target for every batch
		SBF2BFS(batchSize, b, sampleLen, sampleSBF, sampleBFS);
		SBF2BFS(batchSize, b, targetLen, targetSBF, targetBFS);
	}
}
void sDS::setSBF(int batchCnt, int batchSize) {
	for (int b=0; b<batchCnt; b++) {
		//-- populate SBF predictionfor every batch
		BFS2SBF(batchSize, b, targetLen, predictionBFS, predictionSBF);
	}
}
void sDS::SBF2BFS(int batchSamplesCnt, int batchId, int barCnt, numtype* fromSBF, numtype* toBFS) {
	int S=batchSamplesCnt;
	int F=featuresCnt;
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
void sDS::BFS2SBF(int batchSamplesCnt, int batchId, int barCnt, numtype* fromBFS, numtype* toSBF) {
	int S=batchSamplesCnt;
	int F=featuresCnt;
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

void sDS::target2prediction() {
	int i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				predictionSBF[i]=targetSBF[i];
				i++;
			}
		}
	}
}

