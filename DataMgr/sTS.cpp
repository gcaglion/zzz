#include "sTS.h"

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
		safecall(cfg, setKey, (newsname("DataSource%d", d))->base);
		setDataSource(&dsrc[d]);
		//safecall(cfgKey, getParm, &selF[d], (newsname("DataSource%d/SelectedFeatures", d))->base, false, &selFcnt[d]);
		safecall(cfg->currentKey, getParm, &selF[d], "SelectedFeatures", false, &selFcnt[d]);
		safecall(cfg, setKey, "../");
		featuresCnt+=selFcnt[d];
	}

	timestamp=(char**)malloc(stepsCnt*sizeof(char*)); for (int i=0; i<stepsCnt; i++) timestamp[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype*)malloc(stepsCnt*featuresCnt*sizeof(numtype));
	timestampB=(char*)malloc(DATE_FORMAT_LEN);
	valB=(numtype*)malloc(featuresCnt*sizeof(numtype));

	//char* pDate0, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar, numtype* oBarWidth

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

	//-- free temps
	for (int d=0; d<dsrcCnt; d++) {
		free(selF[d]);
		free(tmpval[d]); free(tmpvalB[d]); free(tmpbw[d]);
	}
	free(selF); free(selFcnt);
	free(tmpval); free(tmpvalB); free(tmpbw);
	for (int i=0; i<stepsCnt; i++) free(tmptime[i]); 
	free(tmptime); free(tmptimeB);

}
sTS::~sTS() {
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
	sprintf_s(dumpFileName, "%s/%s_dump_p%d_t%d_%p.csv", dumpPath, name->base, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime");
	for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",F%d", f);
	fprintf(dumpFile, "\n%d,%s", -1, timestampB);
	for (f=0; f<featuresCnt; f++) {
		fprintf(dumpFile, ",%f", valB[f]);
		//for (int ff=0; ff<(featuresCnt-3); ff++) fprintf(dumpFile, ",");
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(dumpFile, "\n%d, %s", s, timestamp[s]);
		for (f=0; f<featuresCnt; f++) {
			fprintf(dumpFile, ",%f", val[s*featuresCnt+f]);
		}
	}
	fprintf(dumpFile, "\n");

	fclose(dumpFile);

}
