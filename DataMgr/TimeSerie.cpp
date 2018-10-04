//#include <vld.h>
#include "TimeSerie.h"

//-- sTimeSerie, constructors / destructor
void sTimeSerie::sTimeSeriecommon(int steps_, int featuresCnt_, int tsfCnt_, int* tsf_) {

	steps=steps_;
	featuresCnt=featuresCnt_;
	len=steps*featuresCnt;
	dmin=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(featuresCnt*sizeof(numtype));
	for (int f=0; f<featuresCnt; f++) {
		dmin[f]=1e8; dmax[f]=-1e8;
	}
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dtime=(char**)malloc(len*sizeof(char*)); for (int i=0; i<len; i++) dtime[i]=(char*)malloc(12+1);
	bdtime=(char*)malloc(12+1);
	d=(numtype*)malloc(len*sizeof(numtype));
	bd=(numtype*)malloc(featuresCnt*sizeof(numtype));
	d_tr=(numtype*)malloc(len*sizeof(numtype));
	d_trs=(numtype*)malloc(len*sizeof(numtype));

}

//-------- To fix --------------
sTimeSerie::sTimeSerie(sCfgObjParmsDef, int steps_, int featuresCnt_) : sCfgObj(sCfgObjParmsVal) {
	sTimeSeriecommon(steps_, featuresCnt_, 0, nullptr);
}
sTimeSerie::sTimeSerie(sCfgObjParmsDef, sFXData* dataSource_, int steps_, char* date0_, int dt_) : sCfgObj(sCfgObjParmsVal) {
	//-- 1. create
	sTimeSeriecommon(steps_, FXDATA_FEATURESCNT, 0, nullptr);
	//-- 2. load data
	safecall(dataSource_, load, date0_, steps, dtime, d, bdtime, bd);	
	//-- 3. transform
	safecall(this, transform, dt_);
}
sTimeSerie::sTimeSerie(sCfgObjParmsDef, sFileData* dataSource_, int steps_, int featuresCnt_, char* date0_, int dt_) : sCfgObj(sCfgObjParmsVal) {
	featuresCnt=featuresCnt_;
}

bool sTimeSerie::setDataSource(sCfg* cfg) {

	bool found=false;
	sFXData* fxData;
	sFileData* fileData;
	tMT4Data* mt4Data;

	safecall(cfg, setKey, "File_DataSource", true, &found);	//-- ignore error
	if (found) {
		safecall(cfg, setKey, "../"); //-- get back;
		safespawn(false, fileData, newsname("File_DataSource"), nullptr, cfg, "File_DataSource");
		sourceData=fileData;
	} else {
		safecall(cfg, setKey, "FXDB_DataSource", true, &found);	//-- ignore error
		if (found) {
			safecall(cfg, setKey, "../"); //-- get back;
			safespawn(false, fxData, newsname("FXDB_DataSource"), nullptr, cfg, "FXDB_DataSource");
			sourceData=fxData;
		} else {
			safecall(cfg, setKey, "MT4_DataSource", true, &found);	//-- ignore error
			if (found) {
				safecall(cfg, setKey, "../"); //-- get back;
				safespawn(false, mt4Data, newsname("MT4_DataSource"), nullptr, cfg, "MT4_DataSource");
				sourceData=mt4Data;
			}
		}
	}
	return found;
}

