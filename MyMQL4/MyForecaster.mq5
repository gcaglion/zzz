#property copyright "gcaglion"
#property link      "https://algoinvest.org"
#property version   "1.00"
#property strict

#import "Forecaster.dll"
int _dioPorco(int i1, uchar& oEnv[], int &o1);
int _createEnv(int accountId_, uchar& oEnv[], int &oSampleLen_, int &oPredictionLen_);
int _getForecast(uchar& iEnv[], double &iBarO[], double &iBarH[], double &iBarL[], double &iBarC[], double &iBarV[], double &oForecastH[], double &oForecastL[]);
int _destroyEnv(uchar& iEnv[]);
#import

//--- input parameters - Forecaster dll stuff
input int EnginePid;
input int DataTransformation	= 1;
input int  ValidationShift		= 0;
input bool SaveLogs				= true;
input int  Max_Retries			= 3;
//-- input parameters - Trade stuff
input double TradeSizeMin		= 0.01;
input double TradeSizeMax		= 2.00;
input double TradeSizeDef		= 0.30;
input double RiskRatio			= 0.20;
input bool   CloseOpenTrades	= true;
input int	 Default_Slippage	= 8;
input int    MinProfitPIPs		= 5;

//--- local variables
int vSampleLen=0;
int vPredictionLen=0;
int vValidationShift=-ValidationShift;

//--- data variables to be passed in MTgetForecast() call
string vSampleDataT[], vSampleBaseValT;
double vSampleDataO[], vSampleBaseValO;
double vSampleDataH[], vSampleBaseValH;
double vSampleDataL[], vSampleBaseValL;
double vSampleDataC[], vSampleBaseValC;
double vSampleDataV[];
double vFutureDataO[];
double vFutureDataH[];
double vFutureDataL[];
double vFutureDataC[];
double vPredictedDataH[], vPredictedDataL[];
double vSampleBW[];
double vFutureBW[];

//--- bars timestamps
string sCurrentBarTime; uchar vCurrentBarTime[];
string vSampleTime[], vValidationTime[];
string vSampleBaseTime, vValidationBaseTime;

//--- miscellaneous variables
uchar vEnvS[];		// Env in char* format
int DllRetVal=0;	// used from OnDeinit() to determine whether to commit changes to DB
string sParamOverride="MT4";
int    vBarId;
uchar vFirstBarT[]; uchar vLastBarT[];
double vPrevFH0 = 0;
double vPrevFL0 = 0;
double vTradeSize=0;			// vTradeSize is global non-static. It is set by NewTrade(), and is needed by MTSaveTradeInfo()
double vTradeTP=0, vTradeSL=0;	// these, too, are set in NewTrade()
bool   fLog;
uint t0, t1;						// Time counters. Used to calc elapsed

