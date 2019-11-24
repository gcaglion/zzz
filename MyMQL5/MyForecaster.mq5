#property copyright "gcaglion"
#property link      "https://algoinvest.org"
#property version   "1.00"
#property strict

#include <Trade/Trade.mqh>

#define MT_MAX_SERIES_CNT 12

#import "Forecaster.dll"
//--
int _createEnv(int accountId_, uchar& clientXMLFile_[], int savedEnginePid_, bool doDump_, uchar& oEnv[], int& oSampleLen_, int &oPredictionLen_, int &oFeaturesCnt_, int &oBatchSize_);
int _getSeriesInfo(uchar& iEnv[], int& oSeriesCnt_, uchar& oSymbolsCSL_[], uchar& oTimeFramesCSL_[], uchar& oFeaturesCSL_[], bool& oChartTrade[]);
int _getForecast(uchar& iEnv[], int seqId_, int extraSteps_, int iseriesCnt_, long& ifeatMask_[], int& iBarT[], double &iBarO[], double &iBarH[], double &iBarL[], double &iBarC[], double &iBarV[], double &iBarMACD[], double &iBarCCI[], double &iBarATR[], double &iBarBOLLH[], double &iBarBOLLM[], double &iBarBOLLL[], double &iBarDEMA[], double &iBarMA[], double &iBarMOM[], int &iBaseBarT[], double &iBaseBarO[], double &iBaseBarH[], double &iBaseBarL[], double &iBaseBarC[], double &iBaseBarV[], double &iBaseBarMACD[], double &iBaseBarCCI[], double &iBaseBarATR[], double &iBaseBarBOLLH[], double &iBaseBarBOLLM[], double &iBaseBarBOLLL[], double &iBaseBarDEMA[], double &iBaseBarMA[], double &iBaseBarMOM[], int oseriesCnt_, long& ofeatMask_[], int &oBarT[], double &oBarO[], double &oBarH[], double &oBarL[], double &oBarC[], double &oBarV[], int &oBaseBarT[], double &oBaseBarO[], double &oBaseBarH[], double &oBaseBarL[], double &oBaseBarC[], double &oBaseBarV[], double &oForecastO[], double &oForecastH[], double &oForecastL[], double &oForecastC[], double &oForecastV[]);
int _getActualFuture(uchar& iEnv[], uchar& iSymbol_[], uchar& iTF_[], uchar& iDate0_[], uchar& oDate1_[], double &oBarO[], double &oBarH[], double &oBarL[], double &oBarC[], double &oBarV[]);
//--
int _saveTradeInfo(uchar& iEnv[], int iPositionTicket, long iPositionOpenTime, long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit);
int _saveClientInfo(uchar& iEnv[], int sequenceId, long iTradeStartTime);
void _commit(uchar& iEnv[]);
int _destroyEnv(uchar& iEnv[]);
#import

enum SLhandling {
	MinDist   = 1,
	RiskRatio = 2,
	Forecast  = 3
};

//--- input parameters - Forecaster dll stuff
input int EnginePid				= 1919;
input string ClientXMLFile		= "C:/Users/gcaglion/dev/zzz/Config/Client.xml";
input bool DumpData				= true;
input bool SaveLogs				= false;
input int PredictionStep		= 1;
input bool GetActualFutureData	= false;
//-- input parameters - Trade stuff
input double TradeVol			= 0.1;
input double RiskRatio			= 0.20;
input double ForecastTolerance	= 2;
input bool tradeClose			= false;
input SLhandling stopsHandling	= 3;

//--- local variables
int vEnginePid=EnginePid;
int vDumpData=DumpData;
//--
ulong vTicket;

//--- miscellaneous variables
string EnvS;
uchar vEnvS[];		// Env in char* format
uchar vClientXMLFileS[];

//--------------------------------------------------------------------------
int seriesCnt; int OUTseriesCnt;
int historyLen;
int predictionLen;
int featuresCnt;
int batchSize;
int extraSteps;
int barsCnt;
int featuresCntFromCfg;
string serieSymbol[MT_MAX_SERIES_CNT];
string serieTimeFrame[MT_MAX_SERIES_CNT];
string serieFeatList[MT_MAX_SERIES_CNT];
long serieFeatMask[MT_MAX_SERIES_CNT];
long OUTserieFeatMask[MT_MAX_SERIES_CNT];
bool chartTrade[MT_MAX_SERIES_CNT];
//--
int ATR_MAperiod=15;
int EMA_fastPeriod=5;
int EMA_slowPeriod=10;
int EMA_signalPeriod=5;
int CCI_MAperiod=15;
int BOLL_period=20;
int BOLL_shift=0;
double BOLL_deviation=2.0;
int DEMA_period=20;
int DEMA_shift=0;
int MA_period=10;
int MA_shift=0;
int MOM_period=4320;
//--
double vopen[], vhigh[], vlow[], vclose[], vvolume[];
double vmacd[], vcci[], vatr[], vbollh[], vbollm[], vbolll[], vdema[], vma[], vmom[];
int vtime[]; string vtimeS[];
double vopenB[], vhighB[], vlowB[], vcloseB[], vvolumeB[];
double vmacdB[], vcciB[], vatrB[], vbollhB[], vbollmB[], vbolllB[], vdemaB[], vmaB[], vmomB[];
int vtimeB[]; string vtimeSB[];
double OUTvopen[], OUTvhigh[], OUTvlow[], OUTvclose[], OUTvvolume[];
int OUTvtime[]; string OUTvtimeS[];
double OUTvopenB[], OUTvhighB[], OUTvlowB[], OUTvcloseB[], OUTvvolumeB[];
int OUTvtimeB[]; string OUTvtimeSB[];

