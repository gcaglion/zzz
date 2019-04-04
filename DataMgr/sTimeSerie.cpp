#include "sTimeSerie.h"
#include <vld.h>

sTimeSerie::sTimeSerie(sObjParmsDef, sDataSource* sourceData_, const char* date0_, int stepsCnt_, int dt_, bool doDump_, const char* dumpPath_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	mallocs1();

	strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, date0_);
	stepsCnt=stepsCnt_;
	featuresCnt=sourceData_->featuresCnt;
	dt=dt_;

	doDump=doDump_;
	if (dumpPath_!=nullptr) {
		strcpy_s(dumpPath, MAX_PATH, dumpPath_);
	} else {
		strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	}

	mallocs2();
	load(sourceData_);
}
sTimeSerie::sTimeSerie(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	mallocs1();
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &date0, "Date0");
	safecall(cfgKey, getParm, &stepsCnt, "HistoryLen"); 
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &doDump, "Dump");
	//-- 0. default dump path is dbg outfilepath
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	safecall(cfgKey, getParm, &dumpPath, "DumpPath", true);
	//-- 2. do stuff and spawn sub-Keys
	sDataSource* _dataSrc;
	safecall(this, setDataSource, &_dataSrc);
	featuresCnt=_dataSrc->featuresCnt;
	mallocs2();
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

	load(_dataSrc);
}
sTimeSerie::~sTimeSerie() {
	frees();
}

