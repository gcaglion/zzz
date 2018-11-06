#include "sTimeSerie.h"

sTimeSerie::sTimeSerie(sObjParmsDef, sDataSource* sourceData_, const char* date0_, int stepsCnt_, int dt_, int tsfCnt_, int* tsf_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	mallocs1();

	strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	stepsCnt=stepsCnt_;
	dt=dt_; 
	tsfCnt=tsfCnt_; for (int i=0; i<tsfCnt; i++) tsf[i]=tsf_[i];
	sourceData=sourceData_;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else{
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	mallocs2();
}
sTimeSerie::sTimeSerie(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	mallocs1();
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &date0, "Date0");
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen");
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &tsf, "StatisticalFeatures", false, &tsfCnt);
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 0. default dump path is dbg outfilepath
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	safecall(cfgKey, getParm, &dumpPath, "DumpPath", true);
	//-- 2. do stuff and spawn sub-Keys
	safecall(this, setDataSource);
	mallocs2();
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
sTimeSerie::~sTimeSerie() {
	frees();
}

void sTimeSerie::load(char* date0_) {
	if (date0_!=nullptr) strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	safecall(sourceData, load, date0, stepsCnt, dtime, valA, bdtime, base);
	transform();
}
void sTimeSerie::transform(int dt_) {
	dt=(dt_==-1) ? dt : dt_;
	int curr=0;
	for (int s=0; s<(stepsCnt); s++) {
		for (int f=0; f<sourceData->featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				trvalA[curr]=valA[curr];
				break;
			case DT_DELTA:
				if (s==0) {
					trvalA[curr]=valA[curr]-base[f];
				} else {
					trvalA[curr]=valA[curr]-valA[(s-1)*sourceData->featuresCnt+f];
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
			if (trvalA[curr]<dmin[f]) dmin[f]=trvalA[curr];
			if (trvalA[curr]>dmax[f]) dmax[f]=trvalA[curr];

			curr++;
		}
	}
	hasTR=true;

	if (doDump) dump();
}
void sTimeSerie::scale(float scaleMin_, float scaleMax_) {

	for (int f=0; f<sourceData->featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(dmax[f]-dmin[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*dmax[f]);
	}

	for (int f=0; f<sourceData->featuresCnt; f++) {
		for (int s=0; s<stepsCnt; s++) {
			trsvalA[s*sourceData->featuresCnt+f]=trvalA[s*sourceData->featuresCnt+f]*scaleM[f]+scaleP[f];
		}
	}
	hasTRS=true;

	if (doDump) dump();
}
void sTimeSerie::untransform() {
	int curr=0;
	for (int s=0; s<(stepsCnt); s++) {
		for (int f=0; f<sourceData->featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				valP[curr]=trvalP[curr];
				break;
			case DT_DELTA:
				if (s==0) {
					valP[curr]=trvalP[curr]+base[f];
				} else {
					valP[curr]=trvalP[curr]+valP[(s-1)*sourceData->featuresCnt+f];
				}
				break;
			case DT_LOG:
				break;
			case DT_DELTALOG:
				break;
			default:
				break;
			}

			curr++;
		}
	}
}
void sTimeSerie::dump(bool prediction_) {
	int s, f;

	char suffix1[12];
	if (!hasTR) {
		strcpy_s(suffix1, 12, "BASE");
	} else {
		if (!hasTRS) {
			strcpy_s(suffix1, 12, "TR");
		} else {
			strcpy_s(suffix1, 12, "TRS");
		}
	}
	char suffix2[12];
	if (prediction_) {
		strcpy_s(suffix2, 12, "PRD");
	} else {
		strcpy_s(suffix2, 12, "ACT");
	}

	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_%s-%s_dump_%p.csv", dumpPath, name->base, date0, suffix1, suffix2, this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime");
	for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",F%d_origA,F%d_origP,F%d_trA,F%d_trP,F%d_trsA,F%d_trsP", f, f, f, f, f, f);
	fprintf(dumpFile, "\n%d,%s", -1, dtime[0]);
	for (f=0; f<sourceData->featuresCnt; f++) {
		fprintf(dumpFile, ",%f,", base[f]);
		for (int ff=0; ff<(sourceData->featuresCnt-3); ff++) fprintf(dumpFile, ",");
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(dumpFile, "\n%d, %s", s, dtime[s]);
		for (f=0; f<sourceData->featuresCnt; f++) {
			fprintf(dumpFile, ",%f,%f", valA[s*sourceData->featuresCnt+f], (prediction_) ? valP[s*sourceData->featuresCnt+f] : 0);
			if (hasTR) {
				fprintf(dumpFile, ",%f,%f", trvalA[s*sourceData->featuresCnt+f], (prediction_) ? trvalP[s*sourceData->featuresCnt+f] : 0);
			} else {
				fprintf(dumpFile, ",,");
			}
			if (hasTRS) {
				fprintf(dumpFile, ",%f,%f", trsvalA[s*sourceData->featuresCnt+f], (prediction_) ? trsvalP[s*sourceData->featuresCnt+f] : 0);
			} else {
				fprintf(dumpFile, ",,");
			}
		}
	}
	fprintf(dumpFile, "\n");

	if (hasTR) {
		fprintf(dumpFile, "\ntr-min:");
		for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",,,%f", dmin[f]);
		fprintf(dumpFile, "\ntr-max:");
		for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",,,%f", dmax[f]);
		fprintf(dumpFile, "\n");
	}
	if (hasTRS) {
		fprintf(dumpFile, "\nscaleM:");
		for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",,,%f", scaleM[f]);
		fprintf(dumpFile, "\nscaleP:");
		for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",,,%f", scaleP[f]);
		fprintf(dumpFile, "\n");

	}

	fclose(dumpFile);

}