double vopenF[], vhighF[], vlowF[], vcloseF[], vvolumeF[];
double vForecastH, vForecastL, vForecastC;
//--
MqlRates serierates[];
//--
CTrade trade;
//--
bool TPhit;
bool SLhit=false;
//--------------------------------------------------------------------------

int OnInit() {

	extraSteps=0;

	trade.SetExpertMagicNumber(123456);
	trade.SetDeviationInPoints(10);
	trade.SetTypeFilling(ORDER_FILLING_RETURN);
	trade.SetAsyncMode(false);
	trade.LogLevel(LOG_LEVEL_ERRORS);

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
	if (_createEnv((int)AccountInfoInteger(ACCOUNT_LOGIN), vClientXMLFileS, vEnginePid, vDumpData, vEnvS, historyLen, predictionLen, featuresCnt, batchSize)!=0) {
		printf("FAILURE: _createEnv() failed. see Forecaster logs.");
		return -1;
	}
	EnvS=CharArrayToString(vEnvS);
	printf("EnginePid=%d ; SampleLen/PredictionLen/FeaturesCnt/BatchSize=%d/%d/%d/%d ; EnvS=%s ; ClientXMLFile=%s", EnginePid, historyLen, predictionLen, featuresCnt, batchSize, EnvS, ClientXMLFile);
	//barsCnt=batchSize+historyLen-1;// +predictionLen;
	barsCnt=historyLen+extraSteps;

	//--
	//printf("Getting TimeSeries Info from Client Config...");
	if (_getSeriesInfo(vEnvS, seriesCnt, uSymbolsCSL, uTimeFramesCSL, uFeaturesCSL, chartTrade)!=0) {
		printf("FAILURE: _getSeriesInfo() failed. see Forecaster logs.");
		return -1;
	}
	sSymbolsCSL=CharArrayToString(uSymbolsCSL);
	sTimeFramesCSL=CharArrayToString(uTimeFramesCSL);
	sFeaturesCSL=CharArrayToString(uFeaturesCSL);
	//printf("seriesCnt=%d ; symbolsCSL=%s ; timeFramesCSL=%s, featuresCSL=%s", seriesCnt, sSymbolsCSL, sTimeFramesCSL, sFeaturesCSL);

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
	string serieFeatTmp[14];
	for (int s=0; s<seriesCnt; s++) {
		//printf("Symbol/TF [%d] : %s/%s", s, serieSymbol[s], serieTimeFrame[s]);
		serieFeatMask[s]=CSL2Mask(serieFeatList[s]);
		//printf("FeatureList [%d] : %s (%d)", s, serieFeatList[s], serieFeatMask[s]);
		//printf("Trade [%d] : %s", s, (chartTrade[s]) ? "TRUE" : "FALSE");
		featuresCntFromCfg+=StringSplit(serieFeatList[s], ',', serieFeatTmp);
	}
	if (featuresCntFromCfg!=featuresCnt) {
		printf("FAILURE: Total Features Count from Client Config (%d) differs from Engine Features Count (%d)", featuresCntFromCfg, featuresCnt);
		return -1;
	}

	OUTseriesCnt=1;
	OUTserieFeatMask[0]=1000000000000+100000000000;	//-- [HIGH,LOW]

	ArrayResize(vtimeB, seriesCnt);
	ArrayResize(vtimeSB, seriesCnt);
	ArrayResize(vopenB, seriesCnt);
	ArrayResize(vhighB, seriesCnt);
	ArrayResize(vlowB, seriesCnt);
	ArrayResize(vcloseB, seriesCnt);
	ArrayResize(vvolumeB, seriesCnt);
	ArrayResize(vmacdB, seriesCnt);
	ArrayResize(vcciB, seriesCnt);
	ArrayResize(vatrB, seriesCnt);
	ArrayResize(vbollhB, seriesCnt);
	ArrayResize(vbollmB, seriesCnt);
	ArrayResize(vbolllB, seriesCnt);
	ArrayResize(vdemaB, seriesCnt);
	ArrayResize(vmaB, seriesCnt);
	ArrayResize(vmomB, seriesCnt);
	//--
	ArrayResize(vtime, barsCnt*seriesCnt);
	ArrayResize(vtimeS, barsCnt*seriesCnt);
	ArrayResize(vopen, barsCnt*seriesCnt);
	ArrayResize(vhigh, barsCnt*seriesCnt);
	ArrayResize(vlow, barsCnt*seriesCnt);
	ArrayResize(vclose, barsCnt*seriesCnt);
	ArrayResize(vvolume, barsCnt*seriesCnt);
	ArrayResize(vmacd, barsCnt*seriesCnt);
	ArrayResize(vcci, barsCnt*seriesCnt);
	ArrayResize(vatr, barsCnt*seriesCnt);
	ArrayResize(vbollh, barsCnt*seriesCnt);
	ArrayResize(vbollm, barsCnt*seriesCnt);
	ArrayResize(vbolll, barsCnt*seriesCnt);
	ArrayResize(vdema, barsCnt*seriesCnt);
	ArrayResize(vma, barsCnt*seriesCnt);
	ArrayResize(vmom, barsCnt*seriesCnt);
	//--
	ArrayResize(OUTvtimeB, 1);
	ArrayResize(OUTvtimeSB, 1);
	ArrayResize(OUTvopenB, 1);
	ArrayResize(OUTvhighB, 1);
	ArrayResize(OUTvlowB, 1);
	ArrayResize(OUTvcloseB, 1);
	ArrayResize(OUTvvolumeB, 1);
	//--
	ArrayResize(OUTvtime, barsCnt*1);
	ArrayResize(OUTvtimeS, barsCnt*1);
	ArrayResize(OUTvopen, barsCnt*1);
	ArrayResize(OUTvhigh, barsCnt*1);
	ArrayResize(OUTvlow, barsCnt*1);
	ArrayResize(OUTvclose, barsCnt*1);
	ArrayResize(OUTvvolume, barsCnt*1);
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

	static bool firstTick=true;
	int tradeScenario=-1;
	int tradeResult=-1;
	datetime positionTime=0;
	vTicket=-1;
	static int sequenceId=0;
	int maxSteps=-1;
	static double tradeTP=0;
	static double tradeSL=0;
	MqlTick tick;

	static double lastForecastO=0;
	static double lastForecastH=0;
	static double lastForecastL=0;
	static double lastForecastC=0;
	static double lastForecastV=0;

	if (maxSteps<0||sequenceId<maxSteps) {

		//-- manually check for TP/SL
		if (stopsHandling>1&&PositionSelect(Symbol())) {
			SymbolInfoTick(Symbol(), tick); //printf("ask=%f ; bid=%f ; tradeTP=%f ; tradeSL=%f", tick.ask, tick.bid, tradeTP, tradeSL);
			if (PositionGetInteger(POSITION_TYPE)==POSITION_TYPE_BUY) {
				if (tick.bid>=tradeTP) {
					printf("TP on BUY reached."); trade.PositionClose(Symbol(), 10);
				}
				if (tick.ask<=tradeSL) {
					printf("SL on BUY reached."); trade.PositionClose(Symbol(), 10);
				}
			} else {
				if (tick.ask<=tradeTP) {
					printf("TP on SELL reached."); trade.PositionClose(Symbol(), 10);
				}
				if (tick.bid>=tradeSL) {
					printf("SL on SELL reached."); trade.PositionClose(Symbol(), 10);
				}
			}
		}

		// Only do this if there's a new bar
		static datetime Time0=0;
		if (Time0==SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE)) return;

		Time0 = (datetime)SeriesInfoInteger(Symbol(), Period(), SERIES_LASTBAR_DATE);
		string Time0S;
		StringConcatenate(Time0S, TimeToString(Time0, TIME_DATE), ".", TimeToString(Time0, TIME_MINUTES));

		//-- close existing position
		if (PositionSelect(Symbol())) {
			printf("Closing existing position");
			trade.PositionClose(Symbol(), 10);
			//printf("trade.PositionClose() returned %d", trade.ResultRetcode());
		}
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
		if (!loadStats()) {
			printf("loadStats() FAILURE! Exiting...");
			return;
		}

		//------ GET FORECAST ---------
		if (_getForecast(vEnvS, sequenceId, extraSteps, seriesCnt, serieFeatMask, vtime, vopen, vhigh, vlow, vclose, vvolume, vmacd, vcci, vatr, vbollh, vbollm, vbolll, vdema, vma, vmom, vtimeB, vopenB, vhighB, vlowB, vcloseB, vvolumeB, vmacdB, vcciB, vatrB, vbollhB, vbollmB, vbolllB, vdemaB, vmaB, vmomB, OUTseriesCnt, OUTserieFeatMask, OUTvtime, OUTvopen, OUTvhigh, OUTvlow, OUTvclose, OUTvvolume, OUTvtimeB, OUTvopenB, OUTvhighB, OUTvlowB, OUTvcloseB, OUTvvolumeB, vopenF, vhighF, vlowF, vcloseF, vvolumeF)!=0) {
			printf("_getForecast() FAILURE! Exiting...");
			return;
		};
		//------ PRINT FORECAST FOR ALL SERIES, AND PICK SERIE FROM CURRENT CHART SYMBOL/TF ---------
		int tradeSerie=-1;
		for (int s=0; s<OUTseriesCnt; s++) {
			for (int bar=0; bar<predictionLen; bar++) {
				printf("OHLCV Forecast, serie %d: %f|%f|%f|%f|%f", s, (vopenF[s*predictionLen+bar]<0) ? 0 : vopenF[s*predictionLen+bar], (vhighF[s*predictionLen+bar]<0) ? 0 : vhighF[s*predictionLen+bar], (vlowF[s*predictionLen+bar]<0) ? 0 : vlowF[s*predictionLen+bar], (vcloseF[s*predictionLen+bar]<0) ? 0 : vcloseF[s*predictionLen+bar], (vvolumeF[s*predictionLen+bar]<0) ? 0 : vvolumeF[s*predictionLen+bar]);
				if (StringCompare(serieSymbol[s], Symbol())==0&&getTimeFrameEnum(serieTimeFrame[s])==Period()) tradeSerie=s;
			}
		}
		if (tradeSerie==-1) {
			printf("Nothing to trade in current chart. Exiting...");
			return;
		}

		//-- take first bar from vhighF, vlowF
		vForecastH=vhighF[tradeSerie*predictionLen+PredictionStep];
		vForecastL=vlowF[tradeSerie*predictionLen+PredictionStep];
		vForecastC=vcloseF[tradeSerie*predictionLen+PredictionStep];

		//=============== Get Actual Future Data, override forecast ======================
		if (GetActualFutureData) {
			string smb="EURUSD"; uchar smbS[]; StringToCharArray(smb, smbS);
			string tf="H1"; uchar tfS[]; StringToCharArray(tf, tfS);
			uchar t0s[]; StringToCharArray(Time0S, t0s);
			string t1="01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"; uchar t1s[]; StringToCharArray(t1, t1s);

			if (_getActualFuture(vEnvS, smbS, tfS, t0s, t1s, vopenF, vhighF, vlowF, vcloseF, vvolumeF)!=0) {
				printf("_getActualFuture() FAILURE! Exiting...");
				return;
			}
			vForecastH=vhighF[0];
			vForecastL=vlowF[0];
			vForecastC=vcloseF[0];
		}
		//===============================================================================

		printf("==== Sequence: %d ; Last Bar(%s): H=%6.5f ; L=%6.5f ; C=%6.5f ; Forecast: H=%6.5f ; L=%6.5f ; C=%6.5f", sequenceId, vtimeS[barsCnt-1], vhigh[barsCnt-1], vlow[barsCnt-1], vclose[barsCnt-1], vForecastH, vForecastL, vForecastC);
		//-- check for forecast consistency in first bar (H>L)
		if (vForecastL>vForecastH) {
			printf("Invalid Forecast: H=%6.5f ; L=%6.5f . Exiting...", vForecastH, vForecastL);
		} else {
			//-- draw rectangle around the current bar extending from vPredictedDataH[0] to vPredictedDataL[0]
			drawForecast(vForecastH, vForecastL);

			//-- define trade scenario based on current price level and forecast
			double tradeVol=TradeVol;

			tradeScenario=getTradeScenario(tradeTP, tradeSL); printf("trade scenario=%d ; tradeTP=%6.5f ; tradeSL=%6.5f", tradeScenario, tradeTP, tradeSL);
			if (tradeScenario>=0) {

				//-- do the actual trade
				//printf("======================================== CALL TO NewTrade() ========================================================================================================================");
				tradeResult=NewTrade(tradeScenario, tradeVol, tradeTP, tradeSL);
				//printf("====================================================================================================================================================================================");

				//-- if trade successful, store position ticket in shared variable
				if (tradeResult==0) {
					vTicket = PositionGetTicket(0);
					int positionId=PositionSelect(Symbol());
					positionTime=(datetime)PositionGetInteger(POSITION_TIME);
				}
			}
		}

		if (SaveLogs) {
			//-- save tradeInfo, even if we do not trade
			int idx=tradeSerie*barsCnt+barsCnt-1;
			if (_saveTradeInfo(vEnvS, (int)vTicket, positionTime, vtime[idx], vopen[idx], vhigh[idx], vlow[idx], vclose[idx], vvolume[idx], lastForecastO, lastForecastH, lastForecastL, lastForecastC, lastForecastV, vopenF[tradeSerie*predictionLen+0], vhighF[tradeSerie*predictionLen+0], vlowF[tradeSerie*predictionLen+0], vcloseF[tradeSerie*predictionLen+0], vvolumeF[tradeSerie*predictionLen+0], tradeScenario, tradeResult, TPhit, SLhit)<0) {
				printf("_saveTradeInfo() failed. see Forecaster logs.");
				return;
			}
			//-- save clientInfo
			if (_saveClientInfo(vEnvS, sequenceId, TimeCurrent())<0) {
				printf("_saveClientInfo() failed. see Forecaster logs.");
				return;
			}

			//-- commit
			_commit(vEnvS);
		}

		lastForecastO=vopenF[tradeSerie*predictionLen+0];
		lastForecastH=vhighF[tradeSerie*predictionLen+0];
		lastForecastL=vlowF[tradeSerie*predictionLen+0];
		lastForecastC=vcloseF[tradeSerie*predictionLen+0];
		lastForecastV=vvolumeF[tradeSerie*predictionLen+0];

	}
	firstTick=false;
	sequenceId++;
}
void OnDeinit(const int reason) {
	EnvS=CharArrayToString(vEnvS);
	printf("OnDeInit() called. EnvS=%s", EnvS);
	if (EnvS!="00000000000000000000000000000000000000000000000000000000000000000") {
		printf("calling _destroyEnv for vEnvS=%s", EnvS);
		_destroyEnv(vEnvS);
	}
	delIndicators();
}
bool loadBars() {
	ENUM_TIMEFRAMES tf;
	int copied;

	//-- INPUT Series/Features
	for (int s=0; s<seriesCnt; s++) {
		tf = getTimeFrameEnum(serieTimeFrame[s]);
		copied=CopyRates(serieSymbol[s], tf, 1, barsCnt+PredictionStep+1, serierates);	//printf("copied[%d]=%d", s, copied);
		if (copied!=(barsCnt+PredictionStep+1)) return false;
		//-- base bar
		vtimeB[s]=serierates[0].time;// +TimeGMTOffset();
		StringConcatenate(vtimeSB[s], TimeToString(vtimeB[s], TIME_DATE), ".", TimeToString(vtimeB[s], TIME_MINUTES));
		vopenB[s]=serierates[0].open;
		vhighB[s]=serierates[0].high;
		vlowB[s]=serierates[0].low;
		vcloseB[s]=serierates[0].close;
		vvolumeB[s]=serierates[0].real_volume;
		//printf("serie=%d ; time=%s ; OHLCV=%f|%f|%f|%f|%f", s, vtimeSB[s], vopenB[s], vhighB[s], vlowB[s], vcloseB[s], vvolumeB[s]);

		//-- [historyLen] bars
		for (int bar=0; bar<barsCnt; bar++) {
			vtime[s*barsCnt+bar]=serierates[bar+1].time;// +TimeGMTOffset();
			StringConcatenate(vtimeS[s*barsCnt+bar], TimeToString(vtime[s*barsCnt+bar], TIME_DATE), ".", TimeToString(vtime[s*barsCnt+bar], TIME_MINUTES));
			vopen[s*barsCnt+bar]=serierates[bar+1].open;
			vhigh[s*barsCnt+bar]=serierates[bar+1].high;
			vlow[s*barsCnt+bar]=serierates[bar+1].low;
			vclose[s*barsCnt+bar]=serierates[bar+1].close;
			vvolume[s*barsCnt+bar]=serierates[bar+1].real_volume; if (MathAbs(vvolume[s*barsCnt+bar])>10000) vvolume[s*barsCnt+bar]=0;
		}
		//for (int bar=0; bar<barsCnt; bar++) printf("time[%d]=%s ; OHLCV[%d]=%f|%f|%f|%f|%f", bar, vtimeS[bar], bar, vopen[bar], vhigh[bar], vlow[bar], vclose[bar], vvolume[bar]);
	}

	//-- OUTPUT Features from current chart
	tf=Period();
	copied=CopyRates(Symbol(), tf, 1, barsCnt+PredictionStep+1, serierates);	//printf("copied[%d]=%d", s, copied);
	if (copied!=(barsCnt+PredictionStep+1)) return false;
	//-- base bar
	OUTvtimeB[0]=serierates[0].time;// +TimeGMTOffset();
	StringConcatenate(OUTvtimeSB[0], TimeToString(vtimeB[0], TIME_DATE), ".", TimeToString(OUTvtimeB[0], TIME_MINUTES));
	OUTvopenB[0]=serierates[0].open;
	OUTvhighB[0]=serierates[0].high;
	OUTvlowB[0]=serierates[0].low;
	OUTvcloseB[0]=serierates[0].close;
	OUTvvolumeB[0]=serierates[0].real_volume;
	//printf("OUT(current) serie, base values: time=%s ; OHLCV=%f|%f|%f|%f|%f", OUTvtimeSB[0], OUTvopenB[0], OUTvhighB[0], OUTvlowB[0], OUTvcloseB[0], OUTvvolumeB[0]);

	for (int bar=0; bar<barsCnt; bar++) {
		OUTvtime[bar]=serierates[bar+1].time;// +TimeGMTOffset();
		StringConcatenate(OUTvtimeS[bar], TimeToString(OUTvtime[bar], TIME_DATE), ".", TimeToString(OUTvtime[bar], TIME_MINUTES));
		OUTvopen[bar]=serierates[bar+1].open;
		OUTvhigh[bar]=serierates[bar+1].high;
		OUTvlow[bar]=serierates[bar+1].low;
		OUTvclose[bar]=serierates[bar+1].close;
		OUTvvolume[bar]=serierates[bar+1].real_volume; if (MathAbs(OUTvvolume[bar])>10000) vvolume[bar]=0;
	}
	//for (int bar=0; bar<barsCnt; bar++) printf("OUT(current) serie : time[%d]=%s ; OHLCV[%d]=%f|%f|%f|%f|%f", bar, OUTvtimeS[bar], bar, OUTvopen[bar], OUTvhigh[bar], OUTvlow[bar], OUTvclose[bar], OUTvvolume[bar]);

	return true;
}
bool loadStats() {
	ENUM_TIMEFRAMES tf;
	int copied;
	int bar;
	int handle; double value1[]; double value2[]; double value3[];

	//-- INPUT Stats
	for (int s=0; s<seriesCnt; s++) {
		tf = getTimeFrameEnum(serieTimeFrame[s]);
		//--
		handle = iMACD(serieSymbol[s], tf, EMA_fastPeriod, EMA_slowPeriod, EMA_signalPeriod, PRICE_CLOSE);
		ArraySetAsSeries(value1, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0) {
			printf("MACD copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vmacdB[s]=value1[0];
		for (bar=0; bar<barsCnt; bar++) vmacd[s*barsCnt+bar]=value1[bar+1];
		//--
		handle = iCCI(serieSymbol[s], tf, CCI_MAperiod, PRICE_CLOSE);
		ArraySetAsSeries(value1, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0) {
			printf("CCI copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vcciB[s]=value1[0];
		for (bar=0; bar<barsCnt; bar++) vcci[s*barsCnt+bar]=value1[bar+1];
		//--
		handle = iATR(serieSymbol[s], tf, ATR_MAperiod);
		ArraySetAsSeries(value1, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0) {
			printf("ATR copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vatrB[s]=value1[0];
		for (bar=0; bar<barsCnt; bar++) vatr[s*barsCnt+bar]=value1[bar+1];
		//--
		handle = iBands(serieSymbol[s], tf, BOLL_period, BOLL_shift, BOLL_deviation, PRICE_CLOSE);
		ArraySetAsSeries(value1, false); ArraySetAsSeries(value2, false); ArraySetAsSeries(value3, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0||CopyBuffer(handle, 1, 0, barsCnt+2, value2)<=0||CopyBuffer(handle, 2, 0, barsCnt+2, value3)<=0) {
			printf("BOLL copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vbollhB[s]=value2[0]; vbollmB[s]=value2[0]; vbolllB[s]=value3[0];
		for (bar=0; bar<barsCnt; bar++) {
			vbollh[s*barsCnt+bar]=value2[bar+1];
			vbollm[s*barsCnt+bar]=value1[bar+1];
			vbolll[s*barsCnt+bar]=value3[bar+1];
		}
		//--
		handle = iDEMA(serieSymbol[s], tf, DEMA_period, DEMA_shift, PRICE_CLOSE);
		ArraySetAsSeries(value1, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0) {
			printf("DEMA copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vdemaB[s]=value1[0];
		for (bar=0; bar<barsCnt; bar++) vdema[s*barsCnt+bar]=value1[bar+1];
		//--
		handle = iMA(serieSymbol[s], tf, MA_period, MA_shift, MODE_SMA, PRICE_CLOSE);
		ArraySetAsSeries(value1, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0) {
			printf("MA copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vmaB[s]=value1[0];
		for (bar=0; bar<barsCnt; bar++) vma[s*barsCnt+bar]=value1[bar+1];
		//--
		handle = iMomentum(serieSymbol[s], tf, MOM_period, PRICE_CLOSE);
		ArraySetAsSeries(value1, false);
		if (CopyBuffer(handle, 0, 0, barsCnt+2, value1)<=0) {
			printf("MOM copyBuffer failed. Error %d", GetLastError());
			return false;
		}
		vmomB[s]=value1[0];
		for (bar=0; bar<barsCnt; bar++) vmom[s*barsCnt+bar]=value1[bar+1];
	}
	return true;
}
int getTradeScenario(double& oTradeTP, double& oTradeSL) {

	int scenario=-1;
	double point=SymbolInfoDouble(Symbol(), SYMBOL_POINT);
	double fTolerance=ForecastTolerance*(10*point);
	double riskRatio=RiskRatio;

	MqlTick tick;
	if (SymbolInfoTick(Symbol(), tick)) {

		double fH=vForecastH;
		double fL=vForecastL;
		double fC=vForecastC;
		double cH=tick.ask;
		double cL=tick.bid;
		double dH=fH-cH;
		double dL=cL-fL;
		double expProfit, expLoss;

		long minDist=SymbolInfoInteger(Symbol(), SYMBOL_TRADE_STOPS_LEVEL);
		long spread=SymbolInfoInteger(Symbol(), SYMBOL_SPREAD);

		printf("getTradeScenario(): cH=%6.5f , cL=%6.5f , fH=%6.5f , fL=%6.5f , fC=%6.5f , dH=%6.5f , dL=%6.5f, minDist=%f , spread=%f", cH, cL, fH, fL, fC, dH, dL, minDist, spread);

		if (tradeClose) {
			//-- forecast Close is above current price high (ask)	=> BUY (1)
			if (fC>cH) {
				scenario=1;
				oTradeTP=fC-fTolerance;
				expProfit=oTradeTP-cH;
				if (stopsHandling==MinDist) expProfit=MathMax((minDist+spread)*point, expProfit);
				oTradeTP=cH+expProfit;
				expLoss=expProfit*riskRatio;
				if (stopsHandling==MinDist) expLoss=MathMax((minDist+spread)*point, expLoss);
				oTradeSL=cL-expLoss;
				if (stopsHandling==Forecast) oTradeSL=fL;
				printf("Scenario 1 (BUY) ; oTradeTP=%6.5f ; expProfit=%6.5f ; expLoss=%6.5f ; oTradeSL=%6.5f", oTradeTP, expProfit, expLoss, oTradeSL);
			}
			//-- forecast Close is below current price low (bid)	=> SELL (2)
			if (fC<cL) {
				scenario=2;
				oTradeTP=fC+fTolerance;
				expProfit=cL-oTradeTP;
				if (stopsHandling==MinDist) expProfit=MathMax((minDist+spread)*point, expProfit);
				oTradeTP=cL-expProfit;
				expLoss=expProfit*riskRatio;
				if (stopsHandling==MinDist) expLoss=MathMax((minDist+spread)*point, expLoss);
				oTradeSL=cL+expLoss;
				if (stopsHandling==Forecast) oTradeSL=fH;
				printf("Scenario 2 (SELL) ; oTradeTP=%6.5f ; expProfit=%6.5f ; expLoss=%6.5f ; oTradeSL=%6.5f", oTradeTP, expProfit, expLoss, oTradeSL);
			}
		} else {
			//-- Current price (tick.ask) is below   ForecastL => BUY (1)
			if (cL<fL) {
				scenario = 1;
				oTradeTP=fH-fTolerance;
				expProfit=oTradeTP-cH;
				if (stopsHandling==MinDist) expProfit=MathMax((minDist+spread)*point, expProfit);
				oTradeTP=cH+expProfit;
				expLoss=expProfit*riskRatio;
				if (stopsHandling==MinDist) expLoss=MathMax((minDist+spread)*point, expLoss);
				oTradeSL=cL-expLoss;
				if (stopsHandling==Forecast) oTradeSL=fL;
				printf("Scenario 1 (BUY) ; oTradeTP=%6.5f ; expProfit=%6.5f ; expLoss=%6.5f ; oTradeSL=%6.5f", oTradeTP, expProfit, expLoss, oTradeSL);
			}

			//-- Current price is above   ForecastH  => SELL (2)
			if (cH>fH) {
				scenario = 2;
				oTradeTP=fL+fTolerance;
				expProfit=cL-oTradeTP;
				if (stopsHandling==MinDist) expProfit=MathMax((minDist+spread)*point, expProfit);
				oTradeTP=cL-expProfit;
				expLoss=expProfit*riskRatio;
				if (stopsHandling==MinDist) expLoss=MathMax((minDist+spread)*point, expLoss);
				oTradeSL=cL+expLoss;
				if (stopsHandling==Forecast) oTradeSL=fH;
				printf("Scenario 2 (SELL) ; oTradeTP=%6.5f ; expProfit=%6.5f ; expLoss=%6.5f ; oTradeSL=%6.5f", oTradeTP, expProfit, expLoss, oTradeSL);
			}

			//-- Current price is between ForecastL and ForecastH, closer to ForecastL  => BUY (3)
			if (cH<=fH && cL>=fL && dL<dH) {
				scenario = 3;
				oTradeTP=fH-fTolerance;
				expProfit=oTradeTP-cH;
				if (stopsHandling==MinDist) expProfit=MathMax((minDist+spread)*point, expProfit);
				oTradeTP=cH+expProfit;
				expLoss=expProfit*riskRatio;
				if (stopsHandling==MinDist) expLoss=MathMax((minDist+spread)*point, expLoss);
				oTradeSL=cL-expLoss;
				if (stopsHandling==Forecast) oTradeSL=fL;
				printf("Scenario 3 (BUY) ; oTradeTP=%6.5f ; expProfit=%6.5f ; expLoss=%6.5f ; oTradeSL=%6.5f", oTradeTP, expProfit, expLoss, oTradeSL);
			}

			//-- Current price is between ForecastL and ForecastH, closer to ForecastH  => SELL (4)
			if (cH<=fH && cL>=fL && dH<dL) {
				scenario = 4;
				oTradeTP=fL+fTolerance;
				expProfit=cL-oTradeTP;
				if (stopsHandling==MinDist) expProfit=MathMax((minDist+spread)*point, expProfit);
				oTradeTP=cL-expProfit;
				expLoss=expProfit*riskRatio;
				if (stopsHandling==MinDist) expLoss=MathMax((minDist+spread)*point, expLoss);
				oTradeSL=cL+expLoss;
				if (stopsHandling==Forecast) oTradeSL=fH;
				printf("Scenario 4 (SELL) ; oTradeTP=%6.5f ; expProfit=%6.5f ; expLoss=%6.5f ; oTradeSL=%6.5f", oTradeTP, expProfit, expLoss, oTradeSL);
			}
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

	double tp=(stopsHandling!=MinDist) ? 0 : TP;
	double sl=(stopsHandling!=MinDist) ? 0 : SL;

	string comment;
	bool ret=false;
	if (cmd==1||cmd==3) {
		//-- Buy
		open_price=SymbolInfoDouble(symbol, SYMBOL_BID);
		comment=StringFormat("Buy  %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(sl, digits), DoubleToString(tp, digits));
		//printf("calling trade.Buy() with open_price=%f , volume=%f , takeprofit=%5.4f , stoploss=%5.4f", open_price, volume, tp, sl);
		ret=trade.Buy(volume, symbol, open_price, sl, tp, comment);
	}
	if (cmd==2||cmd==4) {
		//-- Sell
		open_price=SymbolInfoDouble(symbol, SYMBOL_ASK);
		comment=StringFormat("Sell %s %G lots at %s, SL=%s TP=%s", symbol, volume, DoubleToString(open_price, digits), DoubleToString(sl, digits), DoubleToString(tp, digits));
		//printf("calling trade.Sell() with open_price=%f , volume=%f , takeprofit=%5.4f , stoploss=%5.4f", open_price, volume, tp, sl);
		ret=trade.Sell(volume, symbol, open_price, sl, tp, comment);
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

	string nameE;
	StringConcatenate(nameE, "Forecast", TimeToString(rates[1].time, TIME_DATE), ".", TimeToString(rates[1].time, TIME_MINUTES), " H=", DoubleToString(H, 5), " ; L=", DoubleToString(L, 5));

	//-- draw the rectangle between last bar and new bar
	ObjectCreate(0, nameE, OBJ_ELLIPSE, 0, rates[1].time, H, rates[1].time, L, (rates[0].time+rates[1].time)/2, (H+L)/2);
	ObjectSetInteger(0, nameE, OBJPROP_COLOR, clrBlue);
	ObjectSetInteger(0, nameE, OBJPROP_WIDTH, 2);
	ObjectSetInteger(0, nameE, OBJPROP_HIDDEN, false);

}
void delIndicators() {
	string cname;
	for (int i=0; i<3; i++) {
		cname=ChartIndicatorName(0, 0, 0); //printf("cname=%s", cname);
		if (!ChartIndicatorDelete(0, 0, cname)) {
			printf("Error (%i) trying to delete indicator %s", GetLastError(), cname);
		}
	}
	for (int i=0; i<4; i++) {
		cname=ChartIndicatorName(0, 1, 0); //printf("cname=%s", cname);
		if (!ChartIndicatorDelete(0, 1, cname)) {
			printf("Error (%i) trying to delete indicator %s", GetLastError(), cname);
		}
	}
}
ENUM_TIMEFRAMES getTimeFrameEnum(string tfS) {
	if (tfS=="H1") return PERIOD_H1;
	if (tfS=="D1") return PERIOD_D1;
	return 0;
}
long CSL2Mask(string mask) {
	string selF[14];
	long ret=0;

	int fcnt=StringSplit(mask, ',', selF);
	for (int f=0; f<fcnt; f++) {
		if (StringCompare(selF[f], "0")==0) ret+=10000000000000; //-- OPEN is selected
		if (StringCompare(selF[f], "1")==0) ret+=1000000000000; //-- HIGH is selected
		if (StringCompare(selF[f], "2")==0) ret+=100000000000; //-- LOW is selected
		if (StringCompare(selF[f], "3")==0) ret+=10000000000; //-- CLOSE is selected
		if (StringCompare(selF[f], "4")==0) ret+=1000000000; //-- VOLUME is selected
		if (StringCompare(selF[f], "5")==0) ret+=100000000; //-- MACD is selected
		if (StringCompare(selF[f], "6")==0) ret+=10000000; //-- CCI is selected
		if (StringCompare(selF[f], "7")==0) ret+=1000000; //-- ATR  is selected
		if (StringCompare(selF[f], "8")==0) ret+=100000; //-- BOLLH  is selected
		if (StringCompare(selF[f], "9")==0) ret+=10000; //-- BOLLM  is selected
		if (StringCompare(selF[f], "10")==0) ret+=1000; //-- BOLLL is selected
		if (StringCompare(selF[f], "11")==0) ret+=100; //-- DEMA is selected
		if (StringCompare(selF[f], "12")==0) ret+=10; //-- MA is selected
		if (StringCompare(selF[f], "13")==0) ret+=1; //-- MOM is selected

	}
	return ret;
}