int OnInit() {
	//Print("Bar count is ",Bars(Symbol(), Period()));
	t0 = GetTickCount();

	//--- counters used to keep track of graphic objects on chart
	if(!GlobalVariableCheck("PrevFH0")) GlobalVariableSet("PrevFH0",0);
	if(!GlobalVariableCheck("PrevFL0")) GlobalVariableSet("PrevFL0",0);

	//-- 1. create Env
	string EnvS;
	EnvS = "00000000000000000000000000000000000000000000000000000000000000000"; StringToCharArray(EnvS, vEnvS);
	int in=32;
	int out=0;

/*	printf("Calling _dioPorco()...");
	DllRetVal = _dioPorco(in, vEnvS, out);
	EnvS=CharArrayToString(vEnvS);
	printf("in=%d ; out=%d ; envS=%s", in, out, EnvS);
*/
	printf("Calling _createEnv()...");
	DllRetVal = _createEnv(100, vEnvS, vSampleLen, vPredictionLen);
	EnvS=CharArrayToString(vEnvS);
	printf("EnvS=%s ; vSampleLen=%d ; vPredictionLen=%d", EnvS, vSampleLen, vPredictionLen);

	//--- Resize Sample and Prediction arrays (+1 is for BaseVal)
	ArrayResize(vSampleDataT, vSampleLen);
	ArrayResize(vSampleDataO, vSampleLen);
	ArrayResize(vSampleDataH, vSampleLen);
	ArrayResize(vSampleDataL, vSampleLen);
	ArrayResize(vSampleDataC, vSampleLen);
	ArrayResize(vSampleBW, vSampleLen);
	ArrayResize(vFutureDataH, vPredictionLen);
	ArrayResize(vFutureDataL, vPredictionLen);
	ArrayResize(vFutureBW, vPredictionLen);
	ArrayResize(vPredictedDataH, vPredictionLen);
	ArrayResize(vPredictedDataL, vPredictionLen);
	ArrayResize(vSampleTime, vSampleLen);
	ArrayResize(vValidationTime, vSampleLen);

	return(INIT_SUCCEEDED);
}
void OnTick() {
	// Only do this if there's a new bar
	static datetime Time0=0;
	if (Time0==SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE)) return; 
	Time0 = SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE);

	int vTradeType;
	int ret;

	//-- load bars into arrrays
	printf("Time0=%s . calling LoadBars()...", Time0);
	LoadBars();

	//-- before GetForecast()
	StringToCharArray(vSampleTime[0], vFirstBarT);
	StringToCharArray(vSampleTime[vSampleLen-1], vLastBarT);
	if (vBarId>0) {
		//printf("RunForecast() CheckPoint 2: Previous Forecast (H|L)=%f|%f ; Previous Actual (H|L)=%f|%f => Previous Error (H|L)=%f|%f", vPrevFH0, vPrevFL0, High[1], Low[1], MathAbs(vPrevFH0-High[1]), MathAbs(vPrevFL0-Low[1]));
	}

	//DllRetVal=GetForecast();
}
void LoadBars() {
	//-- This loads bar values into Sample and Base arrays

	MqlRates rates[];
	int copied=CopyRates(NULL, 0, 1, vSampleLen+2, rates);
	if (copied<(vSampleLen+2))
		Print("Error copying price data ", GetLastError());
	else Print("Copied ", ArraySize(rates), " bars");

	//-- base bar, needed for Delta Transformation
	StringConcatenate(vSampleBaseValT, TimeToString(rates[0].time, TIME_DATE), ".", TimeToString(rates[0].time, TIME_MINUTES));
	vSampleBaseValO = rates[0].open;
	vSampleBaseValH = rates[0].high;
	vSampleBaseValL = rates[0].low;
	vSampleBaseValC = rates[0].close;
	printf("Base Bar: T=%s - O=%f - H=%f - L=%f - C=%f", vSampleBaseValT, vSampleBaseValO, vSampleBaseValH, vSampleBaseValL, vSampleBaseValC);
	//-- whole sample
	for (int i = 0; i<vSampleLen; i++) {    // (i=0 is the current bar)
		StringConcatenate(vSampleDataT[i], TimeToString(rates[i+1].time, TIME_DATE), ".", TimeToString(rates[i+1].time, TIME_MINUTES));
		vSampleDataO[i] = rates[i+1].open;
		vSampleDataH[i] = rates[i+1].high;
		vSampleDataL[i] = rates[i+1].low;
		vSampleDataC[i] = rates[i+1].close;
		printf("Bar[%d]: T=%s - O=%f - H=%f - L=%f - C=%f", i, vSampleDataT[i], vSampleDataO[i], vSampleDataH[i], vSampleDataL[i], vSampleDataC[i]);
	}

	//-- call Forecaster
	_getForecast(vEnvS, vSampleDataO, vSampleDataH, vSampleDataL, vSampleDataC, vSampleDataO, vPredictedDataH, vPredictedDataL);
	for (int i=0; i<vPredictionLen; i++) {
		printf("vPredictedDataH[%d]=%f , vPredictedDataL[%d]=%f", i, vPredictedDataH[i], vPredictedDataL[i]);
	}
}