void sTimeSerie::load(sDataSource* dataSrc, char* date0_) {
	if (date0_!=nullptr) strcpy_s(date0, DATE_FORMAT_LEN, date0_);
	safecall(dataSrc, load, date0, stepsCnt, dtime, val[ACTUAL][BASE], bdtime, base, barWidth);
	//-- since the actual date0 we get from query can be different from the one requested, we update it after the load
	//strcpy_s(date0, XMLKEY_PARM_VAL_MAXLEN, dtime[stepsCnt-1]);
	//--
	if (doDump) dump(ACTUAL, BASE);
	//-- 1. calc TSFs
	safecall(this, calcTSFs);
	//-- 2. transform
	safecall(this, transform, ACTUAL);
}
void sTimeSerie::transform(int valSource) {
	int curr=0;
	for (int s=0; s<(stepsCnt); s++) {
		for (int f=0; f<featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				val[valSource][TR][curr]=val[valSource][BASE][curr];
				break;
			case DT_DELTA:
				if (val[valSource][BASE][curr]==EMPTY_VALUE) {
					val[valSource][TR][curr]=EMPTY_VALUE;
				} else {
					if (s==0) {
						val[valSource][TR][curr]=val[valSource][BASE][curr]-base[f];
					} else {
						val[valSource][TR][curr]=val[valSource][BASE][curr]-val[valSource][BASE][(s-1)*featuresCnt+f];
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
			if (val[valSource][TR][curr]!=EMPTY_VALUE&&val[valSource][TR][curr]<dmin[f]) dmin[f]=val[valSource][TR][curr];
			if (val[valSource][TR][curr]!=EMPTY_VALUE&&val[valSource][TR][curr]>dmax[f]) dmax[f]=val[valSource][TR][curr];

			curr++;
		}
	}
	if (doDump) dump(valSource, TR);
}
void sTimeSerie::untransform(int valSource) {
	int curr, prev;
	for (int s=0; s<stepsCnt; s++) {
		for (int f=0; f<featuresCnt; f++) {
			curr=s*featuresCnt+f;
			prev=(s-1)*featuresCnt+f;
			if (dt==DT_NONE) {
				val[valSource][BASE][curr]=val[valSource][TR][curr];
			}
			if (dt==DT_DELTA) {
				if (s>0) {
					if (val[valSource][TR][curr]==EMPTY_VALUE) {
						val[valSource][BASE][curr]=EMPTY_VALUE;
					} else {
						val[valSource][BASE][curr]=val[valSource][TR][curr]+val[ACTUAL][BASE][prev];
						if(val[ACTUAL][BASE][curr]==EMPTY_VALUE) val[ACTUAL][BASE][curr]=val[valSource][BASE][curr];
					}
				} else {
					if (val[valSource][TR][curr]==EMPTY_VALUE) {
						val[valSource][BASE][curr]=EMPTY_VALUE;
					} else {
						val[valSource][BASE][curr]=val[valSource][TR][curr]+base[f];
					}
				}
			}
		}
	}
}
void sTimeSerie::scale(int valSource, int valStatus, float scaleMin_, float scaleMax_) {

	for (int f=0; f<featuresCnt; f++) {
		scaleM[f] = (scaleMin_==scaleMax_) ? 1 : ((scaleMax_-scaleMin_)/(dmax[f]-dmin[f]));
		scaleP[f] = (scaleMin_==scaleMax_) ? 0 : (scaleMax_-scaleM[f]*dmax[f]);
	}

	for (int f=0; f<featuresCnt; f++) {
		for (int s=0; s<stepsCnt; s++) {
			if (val[valSource][valStatus][s*featuresCnt+f]==EMPTY_VALUE) {
				val[valSource][TRS][s*featuresCnt+f]=EMPTY_VALUE;
			} else {
				val[valSource][TRS][s*featuresCnt+f]=val[valSource][valStatus][s*featuresCnt+f]*scaleM[f]+scaleP[f];
			}
		}
	}

	if (doDump) dump(valSource, TRS);
}
void sTimeSerie::unscale(int valSource, float scaleMin_, float scaleMax_, int selectedFeaturesCnt_, int* selectedFeature_, int skipFirstNsteps_) {

	for (int s=0; s<stepsCnt; s++) {
		for (int tf=0; tf<featuresCnt; tf++) {
			for (int df=0; df<selectedFeaturesCnt_; df++) {
				if (selectedFeature_[df]==tf) {
					if (s<skipFirstNsteps_) {
						val[valSource][TR][s*featuresCnt+tf]=EMPTY_VALUE;
					} else {
						val[valSource][TR][s*featuresCnt+tf]=(val[valSource][TRS][s*featuresCnt+tf]-scaleP[tf])/scaleM[tf];
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
		strcpy_s(suffix2, 12, "ACTUAL");
	}

	char dumpFileName[MAX_PATH];
	sprintf_s(dumpFileName, "%s/%s_%s-%s_dump_p%d_t%d_%p.csv", dumpPath, name->base, suffix2, suffix1, GetCurrentProcessId(), GetCurrentThreadId(), this);
	FILE* dumpFile;
	if (fopen_s(&dumpFile, dumpFileName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime");
	for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",F%d", f);
	fprintf(dumpFile, "\n%d,%s", -1, bdtime);
	for (f=0; f<featuresCnt; f++) {
		fprintf(dumpFile, ",%f", base[f]);
		//for (int ff=0; ff<(featuresCnt-3); ff++) fprintf(dumpFile, ",");
	}

	for (s=0; s<stepsCnt; s++) {
		fprintf(dumpFile, "\n%d, %s", s, dtime[s]);
		for (f=0; f<featuresCnt; f++) {
			fprintf(dumpFile, ",%f", val[valSource][valStatus][s*featuresCnt+f]);
		}
	}
	fprintf(dumpFile, "\n");

	if (valStatus==TR) {
		fprintf(dumpFile, "\ntr-min:,");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",%f", dmin[f]);
		fprintf(dumpFile, "\ntr-max:,");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",%f", dmax[f]);
		fprintf(dumpFile, "\n");
	}
	if (valStatus==TRS) {
		fprintf(dumpFile, "\nscaleM:,");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",%f", scaleM[f]);
		fprintf(dumpFile, "\nscaleP:,");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",%f", scaleP[f]);
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
	len=stepsCnt*featuresCnt;
	dtime=(char**)malloc(len*sizeof(char*));
	for (int i=0; i<len; i++) dtime[i]=(char*)malloc(DATE_FORMAT_LEN);

	val=(numtype***)malloc(2*sizeof(numtype**));
	for (int source=0; source<2; source++) {
		val[source]=(numtype**)malloc(3*sizeof(numtype*));
		for (int status=0; status<3; status++) {
			val[source][status]=(numtype*)malloc(len*sizeof(numtype));
		}
	}
	base=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dmin=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
	for (int f=0; f<featuresCnt; f++) { dmin[f]=1e9; dmax[f]=-1e9; }
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
		free(val[source]);
	}
	free(val);
	free(barWidth);

	free(base);
	free(dmin);	free(dmax);
	free(scaleM); free(scaleP);
	free(date0);
	free(dumpPath);
}
void sTimeSerie::setDataSource(sDataSource** dataSrc_) {

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

	//-- then, open
	//safecall(sourceData, open);
}

/*
void sTimeSerie::untransform(int fromValSource, int toValSource, int sampleLen_, int selectedFeaturesCnt_, int* selectedFeature_) {

	int fromStep=sampleLen_;
	int toStep=stepsCnt;

	dataUnTransform(dt, stepsCnt, featuresCnt, fromStep, toStep, val[fromValSource][TR], base, val[ACTUAL][BASE], val[toValSource][BASE]);

}
*/
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

	//-- all of them are calculated on ACTUAL-BASE value

	tsf[TSF_MEAN]=TSMean(len, val[ACTUAL][BASE]);
	tsf[TSF_MAD]=TSMeanAbsoluteDeviation(len, val[ACTUAL][BASE]);
	tsf[TSF_VARIANCE]=TSVariance(len, val[ACTUAL][BASE]);
	tsf[TSF_SKEWNESS]=TSSkewness(len, val[ACTUAL][BASE]);
	tsf[TSF_KURTOSIS]=TSKurtosis(len, val[ACTUAL][BASE]);
	tsf[TSF_TURNINGPOINTS]=TSTurningPoints(len, val[ACTUAL][BASE]);
	tsf[TSF_SHE]=TSShannonEntropy(len, val[ACTUAL][BASE]);
	tsf[TSF_HISTVOL]=TSHistoricalVolatility(len, val[ACTUAL][BASE]);

}

