#include "sTS2.h"

#ifndef MAX_TS_FEATURES
#define MAX_TS_FEATURES 128
#endif

void sTS2::mallocs1() {
	timestamp=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) timestamp[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype****)malloc(stepsCnt*sizeof(numtype***));
	valTR=(numtype****)malloc(stepsCnt*sizeof(numtype***));
	for (int s=0; s<stepsCnt; s++) {
		val[s]=(numtype***)malloc(dataSourcesCnt*sizeof(numtype**));
		valTR[s]=(numtype***)malloc(dataSourcesCnt*sizeof(numtype**));
		for (int d=0; d<dataSourcesCnt; d++) {
			val[s][d]=(numtype**)malloc(featuresCnt[d]*sizeof(numtype*));
			valTR[s][d]=(numtype**)malloc(featuresCnt[d]*sizeof(numtype*));
			for (int f=0; f<featuresCnt[d]; f++) {
				val[s][d][f]=(numtype*)malloc((WTlevel+2)*sizeof(numtype));
				valTR[s][d][f]=(numtype*)malloc((WTlevel+2)*sizeof(numtype));
			}
		}
	}
	timestampB=(char*)malloc(DATE_FORMAT_LEN);
	TRmin=(numtype***)malloc(dataSourcesCnt*sizeof(numtype**));
	TRmax=(numtype***)malloc(dataSourcesCnt*sizeof(numtype**));
	valB=(numtype***)malloc(dataSourcesCnt*sizeof(numtype**));
	for (int d=0; d<dataSourcesCnt; d++) {
		TRmin[d]=(numtype**)malloc(featuresCnt[d]*sizeof(numtype*));
		TRmax[d]=(numtype**)malloc(featuresCnt[d]*sizeof(numtype*));
		valB[d]=(numtype**)malloc(featuresCnt[d]*sizeof(numtype*));
		for (int f=0; f<featuresCnt[d]; f++) {
			TRmin[d][f]=(numtype*)malloc((WTlevel+2)*sizeof(numtype));
			TRmax[d][f]=(numtype*)malloc((WTlevel+2)*sizeof(numtype));
			valB[d][f]=(numtype*)malloc((WTlevel+2)*sizeof(numtype));
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
void sTS2::WTcalc(int d, int f, numtype* dsvalSF) {
	//-- mallocs lfa/hfd
	numtype* lfa=(numtype*)malloc(stepsCnt*sizeof(numtype));
	//--
	numtype** hfd=(numtype**)malloc(WTlevel*sizeof(numtype*));
	for (int l=0; l<WTlevel; l++) hfd[l]=(numtype*)malloc(stepsCnt*sizeof(numtype));
	
	//-- extract single features from valTR
	numtype* tmpf=(numtype*)malloc(stepsCnt*sizeof(numtype));
	for (int s=0; s<stepsCnt; s++) {
		tmpf[s]=dsvalSF[s*featuresCnt[d]+f];
	}
	WaweletDecomp(stepsCnt, tmpf, WTlevel, WTtype, lfa, hfd);

	for (int s=0; s<stepsCnt; s++) {
		val[s][d][f][0]=tmpf[s];
		val[s][d][f][1]=lfa[s];
		for (int l=0; l<WTlevel; l++) val[s][d][f][l+2]=hfd[l][s];
	}


	free(tmpf);
	for (int l=0; l<WTlevel; l++) free(hfd[l]);
	free(lfa); free(hfd);
}
void sTS2::transform(int d, int f, int l) {

	TRmin[d][f][l]=1e9; TRmax[d][f][l]=-1e9;

	for (int s=0; s<(stepsCnt); s++) {
		switch (dt) {
		case DT_NONE:
			valTR[s][d][f][l]=val[s][d][f][l];
			break;
		case DT_DELTA:
			if (val[s][d][f][l]==EMPTY_VALUE) {
				valTR[s][d][f][l]=EMPTY_VALUE;
			} else {
				if (s==0) {
					valTR[s][d][f][l]=val[s][d][f][l]-valB[d][f][l];
				} else {
					valTR[s][d][f][l]=val[s][d][f][l]-val[s-1][d][f][l];
				}
			}
			break;
		case DT_LOG:
			if (val[s][d][f][l]==EMPTY_VALUE) {
				valTR[s][d][f][l]=EMPTY_VALUE;
			} else {
				valTR[s][d][f][l]=log(val[s][d][f][l]);
			}
			break;
		default:
			break;
		}

		//-- min/max calc
		if (valTR[s][d][f][l]!=EMPTY_VALUE&&valTR[s][d][f][l]<TRmin[d][f][l]) TRmin[d][f][l]=valTR[s][d][f][l];
		if (valTR[s][d][f][l]!=EMPTY_VALUE&&valTR[s][d][f][l]>TRmax[d][f][l]) TRmax[d][f][l]=valTR[s][d][f][l];
	}
}
void sTS2::dumpToFile(FILE* file, numtype**** val_) {
	int s, d, f, l;
	fprintf(file, "i, datetime");
	for (d=0; d<dataSourcesCnt; d++) {
		for (f=0; f<featuresCnt[d]; f++) {
			for (l=0; l<(WTlevel+2); l++) {
				fprintf(file, ",D%dF%dL%d", d, f, l);
			}
		}
	}
	fprintf(file, "\n%d,%s", -1, timestampB);
	for (d=0; d<dataSourcesCnt; d++) {
		for (f=0; f<featuresCnt[d]; f++) {
			for (l=0; l<(WTlevel+2); l++) {
				fprintf(file, ",%f", valB[d][f][l]);
			}
		}
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(file, "\n%d, %s", s, timestamp[s]);
		for (d=0; d<dataSourcesCnt; d++) {
			for (f=0; f<featuresCnt[d]; f++) {
				for (l=0; l<(WTlevel+2); l++) {
					fprintf(file, ",%f", val_[s][d][f][l]);
				}
			}
		}
	}
	fclose(file);
}
void sTS2::dump() {
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileTR;
	if (fopen_s(&dumpFileTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	dumpToFile(dumpFile, val);
	dumpToFile(dumpFileTR, valTR);
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

	safecall(cfgKey, getParm, &dataSourcesCnt, "DataSourcesCount");
	sDataSource** dsrc=(sDataSource**)malloc(dataSourcesCnt*sizeof(sDataSource*));
	featuresCnt=(int*)malloc(dataSourcesCnt*sizeof(int));
	int** selF=(int**)malloc(dataSourcesCnt*sizeof(int*));
	for (int d=0; d<dataSourcesCnt; d++) {
		selF[d]=(int*)malloc(MAX_TS_FEATURES*sizeof(int));
		safecall(cfg, setKey, strBuild("DataSource%d", d).c_str());
		setDataSource(&dsrc[d]);
		safecall(cfg->currentKey, getParm, &selF[d], "SelectedFeatures", false, &featuresCnt[d]);
		safecall(cfg, setKey, "../");
	}

	safecall(cfgKey, getParm, &WTtype, "WaveletType");
	safecall(cfgKey, getParm, &WTlevel, "DecompLevel");
	if (WTtype==WT_NONE) WTlevel=0;

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
	for (int d=0; d<dataSourcesCnt; d++) {
		tmpval=(numtype*)malloc(stepsCnt*dsrc[d]->featuresCnt*sizeof(numtype));
		tmpvalB=(numtype*)malloc(dsrc[d]->featuresCnt*sizeof(numtype));
		tmpvalx=(numtype*)malloc(stepsCnt*featuresCnt[d]*sizeof(numtype));
		tmpvalBx=(numtype*)malloc(featuresCnt[d]*sizeof(numtype));
		tmpbw=(numtype*)malloc(stepsCnt*dsrc[d]->featuresCnt*sizeof(numtype));
		safecall(dsrc[d], load, _date0, stepsCnt, tmptime, tmpval, tmptimeB, tmpvalB, tmpbw);

		//-- set timestamps
		for (int s=0; s<stepsCnt; s++) strcpy_s(timestamp[s], DATE_FORMAT_LEN, tmptime[s]);
		strcpy_s(timestampB, DATE_FORMAT_LEN, tmptimeB);

		//-- extract selected features in tmpvalx
		for (int f=0; f<featuresCnt[d]; f++) {
			for (int s=0; s<stepsCnt; s++) {
				for (int df=0; df<dsrc[d]->featuresCnt; df++) {
					if (selF[d][f]==df) {
						tmpvalx[s*featuresCnt[d]+f]=tmpval[s*dsrc[d]->featuresCnt+selF[d][f]];
						//-- FFTcalc for each feature. Also sets original value at position 0
						WTcalc(d, f, tmpvalx);
						//-- base values for each feature. only for original values
						valB[d][f][0]=tmpvalB[selF[d][f]];
						//-- transform for each feature/level.
						for (int l=0; l<(WTlevel+2); l++) transform(d, f, l);
					}
				}
			}
		}
		free(tmpval); free(tmpvalB); free(tmpbw);
		free(tmpvalx); free(tmpvalBx);
	}
	for (int i=0; i<stepsCnt; i++) free(tmptime[i]);
	free(tmptime); free(tmptimeB);
	free(dsrc);

	if (doDump) dump();


}

sTS2::~sTS2() {
	for (int d=0; d<dataSourcesCnt; d++) {
		for (int f=0; f<featuresCnt[d]; f++) {
			free(TRmin[d][f]); free(TRmax[d][f]); free(valB[d][f]);
		}
		free(TRmin[d]); free(TRmax[d]); free(valB[d]);
	}
	free(TRmin); free(TRmax); free(valB);
	free(timestampB);

	for (int s=0; s<stepsCnt; s++) {
		for (int d=0; d<dataSourcesCnt; d++) {
			for (int f=0; f<featuresCnt[d]; f++) {
				free(val[s][d][f]); free(valTR[s][d][f]);
			}
			free(val[s][d]); free(valTR[s][d]);
		}
		free(val[s]); free(valTR[s]);
	}
	free(val); free(valTR);
	for (int i=0; i<stepsCnt; i++) free(timestamp[i]); free(timestamp);
}