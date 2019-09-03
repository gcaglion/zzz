#property copyright "gcaglion"
#property link      "https://algoinvest.org"
#property version   "1.00"
#property strict

#include <Trade/Trade.mqh>

#define MT_MAX_SERIES_CNT 12

#import "Forecaster.dll"
//--
int _createEnv(int accountId_, uchar& clientXMLFile_[], int savedEnginePid_, int dt_, bool doDump_, uchar& oEnv[], int& oSampleLen_, int &oPredictionLen_, int &oFeaturesCnt_, int &oBatchSize_);
int _getSeriesInfo(uchar& iEnv[], int& oSeriesCnt_, uchar& oSymbolsCSL_[], uchar& oTimeFramesCSL_[], uchar& oFeaturesCSL_[], bool& oChartTrade[]);
int _getForecast(uchar& iEnv[], int seriesCnt_, int dt_, int& featMask_[], int& iBarT[], double &iBarO[], double &iBarH[], double &iBarL[], double &iBarC[], double &iBarV[], int &iBaseBarT[], double &iBaseBarO[], double &iBaseBarH[], double &iBaseBarL[], double &iBaseBarC[], double &iBaseBarV[], double &oForecastO[], double &oForecastH[], double &oForecastL[], double &oForecastC[], double &oForecastV[]);
//--
int _saveTradeInfo(uchar& iEnv[], int iPositionTicket, long iPositionOpenTime, long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit);
int _saveClientInfo(uchar& iEnv[], long iTradeStartTime);
void _commit(uchar& iEnv[]);
int _destroyEnv(uchar& iEnv[]);
#import

//--- input parameters - Forecaster dll stuff
input int EnginePid				= 6956;
input string ClientXMLFile		= "C:/Users/gcaglion/dev/zzz/Config/Client.xml";
input int DataTransformation	= 1;
input bool DumpData				= true;
input bool SaveLogs				= true;
input int  Max_Retries			= 3;
//-- input parameters - Trade stuff
input double TradeVol			= 0.1;
input double RiskRatio			= 0.20;

//--- local variables
int vDataTransformation=DataTransformation;
int vEnginePid=EnginePid;
int vDumpData=DumpData;
//--
long vTicket;

//--- miscellaneous variables
string EnvS;
uchar vEnvS[];		// Env in char* format
uchar vClientXMLFileS[];

//--------------------------------------------------------------------------
int seriesCnt;
int historyLen;
int predictionLen;
int featuresCnt;
int batchSize;
int featuresCntFromCfg;
string serieSymbol[MT_MAX_SERIES_CNT];
string serieTimeFrame[MT_MAX_SERIES_CNT];
string serieFeatList[MT_MAX_SERIES_CNT];
int serieFeatMask[MT_MAX_SERIES_CNT];
bool chartTrade[MT_MAX_SERIES_CNT];
//--
double vopen[], vhigh[], vlow[], vclose[], vvolume[];
int vtime[]; string vtimeS[];
double vopenB[], vhighB[], vlowB[], vcloseB[], vvolumeB[];
int vtimeB[]; string vtimeSB[];
double vopenF[], vhighF[], vlowF[], vcloseF[], vvolumeF[];
double vForecastH, vForecastL;
//--
MqlRates serierates[];
//--
CTrade trade;
//--
bool TPhit;
bool SLhit=false;
//--------------------------------------------------------------------------

