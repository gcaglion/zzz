#include <Windows.h>
#include <stdio.h>
typedef float numtype;

extern int _createEnv(int accountId_, char* clientXMLFile_, int savedEnginePid_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_);
extern int _getForecast(char* iEnvS, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, double* oForecastH, double* oForecastL);
extern int _destroyEnv(char* iEnvS);

int main(int argc, char* argv[]) {

	//========= THIS GOES IN .MQL4 ==========
	int ret;
	int accountId=100;
	int enginePid=3724;
	int sampleLen;
	int predictionLen;
	char envS[64]; sprintf_s(envS, 64, "1111111");
	char* clientXML="C:/Users/gcaglion/dev/zzz/Config/Client.xml";

	ret=_createEnv(accountId, clientXML, enginePid, envS, &sampleLen, &predictionLen);

	double* barO=(double*)malloc(sampleLen*sizeof(double));
	double* barH=(double*)malloc(sampleLen*sizeof(double));
	double* barL=(double*)malloc(sampleLen*sizeof(double));
	double* barC=(double*)malloc(sampleLen*sizeof(double));
	double* barV=(double*)malloc(sampleLen*sizeof(double));
	//--
	double* forecastH=(double*)malloc(predictionLen*sizeof(double));
	double* forecastL=(double*)malloc(predictionLen*sizeof(double));

	//-- ... set barO/H/L/C/V

	ret=_getForecast(envS, barO, barH, barL, barC, barV, forecastH, forecastL);

	ret=_destroyEnv(envS);

	//========================================================================

	return 0;
}