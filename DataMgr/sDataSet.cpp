#include "sDataSet.h"

sDataSet::sDataSet(sObjParmsDef, int sourceTScnt_, sTimeSerie** sourceTS_, int* selectedTSfeaturesCnt_, int** selectedTSfeature_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, bool doDump_, char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {

	safespawn(shape, newsname("%s_Shape", name->base), defaultdbg, 0, 0, 0);
	sourceTScnt=sourceTScnt_; 
	mallocs1();
	for (int t=0; t<sourceTScnt; t++) {
		sourceTS[t]=sourceTS_[t];
	}
	selectedTSfeaturesCnt=selectedTSfeaturesCnt_; selectedTSfeature=selectedTSfeature_;
	shape->sampleLen=sampleLen_; shape->predictionLen=predictionLen_; batchSamplesCnt=batchSamplesCnt_;
	for (int t=0; t<sourceTScnt; t++) shape->featuresCnt+=selectedTSfeaturesCnt[t];


	doDump=doDump_;
	//-- 0. default dump path is dbg outfilepath
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	if(dumpPath_!=nullptr) dumpPath=dumpPath_;

	hasTargets=false;

	setSamples(); mallocs2();

}
sDataSet::sDataSet(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	safespawn(shape, newsname("%s_Shape", name->base), defaultdbg, 0, 0, 0);

	safecall(cfgKey, getParm, &shape->sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &shape->predictionLen, "PredictionLen");

	safecall(cfgKey, getParm, &sourceTScnt, "TimeSeriesCount");
	mallocs1();
	for (int t=0; t<sourceTScnt; t++) {
		safespawn(sourceTS[t], newsname("%s_TimeSerie%d", name->base, t), defaultdbg, cfg, (newsname("TimeSerie%d", t))->base);
		safecall(cfgKey, getParm, &selectedTSfeature[t], (newsname("TimeSerie%d/selectedFeatures", t))->base, false, &selectedTSfeaturesCnt[t]);
		shape->featuresCnt+=selectedTSfeaturesCnt[t];
	}

	safecall(cfgKey, getParm, &batchSamplesCnt, "BatchSamplesCount");
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 0. default dump path is dbg outfilepath
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	safecall(cfgKey, getParm, &dumpPath, "DumpPath", true);

	hasTargets=true;


	setSamples(); mallocs2();

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
sDataSet::sDataSet(sObjParmsDef, int sourceDScnt_, sDataSet** sourceDS_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {

	//-- samples are built from sources' predictions
	//-- targets are copied from sources

	//-- calc total sourceTScnt , needed to malloc
	sourceTScnt=0;
	for (int ds=0; ds<sourceDScnt_; ds++) {
		for (int ts=0; ts<sourceDS_[ds]->sourceTScnt; ts++) {
			sourceTScnt++;
		}
	}
	mallocs1();

	//-- sourceTSs and selectedTSfeatures are added together
	int t=0;
	for (int ds=0; ds<sourceDScnt_; ds++) {
		for (int ts=0; ts<sourceDS_[ds]->sourceTScnt; ts++) {
			sourceTS[t]=sourceDS_[ds]->sourceTS[ts];
			selectedTSfeaturesCnt[t]=sourceDS_[ds]->selectedTSfeaturesCnt[ts];
			for (int self=0; self<selectedTSfeaturesCnt[t]; self++) {
				selectedTSfeature[t][self]=sourceDS_[ds]->selectedTSfeature[ts][self];
			}
			t++;
		}
	}

}
sDataSet::~sDataSet() {
	frees();
}

void sDataSet::build(int fromValSource, int fromValStatus) {

	int dsidxS=0;
	int tsidxS=0;
	int dsidxT=0;
	int tsidxT=0;
	for (int sample=0; sample<samplesCnt; sample++) {

		//-- sample
		for (int bar=0; bar<shape->sampleLen; bar++) {
			for (int ts=0; ts<sourceTScnt; ts++) {
				for (int tsf=0; tsf<selectedTSfeaturesCnt[ts]; tsf++) {
					tsidxS=(sample+bar)*sourceTS[ts]->featuresCnt+selectedTSfeature[ts][tsf];
					sampleSBF[dsidxS] = sourceTS[ts]->val[fromValSource][fromValStatus][tsidxS];
					dsidxS++;
				}
			}
		}

		//-- target
		if (hasTargets) {
			for (int bar=0; bar<shape->predictionLen; bar++) {
				for (int ts=0; ts<sourceTScnt; ts++) {
					for (int tsf=0; tsf<selectedTSfeaturesCnt[ts]; tsf++) {
						tsidxT=(sample+bar)*sourceTS[ts]->featuresCnt+selectedTSfeature[ts][tsf];
						tsidxT+=sourceTS[ts]->featuresCnt*shape->sampleLen;
						targetSBF[dsidxT] = sourceTS[ts]->val[fromValSource][fromValStatus][tsidxT];
						dsidxT++;
					}
				}
			}
		}
	}

}
void sDataSet::unbuild(int fromValSource, int toValSource, int toValStatus) {
	int dsidxS=0; int tsidxS=0;
	int dsidxT=0; int tsidxT=0;

	//-- for the first sample, scan all the bars in sample
	for (int bar=0; bar<shape->sampleLen; bar++) {
		for (int ts=0; ts<sourceTScnt; ts++) {
			for (int tsf=0; tsf<selectedTSfeaturesCnt[ts]; tsf++) {
				tsidxS=bar*sourceTS[ts]->featuresCnt+selectedTSfeature[ts][tsf];
				sourceTS[ts]->val[toValSource][toValStatus][tsidxS] = EMPTY_VALUE;
			}
		}
	}

	//-- target
	for (int sample=0; sample<samplesCnt; sample++) {
		for (int bar=0; bar<shape->predictionLen; bar++) {
			for (int ts=0; ts<sourceTScnt; ts++) {
				for (int tsf=0; tsf<selectedTSfeaturesCnt[ts]; tsf++) {
					tsidxT=(sample+bar)*sourceTS[ts]->featuresCnt+selectedTSfeature[ts][tsf];
					tsidxT+=sourceTS[ts]->featuresCnt*shape->sampleLen;

					sourceTS[ts]->val[toValSource][toValStatus][tsidxT] = predictionSBF[dsidxT];
					dsidxT++;
				}
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
	selectedTSfeaturesCnt=(int*)malloc(sourceTScnt*sizeof(int));
	selectedTSfeature=(int**)malloc(sourceTScnt*sizeof(int*));
	sourceTS=(sTimeSerie**)malloc(sourceTScnt*sizeof(sTimeSerie*));
	for (int t=0; t<sourceTScnt; t++) selectedTSfeature[t]=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
	dumpPath=(char*)malloc(MAX_PATH);
}
void sDataSet::setSamples() {
	if (sourceTS[0]->stepsCnt<(shape->sampleLen+shape->predictionLen)) fail("HistoryLen (%d) must be greater or equal than (SampleLen (%d) + PredictionLen (%d) )", sourceTS[0]->stepsCnt, shape->sampleLen, shape->predictionLen);
	samplesCnt=sourceTS[0]->stepsCnt-shape->sampleLen+1;
	/*if(hasTargets)*/ samplesCnt-=shape->predictionLen;
	info("stepsCnt=%d ; sampleLen=%d ; predictionLen=%d ; featuresCnt=%d ; hasTargets=%s ; samplesCnt=%d", sourceTS[0]->stepsCnt, shape->sampleLen, shape->predictionLen, shape->featuresCnt, (hasTargets) ? "TRUE" : "FALSE", samplesCnt);
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

}
void sDataSet::frees() {
	free(sampleSBF);
	free(targetSBF);
	free(predictionSBF);
	free(sampleBFS);
	free(targetBFS);
	free(predictionBFS);

}
void sDataSet::dumpPre(FILE** dumpFile) {
	int b, f, t, i;

	//-- open dumpFile
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s__dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	if (fopen_s(dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	//-- print headers
	fprintf((*dumpFile), "SampleId,");
	for (b=0; b<(shape->sampleLen); b++) {
		for (t=0; t<sourceTScnt; t++) {
			for (f=0; f<selectedTSfeaturesCnt[t]; f++) {
				fprintf((*dumpFile), "Bar%dF%d,", b, selectedTSfeature[t][f]);
			}
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(shape->predictionLen); b++) {
		for (t=0; t<sourceTScnt; t++) {
			for (f=0; f<selectedTSfeaturesCnt[t]; f++) {
				fprintf((*dumpFile), "  Target%dF%d,", b, selectedTSfeature[t][f]);
			}
		}
	}
	fprintf((*dumpFile), ",");
	for (b=0; b<(shape->predictionLen); b++) {
		for (t=0; t<sourceTScnt; t++) {
			for (f=0; f<selectedTSfeaturesCnt[t]; f++) {
				fprintf((*dumpFile), "  Prediction%dF%d,", b, selectedTSfeature[t][f]);
			}
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
void sDataSet::dump() {

	FILE* dumpFile=nullptr;
	dumpPre(&dumpFile);

	int dsidxS=0;
	int tsidxS=0;
	int dsidxT=0;
	int tsidxT=0;
	for (int sample=0; sample<samplesCnt; sample++) {

		//-- sample
		fprintf(dumpFile, "%d,", sample);
		for (int bar=0; bar<shape->sampleLen; bar++) {
			for (int ts=0; ts<sourceTScnt; ts++) {
				for (int tsf=0; tsf<selectedTSfeaturesCnt[ts]; tsf++) {
					tsidxS=(sample+bar)*sourceTS[ts]->featuresCnt+selectedTSfeature[ts][tsf];
					fprintf(dumpFile, "%f,", sampleSBF[dsidxS]);
					dsidxS++;
				}
			}
		}
		fprintf(dumpFile, "|,");

		//-- target
		if (hasTargets) {
			for (int bar=0; bar<shape->predictionLen; bar++) {
				for (int ts=0; ts<sourceTScnt; ts++) {
					for (int tsf=0; tsf<selectedTSfeaturesCnt[ts]; tsf++) {
						tsidxT=(sample+bar)*sourceTS[ts]->featuresCnt+selectedTSfeature[ts][tsf];
						tsidxT+=sourceTS[ts]->featuresCnt*shape->sampleLen;
						fprintf(dumpFile, "%f,", targetSBF[dsidxT]);
						dsidxT++;
					}
				}
			}
		}

		fprintf(dumpFile, "\n");
	}
	fclose(dumpFile);
}
