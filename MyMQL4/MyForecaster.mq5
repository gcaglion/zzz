#property copyright "gcaglion"
#property link      "https://algoinvest.org"
#property version   "1.00"
#property strict

#include <Trade/Trade.mqh>

#define MT_MAX_SERIES_CNT 12

#import "Forecaster.dll"
//--
int _createEnv2(int accountId_, uchar& clientXMLFile_[], int savedEnginePid_, bool useVolume_, int dt_, bool doDump_, uchar& oEnv[]);
int _getSeriesInfo(uchar& iEnv[], int& oSeriesCnt_, int& oHistoryLen_, uchar& oSymbolsCSL_[], uchar& oTimeFramesCSL_[], uchar& oFeaturesCSL_[]);
int _getForecast2(uchar& iEnv[], int seriesCnt_, int historyLen_, int dt_, int& featMask_[], int& iBarT[], double &iBarO[], double &iBarH[], double &iBarL[], double &iBarC[], double &iBarV[], int &iBaseBarT[], double &iBaseBarO[], double &iBaseBarH[], double &iBaseBarL[], double &iBaseBarC[], double &iBaseBarV[], double &oForecastO[], double &oForecastH[], double &oForecastL[], double &oForecastC[], double &oForecastV[]);
//--
int _createEnv(int accountId_, uchar& clientXMLFile_[], int savedEnginePid_, bool useVolume_, int dt_, bool doDump_, uchar& oEnv[], int &oSampleLen_, int &oPredictionLen_);
int _getForecast(uchar& iEnv[], int& iBarT[], double &iBarO[], double &iBarH[], double &iBarL[], double &iBarC[], double &iBarV[], int iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV, double &oForecastO[], double &oForecastH[], double &oForecastL[], double &oForecastC[], double &oForecastV[]);
int _saveTradeInfo(uchar& iEnv[], int iPositionTicket, long iPositionOpenTime, long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult);
int _destroyEnv(uchar& iEnv[]);
#import

//--- input parameters - Forecaster dll stuff
input int EnginePid				= 11740;
input string ClientXMLFile		= "C:/Users/gcaglion/dev/zzz/Config/master/99/Client.xml";
input int DataTransformation	= 1;
input bool UseVolume			= false;
input int  ValidationShift		= 0;
input bool DumpData				= true;
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
int vEnginePid=EnginePid;
int vUseVolume=UseVolume;
int vDumpData=DumpData;
int vDataTransformation=DataTransformation;
//--
int vRectsCnt=0;
//--
long vTicket;

//--- data variables to be passed in MTgetForecast() call
int vSampleDataT[], vSampleBaseValT;
string vSampleDataTs[], vSampleBaseValTs;
double vSampleDataO[], vSampleBaseValO;
double vSampleDataH[], vSampleBaseValH;
double vSampleDataL[], vSampleBaseValL;
double vSampleDataC[], vSampleBaseValC;
double vSampleDataV[], vSampleBaseValV;
double vPredictedDataO[], vPredictedDataH[], vPredictedDataL[], vPredictedDataC[], vPredictedDataV[];
double vSampleBW[];

//--- bars timestamps
string vSampleTime[], vValidationTime[];
string vSampleBaseTime, vValidationBaseTime;

//--- miscellaneous variables
string EnvS;
uchar vEnvS[];		// Env in char* format
uchar vClientXMLFileS[];

int    vBarId;
uchar vFirstBarT[]; uchar vLastBarT[];
double vTradeSize=0;			// vTradeSize is global non-static. It is set by NewTrade(), and is needed by MTSaveTradeInfo()
double vTradeTP=0, vTradeSL=0;	// these, too, are set in NewTrade()
bool   fLog;
uint t0, t1;						// Time counters. Used to calc elapsed