int OnInit() {

	trade.SetExpertMagicNumber(123456);
	trade.SetDeviationInPoints(10);
	trade.SetTypeFilling(ORDER_FILLING_RETURN);
	trade.SetAsyncMode(false);

	EnvS = "00000000000000000000000000000000000000000000000000000000000000000"; StringToCharArray(EnvS, vEnvS);
	string sSymbolsCSL="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	string sTimeFramesCSL="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	string sFeaturesCSL="0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	uchar uSymbolsCSL[]; StringToCharArray(sSymbolsCSL, uSymbolsCSL);
	uchar uTimeFramesCSL[]; StringToCharArray(sTimeFramesCSL, uTimeFramesCSL);
	uchar uFeaturesCSL[]; StringToCharArray(sFeaturesCSL, uFeaturesCSL);
	StringToCharArray(ClientXMLFile, vClientXMLFileS);

	//--
	printf("Creating Environment from saved engine (pid=%d) ...", vEnginePid);
	if (_createEnv(AccountInfoInteger(ACCOUNT_LOGIN), vClientXMLFileS, vEnginePid, vDataTransformation, vDumpData, vEnvS, historyLen, predictionLen, featuresCnt, batchSize)!=0) {
		printf("FAILURE: _createEnv() failed. see Forecaster logs.");
		return -1;
	}
	EnvS=CharArrayToString(vEnvS);
	printf("EnginePid=%d ; SampleLen/PredictionLen/FeaturesCnt/BatchSize=%d/%d/%d/%d ; EnvS=%s ; ClientXMLFile=%s", EnginePid, historyLen, predictionLen, featuresCnt, batchSize, EnvS, ClientXMLFile);

	//--
	printf("Getting TimeSeries Info from Client Config...");
	if (_getSeriesInfo(vEnvS, seriesCnt, uSymbolsCSL, uTimeFramesCSL, uFeaturesCSL, chartTrade)!=0) {
		printf("FAILURE: _getSeriesInfo() failed. see Forecaster logs.");
		return -1;
	}
	sSymbolsCSL=CharArrayToString(uSymbolsCSL);
	sTimeFramesCSL=CharArrayToString(uTimeFramesCSL);
	sFeaturesCSL=CharArrayToString(uFeaturesCSL);
	printf("seriesCnt=%d ; symbolsCSL=%s ; timeFramesCSL=%s, featuresCSL=%s", seriesCnt, sSymbolsCSL, sTimeFramesCSL, sFeaturesCSL);

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

	featuresCntFromCfg=0;
	string serieFeatTmp[5];
	for (int s=0; s<seriesCnt; s++) {
		printf("Symbol/TF [%d] : %s/%s", s, serieSymbol[s], serieTimeFrame[s]);
		serieFeatMask[s]=CSL2Mask(serieFeatList[s]);
		printf("FeatureList [%d] : %s (%d)", s, serieFeatList[s], serieFeatMask[s]);
		printf("Trade [%d] : %s", s, (chartTrade[s]) ? "TRUE" : "FALSE");
		featuresCntFromCfg+=StringSplit(serieFeatList[s], ',', serieFeatTmp);
	}
	if (featuresCntFromCfg!=featuresCnt) {
		printf("FAILURE: Total Features Count from Client Config (%d) differs from Engine Features Count (%d)", featuresCntFromCfg, featuresCnt);
		return -1;
	}

	ArrayResize(vtimeB, seriesCnt);
	ArrayResize(vtimeSB, seriesCnt);
	ArrayResize(vopenB, seriesCnt);
	ArrayResize(vhighB, seriesCnt);
	ArrayResize(vlowB, seriesCnt);
	ArrayResize(vcloseB, seriesCnt);
	ArrayResize(vvolumeB, seriesCnt);
	//--
	ArrayResize(vtime, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	ArrayResize(vtimeS, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	ArrayResize(vopen, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	ArrayResize(vhigh, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	ArrayResize(vlow, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	ArrayResize(vclose, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	ArrayResize(vvolume, (predictionLen+batchSize+historyLen-1)*seriesCnt);
	//--
	ArrayResize(vopenF, predictionLen*seriesCnt);
	ArrayResize(vhighF, predictionLen*seriesCnt);
	ArrayResize(vlowF, predictionLen*seriesCnt);
	ArrayResize(vcloseF, predictionLen*seriesCnt);
	ArrayResize(vvolumeF, predictionLen*seriesCnt);

	OnTick();

	return 0;

}
void OnTick() {

	static bool runOnce=false;
	static bool firstTick=true;
	int tradeScenario;
	int tradeResult=-1;
	datetime positionTime=0;
	vTicket=-1;

	if (!runOnce) {
		// Only do this if there's a new bar
		static datetime Time0=0;
		if (Time0==SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE)) return;

		Time0 = SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE);
		string Time0S;
		StringConcatenate(Time0S, TimeToString(Time0, TIME_DATE), ".", TimeToString(Time0, TIME_MINUTES));

		//-- close existing position
		trade.PositionClose(Symbol(), 10);
		printf("trade.PositionClose() returned %d", trade.ResultRetcode());
		if (!firstTick&&trade.ResultRetcode()!=TRADE_RETCODE_DONE) {
			//-- prev position has already been closed due to TP or SL
			TPhit=true;
		} else {
			TPhit=false;
		}

		//-- load bars into arrrays
		//printf("Time0=%s . calling LoadBars()...", Time0S);
		if (!loadBars()) {
			printf("loadBars() FAILURE! Exiting...");
			return;
		}

		//------ GET FORECAST ---------
		//printf("Getting Forecast...");
		if (_getForecast(vEnvS, seriesCnt, vDataTransformation, serieFeatMask, vtime, vopen, vhigh, vlow, vclose, vvolume, vtimeB, vopenB, vhighB, vlowB, vcloseB, vvolumeB, vopenF, vhighF, vlowF, vcloseF, vvolumeF)!=0) {
			printf("_getForecast() FAILURE! Exiting...");
			return;
		};
		//------ PRINT FORECAST FOR ALL SERIES, AND PICK SERIE FROM CURRENT CHART SYMBOL/TF ---------
		int tradeSerie=-1;
		for (int s=0; s<seriesCnt; s++) {
			for (int bar=0; bar<predictionLen; bar++) {
				//printf("OHLCV Forecast, serie %d: %f|%f|%f|%f|%f", s, (vopenF[s*predictionLen+bar]<0) ? 0 : vopenF[s*predictionLen+bar], (vhighF[s*predictionLen+bar]<0) ? 0 : vhighF[s*predictionLen+bar], (vlowF[s*predictionLen+bar]<0) ? 0 : vlowF[s*predictionLen+bar], (vcloseF[s*predictionLen+bar]<0) ? 0 : vcloseF[s*predictionLen+bar], (vvolumeF[s*predictionLen+bar]<0) ? 0 : vvolumeF[s*predictionLen+bar]);
				if (StringCompare(serieSymbol[s], Symbol())==0&&getTimeFrameEnum(serieTimeFrame[s])==Period()) tradeSerie=s;
			}
		}
		if (tradeSerie==-1) {
			printf("Nothing to trade in current chart. Exiting...");
			return;
		}

		//-- take first bar from vhighF, vlowF
		vForecastH=vhighF[tradeSerie*predictionLen+0];
		vForecastL=vlowF[tradeSerie*predictionLen+0];
		//-- check for forecast consistency in first bar (H>L)
		if (vForecastL>vForecastH) {
			printf("Invalid Forecast: H=%f ; L=%f . Exiting...", vForecastH, vForecastL);
		} else {
			printf("Using Forecast: H=%f ; L=%f", vForecastH, vForecastL);
			//-- draw rectangle around the current bar extending from vPredictedDataH[0] to vPredictedDataL[0]
			drawForecast(vForecastH, vForecastL);

			//-- define trade scenario based on current price level and forecast
			double tradeVol=TradeVol;
			double tradeTP, tradeSL;

			tradeScenario=getTradeScenario(tradeTP, tradeSL); printf("trade scenario=%d ; tradeTP=%5.4f ; tradeSL=%5.4f", tradeScenario, tradeTP, tradeSL);
			if (tradeScenario>=0) {

				//-- do the actual trade
				printf("======================================== CALL TO NewTrade() ========================================================================================================================");
				tradeResult=NewTrade(tradeScenario, tradeVol, tradeTP, tradeSL);
				printf("====================================================================================================================================================================================");

				//-- if trade successful, store position ticket in shared variable
				if (tradeResult==0) {
					vTicket = PositionGetTicket(0);
					int positionId=PositionSelect(Symbol());
					positionTime=PositionGetInteger(POSITION_TIME);
				}
			}
		}

		//-- save tradeInfo, even if we do not trade
		int idx=tradeSerie*historyLen+historyLen-1;
		//printf("calling _saveTradeInfo() with lastBar = %s - %f|%f|%f|%f ; forecast = %f|%f|%f|%f", vtimeS[idx], vopen[idx], vhigh[idx], vlow[idx], vclose[idx], vopenF[tradeSerie*predictionLen+0], vhighF[tradeSerie*predictionLen+0], vlowF[tradeSerie*predictionLen+0], vcloseF[tradeSerie*predictionLen+0], vvolumeF[tradeSerie*predictionLen+0]);
		if (_saveTradeInfo(vEnvS, vTicket, positionTime, vtime[idx], vopen[idx], vhigh[idx], vlow[idx], vclose[idx], vvolume[idx], vopenF[tradeSerie*predictionLen+0], vhighF[tradeSerie*predictionLen+0], vlowF[tradeSerie*predictionLen+0], vcloseF[tradeSerie*predictionLen+0], vvolumeF[tradeSerie*predictionLen+0], tradeScenario, tradeResult, TPhit, SLhit)<0) {
			printf("_saveTradeInfo() failed. see Forecaster logs.");
			return;
		}

		//-- save clientInfo
		if (_saveClientInfo(vEnvS, TimeCurrent())<0) {
			printf("_saveClientInfo() failed. see Forecaster logs.");
			return;
		}

		//-- commit
		_commit(vEnvS);
	}
	//runOnce=true;
	firstTick=false;
}
void OnDeinit(const int reason) {
	CharArrayToString(vEnvS, EnvS);
	printf("OnDeInit() called. EnvS=%s", EnvS);
	if (EnvS!="00000000000000000000000000000000000000000000000000000000000000000") {
		printf("calling _destroyEnv for vEnvS=%s", EnvS);
		_destroyEnv(vEnvS);
	}
}

bool loadBars() {
	int i=0;
	ENUM_TIMEFRAMES tf;
	for (int s=0; s<seriesCnt; s++) {
		tf = getTimeFrameEnum(serieTimeFrame[s]);
		int copied=CopyRates(serieSymbol[s], tf, 1, (batchSize+historyLen-1)+2, serierates);	printf("copied[%d]=%d", s, copied);
		if (copied!=((batchSize+historyLen-1)+2)) return false;
		//-- base bar
		vtimeB[s]=serierates[1].time;// +TimeGMTOffset();
		StringConcatenate(vtimeSB[s], TimeToString(vtimeB[s], TIME_DATE), ".", TimeToString(vtimeB[s], TIME_MINUTES));
		vopenB[s]=serierates[1].open;
		vhighB[s]=serierates[1].high;
		vlowB[s]=serierates[1].low;
		vcloseB[s]=serierates[1].close;
		vvolumeB[s]=serierates[1].real_volume;
		//printf("serie=%d ; time=%s ; OHLCV=%f|%f|%f|%f|%f", s, vtimeSB[s], vopenB[s], vhighB[s], vlowB[s], vcloseB[s], vvolumeB[s]);
		//-- [historyLen] bars
		for (int bar=2; bar<((batchSize+historyLen-1)+2); bar++) {
			vtime[i]=serierates[bar].time;// +TimeGMTOffset();
			StringConcatenate(vtimeS[i], TimeToString(vtime[i], TIME_DATE), ".", TimeToString(vtime[i], TIME_MINUTES));
			vopen[i]=serierates[bar].open;
			vhigh[i]=serierates[bar].high;
			vlow[i]=serierates[bar].low;
			vclose[i]=serierates[bar].close;
			vvolume[i]=serierates[bar].real_volume; if (MathAbs(vvolume[i])>10000) vvolume[i]=0;
			//printf("time[%d]=%s ; OHLCV[%d]=%f|%f|%f|%f|%f", i, vtimeS[i], i, vopen[i], vhigh[i], vlow[i], vclose[i], vvolume[i]);
			i++;
		}
		for (int bar=2; bar<(predictionLen+2); bar++) {
			vtime[i]=serierates[bar].time;// +TimeGMTOffset();
			StringConcatenate(vtimeS[i], TimeToString(vtime[i], TIME_DATE), ".", TimeToString(vtime[i], TIME_MINUTES));
			vopen[i]=serierates[bar].open;
			vhigh[i]=serierates[bar].high;
			vlow[i]=serierates[bar].low;
			vclose[i]=serierates[bar].close;
			vvolume[i]=serierates[bar].real_volume; if (MathAbs(vvolume[i])>10000) vvolume[i]=0;
			//printf("time[%d]=%s ; OHLCV[%d]=%f|%f|%f|%f|%f", i, vtimeS[i], i, vopen[i], vhigh[i], vlow[i], vclose[i], vvolume[i]);
			i++;
		}
	}
	return true;
}
int getTradeScenario(double& oTradeTP, double& oTradeSL) {

	int scenario=-1;
	double point=SymbolInfoDouble(Symbol(), SYMBOL_POINT);
	double fTolerance=0*(10*point);
	double riskRatio=RiskRatio;

	MqlTick tick;
	if (SymbolInfoTick(Symbol(), tick)) {

		double fH=vForecastH;
		double fL=vForecastL;
		double cH=tick.ask;
		double cL=tick.bid;
		double dH=fH-cH;
		double dL=cL-fL;
		double expProfit, expLoss;

		int minDist=SymbolInfoInteger(Symbol(), SYMBOL_TRADE_STOPS_LEVEL);
		int spread=SymbolInfoInteger(Symbol(), SYMBOL_SPREAD);
		double point=SymbolInfoDouble(Symbol(), SYMBOL_POINT);

		printf("getTradeScenario(): cH=%5.4f , cL=%5.4f , fH=%5.4f , fL=%5.4f , dH=%5.4f , dL=%5.4f, minDist=%f , spread=%f", cH, cL, fH, fL, dH, dL, minDist, spread);

		//-- Current price (tick.ask) is below   ForecastL => BUY (1)
		if (cH<fL) {
			scenario = 1;
			oTradeTP=fH-fTolerance;
			expProfit=oTradeTP-cH;
			expProfit=MathMax((minDist+spread)*point, expProfit);
			oTradeTP=cH+expProfit;
			expLoss=expProfit*riskRatio;
			expLoss=MathMax((minDist+spread)*point, expLoss);
			oTradeSL=cL-expLoss;
			printf("Scenario 1 (BUY) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
		}

		//-- Current price is above   ForecastH  => SELL (2)
		if (cL>fH) {
			scenario = 2;
			oTradeTP=fL+fTolerance;
			expProfit=cL-oTradeTP;
			expProfit=MathMax((minDist+spread)*point, expProfit);
			oTradeTP=cL-expProfit;
			expLoss=expProfit*riskRatio;
			expLoss=MathMax((minDist+spread)*point, expLoss);
			oTradeSL=cL+expLoss;
			printf("Scenario 2 (SELL) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
		}

		//-- Current price is between ForecastL and ForecastH, closer to ForecastL  => BUY (3)
		if (cH<=fH && cL>=fL && dL<dH) {
			scenario = 3;
			oTradeTP=fH-fTolerance;
			expProfit=oTradeTP-cH;
			expProfit=MathMax((minDist+spread)*point, expProfit);
			oTradeTP=cH+expProfit;
			expLoss=expProfit*riskRatio;
			expLoss=MathMax((minDist+spread)*point, expLoss);
			oTradeSL=cL-expLoss;
			printf("Scenario 3 (BUY) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
		}

		//-- Current price is between ForecastL and ForecastH, closer to ForecastH  => SELL (4)
		if (cH<=fH && cL>=fL && dH<dL) {
			scenario = 4;
			oTradeTP=fL+fTolerance;
			expProfit=cL-oTradeTP;
			expProfit=MathMax((minDist+spread)*point, expProfit);
			oTradeTP=cL-expProfit;
			expLoss=expProfit*riskRatio;
			expLoss=MathMax((minDist+spread)*point, expLoss);
			oTradeSL=cL+expLoss;
			printf("Scenario 4 (SELL) ; oTradeTP=%5.4f ; expProfit=%5.4f ; expLoss=%5.4f ; oTradeSL=%5.4f", oTradeTP, expProfit, expLoss, oTradeSL);
		}

	}

	return scenario;
}
int NewTrade(int cmd, double volume, double TP, double SL) {

	string symbol=Symbol();
	int    digits=(int)SymbolInfoInteger(symbol, SYMBOL_DIGITS); // number of decimal places
	double point=SymbolInfoDouble(symbol, SYMBOL_POINT);         // point
	double bid=SymbolInfoDouble(symbol, SYMBOL_BID);             // current price for closing LONG
	double ask=SymbolInfoDouble(symbol, SYMBOL_ASK);             // current price for closing SHORT
	double open_price;	//--- receive the current open price for LONG positions

	SL=0;

	string comment;
	bool ret;
	if (cmd==1||cmd==3) {
		//-- Buy
		open_price=SymbolInfoDouble(symbol, SYMBOL_BID);
		string comment=StringFormat("Buy  %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(SL, digits), DoubleToString(TP, digits));
		//printf("calling trade.Buy() with open_price=%f , volume=%f , takeprofit=%5.4f , stoploss=%5.4f", open_price, volume, TP, SL);
		ret=trade.Buy(volume, symbol, open_price, SL, TP, comment);
	}
	if (cmd==2||cmd==4) {
		//-- Sell
		open_price=SymbolInfoDouble(symbol, SYMBOL_ASK);
		string comment=StringFormat("Sell %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(SL, digits), DoubleToString(TP, digits));
		//printf("calling trade.Sell() with open_price=%f , volume=%f , takeprofit=%5.4f , stoploss=%5.4f", open_price, volume, TP, SL);
		ret=trade.Sell(volume, symbol, open_price, SL, TP, comment);
	}
	if (!ret) {
		//--- failure message
		Print("Trade failed. Return code=", trade.ResultRetcode(), ". Code description: ", trade.ResultRetcodeDescription());
		return -1;
	} else {
		//Print("Trade executed successfully. Return code=", trade.ResultRetcode(), " (", trade.ResultRetcodeDescription(), ")");
		return 0;
	}
}
void drawForecast(double H, double L) {
	//-- https://www.youtube.com/watch?v=Y3e1zVROJlY

	//-- get last bar and new bar
	MqlRates rates[];
	int copied=CopyRates(NULL, 0, 0, 2, rates);
	if (copied<=0) Print("Error copying price data ", GetLastError());

	string nameR, nameE;
	//StringConcatenate(nameR, "Rectangle", TimeToString(rates[0].time, TIME_DATE), ".", TimeToString(rates[0].time, TIME_MINUTES));
	StringConcatenate(nameE, "Ellipse", TimeToString(rates[0].time, TIME_DATE), ".", TimeToString(rates[0].time, TIME_MINUTES));

	//	ObjectDelete(_Symbol, name);

	//-- draw the rectangle between last bar and new bar
	//printf("ObjectCreate(H=%f ; L=%f) returns %d", H,L,ObjectCreate(_Symbol, name, OBJ_RECTANGLE, 0, rates[0].time, H, rates[1].time, L));
	//ObjectCreate(_Symbol, nameR, OBJ_RECTANGLE, 0, rates[0].time, H, rates[1].time, L);
	ObjectCreate(_Symbol, nameE, OBJ_ELLIPSE, 0, rates[1].time, H, rates[1].time, L, rates[0].time, (H+L)/2);
	ObjectSetInteger(0, nameE, OBJPROP_COLOR, clrBlue);
	ObjectSetInteger(0, nameE, OBJPROP_WIDTH, 2);
	ObjectSetInteger(0, nameE, OBJPROP_HIDDEN, false);

}

ENUM_TIMEFRAMES getTimeFrameEnum(string tfS) {
	if (tfS=="H1") return PERIOD_H1;
	if (tfS=="D1") return PERIOD_D1;
	return 0;
}
int CSL2Mask(string mask) {
	string selF[5];
	int ret=0;

	int fcnt=StringSplit(mask, ',', selF);
	for (int f=0; f<fcnt; f++) {
		//printf("selF[%d]=%s", f, selF[f]);
		if (StringCompare(selF[f], "0")==0) ret+=10000; //-- OPEN is selected
		if (StringCompare(selF[f], "1")==0) ret+=1000; //-- HIGH is selected
		if (StringCompare(selF[f], "2")==0) ret+=100; //-- LOW is selected
		if (StringCompare(selF[f], "3")==0) ret+=10; //-- CLOSE is selected
		if (StringCompare(selF[f], "4")==0) ret+=1; //-- VOLUME is selected
	}
	return ret;
}