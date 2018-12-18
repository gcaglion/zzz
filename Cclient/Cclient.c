#include <Windows.h>
#include <stdio.h>

typedef float numtype;
typedef boolean bool;

extern int _createEnv(int accountId_, char* clientXMLFile_, int savedEnginePid_, bool useVolume_, int dt_, bool doDump_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_);
extern int _getForecast(char* iEnvS, int* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, int iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV, double* oForecastH, double* oForecastL);
extern int _destroyEnv(char* iEnvS);

int main(int argc, char* argv[]) {

	//========= THIS GOES IN .MQL4 ==========
	int ret;
	int accountId=100;
	int enginePid=3724;
	int sampleLen=50;
	int predictionLen;
	char envS[64]; sprintf_s(envS, 64, "1111111");
	char* clientXML="C:/Users/gcaglion/dev/zzz/Config/Client.xml";

	ret=_createEnv(accountId, clientXML, enginePid, TRUE, 1, TRUE, envS, &sampleLen, &predictionLen);

	int*    barT=(int*)malloc(sampleLen*sizeof(int));
	double* barO=(double*)malloc(sampleLen*sizeof(double));
	double* barH=(double*)malloc(sampleLen*sizeof(double));
	double* barL=(double*)malloc(sampleLen*sizeof(double));
	double* barC=(double*)malloc(sampleLen*sizeof(double));
	double* barV=(double*)malloc(sampleLen*sizeof(double));
	//--
	int baseBarT=0; double baseBarO=0, baseBarH=0, baseBarL=0, baseBarC=0, baseBarV=0;
	//--
	double* forecastH=(double*)malloc(predictionLen*sizeof(double));
	double* forecastL=(double*)malloc(predictionLen*sizeof(double));

	//-- ... set barO/H/L/C/V
	for (int i=0; i<sampleLen; i++) {
		barT[i]=999;
		barO[i]=1.3200;
		barH[i]=1.3400;
		barL[i]=1.3100;
		barC[i]=1.3300;
	}
	//-------------------------

	ret=_getForecast(envS, barT, barO, barH, barL, barC, barV, baseBarT, baseBarO, baseBarH, baseBarL, baseBarC, baseBarV, forecastH, forecastL);

	ret=_destroyEnv(envS);

	//========================================================================

	return 0;
}