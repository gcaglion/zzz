#pragma once

//-- FxData.Features
#define FXDATA_FEATURESCNT 14	//-- OHLCV, plus stats
#define FXOPEN 0
#define FXHIGH 1
#define FXLOW 2
#define FXCLOSE 3
#define FXVOLUME 4

#define FXMACD 5
#define FXCCI 6
#define FXATR 7
#define FXBOLLH 8
#define FXBOLLM 9
#define FXBOLLL 10
#define FXDEMA 11
#define FXMA 12
#define FXMOM 13

#define ATR_MAperiod 15
#define EMA_fastPeriod 5
#define EMA_slowPeriod 10
#define EMA_signalPeriod 5
#define CCI_MAperiod 15
#define BOLL_period 20
#define BOLL_shift 0
#define BOLL_deviation 2.0
#define DEMA_period 20
#define DEMA_shift 0
#define MA_period 10
#define MA_shift 0
#define MOM_period 16