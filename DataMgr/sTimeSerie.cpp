#include "sTimeSerie.h"

sTimeSerie::sTimeSerie(sObjParmsDef, sDataSource* sourceData_, const char* date0_, int stepsCnt_, int dt_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	mallocs1();

	strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	stepsCnt=stepsCnt_;
	dt=dt_;
	sourceData=sourceData_;

	doDump=false;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
		doDump=true;
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	mallocs2();
}
sTimeSerie::sTimeSerie(sCfgObjParmsDef, int extraSteps) : sCfgObj(sCfgObjParmsVal) {
	mallocs1();
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &date0, "Date0");
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen"); 
	stepsCnt+=extraSteps;
	safecall(cfgKey, getParm, &dt, "DataTransformation");
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
	info("date0_=%s", date0_);
	if (date0_!=nullptr) strcpy_s(date0, DATE_FORMAT_LEN, date0_);
	info("CheckPoint 1");
	safecall(sourceData, load, date0, stepsCnt, dtime, val[valSource][valStatus], bdtime, base, barWidth);
	//-- since the actual date0 we get from query can be different from the one requested, we update it after the load
	strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, dtime[stepsCnt-1]);
	//--
	if (doDump) dump(valSource, valStatus);
	//-- 1. calc TSFs
	safecall(this, calcTSFs);
	//-- 2. transform
	safecall(this, transform, valSource, dt);
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

	if (doDump) dump(valSource, TRS);
}
void sTimeSerie::unscale(int valSource, float scaleMin_, float scaleMax_, int selectedFeaturesCnt_, int* selectedFeature_, int skipFirstNsteps_) {

	for (int s=0; s<stepsCnt; s++) {
		for (int tf=0; tf<sourceData->featuresCnt; tf++) {
			for (int df=0; df<selectedFeaturesCnt_; df++) {
				if (selectedFeature_[df]==tf) {
					if (s<skipFirstNsteps_) {
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
	sprintf_s(dumpFileName, "%s/%s_%s-%s_dump_p%d_t%d_%p.csv", dumpPath, name->base, suffix2, suffix1, GetCurrentProcessId(), GetCurrentThreadId(), this);
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
void sTimeSerie::mallocs1() {
	date0=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	dumpPath=(char*)malloc(MAX_PATH);
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
	barWidth=(numtype*)malloc(stepsCnt*sizeof(numtype));

}
void sTimeSerie::frees() {
	for (int i=0; i<len; i++) free(dtime[i]);
	free(dtime);

	for (int source=0; source<2; source++) {
		for (int status=0; status<3; status++) {
			free(val[source][status]);
		}
		//free(val[source]);
	}
	free(val);
	free(barWidth);

	free(base);
	free(dmin);	free(dmax);
	free(scaleM); free(scaleP);
	free(date0);
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

void sTimeSerie::untransform(int fromValSource, int toValSource, int sampleLen_, int selectedFeaturesCnt_, int* selectedFeature_) {

	int fromStep=sampleLen_;
	int toStep=stepsCnt;

	dataUnTransform(dt, stepsCnt, sourceData->featuresCnt, fromStep, toStep, val[fromValSource][TR], base, val[TARGET][BASE], val[toValSource][BASE]);

}

void dataUnTransform(int dt_, int stepsCnt, int featuresCnt_, int fromStep_, int toStep_, numtype* idata, numtype* baseVal, numtype* iActual, numtype* odata) {
	numtype* prev=(numtype*)malloc(featuresCnt_*sizeof(numtype));
	int s;

	for (int f=0; f<featuresCnt_; f++) {
		switch (dt_) {
		case DT_DELTA:
			for (s=fromStep_; s<toStep_; s++) {
				if (s>fromStep_) {
					prev[f] = (iActual[(s-1)*featuresCnt_+f]!=EMPTY_VALUE) ? iActual[(s-1)*featuresCnt_+f] : odata[(s-1)*featuresCnt_+f];
					odata[s*featuresCnt_+f] = idata[s*featuresCnt_+f]+prev[f];
				} else {
					if (fromStep_>0) {
						prev[f] = iActual[(s-1)*featuresCnt_+f];
					} else {
						prev[f] = baseVal[f];
					}
					odata[s*featuresCnt_+f] = EMPTY_VALUE;
				}
			}
			break;
		case DT_LOG:
			for (s = fromStep_; s<toStep_; s++) odata[s*featuresCnt_+f] = exp(idata[s*featuresCnt_+f])+(numtype)1e4-1;
			break;
		case DT_DELTALOG:
			for (s= fromStep_; s<toStep_; s++) {
				//-- 1. unLOG
				odata[s*featuresCnt_+f] = exp(idata[s*featuresCnt_+f])+(numtype)1e4-1;
				//-- 2. unDELTA
				if (s>fromStep_) {
					prev[f] = iActual[(s-1)*featuresCnt_+f];
				} else {
					if (fromStep_>0) {
						prev[f] = iActual[(s-1)*featuresCnt_+f];
					} else {
						prev[f] = baseVal[f];
					}
				}
				odata[s*featuresCnt_+f] = odata[s*featuresCnt_+f]+prev[f];
			}
			break;
		default:
			for (s=fromStep_; s<toStep_; s++) odata[s*featuresCnt_+f] = idata[s*featuresCnt_+f];
			break;
		}

		for (s=0; s<fromStep_; s++) odata[s*featuresCnt_+f] = EMPTY_VALUE;
		for (s=toStep_; s<stepsCnt; s++) odata[s*featuresCnt_+f] = EMPTY_VALUE;
	}

	free(prev);
}
void dataUnScale(numtype* scaleM_, numtype* scaleP_, int stepsCnt, int featuresCnt_, int fromStep_, int toStep_, numtype* idata, numtype* odata) {
}

//-- Timeseries Statistical Features
void sTimeSerie::calcTSFs() {

	//-- all of them are calculated on TARGET-BASE value

	tsf[TSF_MEAN]=TSMean(len, val[TARGET][BASE]);
	tsf[TSF_MAD]=TSMeanAbsoluteDeviation(len, val[TARGET][BASE]);
	tsf[TSF_VARIANCE]=TSVariance(len, val[TARGET][BASE]);
	tsf[TSF_SKEWNESS]=TSSkewness(len, val[TARGET][BASE]);
	tsf[TSF_KURTOSIS]=TSKurtosis(len, val[TARGET][BASE]);
	tsf[TSF_TURNINGPOINTS]=TSTurningPoints(len, val[TARGET][BASE]);
	tsf[TSF_SHE]=TSShannonEntropy(len, val[TARGET][BASE]);
	tsf[TSF_HISTVOL]=TSHistoricalVolatility(len, val[TARGET][BASE]);

}

