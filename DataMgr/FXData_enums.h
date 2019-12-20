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

#define FXACOS 100 // Vector Trigonometric ACos
#define FXAD 101 // Chaikin A/D Line
#define FXADD 102 // Vector Arithmetic Add
#define FXADOSC 103 // Chaikin A/D Oscillator
#define FXADX 104 // Average Directional Movement Index
#define FXADXR 105 // Average Directional Movement Index Rating
#define FXAPO 106 // Absolute Price Oscillator
#define FXAROON 107 // Aroon
#define FXAROONOSC 108 // Aroon Oscillator
#define FXASIN 109 // Vector Trigonometric ASin
#define FXATAN 110 // Vector Trigonometric ATan
//#define FXATR 111 // Average True Range
#define FXAVGPRICE 112 // Average Price
#define FXBBANDS 113 // Bollinger Bands
#define FXBETA 114 // Beta
#define FXBOP 115 // Balance Of Power
//#define FXCCI 116 // Commodity Channel Index
#define FXCDL2CROWS 117 // Two Crows
#define FXCDL3BLACKCROWS 118 // Three Black Crows
#define FXCDL3INSIDE 119 // Three Inside Up/Down
#define FXCDL3LINESTRIKE 120 // Three-Line Strike
#define FXCDL3OUTSIDE 121 // Three Outside Up/Down
#define FXCDL3STARSINSOUTH 122 // Three Stars In The South
#define FXCDL3WHITESOLDIERS 123 // Three Advancing White Soldiers
#define FXCDLABANDONEDBABY 124 // Abandoned Baby
#define FXCDLADVANCEBLOCK 125 // Advance Block
#define FXCDLBELTHOLD 126 // Belt-hold
#define FXCDLBREAKAWAY 127 // Breakaway
#define FXCDLCLOSINGMARUBOZU 128 // Closing Marubozu
#define FXCDLCONCEALBABYSWALL 129 // Concealing Baby Swallow
#define FXCDLCOUNTERATTACK 130 // Counterattack
#define FXCDLDARKCLOUDCOVER 131 // Dark Cloud Cover
#define FXCDLDOJI 132 // Doji
#define FXCDLDOJISTAR 133 // Doji Star
#define FXCDLDRAGONFLYDOJI 134 // Dragonfly Doji
#define FXCDLENGULFING 135 // Engulfing Pattern
#define FXCDLEVENINGDOJISTAR 136 // Evening Doji Star
#define FXCDLEVENINGSTAR 137 // Evening Star
#define FXCDLGAPSIDESIDEWHITE 138 // Up/Down-gap side-by-side white lines
#define FXCDLGRAVESTONEDOJI 139 // Gravestone Doji
#define FXCDLHAMMER 140 // Hammer
#define FXCDLHANGINGMAN 141 // Hanging Man
#define FXCDLHARAMI 142 // Harami Pattern
#define FXCDLHARAMICROSS 143 // Harami Cross Pattern
#define FXCDLHIGHWAVE 144 // High-Wave Candle
#define FXCDLHIKKAKE 145 // Hikkake Pattern
#define FXCDLHIKKAKEMOD 146 // Modified Hikkake Pattern
#define FXCDLHOMINGPIGEON 147 // Homing Pigeon
#define FXCDLIDENTICAL3CROWS 148 // Identical Three Crows
#define FXCDLINNECK 149 // In-Neck Pattern
#define FXCDLINVERTEDHAMMER 150 // Inverted Hammer
#define FXCDLKICKING 151 // Kicking
#define FXCDLKICKINGBYLENGTH 152 // Kicking - bull/bear determined by the longer marubozu
#define FXCDLLADDERBOTTOM 153 // Ladder Bottom
#define FXCDLLONGLEGGEDDOJI 154 // Long Legged Doji
#define FXCDLLONGLINE 155 // Long Line Candle
#define FXCDLMARUBOZU 156 // Marubozu
#define FXCDLMATCHINGLOW 157 // Matching Low
#define FXCDLMATHOLD 158 // Mat Hold
#define FXCDLMORNINGDOJISTAR 159 // Morning Doji Star
#define FXCDLMORNINGSTAR 160 // Morning Star
#define FXCDLONNECK 161 // On-Neck Pattern
#define FXCDLPIERCING 162 // Piercing Pattern
#define FXCDLRICKSHAWMAN 163 // Rickshaw Man
#define FXCDLRISEFALL3METHODS 164 // Rising/Falling Three Methods
#define FXCDLSEPARATINGLINES 165 // Separating Lines
#define FXCDLSHOOTINGSTAR 166 // Shooting Star
#define FXCDLSHORTLINE 167 // Short Line Candle
#define FXCDLSPINNINGTOP 168 // Spinning Top
#define FXCDLSTALLEDPATTERN 169 // Stalled Pattern
#define FXCDLSTICKSANDWICH 170 // Stick Sandwich
#define FXCDLTAKURI 171 // Takuri (Dragonfly Doji with very long lower shadow)
#define FXCDLTASUKIGAP 172 // Tasuki Gap
#define FXCDLTHRUSTING 173 // Thrusting Pattern
#define FXCDLTRISTAR 174 // Tristar Pattern
#define FXCDLUNIQUE3RIVER 175 // Unique 3 River
#define FXCDLUPSIDEGAP2CROWS 176 // Upside Gap Two Crows
#define FXCDLXSIDEGAP3METHODS 177 // Upside/Downside Gap Three Methods
#define FXCEIL 178 // Vector Ceil
#define FXCMO 179 // Chande Momentum Oscillator
#define FXCORREL 180 // Pearson's Correlation Coefficient (r)
#define FXCOS 181 // Vector Trigonometric Cos
#define FXCOSH 182 // Vector Trigonometric Cosh
//#define FXDEMA 183 // Double Exponential Moving Average
#define FXDIV 184 // Vector Arithmetic Div
#define FXDX 185 // Directional Movement Index
#define FXEMA 186 // Exponential Moving Average
#define FXEXP 187 // Vector Arithmetic Exp
#define FXFLOOR 188 // Vector Floor
#define FXHT_DCPERIOD 189 // Hilbert Transform - Dominant Cycle Period
#define FXHT_DCPHASE 190 // Hilbert Transform - Dominant Cycle Phase
#define FXHT_PHASOR 191 // Hilbert Transform - Phasor Components
#define FXHT_SINE 192 // Hilbert Transform - SineWave
#define FXHT_TRENDLINE 193 // Hilbert Transform - Instantaneous Trendline
#define FXHT_TRENDMODE 194 // Hilbert Transform - Trend vs Cycle Mode
#define FXKAMA 195 // Kaufman Adaptive Moving Average
#define FXLINEARREG 196 // Linear Regression
#define FXLINEARREG_ANGLE 197 // Linear Regression Angle
#define FXLINEARREG_INTERCEPT 198 // Linear Regression Intercept
#define FXLINEARREG_SLOPE 199 // Linear Regression Slope
#define FXLN 200 // Vector Log Natural
#define FXLOG10 201 // Vector Log10
//#define FXMA 202 // Moving average
#define FXMACD 203 // Moving Average Convergence/Divergence
#define FXMACDEXT 204 // MACD with controllable MA type
#define FXMACDFIX 205 // Moving Average Convergence/Divergence Fix 12/26
#define FXMAMA 206 // MESA Adaptive Moving Average
#define FXMAVP 207 // Moving average with variable period
#define FXMAX 208 // Highest value over a specified period
#define FXMAXINDEX 209 // Index of highest value over a specified period
#define FXMEDPRICE 210 // Median Price
#define FXMFI 211 // Money Flow Index
#define FXMIDPOINT 212 // MidPoint over period
#define FXMIDPRICE 213 // Midpoint Price over period
#define FXMIN 214 // Lowest value over a specified period
#define FXMININDEX 215 // Index of lowest value over a specified period
#define FXMINMAX 216 // Lowest and highest values over a specified period
#define FXMINMAXINDEX 217 // Indexes of lowest and highest values over a specified period
#define FXMINUS_DI 218 // Minus Directional Indicator
#define FXMINUS_DM 219 // Minus Directional Movement
//#define FXMOM 220 // Momentum
#define FXMULT 221 // Vector Arithmetic Mult
#define FXNATR 222 // Normalized Average True Range
#define FXOBV 223 // On Balance Volume
#define FXPLUS_DI 224 // Plus Directional Indicator
#define FXPLUS_DM 225 // Plus Directional Movement
#define FXPPO 226 // Percentage Price Oscillator
#define FXROC 227 // Rate of change : ((price/prevPrice)-1)*100
#define FXROCP 228 // Rate of change Percentage: (price-prevPrice)/prevPrice
#define FXROCR 229 // Rate of change ratio: (price/prevPrice)
#define FXROCR100 230 // Rate of change ratio 100 scale: (price/prevPrice)*100
#define FXRSI 231 // Relative Strength Index
#define FXSAR 232 // Parabolic SAR
#define FXSAREXT 233 // Parabolic SAR - Extended
#define FXSIN 234 // Vector Trigonometric Sin
#define FXSINH 235 // Vector Trigonometric Sinh
#define FXSMA 236 // Simple Moving Average
#define FXSQRT 237 // Vector Square Root
#define FXSTDDEV 238 // Standard Deviation
#define FXSTOCH 239 // Stochastic
#define FXSTOCHF 240 // Stochastic Fast
#define FXSTOCHRSI 241 // Stochastic Relative Strength Index
#define FXSUB 242 // Vector Arithmetic Substraction
#define FXSUM 243 // Summation
#define FXT3 244 // Triple Exponential Moving Average (T3)
#define FXTAN 245 // Vector Trigonometric Tan
#define FXTANH 246 // Vector Trigonometric Tanh
#define FXTEMA 247 // Triple Exponential Moving Average
#define FXTRANGE 248 // True Range
#define FXTRIMA 249 // Triangular Moving Average
#define FXTRIX 250 // 1-day Rate-Of-Change (ROC) of a Triple Smooth EMA
#define FXTSF 251 // Time Series Forecast
#define FXTYPPRICE 252 // Typical Price
#define FXULTOSC 253 // Ultimate Oscillator
#define FXVAR 254 // Variance
#define FXWCLPRICE 255 // Weighted Close Price
#define FXWILLR 256 // Williams' %R
#define FXWMA 257 // Weighted Moving Average
