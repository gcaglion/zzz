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
double vSampleDataO[];
double vSampleDataH[], vSampleBaseValH;
double vSampleDataL[], vSampleBaseValL;
double vSampleDataC[];
double vSampleDataV[];
double vValidationDataH[], vValidationBaseValH;
double vValidationDataL[], vValidationBaseValL;
double vFutureDataH[];
double vFutureDataL[];
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
	ArrayResize(vSampleDataO, vSampleLen);
	ArrayResize(vSampleDataH, vSampleLen);
	ArrayResize(vSampleDataL, vSampleLen);
	ArrayResize(vSampleDataC, vSampleLen);
	ArrayResize(vSampleBW, vSampleLen);
	ArrayResize(vValidationDataH, vSampleLen);
	ArrayResize(vValidationDataL, vSampleLen);
	ArrayResize(vFutureDataH, vPredictionLen);
	ArrayResize(vFutureDataL, vPredictionLen);
	ArrayResize(vFutureBW, vPredictionLen);
	ArrayResize(vPredictedDataH, vPredictionLen);
	ArrayResize(vPredictedDataL, vPredictionLen);
	ArrayResize(vSampleTime, vSampleLen);
	ArrayResize(vValidationTime, vSampleLen);

	return(INIT_SUCCEEDED);
}
/*void OnDeinit(const int reason) {

	printf("OnDDeInit() reason=%d", reason);

	//-- Disconnect from DB upon exiting
	if(reason==REASON_INITFAILED || reason==REASON_PROGRAM) {
		MTOraDisconnect(vParamOverrideCnt, vParamOverride, vEnvS, 0);
	} else{
		//-- Before commit, save ElapsedTime in ClientInfo
		t1 = GetTickCount();
		printf("t0=%d ; t1=%d ; elapsed=%d", t0, t1, (t1-t0));
		if (MTUpdateClientInfo(vParamOverrideCnt, vParamOverride, vEnvS, (t1-t0)) != 0) Alert("Failed to update Client duration!");

		MTOraDisconnect(vParamOverrideCnt, vParamOverride, vEnvS, 1);
	}
}
void OnTick() {
   // Only do this if there's a new bar
   static datetime Time0; if (Time0 == Time[0]) return; Time0 = Time[0];

   int vTradeType;
   int ret;

   //-- load bars into arrrays
   LoadBars();

   //-- before GetForecast()
   StringToCharArray(vSampleTime[0], vFirstBarT);
   StringToCharArray(vSampleTime[vSampleLen-1], vLastBarT);
   if (vBarId>0) {
	   if (!IsTesting()) printf("RunForecast() CheckPoint 2: Previous Forecast (H|L)=%f|%f ; Previous Actual (H|L)=%f|%f => Previous Error (H|L)=%f|%f", vPrevFH0, vPrevFL0, High[1], Low[1], MathAbs(vPrevFH0-High[1]), MathAbs(vPrevFL0-Low[1]));
   }

   DllRetVal=GetForecast();

   //-- after GetForecast()
   printf("DllRetVal=%d", DllRetVal);
   switch (DllRetVal) {
   case 0:
	   vTradeType = DoTrade();
	   printf("calling MTSaveTradeInfo with vTradeType=%d , vTradeSize=%f, vTradeTP=%f, vTradeSL=%f", vTradeType, vTradeSize, vTradeTP, vTradeSL);
		ret=MTSaveTradeInfo(
			vParamOverrideCnt, vParamOverride, vBarId, vEnvS, 
			vLastBarT, vSampleDataO[vSampleLen-1], vSampleDataH[vSampleLen-1], vSampleDataL[vSampleLen-1], vSampleDataC[vSampleLen-1],
			vFirstBarT,  vSampleDataO[0], vSampleDataH[0], vSampleDataL[0], vSampleDataC[0],
			vPrevFH0, vPrevFL0,
			Bid, Ask,
			vPredictedDataH[0], vPredictedDataL[0],
			vTradeType, vTradeSize, vTradeTP, vTradeSL
		);
		printf("MTSaveTradeInfo() returned %d", ret);
		if (ret!=0) {
			ExpertRemove();
			break;
		}

		vPrevFH0 = vPredictedDataH[0]; vPrevFL0 = vPredictedDataL[0];
		MTOraCommit(vParamOverrideCnt, vParamOverride, vEnvS);
		break;
   case -1:
	   ExpertRemove();
	   break;
   case -2:
	   MTOraCommit(vParamOverrideCnt, vParamOverride, vEnvS);
	   break;
   }

}

void LoadBars() {
	//-- This loads bar values into Sample and Base arrays

	int TesterShift = (IsTesting()) ? 1 : 0;
	//-- 1. Invert bars sequence
	for (int i = 0; i<vSampleLen; i++) {    // (i=0 is the current bar)
		vSampleTime[i] = StringConcatenate(TimeToStr(Time[vSampleLen-i-1+TesterShift], TIME_DATE), " ", TimeToStr(Time[vSampleLen-i-1+TesterShift], TIME_MINUTES));
		vSampleDataO[i] = Open[vSampleLen-i-1+TesterShift];
		vSampleDataH[i] = High[vSampleLen-i-1+TesterShift];
		vSampleDataL[i] = Low[vSampleLen-i-1+TesterShift];
		vSampleDataC[i] = Close[vSampleLen-i-1+TesterShift];
		vSampleBW[i] = vSampleDataH[i]-vSampleDataL[i];
		vValidationTime[i] = StringConcatenate(TimeToStr(Time[vSampleLen+vValidationShift-i-1+TesterShift], TIME_DATE), " ", TimeToStr(Time[vSampleLen+vValidationShift-i-1+TesterShift], TIME_MINUTES));
		vValidationDataH[i] = High[vSampleLen+vValidationShift-i-1+TesterShift];
		vValidationDataL[i] = Low[vSampleLen+vValidationShift-i-1+TesterShift];
	}
	printf("vSampleData[0] =%f|%f", vSampleDataH[0], vSampleDataL[0]);
	printf("vSampleData[899] =%f|%f", vSampleDataH[899], vSampleDataL[899]);

	//-- 2. Add "Base" bar, needed for Delta Transformation
	vSampleBaseValH = High[vSampleLen+TesterShift];
	vSampleBaseValL = Low[vSampleLen+TesterShift];
	vSampleBaseTime = StringConcatenate(TimeToStr(Time[vSampleLen+TesterShift], TIME_DATE), " ", TimeToStr(Time[vSampleLen+TesterShift], TIME_MINUTES));

}
int DoTrade(){
	int vTradeType=-1;

	//-- Determine Trade
	RefreshRates();
	double FH             = vPredictedDataH[0];
	double FL             = vPredictedDataL[0];
	double RR             = RiskRatio;          // Risk Ratio (PIPS to SL / PIPS to TP)
	double SL, TP;
	double StopLevel=MarketInfo(Symbol(), MODE_STOPLEVEL)*MarketInfo(Symbol(), MODE_TICKSIZE);   
	double MinProfit = MinProfitPIPs*MarketInfo(Symbol(), MODE_TICKSIZE)*10;
     
	printf("1. Current Bar Ask=%5.4f ; Bid=%5.4f ; FH=%5.4f ; FL=%5.4f", Ask, Bid, FH, FL);
	//-- Current price (Ask) is below   ForecastL                                     => BUY (1)
	if(Ask<FL){
		vTradeType = 1;
		TP=FH; //printf("Original TP=%f", TP);  //-Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
		if( (TP-Ask)<MinProfit ){
			printf("Profit too small. No Trade.");
			return (-vTradeType);
		} 
		SL=Bid-(TP-Ask)*RR; //printf("Original SL=%f", SL);
		if( (Bid-SL)<StopLevel ) SL=Bid-StopLevel;
		printf("Ask<FL (1); TP=%5.4f ; SL=%5.4f", TP, SL);
		NewTrade(OP_BUY, Ask, SL, TP);
	}
	//-- Current price is above   ForecastH                                     => SELL (2)
	if(Bid>FH){
		vTradeType = 2;
		TP=FL; //printf("Original TP=%f", TP);   //+Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
		if( (Bid-TP)<MinProfit ){
			printf("Profit too small. No Trade.");
			return (-vTradeType);
		}
		SL=Ask+(Bid-TP)*RR; //printf("Original SL=%f", SL);
		if( (SL-Ask)<StopLevel ) SL=Ask+StopLevel;
		printf("Bid>FH (2) ; TP=%5.4f ; SL=%5.4f", TP, SL);
		NewTrade(OP_SELL, Bid, SL, TP);
	}
	//-- Current price is between ForecastL and ForecastH, closer to ForecastL  => BUY (3)
	if( Ask<=FH && Bid>=FL &&(Bid-FL)<=(FH-Ask) ){
		vTradeType = 3;
		TP=FH; //printf("Original TP=%f", TP);   //-Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
		if( (TP-Ask)<MinProfit ){
			printf("Profit too small. No Trade.");
			return (-vTradeType);
		}
		SL=Bid-(TP-Ask)*RR; //printf("Original SL=%f", SL);
		if( (Bid-SL)<StopLevel ) SL=Bid-StopLevel;
		printf("(3)");
		NewTrade(OP_BUY, Ask, SL, TP);
	}
	//-- Current price is between ForecastL and ForecastH, closer to ForecastH  => SELL (4)
	if( Ask<=FH && Bid>=FL && (FH-Ask)<=(Bid-FL) ){
		vTradeType = 4;
		TP=FL; //printf("Original TP=%f", TP);   //+Ferr*MarketInfo(Symbol(),MODE_TICKSIZE)*10;
		if( (Bid-TP)<MinProfit ){
			printf("Profit too small. No Trade.");
			return (-vTradeType);
		}
		SL=Ask+(Bid-TP)*RR; //printf("Original SL=%f", SL);
		if( (SL-Ask)<StopLevel ) SL=Ask+StopLevel;
		printf("(4)");
		NewTrade(OP_SELL, Bid, SL, TP);
	}
	//-- Display Forecasts (only for the next bar)
	DrawForecast(vPredictedDataH[0], vPredictedDataL[0]);
      
	return vTradeType;
}
int GetForecast() {
	int retries=0;
	int TesterShift = (IsTesting()) ? 1 : 0;

	if (!IsTesting()) {
		printf("RunForecast() CheckPoint 1: vBar[%d]: DateTime=%s, High=%f, Low=%f, vBar[%d]: DateTime=%s, High=%f, Low=%f", 0, vSampleTime[0], vSampleDataH[0], vSampleDataL[0], vSampleLen-1+TesterShift, vSampleTime[vSampleLen-1+TesterShift], vSampleDataH[vSampleLen-1+TesterShift], vSampleDataL[vSampleLen-1+TesterShift]);
		printf("RunForecast() CheckPoint 1: vBarBase: DateTime=%s, High=%f, Low=%f", vSampleBaseTime, vSampleBaseValH, vSampleBaseValL);
		printf("RunForecast() CheckPoint 1: vBar_V[%d]: DateTime=%s, High=%f, Low=%f, vBar_V[%d]: DateTime=%s, High=%f, Low=%f", 0, vValidationTime[0], vValidationDataH[0], vValidationDataL[0], vSampleLen-1+TesterShift, vValidationTime[vSampleLen-1+TesterShift], vValidationDataH[vSampleLen-1+TesterShift], vValidationDataL[vSampleLen-1+TesterShift]);
	}

	//-- Then, Call DLL to get Next Bar H,L
	while(retries<Max_Retries){
		printf("Calling MTgetForecast() with vBarId=%d", vBarId);
		int ret=MTgetForecast(
			vParamOverrideCnt, vParamOverride,
			vEnvS,
			vBarId,
			vSampleDataH, vSampleBaseValH,
			vSampleDataL, vSampleBaseValL,
			vSampleBW,
			vValidationDataH, vValidationBaseValH,
			vValidationDataL, vValidationBaseValL,
			vFutureDataH,
			vFutureDataL,
			vFutureBW,
			vPredictedDataH, vPredictedDataL
		);
		if(!IsTesting()) printf("RunForecast() CheckPoint 3: MT4_GetForecast() returned vForecast[0] (H|L) = %f|%f", vPredictedDataH[0], vPredictedDataL[0]);
		if (ret!=0){
			Alert("GetNextBars returned ", DoubleToString(ret,0));
			return -1;
		}
		//-- Make sure forecast bar makes sense... 
		if (vPredictedDataH[0]<vPredictedDataL[0]) {
			retries++;
			printf("Invalid Forecast: H=%5.4f , L=%5.4f . retries=%f", vPredictedDataH[0], vPredictedDataL[0], retries);
		} else {
			GlobalVariableSet("PrevFH0", vPredictedDataH[0]);
			GlobalVariableSet("PrevFL0", vPredictedDataL[0]);
			break;
		}
		vBarId++;
	}
	vBarId++;
	printf("end of while loop. retries=%d ; vBarId=%d", retries, vBarId);

	if (retries==Max_Retries) {
		return -2;
	} else {
		return 0;
	}
 }
void CloseOpenPos(string pComment){
	int PriceMod;
	for (int i=OrdersTotal(); i>0; i--){
			if(OrderSelect(i-1,SELECT_BY_POS)){
				if(OrderComment()==pComment){ 
					if(OrderType()==OP_SELL) {PriceMod=MODE_ASK;} else{PriceMod=MODE_BID;
				}
				RefreshRates();
				if ( !OrderClose(OrderTicket(),OrderLots(),MarketInfo(OrderSymbol(),PriceMod),20) ) printf("CloseOpenPos(%s) failed with error:%d",pComment,GetLastError());
			}
		}
	}
}
double GetLastPL(string pComment){
	string pCommentSL=pComment; StringAdd(pCommentSL,"[sl]");
	double PL=0;
	for (int i=OrdersSampleTotal(); i>0; i--){
		if(OrderSelect(i-1,SELECT_BY_POS, MODE_Sample)){
			if(OrderComment()==pComment || OrderComment()==pCommentSL){ 
				PL=OrderProfit();
				break;
			}
		} else {
			Print("OrderSelect() failed with error: ", GetLastError());
		}
	}
	return PL;
}
int NewTrade(int cmd, double price, double stoploss, double takeprofit){
	static double volume=TradeSizeDef;
	int TktId;
	double prevPL;
	printf("NewTrade() called with cmd=%s , volume=%f , price=%5.4f , stoploss=%5.4f , takeprofit=%5.4f",(cmd==OP_BUY?"BUY":"SELL"),volume,price,stoploss,takeprofit);

	// Use Comments string to avoid duplicates: 
	string vComment="MyEL"+"_"+Symbol();   //+"_"+IntegerToString(Period())+"_"+IntegerToString(TimeYear(Time[0]))+"-"+IntegerToString(TimeMonth(Time[0]))+"-"+IntegerToString(TimeDay(Time[0]))+"-"+IntegerToString(TimeHour(Time[0]))+"-"+IntegerToString(TimeMinute(Time[0]));   
   
	if(CloseOpenTrades) CloseOpenPos(vComment);       // Close Previous Position
	prevPL=GetLastPL(vComment);                       // Get last closed position P/L

	// Anti-Martingale: if last trade was at profit, double TradeSize, otherwise halve it.
	if(prevPL>0) volume=volume*2; if(volume>TradeSizeMax) volume=volume/2;
	if(prevPL<0) volume=volume/2; if(volume<TradeSizeMin) volume=volume*2;
   
	// Open Trade
	TktId=OrderSend(Symbol(), cmd, volume, price, Default_Slippage, stoploss, takeprofit, vComment, 15081970);
 
	if (TktId==-1) Alert("NewTrade() failed with error: ",GetLastError());

	vTradeSize = volume;	// vTradeSize is global non-static, and is needed by MTSaveTradeInfo()
	vTradeTP = takeprofit;
	vTradeSL = stoploss;

	return TktId;
}
void DrawForecast(double H, double L){
	const int ForecastMaxObjects=100;
	string ForecastObjName[1000];
	static int Fidx=0;
	double price1, price2;
	static double prevH=0;
	static double prevL=0;
	if(prevH==0) prevH=High[0];
	if(prevL==0) prevL=Low[0];
   
	//First, draw High
	price1=H;
	price2=H+0.005;
	Fidx++; if(Fidx>(ForecastMaxObjects-1)) Fidx=0;
	ForecastObjName[Fidx]="High"+IntegerToString(Fidx);
	ObjectDelete(ForecastObjName[Fidx]);   // don't care if this fails
	//ObjectCreate(ForecastObjName[Fidx], OBJ_TRIANGLE, 0, Time[0]-Period()*60, price1, Time[0], price2, Time[0]+Period()*60, price1);   
	ObjectCreate(ForecastObjName[Fidx], OBJ_TREND, 0, Time[0], prevH, Time[0]+Period()*60, H); 
	ObjectSetInteger(0, ForecastObjName[Fidx], OBJPROP_RAY_RIGHT, false);
	ObjectSetInteger(0, ForecastObjName[Fidx], OBJPROP_COLOR, clrBlue);
	prevH=H;
  
	//Then, draw Low
	price1=L;
	price2=L-0.005;
	Fidx++; if(Fidx>ForecastMaxObjects) Fidx=1;
	//printf("Fidx=%d",Fidx);
	ForecastObjName[Fidx]="Low"+IntegerToString(Fidx-1);
	ObjectDelete(ForecastObjName[Fidx]);   // don't care if this fails
	//ObjectCreate(ForecastObjName[Fidx], OBJ_TRIANGLE, 0, Time[0]-Period()*60, price1, Time[0], price2, Time[0]+Period()*60, price1);   
	ObjectCreate(ForecastObjName[Fidx], OBJ_TREND, 0, Time[0], prevL, Time[0]+Period()*60, L);
	ObjectSetInteger(0, ForecastObjName[Fidx], OBJPROP_RAY_RIGHT, false);
	ObjectSetInteger(0, ForecastObjName[Fidx], OBJPROP_COLOR, clrRed);
	prevL=L;
}
*/