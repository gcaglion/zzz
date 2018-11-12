#include "sTimeSerie.h"

sTimeSerie::sTimeSerie(sObjParmsDef, sDataSource* sourceData_, const char* date0_, int stepsCnt_, int dt_, int tsfCnt_, int* tsf_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	mallocs1();

	strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	stepsCnt=stepsCnt_;
	dt=dt_; 
	tsfCnt=tsfCnt_; for (int i=0; i<tsfCnt; i++) tsf[i]=tsf_[i];
	sourceData=sourceData_;

	doDump=false;
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

void sTimeSerie::load(int valSource, int valStatus, char* date0_) {
	if (date0_!=nullptr) strcpy_s(date0, DATE_FORMAT_LEN, date0_);
	safecall(sourceData, load, date0, stepsCnt, dtime, val[valSource][valStatus], bdtime, base);
	if (doDump) dump(valSource, valStatus);
	transform(valStatus);
}
void sTimeSerie::transform(int valSource, int dt_) {
	dt=(dt_==-1) ? dt : dt_;
	int curr=0;
	for (int s=0; s<(stepsCnt); s++) {
		for (int f=0; f<sourceData->featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				val[valSource][TR][curr]=val[valSource][BASE][curr];
				break;
			case DT_DELTA:
				if (s==0) {
					val[valSource][TR][curr]=val[valSource][BASE][curr]-base[f];
				} else {
					val[valSource][TR][curr]=val[valSource][BASE][curr]-val[valSource][BASE][(s-1)*sourceData->featuresCnt+f];
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
			if (val[valSource][TR][curr]<dmin[f]) dmin[f]=val[valSource][TR][curr];
			if (val[valSource][TR][curr]>dmax[f]) dmax[f]=val[valSource][TR][curr];

			curr++;
		}
	}
	if (doDump) dump(valSource, TR);
}
void sTimeSerie::scale(int valSource, int valStatus, float scaleMin_, float scaleMax_) {

	for (int f=0; f<sourceData->featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(dmax[f]-dmin[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*dmax[f]);
	}

	for (int f=0; f<sourceData->featuresCnt; f++) {
		for (int s=0; s<stepsCnt; s++) {
			val[valSource][TRS][s*sourceData->featuresCnt+f]=val[valSource][valStatus][s*sourceData->featuresCnt+f]*scaleM[f]+scaleP[f];
		}
	}

	if (doDump) dump(valSource, valStatus);

}
void sTimeSerie::unscale(int valSource, float scaleMin_, float scaleMax_, int selectedFeaturesCnt_, int* selectedFeature_, int skipFirstN_) {

		for (int s=0; s<stepsCnt; s++) {
			for (int tf=0; tf<sourceData->featuresCnt; tf++) {
				for(int df=0; df<selectedFeaturesCnt_; df++) {
					if (selectedFeature_[df]==tf) {
						if (s<skipFirstN_) {
							val[valSource][TR][s*sourceData->featuresCnt+tf]=EMPTY_VALUE;
						} else {
							val[valSource][TR][s*sourceData->featuresCnt+tf]=(val[valSource][TRS][s*sourceData->featuresCnt+tf]-scaleP[tf])/scaleM[tf];
						}
					}
				}
			}
	}
	if (doDump) dump(TR, valSource);
}
void sTimeSerie::dump(int valSource, int valStatus) {
	int s, f;

	char suffix1[10];
	if (valStatus==BASE) strcpy_s(suffix1, 10, "BASE");
	if (valStatus==TR) strcpy_s(suffix1, 10, "TR");
	if (valStatus==TRS) strcpy_s(suffix1, 10, "TRS");

	char suffix2[12];
	if (valSource==PREDICTED) {
		strcpy_s(suffix2, 12, "PREDICTED");
	} else {
		strcpy_s(suffix2, 12, "TARGET");
	}

	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s_%s-%s_dump_p%d_t%d_%p.csv", dumpPath, name->base, date0, suffix2, suffix1, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime");
	for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",F%d", f);
	fprintf(dumpFile, "\n%d,%s", -1, bdtime);
	for (f=0; f<sourceData->featuresCnt; f++) {
		fprintf(dumpFile, ",%f", base[f]);
		//for (int ff=0; ff<(sourceData->featuresCnt-3); ff++) fprintf(dumpFile, ",");
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(dumpFile, "\n%d, %s", s, dtime[s]);
		for (f=0; f<sourceData->featuresCnt; f++) {
			fprintf(dumpFile, ",%f", val[valSource][valStatus][s*sourceData->featuresCnt+f]);
		}
	}
	fprintf(dumpFile, "\n");

	if (valStatus==TR) {
		fprintf(dumpFile, "\ntr-min:");
		for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",,,%f", dmin[f]);
		fprintf(dumpFile, "\ntr-max:");
		for (f=0; f<sourceData->featuresCnt; f++) fprintf(dumpFile, ",,,%f", dmax[f]);
		fprintf(dumpFile, "\n");
	}
	if (valStatus==TRS) {
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
	dtime=(char**)malloc(len*sizeof(char*));
	for (int i=0; i<len; i++) dtime[i]=(char*)malloc(DATE_FORMAT_LEN);

	val=(numtype***)malloc(2*sizeof(numtype**));
	for (int source=0; source<2; source++) {
		val[source]=(numtype**)malloc(2*sizeof(numtype*));
		for (int status=0; status<3; status++) {
			val[source][status]=(numtype*)malloc(len*sizeof(numtype));
		}
	}
	base=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	dmin=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));
	for (int f=0; f<sourceData->featuresCnt; f++) { dmin[f]=1e9; dmax[f]=-1e9; }
	//--

}
void sTimeSerie::frees() {
	for (int i=0; i<len; i++) {
		free(dtime[i]); 
	}
	free(dtime);

	for (int source=0; source<2; source++) {
		for (int status=0; status<3; status++) {
			free(val[source][status]);
		}
		//free(val[source]);
	}
	free(val);

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

void sTimeSerie::untransform(int valSource, int selectedFeaturesCnt_, int* selectedFeature_){

	int i=0;
	
	numtype* prevval = (numtype*)malloc(sourceData->featuresCnt*sizeof(numtype));

	for (int s=0; s<(stepsCnt); s++) {
		for (int tf=0; tf<sourceData->featuresCnt; tf++) {
			for (int df=0; df<selectedFeaturesCnt_; df++) {
				if (selectedFeature_[df]==tf) {
					if (val[valSource][TR][i]==EMPTY_VALUE) {
						val[valSource][BASE][i]=EMPTY_VALUE;
					} else {
						switch (dt) {
						case DT_NONE:
							val[valSource][BASE][i] = val[valSource][TR][i];
							break;
						case DT_DELTA:
							if (s==0) {
								val[valSource][BASE][i] = val[valSource][TR][i]+base[tf];
							} else {
								val[valSource][BASE][i] = val[valSource][TR][i]+prevval[tf];
							}
							prevval[tf] = val[valSource][BASE][i];
							break;
						case DT_LOG:
							break;
						case DT_DELTALOG:
							break;
						default:
							break;
						}
					}
				}
			}
			i++;
		}
	}
	free(prevval);
}