int OnInit() {

	int seriesCnt;
	int historyLen;
	string serieSymbol[MT_MAX_SERIES_CNT];
	string serieTimeFrame[MT_MAX_SERIES_CNT];
	string serieFeatList[MT_MAX_SERIES_CNT];
	int serieFeatMask[MT_MAX_SERIES_CNT];
	//--
	double vopen[], vhigh[], vlow[], vclose[], vvolume[];
	int vtime[]; string vtimeS[];
	double vopenB[], vhighB[], vlowB[], vcloseB[], vvolumeB[];
	int vtimeB[]; string vtimeSB[];
	double vopenF[], vhighF[], vlowF[], vcloseF[], vvolumeF[];
	//--
	MqlRates serierates[];

	EnvS = "00000000000000000000000000000000000000000000000000000000000000000"; StringToCharArray(EnvS, vEnvS);
	StringToCharArray(ClientXMLFile, vClientXMLFileS);
	if (_createEnv2(AccountInfoInteger(ACCOUNT_LOGIN), vClientXMLFileS, vEnginePid, vUseVolume, vDataTransformation, vDumpData, vEnvS)!=0) {
		printf("_createEnv2() failed. see Forecaster logs.");
		return -1;
	}
	EnvS=CharArrayToString(vEnvS);
	printf("EnvS=%s ; ClientXMLFile=%s ; EnginePid=%d", EnvS, ClientXMLFile, EnginePid);

	string sSymbolsCSL="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	string sTimeFramesCSL="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	string sFeaturesCSL="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	uchar uSymbolsCSL[]; StringToCharArray(sSymbolsCSL, uSymbolsCSL);
	uchar uTimeFramesCSL[]; StringToCharArray(sTimeFramesCSL, uTimeFramesCSL);
	uchar uFeaturesCSL[]; StringToCharArray(sFeaturesCSL, uFeaturesCSL);

	if (_getSeriesInfo(vEnvS, seriesCnt, historyLen, uSymbolsCSL, uTimeFramesCSL, uFeaturesCSL)!=0) {
		printf("_getSeriesInfo() failed. see Forecaster logs.");
		return -1;
	}
	sSymbolsCSL=CharArrayToString(uSymbolsCSL);
	sTimeFramesCSL=CharArrayToString(uTimeFramesCSL);
	sFeaturesCSL=CharArrayToString(uFeaturesCSL);
	printf("seriesCnt=%d ; historyLen=%d ; symbolsCSL=%s ; timeFramesCSL=%s, featuresCSL=%s", seriesCnt, historyLen, sSymbolsCSL, sTimeFramesCSL, sFeaturesCSL);

	if (StringSplit(sSymbolsCSL, '|', serieSymbol)!=seriesCnt) {
		printf("Invalid Symbol CSL");
		return -1;
	}
	if (StringSplit(sTimeFramesCSL, '|', serieTimeFrame)!=seriesCnt) {
		printf("Invalid TimeFrame CSL");
		return -1;
	}
	if (StringSplit(sFeaturesCSL, '|', serieFeatList)!=seriesCnt) {
		printf("Invalid Feature CSL");
		return -1;
	}

	for (int s=0; s<seriesCnt; s++) {
		printf("Symbol/TF [%d] : %s/%s", s, serieSymbol[s], serieTimeFrame[s]);
		serieFeatMask[s]=CSL2Mask(serieFeatList[s]);
		printf("FeatureList [%d] : %s (%d)", s, serieFeatList[s], serieFeatMask[s]);
	}

	ArrayResize(vtimeB, seriesCnt);
	ArrayResize(vtimeSB, seriesCnt);
	ArrayResize(vopenB, seriesCnt);
	ArrayResize(vhighB, seriesCnt);
	ArrayResize(vlowB, seriesCnt);
	ArrayResize(vcloseB, seriesCnt);
	ArrayResize(vvolumeB, seriesCnt);
	//--
	ArrayResize(vtime, historyLen*seriesCnt);
	ArrayResize(vtimeS, historyLen*seriesCnt);
	ArrayResize(vopen, historyLen*seriesCnt);
	ArrayResize(vhigh, historyLen*seriesCnt);
	ArrayResize(vlow, historyLen*seriesCnt);
	ArrayResize(vclose, historyLen*seriesCnt);
	ArrayResize(vvolume, historyLen*seriesCnt);
	//--
	ArrayResize(vopenF, historyLen*seriesCnt);
	ArrayResize(vhighF, historyLen*seriesCnt);
	ArrayResize(vlowF, historyLen*seriesCnt);
	ArrayResize(vcloseF, historyLen*seriesCnt);
	ArrayResize(vvolumeF, historyLen*seriesCnt);

	//========== LOAD BARS ==============   THIS GOES INTO OnTick() =========================
	int i=0;
	ENUM_TIMEFRAMES tf;
	for (int s=0; s<seriesCnt; s++) {
		tf = getTimeFrameEnum(serieTimeFrame[s]);
		int copied=CopyRates(serieSymbol[s], tf, 1, historyLen+2, serierates);	printf("copied[%d]=%d", s, copied);
		if (copied!=(historyLen+2)) return -1;
		//-- base bar
		vtimeB[s]=serierates[1].time+TimeGMTOffset();
		StringConcatenate(vtimeSB[s], TimeToString(vtimeB[s], TIME_DATE), ".", TimeToString(vtimeB[s], TIME_MINUTES));
		vopenB[s]=serierates[1].open;
		vhighB[s]=serierates[1].high;
		vlowB[s]=serierates[1].low;
		vcloseB[s]=serierates[1].close;
		vvolumeB[s]=serierates[1].real_volume;
		printf("serie=%d ; time=%s ; OHLCV=%f|%f|%f|%f|%f", s, vtimeSB[s], vopenB[s], vhighB[s], vlowB[s], vcloseB[s], vvolumeB[s]);
		//-- [historyLen] bars
		for (int bar=2; bar<(historyLen+2); bar++) {
			vtime[i]=serierates[bar].time+TimeGMTOffset();
			StringConcatenate(vtimeS[i], TimeToString(vtime[i], TIME_DATE), ".", TimeToString(vtime[i], TIME_MINUTES));
			vopen[i]=serierates[bar].open;
			vhigh[i]=serierates[bar].high;
			vlow[i]=serierates[bar].low;
			vclose[i]=serierates[bar].close;
			vvolume[i]=serierates[bar].real_volume;
			printf("time[%d]=%s ; OHLCV[%d]=%f|%f|%f|%f|%f", i, vtimeS[i], i, vopen[i], vhigh[i], vlow[i], vclose[i], vvolume[i]);
			i++;
		}		
	}
	//--
	//------ GET FORECAST ---------
	if (_getForecast2(vEnvS, seriesCnt, historyLen, vDataTransformation, serieFeatMask, vtime, vopen, vhigh, vlow, vclose, vvolume, vtimeB, vopenB, vhighB, vlowB, vcloseB, vvolumeB, vopenF, vhighF, vlowF, vcloseF, vvolumeF)!=0) {
		printf("_getForecast() FAILURE! Exiting...");
		return -1;
	};
	//===============================================================================
	return -1;
	//--------------------------------------------------------------------------------------------------------
	
}
void OnTick() {

	// Only do this if there's a new bar
	static datetime Time0=0;
	if (Time0==SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE)) return; 
	Time0 = SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE);
	string Time0S;
	StringConcatenate(Time0S, TimeToString(Time0, TIME_DATE), ".", TimeToString(Time0, TIME_MINUTES));

	//-- load bars into arrrays
	printf("Time0=%s . calling LoadBars()...", Time0S);
	LoadBars();


	//-- call Forecaster
	if (_getForecast(vEnvS, vSampleDataT, vSampleDataO, vSampleDataH, vSampleDataL, vSampleDataC, vSampleDataV, vSampleBaseValT, vSampleBaseValO, vSampleBaseValH, vSampleBaseValL, vSampleBaseValC, vSampleBaseValV, vPredictedDataO, vPredictedDataH, vPredictedDataL, vPredictedDataC, vPredictedDataV)!=0) {
		printf("_getForecast() FAILURE! Exiting...");
		return;
	};
	for (int i=0; i<vPredictionLen; i++) printf("vPredictedDataH[%d]=%5.4f , vPredictedDataL[%d]=%5.4f ", i, vPredictedDataH[i], i, vPredictedDataL[i]);

	//-- check for forecast consistency (H>L)
	if (vPredictedDataH[0]<=vPredictedDataL[0]) {
		printf("Invalid Forecast: H=%f ; L=%f", vPredictedDataH[0], vPredictedDataL[0]);
		return;
	}
	//-- draw rectangle around the current bar extending from vPredictedDataH[0] to vPredictedDataL[0]
	drawForecast(vPredictedDataH[0], vPredictedDataL[0]);

	//-- define trade scenario based on current price level and forecast
	double tradeVol=0.1;
	double tradePrice, tradeTP, tradeSL;
	int tradeResult; datetime positionTime;

	int tradeScenario=getTradeScenario(tradePrice, tradeTP, tradeSL); printf("trade scenario=%d ; tradePrice=%5.4f ; tradeTP=%5.4f ; tradeSL=%5.4f", tradeScenario, tradePrice, tradeTP, tradeSL);
	if (tradeScenario>=0) {

		//-- do the actual trade
		tradeResult=NewTrade(tradeScenario, tradeVol, tradePrice, tradeTP, tradeSL);

		//-- if trade successful, store position ticket in shared variable
		if (tradeResult==0) {
			vTicket = PositionGetTicket(0);
			int positionId=PositionSelect(Symbol());
			positionTime=PositionGetInteger(POSITION_TIME);
			//-- save tradeInfo
			if (_saveTradeInfo(vEnvS, vTicket, positionTime, vSampleDataT[vSampleLen-1], vSampleDataO[vSampleLen-1], vSampleDataH[vSampleLen-1], vSampleDataL[vSampleLen-1], vSampleDataC[vSampleLen-1], vSampleDataV[vSampleLen-1], vPredictedDataO[0], vPredictedDataH[0], vPredictedDataL[0], vPredictedDataC[0], vPredictedDataV[0], tradeScenario, tradeResult)<0) {
				printf("_saveTradeInfo() failed. see Forecaster logs.");
				return;
			}
		} else {
			vTicket=-1;
		}
	}
	

}
void OnDeinit(const int reason) {
	CharArrayToString(vEnvS, EnvS);
	printf("OnDeInit() called. EnvS=%s", EnvS);
	if (EnvS!="00000000000000000000000000000000000000000000000000000000000000000") {
		printf("calling _destroyEnv for vEnvS=%s", EnvS);
		_destroyEnv(vEnvS);
	}
}