sTimeSerie::sTimeSerie(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	tsf=(int*)malloc(MAX_TSF_CNT*sizeof(int));
	date0[0]='\0';

	//-- 1. common parameters
	safecall(cfgKey, getParm, &date0, "Date0");
	safecall(cfgKey, getParm, &steps, "HistoryLen");
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &BWcalc, "BWCalc");
	safecall(cfgKey, getParm, &tsf, "StatisticalFeatures", false, &tsfCnt);

	//-- 2. Find and set DataSource
	if(!setDataSource(cfg)) fail("No Valid DataSource Parameters Key found.");

	//-- 3. common stuff (mallocs, ...)
	sTimeSeriecommon(steps, sourceData->featuresCnt, tsfCnt, tsf);
	//-- 4. load data
	if (sourceData->type==FXDB_SOURCE) ((sFXData*)sourceData)->load(date0, steps, dtime, d, bdtime, bd);
	if (sourceData->type==FILE_SOURCE) ((sFileData*)sourceData)->load(date0, steps, dtime, d, bdtime, bd);
	//-- 5. transform
	safecall(this, transform, dt);

	len=steps*featuresCnt;
	dmin=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(featuresCnt*sizeof(numtype));
	for (int f=0; f<featuresCnt; f++) {
		dmin[f]=1e8; dmax[f]=-1e8;
	}
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dtime=(char**)malloc(len*sizeof(char*)); for (int i=0; i<len; i++) dtime[i]=(char*)malloc(12+1);
	bdtime=(char*)malloc(12+1);
	d=(numtype*)malloc(len*sizeof(numtype));
	bd=(numtype*)malloc(featuresCnt*sizeof(numtype));
	d_tr=(numtype*)malloc(len*sizeof(numtype));
	d_trs=(numtype*)malloc(len*sizeof(numtype));

}
sTimeSerie::~sTimeSerie() {
	free(d);
	free(bd);
	free(d_trs);
	free(d_tr);
	for (int i=0; i<len; i++) free(dtime[i]);
	free(dtime); free(bdtime);
	free(tsf);

}

