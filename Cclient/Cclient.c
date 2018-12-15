#include <Windows.h>
#include <stdio.h>
typedef float numtype;

extern int _createEnv(int accountId_, void** oEnv, int* oSampleLen_, int* oPredictionLen_);
extern int _getForecast(void* iEnv, numtype* iBarO, numtype* iBarH, numtype* iBarL, numtype* iBarC, numtype* iBarV, numtype* oForecastH, numtype* oForecastL);
extern int _destroyEnv(void* iEnv);

int main(int argc, char* argv[]) {

	//========= THIS GOES IN .MQL4 ==========
	int ret;
	int accountId=100;
	int sampleLen;
	int predictionLen;
	void* env=NULL;

	ret=_createEnv(accountId, &env, &sampleLen, &predictionLen);

	numtype* barO=(numtype*)malloc(sampleLen*sizeof(numtype));
	numtype* barH=(numtype*)malloc(sampleLen*sizeof(numtype));
	numtype* barL=(numtype*)malloc(sampleLen*sizeof(numtype));
	numtype* barC=(numtype*)malloc(sampleLen*sizeof(numtype));
	numtype* barV=(numtype*)malloc(sampleLen*sizeof(numtype));
	//--
	numtype* forecastH=(numtype*)malloc(predictionLen*sizeof(numtype));
	numtype* forecastL=(numtype*)malloc(predictionLen*sizeof(numtype));

	//-- ... set barO/H/L/C/V

	ret=_getForecast(env, barO, barH, barL, barC, barV, forecastH, forecastL);

	ret=_destroyEnv(env);

	//========================================================================

	return 0;
}