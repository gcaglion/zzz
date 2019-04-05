#include "sTS.h"
//#include <vld.h>

sTS::sTS(sObjParmsDef, sDataSource* sourceData_, const char* date0_, int stepsCnt_, int dt_, bool doDump_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {

/*	int dsrcCnt=1;
	sDataSource** dsrc=(sDataSource**)malloc(dsrcCnt*sizeof(sDataSource*));
	featuresCnt=sourceData_->featuresCnt;
	stepsCnt=stepsCnt_;
	dt=dt_;

	timestamp=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) timestamp[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	valTR=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	timestampB=(char*)malloc(DATE_FORMAT_LEN);
	valB=(numtype*)malloc(featuresCnt*sizeof(numtype));
	TRmin=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) TRmin[f]=1e9;
	TRmax=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) TRmax[f]=-1e9;

	doDump=doDump_;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	//-- load all data sources
	char** tmptime=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) tmptime[i]=(char*)malloc(DATE_FORMAT_LEN);
	numtype** tmpval=(numtype**)malloc(dsrcCnt*sizeof(numtype*));
	char* tmptimeB=(char*)malloc(DATE_FORMAT_LEN);
	numtype** tmpvalB=(numtype**)malloc(dsrcCnt*sizeof(numtype*));
	numtype** tmpbw=(numtype**)malloc(dsrcCnt*sizeof(numtype*));

	for (int d=0; d<dsrcCnt; d++) {
		tmpval[d]=(numtype*)malloc(stepsCnt*dsrc[d]->featuresCnt*sizeof(numtype));
		tmpvalB[d]=(numtype*)malloc(dsrc[d]->featuresCnt*sizeof(numtype));
		tmpbw[d]=(numtype*)malloc(stepsCnt*dsrc[d]->featuresCnt*sizeof(numtype));
		safecall(sourceData_, load, date0_, stepsCnt, tmptime, tmpval[d], tmptimeB, tmpvalB[d], tmpbw[d]);	//-- tmptime is loaded from last ts
	}
*/
	//==== INCOMPLETE !!! ===
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

	timestamp=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) timestamp[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	valTR=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	timestampB=(char*)malloc(DATE_FORMAT_LEN);
	valB=(numtype*)malloc(featuresCnt*sizeof(numtype));
	TRmin=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) TRmin[f]=1e9;
	TRmax=(numtype*)malloc(featuresCnt*sizeof(numtype)); for (int f=0; f<featuresCnt; f++) TRmax[f]=-1e9;

	//-- load all data sources
	char** tmptime=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) tmptime[i]=(char*)malloc(DATE_FORMAT_LEN);
	numtype** tmpval=(numtype**)malloc(dsrcCnt*sizeof(numtype*));
	char* tmptimeB=(char*)malloc(DATE_FORMAT_LEN);
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

void sTS::dump() {
	int s, f;

	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_BASE_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);
	sprintf_s(dumpFileName, "%s/%s_TR_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFileTR;
	if (fopen_s(&dumpFileTR, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime"); fprintf(dumpFileTR, "i, datetime");
	for (f=0; f<featuresCnt; f++) {
		fprintf(dumpFile, ",F%d", f); fprintf(dumpFileTR, ",F%d", f);
	}
	fprintf(dumpFile, "\n%d,%s", -1, timestampB);
	fprintf(dumpFileTR, "\n%d,%s", -1, timestampB);
	for (f=0; f<featuresCnt; f++) {
		fprintf(dumpFile, ",%f", valB[f]);
		fprintf(dumpFileTR, ",%f", valB[f]);
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(dumpFile, "\n%d, %s", s, timestamp[s]);
		fprintf(dumpFileTR, "\n%d, %s", s, timestamp[s]);
		for (f=0; f<featuresCnt; f++) {
			fprintf(dumpFile, ",%f", val[s*featuresCnt+f]);
			fprintf(dumpFileTR, ",%f", valTR[s*featuresCnt+f]);
		}
	}
	fprintf(dumpFile, "\n"); fprintf(dumpFileTR, "\n");

	fclose(dumpFile); fclose(dumpFileTR);

}

void sTS::untransform() {
	int curr, prev;
	for (int s=0; s<stepsCnt; s++) {
		for (int f=0; f<featuresCnt; f++) {
			curr=s*featuresCnt+f;
			prev=(s-1)*featuresCnt+f;
			if (dt==DT_NONE) {
				val[curr]=valTR[curr];
			}
			if (dt==DT_DELTA) {
				if (s>0) {
					if (valTR[curr]==EMPTY_VALUE) {
						val[curr]=EMPTY_VALUE;
					} else {
						val[curr]=valTR[curr]+val[prev];
						if (val[curr]==EMPTY_VALUE) val[curr]=val[curr];
					}
				} else {
					if (valTR[curr]==EMPTY_VALUE) {
						val[curr]=EMPTY_VALUE;
					} else {
						val[curr]=valTR[curr]+valB[f];
					}
				}
			}
		}
	}
}
