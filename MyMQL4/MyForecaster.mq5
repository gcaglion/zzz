#property copyright "gcaglion"
#property link      "https://algoinvest.org"
#property version   "1.00"
#property strict

#include <Trade/Trade.mqh>

#import "Forecaster.dll"
int _createEnv(int accountId_, uchar& clientXMLFile_[], int savedEnginePid_, bool useVolume_, int dt_, bool doDump_, uchar& oEnv[], int &oSampleLen_, int &oPredictionLen_);
int _getForecast(uchar& iEnv[], int& iBarT[], double &iBarO[], double &iBarH[], double &iBarL[], double &iBarC[], double &iBarV[], int iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV, double &oForecastO[], double &oForecastH[], double &oForecastL[], double &oForecastC[], double &oForecastV[]);
int _destroyEnv(uchar& iEnv[]);
#import

//--- input parameters - Forecaster dll stuff
input int EnginePid				= 3724;
input string ClientXMLFile		= "C:/Users/gcaglion/dev/zzz/Config/MetaTrader/Client.xml";
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

//--- data variables to be passed in MTgetForecast() call
int vSampleDataT[], vSampleBaseValT;
string vSampleDataTs[], vSampleBaseValTs;
double vSampleDataO[], vSampleBaseValO;
double vSampleDataH[], vSampleBaseValH;
double vSampleDataL[], vSampleBaseValL;
double vSampleDataC[], vSampleBaseValC;
double vSampleDataV[], vSampleBaseValV;
double vFutureDataO[];
double vFutureDataH[];
double vFutureDataL[];
double vFutureDataC[];
double vPredictedDataO[], vPredictedDataH[], vPredictedDataL[], vPredictedDataC[], vPredictedDataV[];
double vSampleBW[];
double vFutureBW[];

//--- bars timestamps
string sCurrentBarTime; uchar vCurrentBarTime[];
string vSampleTime[], vValidationTime[];
string vSampleBaseTime, vValidationBaseTime;

//--- miscellaneous variables
uchar vEnvS[];		// Env in char* format
uchar vClientXMLFileS[];

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

	StringToCharArray(ClientXMLFile, vClientXMLFileS);

	printf("Calling _createEnv()...");
	if (_createEnv(AccountInfoInteger(ACCOUNT_LOGIN), vClientXMLFileS, vEnginePid, vUseVolume, vDataTransformation, vDumpData, vEnvS, vSampleLen, vPredictionLen)!=0) {
		printf("_createEnv() failed. see Forecaster logs.");
		return -1;
	}
	EnvS=CharArrayToString(vEnvS);
	printf("EnvS=%s ; ClientXMLFile=%s ; EnginePid=%d ; vSampleLen=%d ; vPredictionLen=%d", EnvS, ClientXMLFile, EnginePid, vSampleLen, vPredictionLen);

	//--- Resize Sample and Prediction arrays (+1 is for BaseVal)
	ArrayResize(vSampleDataT, vSampleLen);
	ArrayResize(vSampleDataTs, vSampleLen);
	ArrayResize(vSampleDataO, vSampleLen);
	ArrayResize(vSampleDataH, vSampleLen);
	ArrayResize(vSampleDataL, vSampleLen);
	ArrayResize(vSampleDataC, vSampleLen);
	ArrayResize(vSampleDataV, vSampleLen);
	ArrayResize(vSampleBW, vSampleLen);
	ArrayResize(vFutureDataH, vPredictionLen);
	ArrayResize(vFutureDataL, vPredictionLen);
	ArrayResize(vFutureBW, vPredictionLen);
	ArrayResize(vPredictedDataO, vPredictionLen);
	ArrayResize(vPredictedDataH, vPredictionLen);
	ArrayResize(vPredictedDataL, vPredictionLen);
	ArrayResize(vPredictedDataC, vPredictionLen);
	ArrayResize(vPredictedDataV, vPredictionLen);
	ArrayResize(vSampleTime, vSampleLen);
	ArrayResize(vValidationTime, vSampleLen);

	return(INIT_SUCCEEDED);
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
	for (int i=0; i<vPredictionLen; i++) printf("vPredictedDataO[%d]=%f , vPredictedDataH[%d]=%f , vPredictedDataL[%d]=%f , vPredictedDataC[%d]=%f", i, vPredictedDataO[i], i, vPredictedDataH[i], i, vPredictedDataL[i], i, vPredictedDataC[i]);

	//-- draw rectangle around the current bar extending from vPredictedDataH[0] to vPredictedDataL[0]
	drawForecast(vPredictedDataH[0], vPredictedDataL[0]);

	//-- define trade scenario based on current price level and forecast
	int tradeOp;	//
	double tradeVol, tradePrice, tradeTP, tradeSL;
	int tradeScenario=getTradeScenario(tradePrice, tradeTP, tradeSL); printf("trade scenario=%d ; tradePrice=%f ; tradeTP=%f ; tradeSL=%f", tradeScenario, tradePrice, tradeTP, tradeSL);
	if (tradeScenario<0) return;

	//-- do the actual trade
	tradeVol=0.1;
	int ret=NewTrade(tradeScenario, tradeVol, tradePrice, tradeSL, tradeTP);

}
void OnDeinit(const int reason) {
	_destroyEnv(vEnvS);
}

