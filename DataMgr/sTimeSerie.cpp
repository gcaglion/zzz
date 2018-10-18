//#include <vld.h>
#include "sTimeSerie.h"

//-- sTimeSerie, constructors / destructor
void sTimeSerie::sTimeSeriecommon() {

	len=steps*featuresCnt;
	dmin=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dmax=(numtype*)malloc(featuresCnt*sizeof(numtype));
	for (int f=0; f<featuresCnt; f++) {
		dmin[f]=1e8; dmax[f]=-1e8;
	}
	scaleM=(numtype*)malloc(featuresCnt*sizeof(numtype));
	scaleP=(numtype*)malloc(featuresCnt*sizeof(numtype));
	dtime=(char**)malloc(len*sizeof(char*)); for (int i=0; i<len; i++) dtime[i]=(char*)malloc(DATE_FORMAT_LEN);
	bdtime=(char*)malloc(DATE_FORMAT_LEN);
	d=(numtype*)malloc(len*sizeof(numtype));
	bd=(numtype*)malloc(featuresCnt*sizeof(numtype));
	d_tr=(numtype*)malloc(len*sizeof(numtype));
	d_trs=(numtype*)malloc(len*sizeof(numtype));
}

void sTimeSerie::setDataSource(sCfg* cfg) {

	bool found=false;
	sFXDataSource* fxData;
	sGenericDataSource* fileData;
	tMT4Data* mt4Data;

	//-- first, find and set
	safecall(cfg, setKey, "File_DataSource", true, &found);	//-- ignore error
	if (found) {
		safecall(cfg, setKey, "../"); //-- get back;
		safespawn(fileData, newsname("File_DataSource"), defaultdbg, cfg, "File_DataSource", true);
		featuresCnt=fileData->featuresCnt;
		sourceData=fileData;
	} else {
		safecall(cfg, setKey, "FXDB_DataSource", true, &found);	//-- ignore error
		if (found) {
			safecall(cfg, setKey, "../"); //-- get back;
			safespawn(fxData, newsname("FXDB_DataSource"), defaultdbg, cfg, "FXDB_DataSource", true);
			featuresCnt=FXDATA_FEATURESCNT;
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
	if(!found) fail("No Valid DataSource Parameters Key found.");

	//-- then, open
	safecall(sourceData, open);

}
void sTimeSerie::load(char* date0_) {
	//-- 1. set date0
	strcpy_s(date0, DATE_FORMAT_LEN, date0_);
	//-- 2. load data
	safecall(sourceData, load, date0, steps, dtime, d, bdtime, bd);
	//-- 3. transform
	safecall(this, transform, dt);
	//-- 4. dump
	if (strlen(dumpFileFullName)>0) dump();
}

sTimeSerie::sTimeSerie(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	dumpFileFullName=(char*)malloc(MAX_PATH); dumpFileFullName[0]='\0';
	tsf=(int*)malloc(MAX_TSF_CNT*sizeof(int));

	//-- 1. common parameters
	safecall(cfgKey, getParm, &steps, "HistoryLen");
	safecall(cfgKey, getParm, &dt, "DataTransformation");
	safecall(cfgKey, getParm, &BWcalc, "BWCalc");
	safecall(cfgKey, getParm, &tsf, "StatisticalFeatures", false, &tsfCnt);
	safecall(cfgKey, getParm, &dumpFileFullName, "DumpFileFullName", true);

	//-- 2.1. Find, set, open DataSource
	safecall(this, setDataSource, cfg);
	//-- 2.2. common stuff (mallocs, ...)
	sTimeSeriecommon();

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
sTimeSerie::~sTimeSerie() {
	free(d);
	free(bd);
	free(d_trs);
	free(d_tr);
	for (int i=0; i<len; i++) free(dtime[i]);
	free(dtime); free(bdtime);
	free(tsf);
	free(dumpFileFullName);
}

//-- sTimeSerie, other methods
void sTimeSerie::dump() {
	int s, f;

	FILE* dumpFile;
	strcat_s(dumpFileFullName, MAX_PATH, date0);
	if (fopen_s(&dumpFile, dumpFileFullName, "w")!=0) fail("Could not open dump file %s . Error %d", dumpFileFullName, errno);

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

	fclose(dumpFile);

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

