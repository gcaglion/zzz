#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifdef USE_GPU
#include "../CUDAwrapper/CUDAwrapper.h"	//-- defines MAX_STREAMS
#else
#define MAX_STREAMS 0
#endif

//-- Exceptions
#define FAIL_MALLOC_N "Neurons memory allocation failed. \n"
#define FAIL_MALLOC_W "Weights memory allocation failed. \n"
#define FAIL_FREE_N "Neurons memory free failed. \n"
#define FAIL_FREE_S "Scalar memory free failed. \n"
#define FAIL_FREE_W "Weights memory free failed. \n"
#define FAIL_MALLOC_e "Errors memory allocation failed. \n"
#define FAIL_MALLOC_u "Targets memory allocation failed. \n"
#define FAIL_MALLOC_SCALAR "Scalars memory allocation failed. \n"

struct sAlgebra : public sObj {

	void* cublasH;
	void* cuRandH;
	void* cuStream[1+MAX_STREAMS];
	numtype* ss;	// shared scalar

					//-- class constructor/destructor
	EXPORT sAlgebra(sObjParmsDef);
	EXPORT ~sAlgebra();

	//-- class methods

	//-- memory initializatin
	EXPORT void myMalloc(numtype** var, int size);
	EXPORT void myFree(numtype* var);

	EXPORT void MbyM(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C, bool forceCPU=false);
	EXPORT void getMcol(int Ay, int Ax, numtype* A, int col, numtype* oCol, bool forceCPU);
	//-- CPU<->GPU transfer functions
	EXPORT void h2d(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);
	EXPORT void d2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);
	EXPORT void x2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);
	EXPORT void h2x(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);

	EXPORT bool getMcol_cpu(int Ay, int Ax, numtype* A, int col, numtype* oCol);
	EXPORT bool MbyM_std(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C);
	EXPORT bool Vinit(int Vlen, numtype* v, numtype start, numtype inc);
	EXPORT bool VbyV2V(int Vlen, numtype* V1, numtype* V2, numtype* oV);
	EXPORT bool Vadd(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);
	EXPORT bool Vcopy(int vlen, numtype* v1, numtype* v2);
	EXPORT bool Vscale(int vlen, numtype* v1, numtype scale, numtype* ov);

	EXPORT void VdotV(int vlen, numtype* v1, numtype* v2, numtype* ohvdotv);

	//-- Activation Functions
	EXPORT bool Tanh(int Vlen, numtype* in, numtype* out);
	EXPORT bool dTanh(int Vlen, numtype* in, numtype* out);
	EXPORT bool Exp4(int Vlen, numtype* in, numtype* out);
	EXPORT bool dExp4(int Vlen, numtype* in, numtype* out);
	EXPORT bool Relu(int Vlen, numtype* in, numtype* out);
	EXPORT bool dRelu(int Vlen, numtype* in, numtype* out);
	EXPORT bool SoftPlus(int Vlen, numtype* in, numtype* out);
	EXPORT bool dSoftPlus(int Vlen, numtype* in, numtype* out);

	//-- Vector functions
	EXPORT bool Vssum(int vlen, numtype* v, numtype* ovssum);
	EXPORT bool VinitRnd(int Vlen, numtype* V, numtype rndmin, numtype rndmax, void* cuRandH);
	EXPORT bool Vnorm(int vlen, numtype* v, numtype* ovssum);
} ;

//-- stand-alone functions
EXPORT bool dumpArray(int vlen, numtype* v, const char* fname);
EXPORT bool dumpArrayH(int vlen, numtype* v, const char* fname);
EXPORT bool loadArray(int vlen, numtype* v, const char* fname);

/*
EXPORT void Mprint(int my, int mx, numtype* sm, const char* msg=nullptr, int smy0=-1, int smx0=-1, int smy=-1, int smx=-1);
EXPORT void Msub(int my, int mx, numtype* INm, numtype* OUTsm, int smy0, int smx0, int smy, int smx);
EXPORT int Vsum(int Vlen, int* V);

//-- TODO: CUDA VERSIONS !!!
EXPORT void Vscale(int Vlen, int* V, float s);
//--

//-- vector functions
EXPORT bool Vscale(int vlen, numtype* v, numtype s);
EXPORT bool Vadd(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);
EXPORT bool Vdiff(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);
EXPORT bool Vssum(int vlen, numtype* v, numtype* ovssum);
EXPORT bool Vinit(int size, numtype* v, numtype start, numtype inc);
EXPORT bool VinitRnd(int Vlen, numtype* V, numtype rndmin, numtype rndmax, void* cuRandH=NULL);
EXPORT bool VbyV2V(int Vlen, numtype* V1, numtype* V2, numtype* oV);

//-- TODO: CUDA version!
EXPORT void MbyV(int my, int mx, numtype* m, bool Transpose, numtype* v, numtype* ov);

//-- matrix functions
EXPORT bool Mtranspose(void* cublasH, int my, int mx, numtype* m, numtype* otm);

//EXPORT bool myMalloc(numtype** var, int size);
EXPORT bool myFree(numtype* var);

EXPORT bool dumpArray(int vlen, numtype* v, const char* fname);
EXPORT bool dumpArrayH(int vlen, numtype* v, const char* fname);
EXPORT bool loadArray(int vlen, numtype* v, const char* fname);

EXPORT bool Tanh(int Vlen, numtype* in, numtype* out);
EXPORT bool dTanh(int Vlen, numtype* in, numtype* out);
EXPORT bool Exp4(int Vlen, numtype* in, numtype* out);
EXPORT bool dExp4(int Vlen, numtype* in, numtype* out);
EXPORT bool Relu(int Vlen, numtype* in, numtype* out);
EXPORT bool dRelu(int Vlen, numtype* in, numtype* out);
EXPORT bool SoftPlus(int Vlen, numtype* in, numtype* out);
EXPORT bool dSoftPlus(int Vlen, numtype* in, numtype* out);

EXPORT bool VVVcomp(int Vlen, numtype* V1, numtype* V2, numtype* oV, bool usegpu);
EXPORT bool Vdiffcomp(int Vlen, numtype* V1, numtype scale1, numtype* V2, numtype scale2, numtype* oV, bool usegpu);
EXPORT bool MbyMcomp(void* cublasH, int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C, numtype* T, bool usegpu);
EXPORT bool MbyMcompare(void* cublasH, int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, int Cy, int Cx, numtype* C, numtype* T);
*/

