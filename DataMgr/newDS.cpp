#include "newDS.h"

void sDS::mallocs() {
	//-- malloc patterns
	pattern=(numtype*)malloc(patternsCnt*patternLen*featuresCnt*sizeof(numtype));
	//-- malloc sequance values
	seqval=(numtype*)malloc((patternsCnt+patternLen-1)*featuresCnt*sizeof(numtype));
	//-- malloc scaling info
	minVal=(numtype*)malloc(featuresCnt*sizeof(numtype));
	maxVal=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
}

//-- constructor 1: build from timeserie sequence
sDS::sDS(sObjParmsDef, int featuresCnt_, int stepsCnt_, numtype* sequenceBF_, int patternLen_, int batchSize_, bool doDump_, char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	featuresCnt=featuresCnt_; patternLen=patternLen_;
	patternsCnt=stepsCnt_-patternLen+1;
	batchSize=batchSize_;

	//-- dump settings
	doDump=doDump_;
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	if (dumpPath_!=nullptr) strcpy_s(dumpPath, MAX_PATH, dumpPath_);

	//-- malloc patterns, sequence and scaling info
	mallocs();

	//-- build patterns
	int si=0; int hi;
	for (int s=0; s<patternsCnt; s++) {
		hi=s*featuresCnt;
		for (int b=0; b<patternLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				pattern[si]=sequenceBF_[hi];
				si++; hi++;
			}
		}
	}

	//-- set min/max values
	setMinMax();
}
//-- constructor 2: build by merging existing datasets
sDS::sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {

	//-- consistency checks: patternsCnt and featuresCnt must be identical
	//...
	//--

	featuresCnt=parentDS_[0]->featuresCnt; for (int d=1; d<parentDScnt_; d++) featuresCnt+=parentDS_[d]->featuresCnt;
	patternLen=parentDS_[0]->patternLen; 
	patternsCnt=parentDS_[0]->patternsCnt;
	batchSize=parentDS_[0]->batchSize;
	doDump=parentDS_[0]->doDump;
	strcpy_s(dumpPath, MAX_PATH, parentDS_[0]->dumpPath);
	//-- malloc patterns,sequence and scaling info
	mallocs();

	//-- build patterns
	int i=0, o=0;
	for (int p=0; p<patternsCnt; p++) {
		for (int b=0; b<patternLen; b++) {
			for (int d=0; d<parentDScnt_; d++) {
				for (int df=0; df<parentDS_[d]->featuresCnt; df++) {
					pattern[o]=parentDS_[d]->pattern[p*patternLen*parentDS_[d]->featuresCnt+b*parentDS_[d]->featuresCnt+df];
					o++;
				}
			}
		}
	}

	//-- set min/max values
	setMinMax();

}
//-- constructor 3: build from configuration file
sDS::sDS(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	safecall(cfgKey, getParm, &batchSize, "BatchSize");
	safecall(cfgKey, getParm, &patternLen, "PatternLen");
	safecall(cfgKey, getParm, &doDump, "Dump");
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath); char* _dumpPath=&dumpPath[0];
	safecall(cfgKey, getParm, &_dumpPath, "DumpPath", true);

	int _srcTScnt;
	safecall(cfgKey, getParm, &_srcTScnt, "TimeSeriesCount");
	sTimeSerie** _srcTS=(sTimeSerie**)malloc(_srcTScnt*sizeof(sTimeSerie*));
	int* _srcTSfeatCnt=(int*)malloc(_srcTScnt*sizeof(int));
	int** _srcTSfeat=(int**)malloc(_srcTScnt*sizeof(int*));

	featuresCnt=0;
	for (int t=0; t<_srcTScnt; t++) {
		safespawn(_srcTS[t], newsname("%s_TimeSerie%d", name->base, t), defaultdbg, cfg, (newsname("TimeSerie%d", t))->base);
		//--... consistency checks here ...
		_srcTSfeat[t]=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
		safecall(cfgKey, getParm, &_srcTSfeat[t], (newsname("TimeSerie%d/selectedFeatures", t))->base, false, &_srcTSfeatCnt[t]);
		featuresCnt+=_srcTSfeatCnt[t];
	}

	patternsCnt=_srcTS[0]->stepsCnt-patternLen+1;

	//-- malloc patterns, sequence and scaling info
	mallocs();

	//-- build patterns
	int dsidxS=0, tsidxS=0;
	for (int p=0; p<patternsCnt; p++) {
		for (int b=0; b<patternLen; b++) {
			for (int t=0; t<_srcTScnt; t++) {
				for (int tf=0; tf<_srcTSfeatCnt[t]; tf++) {
					tsidxS=(p+b)*_srcTS[t]->sourceData->featuresCnt+_srcTSfeat[t][tf];
					pattern[dsidxS] = _srcTS[t]->val[ACTUAL][BASE][tsidxS];
					dsidxS++;
				}
			}
		}
	}

	//-- free(s)