void LoadBars() {
	//-- This loads bar values into Sample and Base arrays

	MqlRates rates[];
	int copied=CopyRates(NULL, 0, 1, vSampleLen+2, rates);
	if (copied<(vSampleLen+1))
		Print("Error copying price data ", GetLastError());
	else Print("Copied ", ArraySize(rates), " bars");

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
	int scenario=-1;

	//-- Determine Trade
	double FH             = vPredictedDataH[0];
	double FL             = vPredictedDataL[0];
	double RR             = RiskRatio;          // Risk Ratio (PIPS to SL / PIPS to TP)
	double SL, TP;
	//double StopLevel=MarketInfo(Symbol(), MODE_STOPLEVEL)*MarketInfo(Symbol(), MODE_TICKSIZE);
	double StopLevel=0.0001*5;
	double MinProfit = MinProfitPIPs*0.0001;

	printf("StopLevel=%f ; MinProfit=%f", StopLevel, MinProfit);

	MqlTick tick;
	if(SymbolInfoTick(Symbol(), tick)) {

		printf("1. Current Bar tick.ask=%5.4f ; tick.bid=%5.4f ; FH=%5.4f ; FL=%5.4f", tick.ask, tick.bid, FH, FL);
		//-- Current price (tick.ask) is below   ForecastL                                     => BUY (1)
		if (tick.ask<FL) {
			scenario = 1; printf("Scenario %d -> Buy", scenario);
			TP=FH; //printf("Original TP=%f", TP);  //-Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
			if ((TP-tick.ask)<MinProfit) {
				printf("Profit too small. No Trade.");
				return (-scenario);
			}
			SL=tick.bid-(TP-tick.ask)*RR; //printf("Original SL=%f", SL);
			if ((tick.bid-SL)<StopLevel) SL=tick.bid-StopLevel;
			printf("tick.ask<FL (1); TP=%5.4f ; SL=%5.4f", TP, SL);
		}
		//-- Current price is above   ForecastH                                     => SELL (2)
		if (tick.bid>FH) {
			scenario = 2; printf("Scenario %d -> Sell", scenario);
			TP=FL; //printf("Original TP=%f", TP);   //+Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
			if ((tick.bid-TP)<MinProfit) {
				printf("Profit too small. No Trade.");
				return (-scenario);
			}
			SL=tick.ask+(tick.bid-TP)*RR; //printf("Original SL=%f", SL);
			if ((SL-tick.ask)<StopLevel) SL=tick.ask+StopLevel;
			printf("tick.bid>FH (2) ; TP=%5.4f ; SL=%5.4f", TP, SL);
		}
		//-- Current price is between ForecastL and ForecastH, closer to ForecastL  => BUY (3)
		if (tick.ask<=FH && tick.bid>=FL&&(tick.bid-FL)<=(FH-tick.ask)) {
			scenario = 3; printf("Scenario %d -> Buy", scenario);
			TP=FH; //printf("Original TP=%f", TP);   //-Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
			if ((TP-tick.ask)<MinProfit) {
				printf("Profit too small. No Trade.");
				return (-scenario);
			}
			SL=tick.bid-(TP-tick.ask)*RR; //printf("Original SL=%f", SL);
			if ((tick.bid-SL)<StopLevel) SL=tick.bid-StopLevel;
		}
		//-- Current price is between ForecastL and ForecastH, closer to ForecastH  => SELL (4)
		if (tick.ask<=FH && tick.bid>=FL&&(FH-tick.ask)<=(tick.bid-FL)) {
			scenario = 4; printf("Scenario %d -> Sell", scenario);
			TP=FL; //printf("Original TP=%f", TP);   //+Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
			if ((tick.bid-TP)<MinProfit) {
				printf("Profit too small. No Trade.");
				return (-scenario);
			}
			SL=tick.ask+(tick.bid-TP)*RR; //printf("Original SL=%f", SL);
			if ((SL-tick.ask)<StopLevel) SL=tick.ask+StopLevel;
		}
		//-- Display Forecasts (only for the next bar)
		//DrawForecast(vPredictedDataH[0], vPredictedDataL[0]);
	}

	return scenario;
}
int NewTrade(int cmd, double volume, double price, double stoploss, double takeprofit) {

	CTrade trade;
	trade.SetExpertMagicNumber(123456);
	trade.SetDeviationInPoints(10);
	trade.SetTypeFilling(ORDER_FILLING_RETURN);
	trade.SetAsyncMode(false);

	printf("NewTrade() called with cmd=%s , volume=%f , price=%5.4f , stoploss=%5.4f , takeprofit=%5.4f", (cmd==0 ? "BUY" : "SELL"), volume, price, stoploss, takeprofit);
	
	printf("First, closing existing position...");
	trade.PositionClose(Symbol(), 10);

	string symbol=Symbol();
	int    digits=(int)SymbolInfoInteger(symbol, SYMBOL_DIGITS); // number of decimal places
	double point=SymbolInfoDouble(symbol, SYMBOL_POINT);         // point
	double bid=SymbolInfoDouble(symbol, SYMBOL_BID);             // current price for closing LONG
	double ask=SymbolInfoDouble(symbol, SYMBOL_ASK);             // current price for closing SHORT
	double SL;	// unnormalized SL value
	double TP;	// unnormalized TP value
	double open_price;	//--- receive the current open price for LONG positions
	

	string comment;
	bool ret;
	if (cmd==1||cmd==3) {
		//-- Buy
		open_price=SymbolInfoDouble(symbol, SYMBOL_ASK);
		SL = bid-1000*point;
		SL=NormalizeDouble(SL, digits); // normalizing Stop Loss
		TP = bid+1000*point;
		TP=NormalizeDouble(TP, digits);                              // normalizing Take Profit
		string comment=StringFormat("Buy  %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(SL, digits), DoubleToString(TP, digits));
		ret=trade.Buy(volume, symbol, open_price, SL, TP, comment);
	}
	if (cmd==2||cmd==4) {
		//-- Sell
		open_price=SymbolInfoDouble(symbol, SYMBOL_ASK);
		SL = ask+1000*point;
		SL=NormalizeDouble(SL, digits); // normalizing Stop Loss
		TP = ask-1000*point;
		TP=NormalizeDouble(TP, digits);                              // normalizing Take Profit
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

	string name="Rectangle";

	MqlRates rates[];
	int copied=CopyRates(NULL, 0, 0, 10, rates);
	if (copied<=0)
		Print("Error copying price data ", GetLastError());
	else Print("Copied ", ArraySize(rates), " bars");

	Print("ObjectCreate() returns ", ObjectCreate(_Symbol, name, 0, rates[2].time, H, rates[0].time, L));
	ObjectSetInteger(0, name, OBJPROP_COLOR, clrBlue);
	ObjectSetInteger(0, name, OBJPROP_FILL, clrBlue);
	ObjectSetInteger(0, name, OBJPROP_HIDDEN, false);
}