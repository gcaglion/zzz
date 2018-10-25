#include "sTimeSerie.h"

sTimeSerie::sTimeSerie(sObjParmsDef, sDataSource* sourceData_, char* date0_, int stepsCnt_, int dt_, int tsfCnt_, int* tsf_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	date0=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	tsf=(int*)malloc(MAX_TSF_CNT*sizeof(int));
	strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	stepsCnt=stepsCnt_;
	dt=dt_; 
	tsfCnt=tsfCnt_; for (int i=0; i<tsfCnt; i++) tsf[i]=tsf_[i];
	sourceData=sourceData_;

	mallocs();
}
sTimeSerie::sTimeSerie(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	date0=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	tsf=(int*)malloc(MAX_TSF_CNT*sizeof(int));
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &date0, "Date0");
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen");
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &tsf, "StatisticalFeatures", false, &tsfCnt);
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 2. do stuff and spawn sub-Keys
	setDataSource();
	mallocs();
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
sTimeSerie::~sTimeSerie() {
	frees();
}

void sTimeSerie::load(char* date0_) {
	if (date0_!=nullptr) strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	sourceData->load(date0, stepsCnt, dtime, val, bdtime, base);
	if (doDump) dump();
	transform();
	if (doDump) dump();
}
void sTimeSerie::transform(int dt_) {
	dt=(dt_==-1)?dt:dt_;
	int curr=0;
	for (int s=0; s<(stepsCnt); s++) {
		for (int f=0; f<sourceData->featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				trval[curr]=val[curr];
				break;
			case DT_DELTA:
				if (s==0) {
					trval[curr]=val[curr]-base[f];
				} else {
					trval[curr]=val[curr]-val[(s-1)*sourceData->featuresCnt+f];
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
			if (trval[curr]<dmin[f]) dmin[f]=trval[curr];
			if (trval[curr]>dmax[f]) dmax[f]=trval[curr];

			curr++;
		}
	}
	hasTR=true;
}
void sTimeSerie::scale(float scaleMin_, float scaleMax_, numtype** oScaleM_, numtype** oScaleP_, numtype** oDmin_, numtype** oDmax_) {

	//if (!hasTR) fail("-- must transform before scaling! ---");

	for (int f=0; f<sourceData->featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(dmax[f]-dmin[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*dmax[f]);
	}

	for (int f=0; f<sourceData->featuresCnt; f++) {
		for (int s=0; s<stepsCnt; s++) {
			trsval[s*sourceData->featuresCnt+f]=trval[s*sourceData->featuresCnt+f]*scaleM[f]+scaleP[f];
		}
	}
	//-- copy scaleM/P pointers (with all sourcedata features) to dataset
	(*oScaleM_)=scaleM; (*oScaleP_)=scaleP;
	//-- copy dmin/dmax pointers (with all sourcedata features) to dataset
	(*oDmin_)=dmin; (*oDmax_)=dmax;

	hasTRS=true;
}
void sTimeSerie::dump() {
	int s, f;

	char suffix[12];
	if (!hasTR) {
		strcpy_s(suffix, 12, "BASE");
	} else {
		if (!hasTRS) {
			strcpy_s(suffix, 12, "TR");
		} else {
			strcpy_s(suffix, 12, "TRS");
		}
	}
	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_%s_dump_%p.csv", dbg->outfilepath, name->base, date0, suffix, this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime");
	for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",F%d_orig,F%d_tr,F%d_trs", f, f, f);
	fprintf(dumpFile, "\n%d,%s", -1, dtime[0]);
	for (f=0; f<sourceData->featuresCnt; f++) {
		fprintf(dumpFile, ",%f", base[f]);
		for (int ff=0; ff<(sourceData->featuresCnt-3); ff++) fprintf(dumpFile, ",");
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(dumpFile, "\n%d, %s", s, dtime[s]);
		for (f=0; f<sourceData->featuresCnt; f++) {
			fprintf(dumpFile, ",%f", val[s*sourceData->featuresCnt+f]);
			if (hasTR) {
				fprintf(dumpFile, ",%f", trval[s*sourceData->featuresCnt+f]);
			} else {
				fprintf(dumpFile, ",");
			}
			if (hasTRS) {
				fprintf(dumpFile, ",%f", trsval[s*sourceData->featuresCnt+f]);
			} else {
				fprintf(dumpFile, ",");
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
void sTimeSerie::mallocs() {
	len=stepsCnt*sourceData->featuresCnt;
	hasTR=false; hasTRS=false;
	dtime=(char**)malloc(len*sizeof(char*)); 
	for (int i=0; i<len; i++) dtime[i]=(char*)malloc(DATE_FORMAT_LEN);
	val=(numtype*)malloc(len*sizeof(numtype));
	trval=(numtype*)malloc(len*sizeof(numtype));
	trsval=(numtype*)malloc(len*sizeof(numtype));
	outval=(numtype*)malloc(len*sizeof(numtype));
	outtrval=(numtype*)malloc(len*sizeof(numtype));
	outtrsval=(numtype*)malloc(len*sizeof(numtype));
	base=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	dmin=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	for (int f=0; f<sourceData->featuresCnt; f++) { dmin[f]=1e9; dmax[f]=-1e9; }
}
void sTimeSerie::frees() {
	for (int i=0; i<len; i++) free(dtime[i]); free(dtime);
	free(val);
	free(trval);
	free(trsval);
	free(outval);
	free(outtrval);
	free(outtrsval);
	free(base);
	free(dmin);	free(dmax);
	free(scaleM); free(scaleP);
	free(tsf);
}
void sTimeSerie::setDataSource() {

	bool found=false;
	sFXDataSource* fxData;
	sGenericDataSource* fileData;
	tMT4Data* mt4Data;

	//-- first, find and set
	safecall(cfg, setKey, "File_DataSource", true, &found);	//-- ignore error
	if (found) {
		safecall(cfg, setKey, "../"); //-- get back;
		safespawn(fileData, newsname("File_DataSource"), defaultdbg, cfg, "File_DataSource", true);
		sourceData=fileData;
	} else {
		safecall(cfg, setKey, "FXDB_DataSource", true, &found);	//-- ignore error
		if (found) {
			safecall(cfg, setKey, "../"); //-- get back;
			safespawn(fxData, newsname("FXDB_DataSource"), defaultdbg, cfg, "FXDB_DataSource", false);
			sourceData=fxData;
		} else {
			safecall(cfg, setKey, "MT4_DataSource", true, &found);	//-- ignore error
			if (found) {
				safecall(cfg, setKey, "../"); //-- get back;
				safespawn(mt4Data, newsname("MT4_DataSource"), defaultdbg, cfg, "MT4_DataSource", true);
				sourceData=mt4Data;
			}
		}
	}
	if (!found) fail("No Valid DataSource Parameters Key found.");

	//-- then, open
	safecall(sourceData, open);

}