/*	for (int t=0; t<_srcTScnt; t++) {
		free(_srcTS[t]);
		free(_srcTSfeat[t]);
	}
	free(_srcTS);
	free(_srcTSfeat);
	free(_srcTSfeatCnt);
*/
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
//-- destructor
sDS::~sDS() {
	free(pattern);
	free(seqval);
	free(minVal); free(maxVal);
	free(scaleM); free(scaleP);
}

void sDS::setSequence() {
	int si=0;
	for (int p=0; p<patternsCnt; p++) {
		for (int f=0; f<featuresCnt; f++) {
			seqval[si]=pattern[p*patternLen*featuresCnt+f];
			si++;
		}
	}
	for (int b=1; b<patternLen; b++) {
		for (int f=0; f<featuresCnt; f++) {
			seqval[si]=pattern[(patternsCnt-1)*patternLen*featuresCnt+b*featuresCnt+f];
			si++;
		}
	}
}
void sDS::dumpPre(FILE** dumpFile) {
	int b, f, i;

	//-- open dumpFile
	char dumpFileName[MAX_PATH];
	//	sprintf_s(dumpFileName, "%s/%s_%s_dump_p%d_t%d_%p.csv", dumpPath, name->base, suffix1, GetCurrentProcessId(), GetCurrentThreadId(), this);
	sprintf_s(dumpFileName, "%s/%s_dump_p%d_t%d_%p.csv", dumpPath, "DataSet", GetCurrentProcessId(), GetCurrentThreadId(), this);
	//if (fopen_s(dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	fopen_s(dumpFile, dumpFileName, "w");

	//-- print headers
	fprintf((*dumpFile), "SampleId,");
	for (b=0; b<(patternLen); b++) {
		for (f=0; f<featuresCnt; f++) {
			fprintf((*dumpFile), "Bar%dF%d,", b, f);
		}
	}
	fprintf((*dumpFile), "\n");
	for (i=0; i<=(patternLen*featuresCnt); i++) fprintf((*dumpFile), "---------,");
	fprintf((*dumpFile), "\n");
}
void sDS::dump() {
	FILE* dumpFile=nullptr;
	dumpPre(&dumpFile);

	int dsidxS=0;
	for (int s=0; s<patternsCnt; s++) {

		fprintf(dumpFile, "%d,", s);
		for (int bar=0; bar<patternLen; bar++) {
			for (int f=0; f<featuresCnt; f++) {
				fprintf(dumpFile, "%f,", pattern[dsidxS]);
				dsidxS++;
			}
		}
		fprintf(dumpFile, "\n");
	}
	fclose(dumpFile);
}
void sDS::setMinMax() {
	int pi;
	for (int f=0; f<featuresCnt; f++) {
		minVal[f]=(numtype)1e19; maxVal[f]=(numtype)-1e19;
		for (int p=0; p<patternsCnt; p++) {
			pi=p*patternLen*featuresCnt+f;
			if (pattern[pi]>maxVal[f]) maxVal[f]=pattern[pi];
			if (pattern[pi]<minVal[f]) minVal[f]=pattern[pi];
		}
		for (int b=0; b<patternLen; b++) {
			pi=(patternsCnt-1)*patternLen*featuresCnt+b*featuresCnt+f;
			if (pattern[pi]>maxVal[f]) maxVal[f]=pattern[pi];
			if (pattern[pi]<minVal[f]) minVal[f]=pattern[pi];
		}
	}

}
void sDS::scale(float scaleMin_, float scaleMax_) {

	for (int f=0; f<featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(maxVal[f]-minVal[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*maxVal[f]);
	}

	int i=0;
	for (int p=0; p<patternsCnt; p++) {
		for (int b=0; b<patternLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				pattern[i]=pattern[i]*scaleM[f]+scaleP[f];
				i++;
			}
		}
	}
}
void sDS::unscale() {
	int i=0;
	for (int p=0; p<patternsCnt; p++) {
		for (int b=0; b<patternLen; b++) {
			for (int f=0; f<featuresCnt; f++) {
				pattern[i]=(pattern[i]-scaleP[f])/scaleM[f];
				i++;
			}
		}
	}
}