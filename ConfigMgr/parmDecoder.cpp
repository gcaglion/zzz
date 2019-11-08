#include "sCfgParm.h"
#include "XMLdefs.h"

#include "../DataMgr/DataSource_enums.h"
#include "../DataMgr/FileData_enums.h"
#include "../DataMgr/FXData_enums.h"
#include "../DataMgr/DBConnection_enums.h"
#include "../DataMgr/TimeSerie_enums.h"
#include "../Logger/Logger_enums.h"
#include "../EngineMgr/Engine_enums.h"
#include "../EngineMgr/Core_enums.h"
#include "../EngineMgr/sNNenums.h"
#include "../EngineMgr/sGAenums.h"
#include "../EngineMgr/sSVMenums.h"
#include "../EngineMgr/sSOMenums.h"
#include "../Wavelib/wavelet2d_enums.h"

#define optionLookup(option, e) { \
	if (_stricmp(valS[elementId], #option)==0) { \
		(*oVal)=option; \
		return; \
	} \
}
void sCfgParm::decode(int elementId, int* oVal) { 
	bool success=false;

	//-- DataSource_enums
	optionLookup(DB_SOURCE, elementId);
	optionLookup(FILE_SOURCE, elementId);
	optionLookup(MT4_SOURCE, elementId);

	//-- Logger enums
	optionLookup(ORADATA, elementId);
	optionLookup(FILEDATA, elementId);
	//-- Debugger enums
//	optionLookup(DEFAULT_DBG_VERBOSITY, elementId);
//	optionLookup(DEFAULT_DBG_TIMING, elementId);
//	optionLookup(DEFAULT_DBG_PAUSERR, elementId);
//	optionLookup(DBG_DEST_FILE, elementId);
//	optionLookup(DBG_DEST_SCREEN, elementId);
//	optionLookup(DBG_DEST_BOTH, elementId);
	//-- FileData_enums
	optionLookup(COMMA_SEPARATOR, elementId);
	optionLookup(TAB_SEPARATOR, elementId);
	optionLookup(SPACE_SEPARATOR, elementId);

	//-- FXData enums
	optionLookup(FXOPEN, elementId);
	optionLookup(FXHIGH, elementId);
	optionLookup(FXLOW, elementId);
	optionLookup(FXCLOSE, elementId);
	optionLookup(FXVOLUME, elementId);
	//-- TimeSerie enums
	optionLookup(DT_NONE, elementId);
	optionLookup(DT_DELTA, elementId);
	optionLookup(DT_LOG, elementId);
	optionLookup(DT_DELTALOG, elementId);
	optionLookup(TSF_MEAN, elementId);
	optionLookup(TSF_MAD, elementId);
	optionLookup(TSF_VARIANCE, elementId);
	optionLookup(TSF_SKEWNESS, elementId);
	optionLookup(TSF_KURTOSIS, elementId);
	optionLookup(TSF_TURNINGPOINTS, elementId);
	optionLookup(TSF_SHE, elementId);
	optionLookup(TSF_HISTVOL, elementId);

	//-- Engine enums
	optionLookup(ENGINE_XIE, elementId);
	optionLookup(ENGINE_WNN, elementId);
	optionLookup(ENGINE_CUSTOM, elementId);
	optionLookup(WT_NONE, elementId);
	optionLookup(WT_HAAR, elementId);
	optionLookup(WT_DB1, elementId);
	optionLookup(WT_DB2, elementId);
	optionLookup(WT_DB3, elementId);
	optionLookup(WT_DB4, elementId);
	optionLookup(WT_DB5, elementId);
	optionLookup(WT_DB6, elementId);
	optionLookup(WT_DB7, elementId);
	optionLookup(WT_DB8, elementId);
	optionLookup(WT_DB9, elementId);
	optionLookup(WT_DB10, elementId);
	optionLookup(WT_DB11, elementId);
	optionLookup(WT_DB12, elementId);
	optionLookup(WT_BIOR11, elementId);
	optionLookup(WT_BIOR13, elementId);
	optionLookup(WT_BIOR15, elementId);
	optionLookup(WT_BIOR22, elementId);
	optionLookup(WT_BIOR24, elementId);
	optionLookup(WT_BIOR26, elementId);
	optionLookup(WT_BIOR28, elementId);
	optionLookup(WT_BIOR31, elementId);
	optionLookup(WT_BIOR33, elementId);
	optionLookup(WT_BIOR35, elementId);
	optionLookup(WT_BIOR37, elementId);
	optionLookup(WT_BIOR39, elementId);
	optionLookup(WT_BIOR44, elementId);
	optionLookup(WT_BIOR55, elementId);
	optionLookup(WT_BIOR68, elementId);
	optionLookup(WT_COIF1, elementId);
	optionLookup(WT_COIF2, elementId);
	optionLookup(WT_COIF3, elementId);
	optionLookup(WT_COIF4, elementId);
	optionLookup(WT_COIF5, elementId);
	optionLookup(WT_SYM2, elementId);
	optionLookup(WT_SYM3, elementId);
	optionLookup(WT_SYM4, elementId);
	optionLookup(WT_SYM5, elementId);
	optionLookup(WT_SYM6, elementId);
	optionLookup(WT_SYM7, elementId);
	optionLookup(WT_SYM8, elementId);
	optionLookup(WT_SYM9, elementId);
	optionLookup(WT_SYM10, elementId);

	//-- Core enums
	optionLookup(CORE_NN, elementId);
	optionLookup(CORE_GA, elementId);
	optionLookup(CORE_SVM, elementId);
	optionLookup(CORE_SOM, elementId);
	optionLookup(CORE_DUMB, elementId);
	//-- Core connections
	optionLookup(CONN_DENSE, elementId);
	optionLookup(CONN_LINEAR, elementId);
	optionLookup(CONN_TRANSFORM, elementId);
	//-- NN enums
	optionLookup(TP_STOCHASTIC, elementId);
	optionLookup(TP_BATCH, elementId);
	optionLookup(TP_ONLINE, elementId);
	optionLookup(NN_ACTIVATION_TANH, elementId);
	optionLookup(NN_ACTIVATION_TANH2, elementId);
	optionLookup(NN_ACTIVATION_EXP4, elementId);
	optionLookup(NN_ACTIVATION_RELU, elementId);
	optionLookup(NN_ACTIVATION_SOFTPLUS, elementId);
	optionLookup(BP_STD, elementId);
	optionLookup(BP_QING, elementId);
	optionLookup(BP_RPROP, elementId);
	optionLookup(BP_QUICKPROP, elementId);
	optionLookup(BP_SCGD, elementId);
	optionLookup(BP_LM, elementId);

	fail("could not decode value %s for parameter %s", valS[elementId], name->full);

}