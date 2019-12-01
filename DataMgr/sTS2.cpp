#include "sTS2.h"
//#include <vld.h>

#ifndef MAX_TS_FEATURES
#define MAX_TS_FEATURES 128
#endif

void sTS2::mallocs1() {
	timestamp=(char***)malloc((stepsCnt+targetLen)*sizeof(char**));
	for (int s=0; s<(stepsCnt+targetLen); s++) {
		timestamp[s]=(char**)malloc(2*sizeof(char*));
		for (int i=0; i<2; i++) {
			timestamp[s][i]=(char*)malloc(DATE_FORMAT_LEN);
		}
	}
	for (int s=stepsCnt; s<(stepsCnt+targetLen); s++) {
		for (int i=0; i<2; i++) sprintf_s(timestamp[s][i], DATE_FORMAT_LEN, "9999-99-99-99:%02d", s);
	}
	val=(numtype*****)malloc((stepsCnt+targetLen)*sizeof(numtype****));
	valTR=(numtype*****)malloc((stepsCnt+targetLen)*sizeof(numtype****));
	valTRS=(numtype*****)malloc((stepsCnt+targetLen)*sizeof(numtype****));
	prd=(numtype*****)malloc((stepsCnt+targetLen)*sizeof(numtype****));
	prdTR=(numtype*****)malloc((stepsCnt+targetLen)*sizeof(numtype****));
	prdTRS=(numtype*****)malloc((stepsCnt+targetLen)*sizeof(numtype****));

	for (int s=0; s<(stepsCnt+targetLen); s++) {
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

					if (s>=stepsCnt) {
						for (int l=0; l<(WTlevel[i]+2); l++) {
							val[s][i][d][f][l]=EMPTY_VALUE;
							valTR[s][i][d][f][l]=EMPTY_VALUE;
							valTRS[s][i][d][f][l]=EMPTY_VALUE;
						}
					}

				}
			}
		}
	}
	timestampB=(char**)malloc(2*sizeof(char*)); for(int i=0; i<2; i++) timestampB[i]= (char*)malloc(DATE_FORMAT_LEN);
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
	
	//-- extract single features from dsvalSF
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
		switch (dt[i][d][f]) {
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
	for (int s=0; s<(stepsCnt+targetLen); s++) {
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

	for (int s=0; s<(stepsCnt+targetLen); s++) {
		for (int i=0; i<2; i++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					for (int l=0; l<(WTlevel[i]+2); l++) {
						if (dt[i][d][f]==DT_NONE) {
							prd[s][i][d][f][l]=prdTR[s][i][d][f][l];
						}
						if (dt[i][d][f]==DT_DELTA) {
							if (s>0) {								
								if (prdTR[s][i][d][f][l]==EMPTY_VALUE) {
									prd[s][i][d][f][l]=EMPTY_VALUE;
								} else {
									if (s<=stepsCnt) {
										prd[s][i][d][f][l]=prdTR[s][i][d][f][l]+val[s-1][i][d][f][l];
										//if (val[s][i][d][f][l]==EMPTY_VALUE) val[s][i][d][f][l]=prd[s][i][d][f][l];
									} else {
										if (prd[s-1][i][d][f][l]==EMPTY_VALUE) {
											prd[s][i][d][f][l]=prdTR[s][i][d][f][l]+val[stepsCnt-1][i][d][f][l];
										} else {
											prd[s][i][d][f][l]=prdTR[s][i][d][f][l]+prd[s-1][i][d][f][l];
										}
									}
								}
							} else {
								if (prdTR[s][i][d][f][l]==EMPTY_VALUE) {
									prd[s][i][d][f][l]=EMPTY_VALUE;
								} else {
									prd[s][i][d][f][l]=prdTR[s][i][d][f][l]+valB[i][d][f][l];
								}
							}
						}
						
						if (dt[i][d][f]==DT_LOG) {
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
void sTS2::invert() {

	int totFeaturesCnt[2]={ 0,0 };
	numtype* _val[2]; numtype* _valTR[2]; numtype* _valTRS[2];
	numtype* _prd[2]; numtype* _prdTR[2]; numtype* _prdTRS[2];
	int idx;

	for (int i=0; i<2; i++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				for (int l=0; l<(WTlevel[i]+2); l++) {
					totFeaturesCnt[i]++;
				}
			}
		}
		idx=0;
		_val[i]=(numtype*)malloc(stepsCnt*totFeaturesCnt[i]*sizeof(numtype));
		_valTR[i]=(numtype*)malloc(stepsCnt*totFeaturesCnt[i]*sizeof(numtype));
		_valTRS[i]=(numtype*)malloc(stepsCnt*totFeaturesCnt[i]*sizeof(numtype));
		_prd[i]=(numtype*)malloc(stepsCnt*totFeaturesCnt[i]*sizeof(numtype));
		_prdTR[i]=(numtype*)malloc(stepsCnt*totFeaturesCnt[i]*sizeof(numtype));
		_prdTRS[i]=(numtype*)malloc(stepsCnt*totFeaturesCnt[i]*sizeof(numtype));
		for (int s=0; s<stepsCnt; s++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					for (int l=0; l<(WTlevel[i]+2); l++) {
						_val[i][idx]=val[stepsCnt-1-s][i][d][f][l];
						_valTR[i][idx]=valTR[stepsCnt-1-s][i][d][f][l];
						_valTRS[i][idx]=valTRS[stepsCnt-1-s][i][d][f][l];
						_prd[i][idx]=prd[stepsCnt-1-s][i][d][f][l];
						_prdTR[i][idx]=prdTR[stepsCnt-1-s][i][d][f][l];
						_prdTRS[i][idx]=prdTRS[stepsCnt-1-s][i][d][f][l];
						idx++;
					}
				}
			}
		}
		idx=0;
		for (int s=0; s<stepsCnt; s++) {
			for (int d=0; d<dataSourcesCnt[i]; d++) {
				for (int f=0; f<featuresCnt[i][d]; f++) {
					for (int l=0; l<(WTlevel[i]+2); l++) {
						val[s][i][d][f][l]=_val[i][idx];
						valTR[s][i][d][f][l]=_valTR[i][idx];
						valTRS[s][i][d][f][l]=_valTRS[i][idx];
						prd[s][i][d][f][l]=_prd[i][idx];
						prdTR[s][i][d][f][l]=_prdTR[i][idx];
						prdTRS[s][i][d][f][l]=_prdTRS[i][idx];
						idx++;
					}
				}
			}
		}
		free(_val[i]); free(_valTR[i]); free(_valTRS[i]);
		free(_prd[i]); free(_prdTR[i]); free(_prdTRS[i]);
	}
	
}
void sTS2::invertDS() {

	numtype* _sample=(numtype*)malloc(samplesCnt*inputCnt*sizeof(numtype));
	numtype* _target=(numtype*)malloc(samplesCnt*outputCnt*sizeof(numtype));
	numtype* _prediction=(numtype*)malloc(samplesCnt*outputCnt*sizeof(numtype));
	numtype* _sampleTRS=(numtype*)malloc(samplesCnt*inputCnt*sizeof(numtype));
	numtype* _targetTRS=(numtype*)malloc(samplesCnt*outputCnt*sizeof(numtype));
	numtype* _predictionTRS=(numtype*)malloc(samplesCnt*outputCnt*sizeof(numtype));
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<inputCnt; b++) {
			_sample[s*inputCnt+b]=sample[(samplesCnt-s-1)*inputCnt+b];
			_sampleTRS[s*inputCnt+b]=sampleTRS[(samplesCnt-s-1)*inputCnt+b];
		}
		for (int b=0; b<outputCnt; b++) {
			_target[s*outputCnt+b]=target[(samplesCnt-s-1)*outputCnt+b];
			_targetTRS[s*outputCnt+b]=targetTRS[(samplesCnt-s-1)*outputCnt+b];
		}
		for (int b=0; b<outputCnt; b++) {
			_prediction[s*outputCnt+b]=prediction[(samplesCnt-s-1)*outputCnt+b];
			_predictionTRS[s*outputCnt+b]=predictionTRS[(samplesCnt-s-1)*outputCnt+b];
		}
	}
	memcpy_s(sample, samplesCnt*inputCnt*sizeof(numtype), _sample, samplesCnt*inputCnt*sizeof(numtype));
	memcpy_s(target, samplesCnt*outputCnt*sizeof(numtype), _target, samplesCnt*outputCnt*sizeof(numtype));
	memcpy_s(prediction, samplesCnt*outputCnt*sizeof(numtype), _prediction, samplesCnt*outputCnt*sizeof(numtype));
	memcpy_s(sampleTRS, samplesCnt*inputCnt*sizeof(numtype), _sampleTRS, samplesCnt*inputCnt*sizeof(numtype));
	memcpy_s(targetTRS, samplesCnt*outputCnt*sizeof(numtype), _targetTRS, samplesCnt*outputCnt*sizeof(numtype));
	memcpy_s(predictionTRS, samplesCnt*outputCnt*sizeof(numtype), _predictionTRS, samplesCnt*outputCnt*sizeof(numtype));

	free(_sample); free(_target); free(_prediction);
	free(_sampleTRS); free(_targetTRS); free(_predictionTRS);
}
void sTS2::slide(int steps_) {
	for (int x=0; x<steps_; x++) {
		for (int s=0; s<(stepsCnt-1); s++) {
			for (int i=0; i<2; i++) {
				for (int d=0; d<dataSourcesCnt[i]; d++) {
					for (int f=0; f<featuresCnt[i][d]; f++) {
						for (int l=0; l<(WTlevel[i]+2); l++) {
							val[s][i][d][f][l]=val[s+1][i][d][f][l];
							valTR[s][i][d][f][l]=valTR[s+1][i][d][f][l];
							valTRS[s][i][d][f][l]=valTRS[s+1][i][d][f][l];
							prd[s][i][d][f][l]=prd[s+1][i][d][f][l];
							prdTR[s][i][d][f][l]=prdTR[s+1][i][d][f][l];
							prdTRS[s][i][d][f][l]=prdTRS[s+1][i][d][f][l];
						}
					}
				}
			}
		}
	}
}
void sTS2::slideDS(int steps_) {
	for (int r=0; r<steps_; r++) {
		for (int s=0; s<(samplesCnt-1); s++) {
			for (int i=0; i<inputCnt; i++) {
				sample[s*inputCnt+i]=sample[(s+1)*inputCnt+i];
				sampleTRS[s*inputCnt+i]=sampleTRS[(s+1)*inputCnt+i];
			}
			for (int o=0; o<outputCnt; o++) {
				target[s*outputCnt+o]=target[(s+1)*outputCnt+o];
				prediction[s*outputCnt+o]=prediction[(s+1)*outputCnt+o];
				targetTRS[s*outputCnt+o]=targetTRS[(s+1)*outputCnt+o];
				predictionTRS[s*outputCnt+o]=predictionTRS[(s+1)*outputCnt+o];
			}
		}
	}
}