//-- sTimeSerie, other methods
/*void sTimeSerie::load(sFXData* tsFXData_, char* pDate0) {
	fxData=tsFXData_;
	sourceType=FXDB_SOURCE;
	safecall(fxData, load, pDate0, steps, dtime, d, bdtime, bd);
}
void sTimeSerie::load(sFileData* tsFileData, char* pDate0) {
	fail("pDate0=%s", pDate0);
}
void sTimeSerie::load(tMT4Data* tsMT4Data, char* pDate0) {
	safeThrow("", 0);
}*/
void sTimeSerie::dump(char* dumpFileName) {
	int s, f;

	FILE* dumpFile;
	if( fopen_s(&dumpFile, dumpFileName, "w") !=0) fail("Could not open dump file %s . Error %d", dumpFileName, errno);

	fprintf(dumpFile, "i, datetime");
	for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",F%d_orig,F%d_tr,F%d_trs", f, f, f);
	fprintf(dumpFile, "\n%d,%s", -1, bdtime);
	for (f=0; f<featuresCnt; f++) {
		fprintf(dumpFile, ",%f", bd[f]);
		for (int ff=0; ff<(featuresCnt-3); ff++) fprintf(dumpFile, ",");
	}

	for (s=0; s<steps; s++) {
		fprintf(dumpFile, "\n%d, %s", s, dtime[s]);
		for (f=0; f<featuresCnt; f++) {
			fprintf(dumpFile, ",%f", d[s*featuresCnt+f]);
			if (hasTR) {
				fprintf(dumpFile, ",%f", d_tr[s*featuresCnt+f]);
			} else {
				fprintf(dumpFile, ",");
			}
			if (hasTRS) {
				fprintf(dumpFile, ",%f", d_trs[s*featuresCnt+f]);
			} else {
				fprintf(dumpFile, ",");
			}
		}
	}
	fprintf(dumpFile, "\n");

	if (hasTR) {
		fprintf(dumpFile, "\ntr-min:");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",,,%f", dmin[f]);
		fprintf(dumpFile, "\ntr-max:");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",,,%f", dmax[f]);
		fprintf(dumpFile, "\n");
	}
	if (hasTRS) {
		fprintf(dumpFile, "\nscaleM:");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",,,%f", scaleM[f]);
		fprintf(dumpFile, "\nscaleP:");
		for (f=0; f<featuresCnt; f++) fprintf(dumpFile, ",,,%f", scaleP[f]);
		fprintf(dumpFile, "\n");

		//fprintf(dumpFile, "scaleM:,,%f,,,%f,,,%f,,,%f,,,%f \n", scaleM[0], scaleM[1], scaleM[2], scaleM[3], scaleM[4]);
		//fprintf(dumpFile, "scaleP:,,%f,,,%f,,,%f,,,%f,,,%f \n", scaleP[0], scaleP[1], scaleP[2], scaleP[3], scaleP[4]);
	}

	delete dumpFile;

}
void sTimeSerie::transform(int dt_) {
	dt=dt_;
	for (int s=0; s<steps; s++) {
		for (int f=0; f<featuresCnt; f++) {
			switch (dt) {
			case DT_NONE:
				break;
			case DT_DELTA:
				if (s>0) {
					d_tr[s*featuresCnt+f]=d[s*featuresCnt+f]-d[(s-1)*featuresCnt+f];
				} else {
					d_tr[s*featuresCnt+f]=d[s*featuresCnt+f]-bd[f];
				}
				break;
			case DT_LOG:
				break;
			case DT_DELTALOG:
				break;
			default:
				break;
			}
			if (d_tr[s*featuresCnt+f]<dmin[f]) {
				dmin[f]=d_tr[s*featuresCnt+f];
			}
			if (d_tr[s*featuresCnt+f]>dmax[f]) {
				dmax[f]=d_tr[s*featuresCnt+f];
			}
		}
	}

	hasTR=true;
}
void sTimeSerie::scale(numtype scaleMin_, numtype scaleMax_) {
	//-- ScaleMin/Max depend on the core, scaleM/P are specific for each feature

	scaleMin=scaleMin_; scaleMax=scaleMax_;

	if (!hasTR) fail("-- must transform before scaling! ---");

	for (int f=0; f<featuresCnt; f++) {
		scaleM[f] = (scaleMax-scaleMin)/(dmax[f]-dmin[f]);
		scaleP[f] = scaleMax-scaleM[f]*dmax[f];
	}

	for (int s=0; s<steps; s++) {
		for (int f=0; f<featuresCnt; f++) {
			d_trs[s*featuresCnt+f]=d_tr[s*featuresCnt+f]*scaleM[f]+scaleP[f];
		}
	}

	hasTRS=true;
}
void sTimeSerie::TrS(int dt_, numtype scaleMin_, numtype scaleMax_) {
	dt=dt_;

	int s, f;
	//-- first, transform
	for (s=0; s<steps; s++) {
		for (f=0; f<featuresCnt; f++) {
			//dbg->write(DBG_LEVEL_DET, ",%f", 1, d[s*featuresCnt+f]);
			switch (dt) {
			case DT_NONE:
				break;
			case DT_DELTA:
				if (s>0) {
					d_tr[s*featuresCnt+f]=d[s*featuresCnt+f]-d[(s-1)*featuresCnt+f];
				} else {
					d_tr[s*featuresCnt+f]=d[s*featuresCnt+f]-bd[f];
				}
				break;
			case DT_LOG:
				break;
			case DT_DELTALOG:
				break;
			default:
				break;
			}
			if (d_tr[s*featuresCnt+f]<dmin[f]) {
				dmin[f]=d_tr[s*featuresCnt+f];
			}
			if (d_tr[s*featuresCnt+f]>dmax[f]) {
				dmax[f]=d_tr[s*featuresCnt+f];
			}
		}
	}


	//-- then, scale. ScaleMin/Max depend on the core, scaleM/P are specific for each feature
	for (f=0; f<featuresCnt; f++) {
		scaleM[f] = (scaleMax_-scaleMin_)/(dmax[f]-dmin[f]);
		scaleP[f] = scaleMax_-scaleM[f]*dmax[f];
	}

	for (s=0; s<steps; s++) {
		for (f=0; f<featuresCnt; f++) {
			d_trs[s*featuresCnt+f]=d_tr[s*featuresCnt+f]*scaleM[f]+scaleP[f];
			//dbg->write(DBG_LEVEL_DET, "%d,%s,,,%f", 3, s, dtime[s], d_trs[s*featuresCnt+f]);
		}
		//dbg->write(DBG_LEVEL_DET, "\n", 0);
	}

}
void sTimeSerie::unTrS(numtype scaleMin_, numtype scaleMax_) {
}