void LoadBars() {
	//-- This loads bar values into Sample and Base arrays

	MqlRates rates[];
	int copied=CopyRates(NULL, 0, 1, vSampleLen+2, rates);
	if (copied<(vSampleLen+1)) Print("Error copying price data ", GetLastError());
	//else Print("Copied ", ArraySize(rates), " bars");

	//-- base bar, needed for Delta Transformation
	vSampleBaseValT = rates[1].time; StringConcatenate(vSampleBaseValTs, TimeToString(vSampleBaseValT, TIME_DATE), ".", TimeToString(vSampleBaseValT, TIME_MINUTES));
	vSampleBaseValO = rates[1].open;
	vSampleBaseValH = rates[1].high;
	vSampleBaseValL = rates[1].low;
	vSampleBaseValC = rates[1].close;
	vSampleBaseValV = rates[1].real_volume;
	//printf("Base Bar: T=%s - O=%f - H=%f - L=%f - C=%f - V=%f", vSampleBaseValTs, vSampleBaseValO, vSampleBaseValH, vSampleBaseValL, vSampleBaseValC, vSampleBaseValV);
	//-- whole sample
	for (int i = 0; i<vSampleLen; i++) {    // (i=0 is the current bar)
		vSampleDataT[i] = rates[i+2].time; StringConcatenate(vSampleDataTs[i], TimeToString(vSampleDataT[i], TIME_DATE), ".", TimeToString(vSampleDataT[i], TIME_MINUTES));
		vSampleDataO[i] = rates[i+2].open;
		vSampleDataH[i] = rates[i+2].high;
		vSampleDataL[i] = rates[i+2].low;
		vSampleDataC[i] = rates[i+2].close;
		vSampleDataV[i] = rates[i+2].real_volume;
		//printf("Bar[%d]: T=%s - O=%f - H=%f - L=%f - C=%f - V=%f", i, vSampleDataTs[i], vSampleDataO[i], vSampleDataH[i], vSampleDataL[i], vSampleDataC[i], vSampleDataV[i]);
	}

}
int getTradeScenario(double& oTradePrice, double& oTradeTP, double oTradeSL) {

	int scenario;
	double fTolerance=0.0005;
	double riskRatio=0.5;	// 
	double minProfit=0.0003;

	MqlTick tick;
	if(SymbolInfoTick(Symbol(), tick)) {

		double fH=vPredictedDataH[0];
		double fL=vPredictedDataL[0];
		double cH=tick.ask;
		double cL=tick.bid;
		double dH=fH-cH;
		double dL=cL-fL;
		double expProfit, expLoss;

		printf("getTradeScenario(): cH=%5.4f , cL=%5.4f , fH=%5.4f , fL=%5.4f , dH=%5.4f , dL=%5.4f", cH, cL, fH, fL, dH, dL);
		
		//-- Current price (tick.ask) is below   ForecastL => BUY (1)
		if (cH<fL) {
			scenario = 1; 
			oTradeTP=fH-fTolerance;
			expProfit=oTradeTP-cH; 
			expLoss=expProfit*riskRatio;
			oTradeSL=cL-expLoss;
			printf("Scenario 1 (BUY) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
			if (expProfit<minProfit) {
				printf("Profit too small. No trade."); scenario=-scenario;
			}
		}

		//-- Current price is above   ForecastH  => SELL (2)
		if (cL>fH) {
			scenario = 2;
			oTradeTP=fL+fTolerance;
			expProfit=cL-oTradeTP;
			expLoss=expProfit*riskRatio;
			oTradeSL=cL+expLoss;
			printf("Scenario 2 (SELL) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
			if (expProfit<minProfit) {
				printf("Profit too small. No trade."); scenario=-scenario;
			}
		}

		//-- Current price is between ForecastL and ForecastH, closer to ForecastL  => BUY (3)
		if (cH<=fH && cL>=fL && dL<dH ) {
			scenario = 3;
			oTradeTP=fH-fTolerance;
			expProfit=oTradeTP-cH;
			expLoss=expProfit*riskRatio;
			oTradeSL=cL-expLoss;
			printf("Scenario 3 (BUY) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
			if (expProfit<minProfit) {
				printf("Profit too small. No trade."); scenario=-scenario;
			}
		}

		//-- Current price is between ForecastL and ForecastH, closer to ForecastH  => SELL (4)
		if (cH<=fH && cL>=fL && dH<dL) {
			scenario = 4;
			oTradeTP=fL+fTolerance;
			expProfit=cL-oTradeTP;
			expLoss=expProfit*riskRatio;
			oTradeSL=cL+expLoss;
			printf("Scenario 4 (SELL) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
			if (expProfit<minProfit) {
				printf("Profit too small. No trade."); scenario=-scenario;
			}
		}

	}

	return scenario;
}
int NewTrade(int cmd, double volume, double price, double TP, double SL) {

	CTrade trade;
	trade.SetExpertMagicNumber(123456);
	trade.SetDeviationInPoints(10);
	trade.SetTypeFilling(ORDER_FILLING_RETURN);
	trade.SetAsyncMode(false);

	printf("NewTrade() called with cmd=%s , volume=%f , price=%5.4f , takeprofit=%5.4f , stoploss=%5.4f", ((cmd==1||cmd==3) ? "BUY" : "SELL"), volume, price, TP, SL);
	
	//printf("First, closing existing position...");
	trade.PositionClose(Symbol(), 10);

	string symbol=Symbol();
	int    digits=(int)SymbolInfoInteger(symbol, SYMBOL_DIGITS); // number of decimal places
	double point=SymbolInfoDouble(symbol, SYMBOL_POINT);         // point
	double bid=SymbolInfoDouble(symbol, SYMBOL_BID);             // current price for closing LONG
	double ask=SymbolInfoDouble(symbol, SYMBOL_ASK);             // current price for closing SHORT
	double open_price;	//--- receive the current open price for LONG positions
	

	string comment;
	bool ret;
	if (cmd==1||cmd==3) {
		//-- Buy
		open_price=SymbolInfoDouble(symbol, SYMBOL_BID);
		string comment=StringFormat("Buy  %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(SL, digits), DoubleToString(TP, digits));
		ret=trade.Buy(volume, symbol, open_price, SL, TP, comment);
	}
	if (cmd==2||cmd==4) {
		//-- Sell
		open_price=SymbolInfoDouble(symbol, SYMBOL_ASK);
		string comment=StringFormat("Sell %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(SL, digits), DoubleToString(TP, digits));
		ret=trade.Sell(volume, symbol, open_price, SL, TP, comment);
	}
	if (!ret) {
		//--- failure message
		Print("Trade failed. Return code=", trade.ResultRetcode(), ". Code description: ", trade.ResultRetcodeDescription());
		return -1;
	} else	{
		Print("Trade executed successfully. Return code=", trade.ResultRetcode(), " (", trade.ResultRetcodeDescription(), ")");
		return 0;
	}
}
void drawForecast(double H, double L) {
	//-- https://www.youtube.com/watch?v=Y3e1zVROJlY

	//-- get last bar and new bar
	MqlRates rates[];
	int copied=CopyRates(NULL, 0, 0, 2, rates);
	if (copied<=0) Print("Error copying price data ", GetLastError());

	string name;
	StringConcatenate(name, "Rectangle", TimeToString(rates[0].time, TIME_DATE), ".", TimeToString(rates[0].time, TIME_MINUTES));

//	ObjectDelete(_Symbol, name);

	//-- draw the rectangle between last bar and new bar
	//printf("ObjectCreate(H=%f ; L=%f) returns %d", H,L,ObjectCreate(_Symbol, name, OBJ_RECTANGLE, 0, rates[0].time, H, rates[1].time, L));
	ObjectCreate(_Symbol, name, OBJ_RECTANGLE, 0, rates[0].time, H, rates[1].time, L);
	ObjectSetInteger(0, name, OBJPROP_COLOR, clrBlue);
	ObjectSetInteger(0, name, OBJPROP_WIDTH, 3);
	//ObjectSetInteger(0, name, OBJPROP_FILL, true);
	ObjectSetInteger(0, name, OBJPROP_HIDDEN, false);

	vRectsCnt++;
}

ENUM_TIMEFRAMES getTimeFrameEnum(string tfS) {
	if (tfS=="H1") return PERIOD_H1;
	return 0;
}
int CSL2Mask(string mask) {
	string selF[5];
	int ret=0;

	int fcnt=StringSplit(mask, ',', selF);
	for (int f=0; f<fcnt; f++) {
		//printf("selF[%d]=%s", f, selF[f]);
		if (StringCompare(selF[f],"0")==0) ret+=10000; //-- OPEN is selected
		if (StringCompare(selF[f], "1")==0) ret+=1000; //-- HIGH is selected
		if (StringCompare(selF[f], "2")==0) ret+=100; //-- LOW is selected
		if (StringCompare(selF[f], "3")==0) ret+=10; //-- CLOSE is selected
		if (StringCompare(selF[f], "4")==0) ret+=1; //-- VOLUME is selected
	}
	return ret;
}