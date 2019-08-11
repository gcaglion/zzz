#include "sDS.h"
//#include <vld.h>

void sDS::mallocs1() {
	sampleSBF=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	targetSBF=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	predictionSBF=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	sampleBFS=(numtype*)malloc(samplesCnt*sampleLen*featuresCnt*sizeof(numtype));
	targetBFS=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	predictionBFS=(numtype*)malloc(samplesCnt*targetLen*featuresCnt*sizeof(numtype));
	TRmin=(numtype*)malloc(featuresCnt*sizeof(numtype));
	TRmax=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
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
	sampleLen=sampleLen_; targetLen=targetLen_; batchSize=batchSize_; doDump=doDump_;
	featuresCnt=fromTS_->featuresCnt;
	samplesCnt=fromTS_->stepsCnt-sampleLen-targetLen+1;
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

	//-- build sample SBF from parentDSs' predictionSBF
	int sbfi=0; int i=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<targetLen; b++) {
				for (int d=0; d<parentDScnt_; d++) {
					for (int f=0; f<featuresCnt; f++) {
						sbfi=s*targetLen*featuresCnt+b*featuresCnt+f;
						sampleSBF[i]=parentDS_[d]->predictionSBF[sbfi];
					i++;
				}				
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
sDS::~sDS(){
	free(sampleSBF); free(targetSBF); free(predictionSBF);
	free(sampleBFS); free(targetBFS); free(predictionBFS);
	//free(TRmin); free(TRmax); 
	free(scaleM); free(scaleP);
}

void sDS::dump(bool isScaled) {
	FILE* dumpFile=nullptr;
	dumpPre(isScaled, &dumpFile);

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

void sDS::getSeq(int trg_vs_prd, numtype* oVal, sDS* baseDS) {
	int si=0, ti=0;

	for (int b=0; b<baseDS->sampleLen; b++) {
		for (int f=0; f<baseDS->featuresCnt; f++) {
			si=b*baseDS->featuresCnt+f;
			oVal[ti]=baseDS->sampleSBF[si];
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

void sDS::scale(float scaleMin_, float scaleMax_) {

	for (int f=0; f<featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(TRmax[f]-TRmin[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*TRmax[f]);
	}

	int si=0, ti=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				sampleSBF[si]=sampleSBF[si]*scaleM[f]+scaleP[f];
				si++;
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				targetSBF[ti]=(targetSBF[ti]==EMPTY_VALUE) ? EMPTY_VALUE : targetSBF[ti]*scaleM[f]+scaleP[f];
				predictionSBF[ti]=predictionSBF[ti]*scaleM[f]+scaleP[f];
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
				sampleSBF[si]=(sampleSBF[si]==EMPTY_VALUE) ? EMPTY_VALUE : (sampleSBF[si]-scaleP[f])/scaleM[f];
				si++;
			}
		}
		for (int b=0; b<targetLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				targetSBF[ti]=(targetSBF[ti]==EMPTY_VALUE) ? EMPTY_VALUE : (targetSBF[ti]-scaleP[f])/scaleM[f];
				predictionSBF[ti]=(predictionSBF[ti]-scaleP[f])/scaleM[f];
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
		}
	}
}
