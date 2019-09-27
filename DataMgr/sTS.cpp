#include "sTS.h"
//#include <vld.h>

void sTS::mallocs1(){
	timestamp=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) timestamp[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	valTR=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	timestampB=(char*)malloc(DATE_FORMAT_LEN);
	valB=(numtype*)malloc(featuresCnt*sizeof(numtype));
	TRmin=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) TRmin[f]=1e9;
	TRmax=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) TRmax[f]=-1e9;
}

sTS::sTS(sObjParmsDef, int stepsCnt_, int featuresCnt_, int dt_, char** timestamp_, numtype* val_, char* timestampB_, numtype* valB_, bool doDump_, char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, "") {
	stepsCnt=stepsCnt_; featuresCnt=featuresCnt_; dt=dt_; doDump=doDump_;
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath); if (dumpPath_!=nullptr) strcpy_s(dumpPath, MAX_PATH, dumpPath_);

	mallocs1();

	timestamp=timestamp_; val=val_;
	timestampB=timestampB_; valB=valB_;

	transform();

	//-- dump
	if (doDump) dump();

}
sTS::sTS(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen");
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &doDump, "Dump");
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	char* _dumpPath=&dumpPath[0]; 
	safecall(cfgKey, getParm, &_dumpPath, "DumpPath", true);

	char _date0[DATE_FORMAT_LEN]; char* _date0p=&_date0[0];
	safecall(cfgKey, getParm, &_date0p, "Date0");

	int dsrcCnt;
	safecall(cfgKey, getParm, &dsrcCnt, "DataSourcesCount");
	sDataSource** dsrc=(sDataSource**)malloc(dsrcCnt*sizeof(sDataSource*));
	int* selFcnt=(int*)malloc(dsrcCnt*sizeof(int));
	int** selF=(int**)malloc(dsrcCnt*sizeof(int*));

	featuresCnt=0;
	for (int d=0; d<dsrcCnt; d++) {
		selF[d]=(int*)malloc(MAX_TS_FEATURES*sizeof(int));
		safecall(cfg, setKey, strBuild("DataSource%d", d).c_str());
		setDataSource(&dsrc[d]);
		safecall(cfg->currentKey, getParm, &selF[d], "SelectedFeatures", false, &selFcnt[d]);
		safecall(cfg, setKey, "../");
		featuresCnt+=selFcnt[d];
	}

	mallocs1();

	//-- load all data sources
	char** tmptime=(char**)malloc(stepsCnt*sizeof(char*)); 
	for (int i=0; i<stepsCnt; i++) {
		tmptime[i]=(char*)malloc(DATE_FORMAT_LEN); tmptime[i][0]='\0';
	}
	numtype** tmpval=(numtype**)malloc(dsrcCnt*sizeof(numtype*));
	char* tmptimeB=(char*)malloc(DATE_FORMAT_LEN); tmptimeB[0]='\0';
	numtype** tmpvalB=(numtype**)malloc(dsrcCnt*sizeof(numtype*));
	numtype** tmpbw=(numtype**)malloc(dsrcCnt*sizeof(numtype*));

	for (int d=0; d<dsrcCnt; d++) {
		tmpval[d]=(numtype*)malloc(stepsCnt*dsrc[d]->featuresCnt*sizeof(numtype));
		tmpvalB[d]=(numtype*)malloc(dsrc[d]->featuresCnt*sizeof(numtype));
		tmpbw[d]=(numtype*)malloc(stepsCnt*dsrc[d]->featuresCnt*sizeof(numtype));
		safecall(dsrc[d], load, _date0, stepsCnt, tmptime, tmpval[d], tmptimeB, tmpvalB[d], tmpbw[d]);	//-- tmptime is loaded from last ts
	}

	//-- merge features values
	int i=0;
	for (int s=0; s<stepsCnt; s++) {
		for (int d=0; d<dsrcCnt; d++) {
			for (int f=0; f<selFcnt[d]; f++) {
				val[i]=tmpval[d][s*dsrc[d]->featuresCnt+selF[d][f]];
				i++;
			}
		}
	}
	//-- set timestamps
	for (int s=0; s<stepsCnt; s++) {
		strcpy_s(timestamp[s], DATE_FORMAT_LEN, tmptime[s]);
	}

	//-- merge base values
	i=0;
	for (int d=0; d<dsrcCnt; d++) {
		for (int f=0; f<selFcnt[d]; f++) {
			valB[i]=tmpvalB[d][selF[d][f]];
			i++;
		}
	}

	//-- set base timestamp
	strcpy_s(timestampB, DATE_FORMAT_LEN, tmptimeB);

	//-- transform
	transform();

	//-- dump
	if (doDump) dump();

	//-- free temps
	for (int d=0; d<dsrcCnt; d++) {
		free(selF[d]);
		free(tmpval[d]); free(tmpvalB[d]); free(tmpbw[d]);
	}
	free(selF); free(selFcnt);
	free(tmpval); free(tmpvalB); free(tmpbw);
	for (int i=0; i<stepsCnt; i++) free(tmptime[i]); 
	free(tmptime); free(tmptimeB);

	free(dsrc);

}
sTS::~sTS() {
	for (int i=0; i<stepsCnt; i++) free(timestamp[i]);
	free(timestamp); free(timestampB);
	free(val);
	free(valTR);
	free(TRmin); free(TRmax);
	free(valB);
	if (decompLevel>0) {
		for (int l=0; l<(decompLevel+1); l++) {
			free(FFTval[l]); free(FFTmin[l]); free(FFTmax[l]);
		}
		free(FFTval); free(FFTmin); free(FFTmax);
	}
}

