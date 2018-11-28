#pragma once

#include "../common.h"

#define CUDA_BLOCK_SIZE 64
const int MAX_STREAMS = 0;

//-- CUDA Exceptions
#define FAIL_INITCUDA "CUDA Initialization Failed. \n"
#define FAIL_INITCUBLAS "CUBLAS Initialization Failed. \n"
#define FAIL_INITCU "CUDA/CUBLAS Initialization Failed. \n"
#define FAIL_CUDAMALLOC "CUDA malloc failed. \n"

EXPORT void initCUDA();
EXPORT void initCUBLAS(void* cublasH);
EXPORT void initCURand(void* cuRandH);
EXPORT void initCUstreams(void* cuStream[]);

EXPORT void Malloc_cu(numtype** var, int size);
EXPORT void Free_cu(numtype* var);

//-- CPU<->GPU transfer functions
EXPORT void h2d_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]);
EXPORT void d2h_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]);

EXPORT bool loadBatchData_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]);
EXPORT bool MbyM_cu(void* cublasH, int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C);

//-- scalar functions
EXPORT bool Sadd_cu(numtype* s1, numtype* s2, numtype* ssum);

//-- vector functions;
EXPORT bool getMcol_cu(void* cublasH, int Ay, int Ax, numtype* A, int col, numtype* oCol);
EXPORT bool Vscale_cu(int vlen, numtype* v, numtype s);
EXPORT bool Vcopy_cu(int vlen, numtype* v1, numtype* v2);
EXPORT bool Vadd_cu(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);
EXPORT bool Vdiff_cu(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);
EXPORT bool Vsum_cu(int Vlen, numtype* V, numtype* oSum, numtype* ss_d);
EXPORT bool Vssum_cu(int Vlen, numtype* V, numtype* oVssum);
EXPORT bool Vnorm_cu(void* cublasH, int Vlen, numtype* V, numtype* oVnorm, numtype* ss_d);
EXPORT bool Vinit_cu(int vlen, numtype* v, numtype start, numtype inc);
EXPORT bool VbyV2V_cu(int vlen, numtype* v1, numtype* v2, numtype* ov);
EXPORT bool VinitRnd_cu(int vlen, numtype* v, numtype rndmin, numtype rndmax, void* cuRandH);
EXPORT bool VdotV_cu(int vlen, numtype* v1, numtype* v2, numtype* ovdotv);

//-- kernel functions wrappers
EXPORT void initGPUData(float *data, int numElements, float value);
EXPORT bool dumpArray_cu(int vlen, numtype* v, const char* fname);
EXPORT bool loadArray_cu(int vlen, numtype* v, const char* fname);

//-- matrix functions
EXPORT bool cuMtr_cublas(void* cublasH, int my, int mx, numtype* m, numtype* otm);

EXPORT bool Tanh_cu(int vlen, numtype* in, numtype* out);
EXPORT bool dTanh_cu(int vlen, numtype* in, numtype* out);
EXPORT bool Exp4_cu(int vlen, numtype* in, numtype* out);
EXPORT bool dExp4_cu(int vlen, numtype* in, numtype* out);
EXPORT bool Relu_cu(int vlen, numtype* in, numtype* out);
EXPORT bool dRelu_cu(int vlen, numtype* in, numtype* out);
EXPORT bool SoftPlus_cu(int vlen, numtype* in, numtype* out);
EXPORT bool dSoftPlus_cu(int vlen, numtype* in, numtype* out);

EXPORT bool cuMtr_naive(int my, int mx, numtype* m, numtype* omt);

#define CU_MSG_MAXLEN 2048

#define info(msgMask_, ...) dbg->out(DBG_MSG_INFO, __func__, depth, msgMask_, __VA_ARGS__)
#define err(msgMask_, ...) dbg->out(DBG_MSG_ERR, __func__, depth, msgMask_, __VA_ARGS__)
#define fail(msgMask_, ...) { dbg->out(DBG_MSG_ERR, __func__, depth, msgMask_, __VA_ARGS__); throw std::exception(dbg->msg);}


#define CUWfail(msgMask_, ...) { \
	char failmsg[CU_MSG_MAXLEN]; \
	sprintf_s(failmsg, CU_MSG_MAXLEN, msgMask_, __VA_ARGS__); throw std::exception(failmsg); \
}

#define CUWsafecall(func_, ...){ \
	try{ \
		func_(__VA_ARGS__); \
	} catch (std::exception exc) { \
		fail("%s FAILURE : %s . Exception: %s", name->base, cmd, exc.what()); \
	}\
}
#define CUWsafecallSilent(func_, ...){ \
	try{ \
		func_(__VA_ARGS__); \
	} catch (std::exception exc) { \
		fail("%s FAILURE : Exception: %s", name->base, exc.what()); \
	}\
}