//-- private stuff
void sTimeSerie::mallocs1(){
	date0=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	dumpPath=(char*)malloc(MAX_PATH);
	tsf=(int*)malloc(MAX_TSF_CNT*sizeof(int));
}
void sTimeSerie::mallocs2() {
	len=stepsCnt*sourceData->featuresCnt;
	hasTR=false; hasTRS=false;
	dtime=(char**)malloc(len*sizeof(char*)); 
	for (int i=0; i<len; i++) dtime[i]=(char*)malloc(DATE_FORMAT_LEN);
	valA=(numtype*)malloc(len*sizeof(numtype));
	trvalA=(numtype*)malloc(len*sizeof(numtype));
	trsvalA=(numtype*)malloc(len*sizeof(numtype));
	valP=(numtype*)malloc(len*sizeof(numtype));
	trvalP=(numtype*)malloc(len*sizeof(numtype));
	trsvalP=(numtype*)malloc(len*sizeof(numtype));
	base=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	dmin=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	for (int f=0; f<sourceData->featuresCnt; f++) { dmin[f]=1e9; dmax[f]=-1e9; }
}
void sTimeSerie::frees() {
	for (int i=0; i<len; i++) free(dtime[i]); free(dtime);
	free(valA);
	free(trvalA);
	free(trsvalA);
	free(valP);
	free(trvalP);
	free(trsvalP);
	free(base);
	free(dmin);	free(dmax);
	free(scaleM); free(scaleP);
	free(tsf);
	free(dumpPath);
}
void sTimeSerie::setDataSource() {

	bool found=false;
	sFXDataSource* fxData;
	sGenericDataSource* fileData;
	sMT4DataSource* mt4Data;

	//-- first, find and set
	safecall(cfg, setKey, "File_DataSource", true, &found);	//-- ignore error
	if (found) {
		safecall(cfg, setKey, "../"); //-- get back;
		safespawn(fileData, newsname("File_DataSource"), defaultdbg, cfg, "File_DataSource");
		sourceData=fileData;
	} else {
		safecall(cfg, setKey, "FXDB_DataSource", true, &found);	//-- ignore error
		if (found) {
			safecall(cfg, setKey, "../"); //-- get back;
			safespawn(fxData, newsname("FXDB_DataSource"), defaultdbg, cfg, "FXDB_DataSource");
			sourceData=fxData;
		} else {
			safecall(cfg, setKey, "MT4_DataSource", true, &found);	//-- ignore error
			if (found) {
				safecall(cfg, setKey, "../"); //-- get back;
				safespawn(mt4Data, newsname("MT4_DataSource"), defaultdbg, cfg, "MT4_DataSource");
				sourceData=mt4Data;
			}
		}
	}
	if (!found) fail("No Valid DataSource Parameters Key found.");

	//-- then, open
	//safecall(sourceData, open);
}