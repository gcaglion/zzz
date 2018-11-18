#pragma once
#include "../common.h"

numtype TSMean(int VLen, numtype* V);
numtype TSMeanAbsoluteDeviation(int VLen, numtype* V);
numtype TSVariance(int VLen, numtype* V);
numtype TSSkewness(int VLen, numtype* V);
numtype TSKurtosis(int VLen, numtype* V);
numtype TSTurningPoints(int VLen, numtype* V);
numtype TSShannonEntropy(int VLen, numtype* V);
numtype TSHistoricalVolatility(int VLen, numtype* V);
