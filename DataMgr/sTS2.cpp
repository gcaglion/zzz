#include "sTS2.h"

#ifndef MAX_TS_FEATURES
#define MAX_TS_FEATURES 128
#endif

void sTS2::mallocs1() {
	timestamp=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) timestamp[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype*****)malloc(stepsCnt*sizeof(numtype****));
	valTR=(numtype*****)malloc(stepsCnt*sizeof(numtype****));
	valTRS=(numtype*****)malloc(stepsCnt*sizeof(numtype****));
	prd=(numtype*****)malloc(stepsCnt*sizeof(numtype****));
	prdTR=(numtype*****)malloc(stepsCnt*sizeof(numtype****));
	prdTRS=(numtype*****)malloc(stepsCnt*sizeof(numtype****));
	for (int s=0; s<stepsCnt; s++) {
		val[s]=(numtype****)malloc(2*sizeof(numtype***));
		valTR[s]=(numtype****)malloc(2*sizeof(numtype***));
		valTRS[s]=(numtype****)malloc(2*sizeof(numtype***));
		prd[s]=(numtype****)malloc(2*sizeof(numtype***));
		prdTR[s]=(numtype****)malloc(2*sizeof(numtype***));
		prdTRS[s]=(numtype****)malloc(2*sizeof(numtype***));
		for (int i=0; i<2; i++) {
			val[s][i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
			valTR[s][i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
			valTRS[s][i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
			prd[s][i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
			prdTR[s][i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
			prdTRS[s][i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				val[s][i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
				valTR[s][i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
				valTRS[s][i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
				prd[s][i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
				prdTR[s][i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
				prdTRS[s][i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
				for (int f=0; f<featuresCnt[i][d]; f++) {
					val[s][i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
					valTR[s][i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
					valTRS[s][i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
					prd[s][i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
					prdTR[s][i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
					prdTRS[s][i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
				}
			}
		}
	}
	timestampB=(char*)malloc(DATE_FORMAT_LEN);
	TRmin=(numtype****)malloc(2*sizeof(numtype***));
	TRmax=(numtype****)malloc(2*sizeof(numtype***));
	scaleM=(numtype****)malloc(2*sizeof(numtype***));
	scaleP=(numtype****)malloc(2*sizeof(numtype***));
	valB=(numtype****)malloc(2*sizeof(numtype***));
	for (int i=0; i<2; i++) {
		TRmin[i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
		TRmax[i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
		scaleM[i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
		scaleP[i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
		valB[i]=(numtype***)malloc(dataSourcesCnt[i]*sizeof(numtype**));
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			TRmin[i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
			TRmax[i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
			scaleM[i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
			scaleP[i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
			valB[i][d]=(numtype**)malloc(featuresCnt[i][d]*sizeof(numtype*));
			for (int f=0; f<featuresCnt[i][d]; f++) {
				TRmin[i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
				TRmax[i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
				scaleM[i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
				scaleP[i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
				valB[i][d][f]=(numtype*)malloc((WTlevel[i]+2)*sizeof(numtype));
			}
		}
	}
}
void sTS2::setDataSource(sDataSource** dataSrc_) {

	bool found=false;
	sFXDataSource* fxData;
	sGenericDataSource* fileData;
	sMT4DataSource* mtData;

	//-- first, find and set
	safecall(cfg, setKey, "File_DataSource", true, &found);	//-- ignore error
	if (found) {
		safecall(cfg, setKey, "../"); //-- get back;
		safespawn(fileData, newsname("File_DataSource"), defaultdbg, cfg, "File_DataSource");
		(*dataSrc_)=fileData;
	} else {
		safecall(cfg, setKey, "FXDB_DataSource", true, &found);	//-- ignore error
		if (found) {
			safecall(cfg, setKey, "../"); //-- get back;
			safespawn(fxData, newsname("FXDB_DataSource"), defaultdbg, cfg, "FXDB_DataSource");
			(*dataSrc_)=fxData;
		} else {
			safecall(cfg, setKey, "MT4_DataSource", true, &found);	//-- ignore error
			if (found) {
				safecall(cfg, setKey, "../"); //-- get back;
				safespawn(mtData, newsname("MT_DataSource"), defaultdbg, cfg, "MT_DataSource");
				(*dataSrc_)=mtData;
			}
		}
	}
	if (!found) fail("No Valid DataSource Parameters Key found.");

}
void sTS2::WTcalc(int i, int d, int f, numtype* dsvalSF) {
	
	//-- mallocs lfa/hfd
	numtype* lfa=(numtype*)malloc(stepsCnt*sizeof(numtype));
	//--
	numtype** hfd=(numtype**)malloc(WTlevel[i]*sizeof(numtype*));
	for (int l=0; l<WTlevel[i]; l++) hfd[l]=(numtype*)malloc(stepsCnt*sizeof(numtype));
	
	//-- extract single features from valTR
	numtype* tmpf=(numtype*)malloc(stepsCnt*sizeof(numtype));
	for (int s=0; s<stepsCnt; s++) {
		tmpf[s]=dsvalSF[s*featuresCnt[i][d]+f];
	}
	if (WTlevel[i]>0) WaweletDecomp(stepsCnt, tmpf, WTlevel[i], WTtype[i], lfa, hfd);

	for (int s=0; s<stepsCnt; s++) {
		val[s][i][d][f][0]=tmpf[s];
		if (WTlevel[i]>0) {
			val[s][i][d][f][1]=lfa[s];
			for (int l=0; l<WTlevel[i]; l++) val[s][i][d][f][l+2]=hfd[l][s];
		}
	}


	free(tmpf);
	for (int l=0; l<WTlevel[i]; l++) free(hfd[l]);
	free(lfa); free(hfd);
}
void sTS2::transform(int i, int d, int f, int l) {

	TRmin[i][d][f][l]=1e9; TRmax[i][d][f][l]=-1e9;

	for (int s=0; s<(stepsCnt); s++) {
		switch (dt) {
		case DT_NONE:
			valTR[s][i][d][f][l]=val[s][i][d][f][l];
			break;
		case DT_DELTA:
			if (val[s][i][d][f][l]==EMPTY_VALUE) {
				valTR[s][i][d][f][l]=EMPTY_VALUE;
			} else {
				if (s==0) {
					valTR[s][i][d][f][l]=val[s][i][d][f][l]-valB[i][d][f][l];
				} else {
					valTR[s][i][d][f][l]=val[s][i][d][f][l]-val[s-1][i][d][f][l];
				}
			}
			break;
		case DT_LOG:
			if (val[s][i][d][f][l]==EMPTY_VALUE) {
				valTR[s][i][d][f][l]=EMPTY_VALUE;
			} else {
				valTR[s][i][d][f][l]=log(val[s][i][d][f][l]);
			}
			break;
		default:
			break;
		}

		//-- min/max calc
		if (valTR[s][i][d][f][l]!=EMPTY_VALUE&&valTR[s][i][d][f][l]<TRmin[i][d][f][l]) TRmin[i][d][f][l]=valTR[s][i][d][f][l];
		if (valTR[s][i][d][f][l]!=EMPTY_VALUE&&valTR[s][i][d][f][l]>TRmax[i][d][f][l]) TRmax[i][d][f][l]=valTR[s][i][d][f][l];
	}
}
void sTS2::scale(float scaleMin_, float scaleMax_) {

	for (int i=0; i<2; i++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				for (int l=0; l<(WTlevel[i]+2); l++) {
					scaleM[i][d][f][l] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(TRmax[i][d][f][l]-TRmin[i][d][f][l]));
					scaleP[i][d][f][l] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[i][d][f][l]*TRmax[i][d][f][l]);
				}
			}
		}
	}

	for (int s=0; s<stepsCnt; s++) {
		for (int i=0; i<2; i++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					for (int l=0; l<(WTlevel[i]+2); l++) {
						valTRS[s][i][d][f][l]=(valTRS[s][i][d][f][l]==EMPTY_VALUE) ? EMPTY_VALUE : valTR[s][i][d][f][l]*scaleM[i][d][f][l]+scaleP[i][d][f][l];
					}
				}
			}
		}
	}

	if (doDump) dump();
}
void sTS2::unscale() {
	for (int s=0; s<stepsCnt; s++) {
		for (int i=0; i<2; i++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					for (int l=0; l<(WTlevel[i]+2); l++) {
						prdTR[s][i][d][f][l]=(prdTRS[s][i][d][f][l]==EMPTY_VALUE) ? EMPTY_VALUE : (prdTRS[s][i][d][f][l]-scaleP[i][d][f][l])/scaleM[i][d][f][l];
					}
				}
			}
		}
	}
}
void sTS2::untransform() {

	for (int s=0; s<stepsCnt; s++) {
		for (int i=0; i<2; i++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					for (int l=0; l<(WTlevel[i]+2); l++) {
						if (dt==DT_NONE) {
							prd[s][i][d][f][l]=prdTR[s][i][d][f][l];
						}
						if (dt==DT_DELTA) {
							if (s>0) {
								if (prdTR[s][i][d][f][l]==EMPTY_VALUE) {
									prd[s][i][d][f][l]=EMPTY_VALUE;
								} else {
									prd[s][i][d][f][l]=prdTR[s][i][d][f][l]+val[s-1][i][d][f][l];
									if (val[s][i][d][f][l]==EMPTY_VALUE) val[s][i][d][f][l]=prd[s][i][d][f][l];
								}
							} else {
								if (prdTR[s][i][d][f][l]==EMPTY_VALUE) {
									prd[s][i][d][f][l]=EMPTY_VALUE;
								} else {
									prd[s][i][d][f][l]=prdTR[s][i][d][f][l]+valB[i][d][f][l];
								}
							}
						}
						
						if (dt==DT_LOG) {
							if (prdTR[s][i][d][f][l]==EMPTY_VALUE) {
								prd[s][i][d][f][l]=EMPTY_VALUE;
							} else {
								prd[s][i][d][f][l]=exp(prdTR[s][i][d][f][l]);
							}
						}

					}
				}
			}
		}
	}
}

void sTS2::dumpToFile(FILE* file, int i, numtype***** val_) {
	int s, d, f, l;
	fprintf(file, "i, datetime");
	for (d=0; d<dataSourcesCnt[i]; d++) {
		for (f=0; f<featuresCnt[i][d]; f++) {
			for (l=0; l<(WTlevel[i]+2); l++) {
				fprintf(file, ",D%dF%dL%d", d, f, l);
			}
		}
	}
	fprintf(file, "\n%d,%s", -1, timestampB);
	for (d=0; d<dataSourcesCnt[i]; d++) {
		for (f=0; f<featuresCnt[i][d]; f++) {
			for (l=0; l<(WTlevel[i]+2); l++) {
				fprintf(file, ",%f", valB[i][d][f][l]);
			}
		}
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(file, "\n%d, %s", s, timestamp[s]);
		for (d=0; d<dataSourcesCnt[i]; d++) {
			for (f=0; f<featuresCnt[i][d]; f++) {
				for (l=0; l<(WTlevel[i]+2); l++) {
					fprintf(file, ",%f", val_[s][i][d][f][l]);
				}
			}
		}
	}

	fprintf(file, "\n\n");
	fprintf(file, "Min: ,");
	for (d=0; d<dataSourcesCnt[i]; d++) {
		for (f=0; f<featuresCnt[i][d]; f++) {
			for (l=0; l<(WTlevel[i]+2); l++) {
				fprintf(file, ",%f", TRmin[i][d][f][l]);
			}
		}
	}
	fprintf(file, "\n");
	fprintf(file, "Max: ,");
	for (d=0; d<dataSourcesCnt[i]; d++) {
		for (f=0; f<featuresCnt[i][d]; f++) {
			for (l=0; l<(WTlevel[i]+2); l++) {
				fprintf(file, ",%f", TRmax[i][d][f][l]);
			}
		}
	}


	fclose(file);
}
void sTS2::dump(bool predicted) {
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_IN_ACT_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileINACTBASE;
	if (fopen_s(&dumpFileINACTBASE, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_IN_ACT_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileINACTTR;
	if (fopen_s(&dumpFileINACTTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_IN_ACT_TRS_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileINACTTRS;
	if (fopen_s(&dumpFileINACTTRS, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_IN_PRD_TRS_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileINPRDTRS;
	if (fopen_s(&dumpFileINPRDTRS, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_IN_PRD_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileINPRDTR;
	if (fopen_s(&dumpFileINPRDTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_IN_PRD_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileINPRDBASE;
	if (fopen_s(&dumpFileINPRDBASE, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	sprintf_s(dumpFileName, "%s/%s_OUT_ACT_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTACTBASE;
	if (fopen_s(&dumpFileOUTACTBASE, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_OUT_ACT_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTACTTR;
	if (fopen_s(&dumpFileOUTACTTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_OUT_PRD_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTPRDBASE;
	if (fopen_s(&dumpFileOUTPRDBASE, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_OUT_PRD_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTPRDTR;
	if (fopen_s(&dumpFileOUTPRDTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	dumpToFile(dumpFileINACTBASE, 0, val);
	dumpToFile(dumpFileINACTTR, 0, valTR);
	dumpToFile(dumpFileINACTTRS, 0, valTRS);
	dumpToFile(dumpFileINPRDTRS, 0, prdTRS);
	dumpToFile(dumpFileINPRDTR, 0, prdTR);
	dumpToFile(dumpFileINPRDBASE, 0, prd);
	dumpToFile(dumpFileOUTACTBASE, 1, val);
	dumpToFile(dumpFileOUTACTTR, 1, valTR);
	dumpToFile(dumpFileOUTPRDBASE, 1, prd);
	dumpToFile(dumpFileOUTPRDTR, 1, prdTR);
}

void sTS2::getDataSet(int sampleLen_, int targetLen_, int* oSamplesCnt, int* oInputCnt, int* oOutputCnt, numtype** oSample, numtype** oTarget, numtype** oPrediction) {
	(*oSamplesCnt)=stepsCnt-sampleLen_-targetLen_+1;

	
	(*oInputCnt)=0;
	for (int bar=0; bar<sampleLen_; bar++) {
		for (int d=0; d<dataSourcesCnt[0]; d++) {
			for (int f=0; f<featuresCnt[0][d]; f++) {
				for (int l=0; l<(WTlevel[0]+2); l++) {
					(*oInputCnt)++;
				}
			}
		}
	}

	(*oSample)=(numtype*)malloc((*oInputCnt)*(*oSamplesCnt)*sizeof(numtype));

	int dsidx=0;
	for (int s=0; s<(*oSamplesCnt); s++) {
		for (int bar=0; bar<sampleLen_; bar++) {
			for (int d=0; d<dataSourcesCnt[0]; d++) {
				for (int f=0; f<featuresCnt[0][d]; f++) {
					for (int l=0; l<(WTlevel[0]+2); l++) {
						(*oSample)[dsidx] = valTRS[s+bar][0][d][f][l];
						dsidx++;
					}
				}
			}
		}
	}

	(*oOutputCnt)=0;
	for (int bar=0; bar<targetLen_; bar++) {
		for (int d=0; d<dataSourcesCnt[0]; d++) {
			for (int f=0; f<featuresCnt[0][d]; f++) {
				for (int l=0; l<(WTlevel[0]+2); l++) {
					(*oOutputCnt)++;
				}
			}
		}
	}

	(*oTarget)=(numtype*)malloc((*oOutputCnt)*(*oSamplesCnt)*sizeof(numtype));

	dsidx=0;
	for (int s=0; s<(*oSamplesCnt); s++) {
		for (int bar=0; bar<targetLen_; bar++) {
			for (int d=0; d<dataSourcesCnt[1]; d++) {
				for (int f=0; f<featuresCnt[1][d]; f++) {
					for (int l=0; l<(WTlevel[1]+2); l++) {
						(*oTarget)[dsidx] = valTRS[s+bar][1][d][f][l];
						dsidx++;
					}
				}
			}
		}
	}

	(*oPrediction)=(numtype*)malloc((*oOutputCnt)*(*oSamplesCnt)*sizeof(numtype));

}

sTS2::sTS2(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen");
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &doDump, "Dump");
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	char* _dumpPath=&dumpPath[0];
	safecall(cfgKey, getParm, &_dumpPath, "DumpPath", true);

	char _date0[DATE_FORMAT_LEN]; char* _date0p=&_date0[0];
	safecall(cfgKey, getParm, &_date0p, "Date0");

	WTtype=(int*)malloc(2*sizeof(int));
	WTlevel=(int*)malloc(2*sizeof(int));
	dataSourcesCnt=(int*)malloc(2*sizeof(int));
	sDataSource*** dsrc=(sDataSource***)malloc(2*sizeof(sDataSource**));
	featuresCnt=(int**)malloc(2*sizeof(int*));
	int*** selF=(int***)malloc(2*sizeof(int**));

	for (int i=0; i<2; i++) {	
		safecall(cfg, setKey, strBuild("%sput", (i>0) ? "Out" : "In").c_str());
		safecall(cfg->currentKey, getParm, &dataSourcesCnt[i], "DataSourcesCount");
		dsrc[i]=(sDataSource**)malloc(dataSourcesCnt[i]*sizeof(sDataSource*));
		featuresCnt[i]=(int*)malloc(dataSourcesCnt[i]*sizeof(int));
		selF[i]=(int**)malloc(dataSourcesCnt[i]*sizeof(int*));
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			selF[i][d]=(int*)malloc(MAX_TS_FEATURES*sizeof(int));
			safecall(cfg, setKey, strBuild("DataSource%d", d).c_str());
			setDataSource(&dsrc[i][d]);
			safecall(cfg->currentKey, getParm, &selF[i][d], "SelectedFeatures", false, &featuresCnt[i][d]);
			safecall(cfg, setKey, "../");
		}
		safecall(cfg->currentKey, getParm, &WTtype[i], "WaveletType");
		safecall(cfg->currentKey, getParm, &WTlevel[i], "DecompLevel");
		if (WTtype[i]==WT_NONE) WTlevel[i]=-1;
		safecall(cfg, setKey, "../");
	}

	mallocs1();

	//-- load all data sources
	char** tmptime=(char**)malloc(stepsCnt*sizeof(char*));
	for (int i=0; i<stepsCnt; i++) {
		tmptime[i]=(char*)malloc(DATE_FORMAT_LEN); tmptime[i][0]='\0';
	}
	char* tmptimeB=(char*)malloc(DATE_FORMAT_LEN); tmptimeB[0]='\0';
	numtype* tmpval;
	numtype* tmpvalB;
	numtype* tmpvalx;
	numtype* tmpvalBx;
	numtype* tmpbw;

	//-- load datasources
	for (int i=0; i<2; i++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			tmpval=(numtype*)malloc(stepsCnt*dsrc[i][d]->featuresCnt*sizeof(numtype));
			tmpvalB=(numtype*)malloc(dsrc[i][d]->featuresCnt*sizeof(numtype));
			tmpvalx=(numtype*)malloc(stepsCnt*featuresCnt[i][d]*sizeof(numtype));
			tmpvalBx=(numtype*)malloc(featuresCnt[i][d]*sizeof(numtype));
			tmpbw=(numtype*)malloc(stepsCnt*dsrc[i][d]->featuresCnt*sizeof(numtype));
			safecall(dsrc[i][d], load, _date0, stepsCnt, tmptime, tmpval, tmptimeB, tmpvalB, tmpbw);

			//-- set timestamps
			for (int s=0; s<stepsCnt; s++) strcpy_s(timestamp[s], DATE_FORMAT_LEN, tmptime[s]);
			strcpy_s(timestampB, DATE_FORMAT_LEN, tmptimeB);

			//-- extract selected features in tmpvalx
			for (int f=0; f<featuresCnt[i][d]; f++) {
				for (int s=0; s<stepsCnt; s++) {
					for (int df=0; df<dsrc[i][d]->featuresCnt; df++) {
						if (selF[i][d][f]==df) {
							tmpvalx[s*featuresCnt[i][d]+f]=tmpval[s*dsrc[i][d]->featuresCnt+selF[i][d][f]];
						}
					}
				}
			}

			for (int f=0; f<featuresCnt[i][d]; f++) {
				//-- FFTcalc for each feature. Also sets original value at position 0
				WTcalc(i, d, f, tmpvalx);
				//-- base values for each feature. only for original values
				valB[i][d][f][0]=tmpvalB[selF[i][d][f]];
				//-- base values for each level. we don't have it, so we set it equal to the first value of the level serie
				for (int l=1; l<(WTlevel[i]+2); l++) {
					valB[i][d][f][l]=val[0][i][d][f][l];
				}
				//-- transform for each feature/level.
				for (int l=0; l<(WTlevel[i]+2); l++) transform(i, d, f, l);
			}
			free(tmpval); free(tmpvalB); free(tmpbw);
			free(tmpvalx); free(tmpvalBx);
		}
	}
	for (int i=0; i<stepsCnt; i++) free(tmptime[i]);
	free(tmptime); free(tmptimeB);
	free(dsrc);

	if (doDump) dump();


}
sTS2::~sTS2() {
	for (int i=0; i<2; i++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				free(TRmin[i][d][f]); free(TRmax[i][d][f]); free(scaleM[i][d][f]); free(scaleP[i][d][f]); free(valB[i][d][f]);
			}
			free(TRmin[i][d]); free(TRmax[i][d]); free(scaleM[i][d]); free(scaleP[i][d]); free(valB[i][d]);
		}
		free(TRmin[i]); free(TRmax[i]); free(scaleM[i]); free(scaleP[i]); free(valB[i]);
	}
	free(TRmin); free(TRmax); free(scaleM); free(scaleP); free(valB);
	free(timestampB);

	for (int s=0; s<stepsCnt; s++) {
		for (int i=0; i<2; i++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					free(val[s][i][d][f]); free(valTR[s][i][d][f]); free(valTRS[s][i][d][f]);
					free(prd[s][i][d][f]); free(prdTR[s][i][d][f]); free(prdTRS[s][i][d][f]);
				}
				free(val[s][i][d]); free(valTR[s][i][d]); free(valTRS[s][i][d]);
				free(prd[s][i][d]); free(prdTR[s][i][d]); free(prdTRS[s][i][d]);
			}
			free(val[s][i]); free(valTR[s][i]); free(valTRS[s][i]);
			free(prd[s][i]); free(prdTR[s][i]); free(prdTRS[s][i]);
		}
		free(val[s]); free(valTR[s]); free(valTRS[s]);
		free(prd[s]); free(prdTR[s]); free(prdTRS[s]);
	}
	free(val); free(valTR); free(valTRS);
	free(prd); free(prdTR); free(prdTRS);
	for (int i=0; i<stepsCnt; i++) free(timestamp[i]); free(timestamp);
}