void sTS::setDataSource(sDataSource** dataSrc_) {

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

void sTS::dumpToFile(FILE* file, numtype* val_) {
	int f, s;
	fprintf(file, "i, datetime");
	for (f=0; f<featuresCnt; f++) {
		fprintf(file, ",F%d", f);
	}
	fprintf(file, "\n%d,%s", -1, timestampB);
	for (f=0; f<featuresCnt; f++) {
		fprintf(file, ",%f", valB[f]);
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(file, "\n%d, %s", s, timestamp[s]);
		for (f=0; f<featuresCnt; f++) {
			fprintf(file, ",%f", val_[s*featuresCnt+f]);
		}
	}
	fclose(file);
}
void sTS::dump() {
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileTR;
	if (fopen_s(&dumpFileTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	dumpToFile(dumpFile, val);
	dumpToFile(dumpFileTR, valTR);

	FILE* dumpFileTRLFA; FILE** dumpFileTRHFD;
	if (decompLevel>0) {
		dumpFileTRHFD=(FILE**)malloc(decompLevel*sizeof(FILE*));
		sprintf_s(dumpFileName, "%s/%s_TR-LFA_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
		if (fopen_s(&dumpFileTRLFA, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
		for (int l=0; l<decompLevel; l++) {
			sprintf_s(dumpFileName, "%s/%s_TR-HFD%d_dump_p%d_t%d_%p.csv", dumpPath, name->base, l, GetCurrentProcessId(), GetCurrentThreadId(), this);
			if (fopen_s(&dumpFileTRHFD[l], dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
			dumpToFile(dumpFileTRHFD[l], FFTval[l+1]);
		}
		dumpToFile(dumpFileTRLFA, FFTval[0]);
		free(dumpFileTRHFD);
	}

}

void sTS::transform() {
	int curr=0;
	for (int s=0; s<(stepsCnt); s++) {
		for (int f=0; f<featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				valTR[curr]=val[curr];
				break;
			case DT_DELTA:
				if (val[curr]==EMPTY_VALUE) {
					valTR[curr]=EMPTY_VALUE;
				} else {
					if (s==0) {
						valTR[curr]=val[curr]-valB[f];
					} else {
						valTR[curr]=val[curr]-val[(s-1)*featuresCnt+f];
					}
				}
				break;
			case DT_LOG:
				break;
			case DT_DELTALOG:
				break;
			default:
				break;
			}

			//-- min/max calc
			if (valTR[curr]!=EMPTY_VALUE&&valTR[curr]<TRmin[f]) TRmin[f]=valTR[curr];
			if (valTR[curr]!=EMPTY_VALUE&&valTR[curr]>TRmax[f]) TRmax[f]=valTR[curr];

			curr++;
		}
	}
}

void sTS::FFTcalc(int decompLevel_, int waveletType_) {
	decompLevel=decompLevel_;
	//-- mallocs lfa/hfd
	numtype** lfa=(numtype**)malloc(featuresCnt*sizeof(numtype*));
	for (int f=0; f<featuresCnt; f++) lfa[f]=(numtype*)malloc(stepsCnt*sizeof(numtype));
	//--
	numtype*** hfd=(numtype***)malloc(featuresCnt*sizeof(numtype**));
	for (int f=0; f<featuresCnt; f++) {
		hfd[f]=(numtype**)malloc(decompLevel*sizeof(numtype*));
		for (int l=0; l<decompLevel; l++) hfd[f][l]=(numtype*)malloc(stepsCnt*sizeof(numtype));
	}
	//-- extract single features from valTR
	numtype* tmpf=(numtype*)malloc(stepsCnt*sizeof(numtype));
	for (int f=0; f<featuresCnt; f++) {
		for (int s=0; s<stepsCnt; s++) {
			tmpf[s]=valTR[s*featuresCnt+f];
		}
		WaweletDecomp(stepsCnt, tmpf, decompLevel, waveletType_, lfa[f], hfd[f]);
	}
	free(tmpf);

	/*for (int f=0; f<featuresCnt; f++) {
		dumpArrayH(stepsCnt, lfa[f], strBuild("lfa_F%d.csv", f).c_str());
		for (int l=0; l<decompLevel; l++) dumpArrayH(stepsCnt, hfd[f][l], strBuild("hfd%d_F%d.csv",l,f).c_str());
	}
	*/
	FFTval = (numtype**)malloc((decompLevel+1)*sizeof(numtype*));
	FFTmin = (numtype**)malloc((decompLevel+1)*sizeof(numtype*));
	FFTmax = (numtype**)malloc((decompLevel+1)*sizeof(numtype*));
	int i;
	for (int l=0; l<(decompLevel+1); l++) {
		FFTval[l]=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
		FFTmin[l]=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) FFTmin[l][f]=1e9;
		FFTmax[l]=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) FFTmax[l][f]=-1e9;
		i=0;
		for (int s=0; s<stepsCnt; s++) {
			for (int f=0; f<featuresCnt; f++) {
				if (l>0) {
					FFTval[l][i]=hfd[f][l-1][s];
				} else {
					FFTval[l][i]=lfa[f][s];
				}
				if (FFTval[l][i]<FFTmin[l][f]) FFTmin[l][f]=FFTval[l][i];
				if (FFTval[l][i]>FFTmax[l][f]) FFTmax[l][f]=FFTval[l][i];
				i++;
			}
		}
	}

	//-- dump
	if (doDump) dump();

	//-- frees
	for (int f=0; f<featuresCnt; f++) {
		for (int l=0; l<decompLevel; l++) free(hfd[f][l]);
		free(hfd[f]); free(lfa[f]);
	}
	free(hfd); free(lfa);
}

void sTS::slide(int steps_) {
	int f, s;
	for (int kaz=0; kaz<steps_; kaz++) {
		//-- set valB = first step
		for (f=0; f<featuresCnt; f++) {
			valB[f]=val[0*featuresCnt+f];
		}
		//-- set timestampB
		strcpy_s(timestampB, DATE_FORMAT_LEN, timestamp[0]);
		//-- set val
		for (s=0; s<(stepsCnt-1-steps_); s++) {
			for (f=0; f<featuresCnt; f++) {
				val[s*featuresCnt+f]=val[(s+1)*featuresCnt+f];
				valTR[s*featuresCnt+f]=valTR[(s+1)*featuresCnt+f];
			}
			//-- set timestamp
			strcpy_s(timestamp[s], DATE_FORMAT_LEN, timestamp[s+1]);
		}
		//-- last step is unchanged, except for timestamp
		timestamp[s][0]='9';
		timestamp[s][15]=49+kaz;	// 1,2,3...
	}
}

void sTS::invertSequence(int skipLastN) {
	numtype* ival=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	numtype* ivalTR=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));

	for (int s=0; s<(stepsCnt-skipLastN); s++) {
		for (int f=0; f<featuresCnt; f++) {
			ival[s*featuresCnt+f]=val[(stepsCnt-skipLastN-s-1)*featuresCnt+f];
			ivalTR[s*featuresCnt+f]=valTR[(stepsCnt-skipLastN-s-1)*featuresCnt+f];
		}
	}

	memcpy_s(val, stepsCnt*featuresCnt*sizeof(numtype), ival, stepsCnt*featuresCnt*sizeof(numtype));
	memcpy_s(valTR, stepsCnt*featuresCnt*sizeof(numtype), ivalTR, stepsCnt*featuresCnt*sizeof(numtype));

	free(ival); free(ivalTR);
}