void sTS2::dumpToFile(FILE* file, int i, bool predicted, numtype***** val_) {
	int s, d, f, l;
	fprintf(file, "i, datetime");
	for (d=0; d<dataSourcesCnt[i]; d++) {
		for (f=0; f<featuresCnt[i][d]; f++) {
			for (l=0; l<(WTlevel[i]+2); l++) {
				fprintf(file, ",D%dF%dL%d", d, f, l);
			}
		}
	}
	fprintf(file, "\n%d,%s", -1, timestampB[i]);
	for (d=0; d<dataSourcesCnt[i]; d++) {
		for (f=0; f<featuresCnt[i][d]; f++) {
			for (l=0; l<(WTlevel[i]+2); l++) {
				fprintf(file, ",%f", valB[i][d][f][l]);
			}
		}
	}

	for (s=0; s<(stepsCnt+((predicted)?targetLen:0)); s++) {
		fprintf(file, "\n%d, %s", s, timestamp[s][i]);
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
void sTS2::dump() {
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
	sprintf_s(dumpFileName, "%s/%s_OUT_ACT_TRS_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTACTTRS;
	if (fopen_s(&dumpFileOUTACTTRS, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_OUT_PRD_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTPRDBASE;
	if (fopen_s(&dumpFileOUTPRDBASE, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_OUT_PRD_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTPRDTR;
	if (fopen_s(&dumpFileOUTPRDTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_OUT_PRD_TRS_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileOUTPRDTRS;
	if (fopen_s(&dumpFileOUTPRDTRS, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	dumpToFile(dumpFileINACTBASE, 0, false, val);
	dumpToFile(dumpFileINACTTR, 0, false, valTR);
	dumpToFile(dumpFileINACTTRS, 0, false, valTRS);
	dumpToFile(dumpFileINPRDTRS, 0, true, prdTRS);
	dumpToFile(dumpFileINPRDTR, 0, true, prdTR);
	dumpToFile(dumpFileINPRDBASE, 0, true, prd);
	dumpToFile(dumpFileOUTACTBASE, 1, false, val);
	dumpToFile(dumpFileOUTACTTR, 1, false, valTR);
	dumpToFile(dumpFileOUTACTTRS, 1, false, valTRS);
	dumpToFile(dumpFileOUTPRDBASE, 1, true, prd);
	dumpToFile(dumpFileOUTPRDTR, 1, true, prdTR);
	dumpToFile(dumpFileOUTPRDTRS, 1, true, prdTRS);
}

void sTS2::_dumpDS(FILE* file, numtype* prs, numtype* prt, numtype* prp) {

	fprintf(file, "SampleId");
	for (int bar=0; bar<sampleLen; bar++) {
		for (int d=0; d<dataSourcesCnt[0]; d++) {
			for (int f=0; f<featuresCnt[0][d]; f++) {
				for (int l=0; l<(WTlevel[0]+2); l++) {
					fprintf(file, ",B%dD%dF%dL%d", bar, d, f, l);
				}
			}
		}
	}
	fprintf(file, ",");
	for (int bar=0; bar<targetLen; bar++) {
		for (int d=0; d<dataSourcesCnt[1]; d++) {
			for (int f=0; f<featuresCnt[1][d]; f++) {
				for (int l=0; l<(WTlevel[1]+2); l++) {
					fprintf(file, ",B%dD%dF%dL%d", bar, d, f, l);
				}
			}
		}
	}
	fprintf(file, ",");
	for (int bar=0; bar<targetLen; bar++) {
		for (int d=0; d<dataSourcesCnt[1]; d++) {
			for (int f=0; f<featuresCnt[1][d]; f++) {
				for (int l=0; l<(WTlevel[1]+2); l++) {
					fprintf(file, ",B%dD%dF%dL%d", bar, d, f, l);
				}
			}
		}
	}
	int sidx=0, tidx=0, pidx=0;
	for (int s=0; s<samplesCnt; s++) {
		fprintf(file, "\n%d", s);
		for (int bar=0; bar<sampleLen; bar++) {
			for (int d=0; d<dataSourcesCnt[0]; d++) {
				for (int f=0; f<featuresCnt[0][d]; f++) {
					for (int l=0; l<(WTlevel[0]+2); l++) {
						fprintf(file, ",%f", prs[sidx]);
						sidx++;
					}
				}
			}
		}
		fprintf(file, ",");
		for (int bar=0; bar<targetLen; bar++) {
			for (int d=0; d<dataSourcesCnt[1]; d++) {
				for (int f=0; f<featuresCnt[1][d]; f++) {
					for (int l=0; l<(WTlevel[1]+2); l++) {
						fprintf(file, ",%f", prt[tidx]);
						tidx++;
					}
				}
			}
		}
		fprintf(file, ",");
		for (int bar=0; bar<targetLen; bar++) {
			for (int d=0; d<dataSourcesCnt[1]; d++) {
				for (int f=0; f<featuresCnt[1][d]; f++) {
					for (int l=0; l<(WTlevel[1]+2); l++) {
						fprintf(file, ",%f", prp[pidx]);
						pidx++;
					}
				}
			}
		}

	}
}
void sTS2::dumpDS() {
	char fdsname[MAX_PATH];	FILE* fds;
	
	sprintf_s(fdsname, "%s/dataSet_BASE_%p.csv", dumpPath, this);
	fopen_s(&fds, fdsname, "w");
	_dumpDS(fds, sample, target, prediction);
	fclose(fds);
	sprintf_s(fdsname, "%s/dataSet_TRS_%p.csv", dumpPath, this);
	fopen_s(&fds, fdsname, "w");
	_dumpDS(fds, sampleTRS, targetTRS, predictionTRS);
	fclose(fds);
}
void sTS2::buildDataSet() {
	samplesCnt=stepsCnt-sampleLen+1;
		
	inputCnt=0;
	for (int bar=0; bar<sampleLen; bar++) {
		for (int d=0; d<dataSourcesCnt[0]; d++) {
			for (int f=0; f<featuresCnt[0][d]; f++) {
				for (int l=0; l<(WTlevel[0]+2); l++) {
					inputCnt++;
				}
			}
		}
	}

	sample=(numtype*)malloc(inputCnt*samplesCnt*sizeof(numtype));
	sampleTRS=(numtype*)malloc(inputCnt*samplesCnt*sizeof(numtype));

	int dsidx=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int bar=0; bar<sampleLen; bar++) {
			for (int d=0; d<dataSourcesCnt[0]; d++) {
				for (int f=0; f<featuresCnt[0][d]; f++) {
					for (int l=0; l<(WTlevel[0]+2); l++) {
						sample[dsidx] = val[s+bar][0][d][f][l];
						sampleTRS[dsidx] = valTRS[s+bar][0][d][f][l];
						dsidx++;
					}
				}
			}
		}
	}

	outputCnt=0;
	for (int bar=0; bar<targetLen; bar++) {
		for (int d=0; d<dataSourcesCnt[1]; d++) {
			for (int f=0; f<featuresCnt[1][d]; f++) {
				for (int l=0; l<(WTlevel[1]+2); l++) {
					outputCnt++;
				}
			}
		}
	}

	target=(numtype*)malloc(outputCnt*samplesCnt*sizeof(numtype));
	targetTRS=(numtype*)malloc(outputCnt*samplesCnt*sizeof(numtype));

	dsidx=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int bar=0; bar<targetLen; bar++) {
			for (int d=0; d<dataSourcesCnt[1]; d++) {
				for (int f=0; f<featuresCnt[1][d]; f++) {
					for (int l=0; l<(WTlevel[1]+2); l++) {
						target[dsidx] = (s>=(samplesCnt-targetLen)) ? EMPTY_VALUE : val[s+sampleLen+bar][1][d][f][l];
						targetTRS[dsidx] = (s>=(samplesCnt-targetLen)) ? EMPTY_VALUE : valTRS[s+sampleLen+bar][1][d][f][l];
						dsidx++;
					}
				}
			}
		}
	}

	prediction=(numtype*)malloc(outputCnt*samplesCnt*sizeof(numtype));
	predictionTRS=(numtype*)malloc(outputCnt*samplesCnt*sizeof(numtype));

	//-- Print
	if (doDump) dumpDS();
	
}
void sTS2::getPrediction() {
	int s, b;

	for (b=0; b<sampleLen; b++) {
		for (int d=0; d<dataSourcesCnt[1]; d++) {
			for (int f=0; f<featuresCnt[1][d]; f++) {
				for (int l=0; l<(WTlevel[1]+2); l++) {
					prdTRS[b][1][d][f][l]=EMPTY_VALUE;
				}
			}
		}
	}

	for (s=0; s<samplesCnt; s++) {
		for (b=0; b<1; b++) {
			for (int d=0; d<dataSourcesCnt[1]; d++) {
				for (int f=0; f<featuresCnt[1][d]; f++) {
					for (int l=0; l<(WTlevel[1]+2); l++) {
						prdTRS[s+sampleLen][1][d][f][l]=predictionTRS[(s)*outputCnt +b*dataSourcesCnt[1]*featuresCnt[1][d]*(WTlevel[1]+2) +d*featuresCnt[1][d]*(WTlevel[1]+2) +f*(WTlevel[1]+2) +l];
					}
				}
			}
		}
	}


	for (b=0; b<targetLen; b++) {
		for (int d=0; d<dataSourcesCnt[1]; d++) {
			for (int f=0; f<featuresCnt[1][d]; f++) {
				for (int l=0; l<(WTlevel[1]+2); l++) {
					prdTRS[stepsCnt+b][1][d][f][l]=predictionTRS[(samplesCnt-1)*outputCnt +b*dataSourcesCnt[1]*featuresCnt[1][d]*(WTlevel[1]+2) +d*featuresCnt[1][d]*(WTlevel[1]+2) +f*(WTlevel[1]+2) +l];
				}
			}
		}
	}
}

sTS2::sTS2(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen");
	safecall(cfgKey, getParm, &doDump, "Dump");
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	char* _dumpPath=&dumpPath[0];
	safecall(cfgKey, getParm, &_dumpPath, "DumpPath", true);

	safecall(cfgKey, getParm, &sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &targetLen, "TargetLen");
	safecall(cfgKey, getParm, &batchSize, "BatchSize");

	char _date0[DATE_FORMAT_LEN]; char* _date0p=&_date0[0];
	safecall(cfgKey, getParm, &_date0p, "Date0");

	safecall(cfgKey, getParm, &IOshift, "IOshift");

	dt=(int***)malloc(2*sizeof(int**));
	WTtype=(int*)malloc(2*sizeof(int));
	WTlevel=(int*)malloc(2*sizeof(int));
	dataSourcesCnt=(int*)malloc(2*sizeof(int));
	sDataSource*** dsrc=(sDataSource***)malloc(2*sizeof(sDataSource**));
	featuresCnt=(int**)malloc(2*sizeof(int*));
	feature=(int***)malloc(2*sizeof(int**));

	for (int i=0; i<2; i++) {	
		safecall(cfg, setKey, strBuild("%sput", (i>0) ? "Out" : "In").c_str());
		safecall(cfg->currentKey, getParm, &dataSourcesCnt[i], "DataSourcesCount");
		dsrc[i]=(sDataSource**)malloc(dataSourcesCnt[i]*sizeof(sDataSource*));
		featuresCnt[i]=(int*)malloc(dataSourcesCnt[i]*sizeof(int));
		feature[i]=(int**)malloc(dataSourcesCnt[i]*sizeof(int*));
		dt[i]=(int**)malloc(dataSourcesCnt[i]*sizeof(int*));
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			feature[i][d]=(int*)malloc(MAX_TS_FEATURES*sizeof(int));
			safecall(cfg, setKey, strBuild("DataSource%d", d).c_str());
			setDataSource(&dsrc[i][d]);
			safecall(cfg->currentKey, getParm, &feature[i][d], "SelectedFeatures", false, &featuresCnt[i][d]);
			dt[i][d]=(int*)malloc(featuresCnt[i][d]*sizeof(int));
			safecall(cfg->currentKey, getParm, &dt[i][d], "DataTransformation");
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
		tmptime[i]=(char*)malloc(DATE_FORMAT_LEN); 
		tmptime[i][0]='\0';
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
			safecall(dsrc[i][d], load, _date0, (i>0)?IOshift:0, stepsCnt, tmptime, tmpval, tmptimeB, tmpvalB, tmpbw);

			//-- set timestamps
			for (int s=0; s<stepsCnt; s++) strcpy_s(timestamp[s][i], DATE_FORMAT_LEN, tmptime[s]);
			strcpy_s(timestampB[i], DATE_FORMAT_LEN, tmptimeB);

			//-- extract selected features in tmpvalx
			for (int f=0; f<featuresCnt[i][d]; f++) {
				for (int s=0; s<stepsCnt; s++) {
					for (int df=0; df<dsrc[i][d]->featuresCnt; df++) {
						if (feature[i][d][f]==df) {
							tmpvalx[s*featuresCnt[i][d]+f]=tmpval[s*dsrc[i][d]->featuresCnt+feature[i][d][f]];
						}
					}
				}
			}

			for (int f=0; f<featuresCnt[i][d]; f++) {
				//-- FFTcalc for each feature. Also sets original value at position 0
				WTcalc(i, d, f, tmpvalx);
				//-- base values for each feature. only for original values
				valB[i][d][f][0]=tmpvalB[feature[i][d][f]];
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
sTS2::sTS2(sObjParmsDef, \
	int ioShift_, int stepsCnt_, int*** dt_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, \
	char*** INtimestamp_, char** INtimestampB_, \
	int INdataSourcesCnt_, int* INfeaturesCnt_, int INWTtype_, int INWTlevel_, numtype* INval_, numtype* INvalB_, \
	char*** OUTtimestamp_, char** OUTtimestampB_, \
	int OUTdataSourcesCnt_, int* OUTfeaturesCnt_, int OUTWTtype_, int OUTWTlevel_, numtype* OUTval_, numtype* OUTvalB_\
) : sCfgObj(sObjParmsVal, nullptr, "") {
	
	IOshift=ioShift_; stepsCnt=stepsCnt_; dt=dt_; doDump=doDump_;
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	sampleLen=sampleLen_; targetLen=targetLen_; batchSize=batchSize_;
	
	WTtype=(int*)malloc(2*sizeof(int));
	WTlevel=(int*)malloc(2*sizeof(int));
	dataSourcesCnt=(int*)malloc(2*sizeof(int));
	sDataSource*** dsrc=(sDataSource***)malloc(2*sizeof(sDataSource**));
	featuresCnt=(int**)malloc(2*sizeof(int*));

	dataSourcesCnt[0]=INdataSourcesCnt_;
	featuresCnt[0]=(int*)malloc(dataSourcesCnt[0]*sizeof(int));
	feature[0]=(int**)malloc(dataSourcesCnt[0]*sizeof(int*));
	for (int d=0; d<dataSourcesCnt[0]; d++) {
		featuresCnt[0][d]=INfeaturesCnt_[d];
		feature[0][d]=(int*)malloc(featuresCnt[0][d]*sizeof(int));
	}
	WTtype[0]=INWTtype_; WTlevel[0]=INWTlevel_;
	//--
	dataSourcesCnt[1]=OUTdataSourcesCnt_;
	featuresCnt[1]=(int*)malloc(dataSourcesCnt[1]*sizeof(int));
	feature[1]=(int**)malloc(dataSourcesCnt[0]*sizeof(int*));
	for (int d=0; d<dataSourcesCnt[0]; d++) {
		featuresCnt[1][d]=INfeaturesCnt_[d];
		feature[1][d]=(int*)malloc(featuresCnt[1][d]*sizeof(int));
	}
	WTtype[1]=OUTWTtype_; WTlevel[1]=OUTWTlevel_;

	mallocs1();
	
	//=== BUILDING INPUT SIDE ===
	//-- timestamps
	for (int s=0; s<stepsCnt; s++) strcpy_s(timestamp[s][0], DATE_FORMAT_LEN, (*INtimestamp_)[s]);
	//-- timestampB
	strcpy_s(timestampB[0], DATE_FORMAT_LEN, (*INtimestampB_));

	int i=0;
	int idx=0;
	//-- *val comes in flat, ordered by [step][dsXfeature]. We need to put these values in the appropriate val sub-array, with WTlevel=0
	for (int s=0; s<stepsCnt; s++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				val[s][i][d][f][0]=INval_[idx];
				idx++;
			}
		}
	}
	//-- now we need to calc wavelets.
	if (WTtype[i]!=WT_NONE) {
		numtype* tmpvalx=(numtype*)malloc((stepsCnt)*sizeof(numtype));
		numtype* lfa=(numtype*)malloc((stepsCnt)*sizeof(numtype));
		numtype** hfd=(numtype**)malloc(WTlevel[i]*sizeof(numtype*)); for (int l=0; l<WTlevel[i]; l++) hfd[l]=(numtype*)malloc((stepsCnt)*sizeof(numtype));
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				for (int s=0; s<(stepsCnt); s++) {
					tmpvalx[s]=val[s][i][d][f][0];
				}
				WaweletDecomp((stepsCnt), tmpvalx, WTlevel[i], WTtype[i], lfa, hfd);
				for (int s=0; s<(stepsCnt); s++) {
					val[s][i][d][f][1]=lfa[s];
					for (int l=0; l<WTlevel[i]; l++) val[s][i][d][f][l+2]=hfd[l][s];
				}
			}
		}
		for (int l=0; l<WTlevel[i]; l++) free(hfd[l]);
		free(lfa); free(hfd);
	}
	//-- *valB comes in flat, ordered by [dsXfeature]
	idx=0;
	for (int d=0; d<dataSourcesCnt[i]; d++) {
		for (int f=0; f<featuresCnt[i][d]; f++) {
			valB[i][d][f][0]=INvalB_[idx];
			idx++;
		}
	}

	for (int d=0; d<dataSourcesCnt[i]; d++) {
		for (int f=0; f<featuresCnt[i][d]; f++) {
			//-- base values for each level. we don't have it, so we set it equal to the first value of the level serie
			for (int l=1; l<(WTlevel[i]+2); l++) {
				valB[i][d][f][l]=val[0][i][d][f][l];
			}
			//-- transform for each feature/level.
			for (int l=0; l<(WTlevel[i]+2); l++) transform(i, d, f, l);
		}
	}

	//=== BUILDING OUTPUT SIDE ===

	//-- timestamps
	for (int s=0; s<stepsCnt; s++) strcpy_s(timestamp[s][1], DATE_FORMAT_LEN, (*OUTtimestamp_)[s]);
	//-- timestampB
	strcpy_s(timestampB[1], DATE_FORMAT_LEN, (*OUTtimestampB_));

	i=1;
	//-- *valB comes in flat, ordered by [dsXfeature]
	idx=0;
	for (int d=0; d<dataSourcesCnt[i]; d++) {
		for (int f=0; f<featuresCnt[i][d]; f++) {
			valB[i][d][f][0]=OUTvalB_[idx];
			idx++;
		}
	}
	idx=0;
	//-- *val comes in flat, ordered by [step][dsXfeature]. We need to put these values in the appropriate val sub-array, with WTlevel=0
	for (int s=0; s<stepsCnt; s++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				val[s][i][d][f][0]=OUTval_[idx];
				idx++;
			}
		}
	}

	//-- transform for each feature/level.
	for (int d=0; d<dataSourcesCnt[i]; d++) {
		for (int f=0; f<featuresCnt[i][d]; f++) {
			for (int l=0; l<(WTlevel[i]+2); l++) transform(i, d, f, l);
		}
	}
}
sTS2::~sTS2() {

	free(WTtype); free(WTlevel);

	for (int i=0; i<2; i++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			for (int f=0; f<featuresCnt[i][d]; f++) {
				free(TRmin[i][d][f]); free(TRmax[i][d][f]); free(scaleM[i][d][f]); free(scaleP[i][d][f]); free(valB[i][d][f]);
			}
			free(TRmin[i][d]); free(TRmax[i][d]); free(scaleM[i][d]); free(scaleP[i][d]); free(valB[i][d]);	//free(dt[i][d]);
		}
		free(TRmin[i]); free(TRmax[i]); free(scaleM[i]); free(scaleP[i]); free(valB[i]); //free(dt[i]);
	}
	free(TRmin); free(TRmax); free(scaleM); free(scaleP); free(valB); //free(dt);

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

	for (int i=0; i<2; i++) {
		for (int d=0; d<dataSourcesCnt[i]; d++) {
			free(feature[i][d]);
		}
		free(feature[i]);
	}
	free(feature);

	for (int s=0; s<stepsCnt; s++) {
		for (int i=0; i<2; i++) {
			free(timestamp[s][i]);
		}
		free(timestamp[s]);
	}
	free(timestamp);
	for (int i=0; i<2; i++) free(timestampB[i]);
	free(timestampB);
	for (int i=0; i<2; i++) free(featuresCnt[i]);
	free(featuresCnt);
	free(dataSourcesCnt);

	if (samplesCnt>0) {
		free(sample); free(target); free(prediction);
		free(sampleTRS); free(targetTRS); free(predictionTRS);
	}
}