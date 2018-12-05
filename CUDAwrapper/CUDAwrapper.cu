#include "CUDAwrapper.h"
//--
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <curand.h>
#include <stdio.h>
//--

void swap(int* v1, int* v2) {
	int tmp=(*v1);
	(*v1)=(*v2);
	(*v2)=tmp;
}

static const char *cudaGetErrorEnum(cublasStatus_t error)
{
	switch (error)
	{
	case CUBLAS_STATUS_SUCCESS:
		return "CUBLAS_STATUS_SUCCESS";

	case CUBLAS_STATUS_NOT_INITIALIZED:
		return "CUBLAS_STATUS_NOT_INITIALIZED";

	case CUBLAS_STATUS_ALLOC_FAILED:
		return "CUBLAS_STATUS_ALLOC_FAILED";

	case CUBLAS_STATUS_INVALID_VALUE:
		return "CUBLAS_STATUS_INVALID_VALUE";

	case CUBLAS_STATUS_ARCH_MISMATCH:
		return "CUBLAS_STATUS_ARCH_MISMATCH";

	case CUBLAS_STATUS_MAPPING_ERROR:
		return "CUBLAS_STATUS_MAPPING_ERROR";

	case CUBLAS_STATUS_EXECUTION_FAILED:
		return "CUBLAS_STATUS_EXECUTION_FAILED";

	case CUBLAS_STATUS_INTERNAL_ERROR:
		return "CUBLAS_STATUS_INTERNAL_ERROR";
	}

	return "<unknown>";
}

EXPORT void initCUDA() {
	// init CUDA GPU
	int ret=cudaSetDevice(0);
	if (ret!=cudaSuccess) CUWfail("cudaSetDevice failed!  Error %d . Do you have a CUDA-capable GPU installed?\n", ret);
}
EXPORT void initCUBLAS(void* cublasH) {

	if (cublasCreate((cublasHandle_t*)cublasH)!=CUBLAS_STATUS_SUCCESS) CUWfail("CUBLAS initialization error!\n");

}
EXPORT void initCURand(void* cuRandH) {
	if (curandCreateGenerator((curandGenerator_t*)cuRandH, CURAND_RNG_PSEUDO_DEFAULT)!=CURAND_STATUS_SUCCESS) {
		//if (curandCreateGenerator((curandGenerator_t*)cuRandH, CURAND_RNG_PSEUDO_DEFAULT)!=CURAND_STATUS_SUCCESS) {
		CUWfail("CURAND initialization error!\n");
	}
	/* Set seed */
	if (curandSetPseudoRandomGeneratorSeed((*(curandGenerator_t*)cuRandH), timeGetTime())!=CURAND_STATUS_SUCCESS) {
		CUWfail("CURAND initialization error!\n");
	}
}
EXPORT void initCUstreams(void* cuStream[]) {
	for (int s=0; s<MAX_STREAMS; s++) {
		if (cudaStreamCreate((cudaStream_t*)cuStream[s])!=cudaSuccess) CUWfail("CU stream %d creation failed.", s);
	}
}

EXPORT void Malloc_cu(numtype** var, int size) {
	if (cudaMalloc(var, size*sizeof(numtype))!=cudaSuccess) CUWfail("F41LUR3!-1111");
}
EXPORT void Free_cu(numtype* var) {
	if (cudaFree(var)!=cudaSuccess) CUWfail("F41LUR3!-2222");
}

//-- CPU<->GPU transfer functions
/*EXPORT void h2d_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]) {
	if(MAX_STREAMS==0) {
		if (cudaMemcpy(destAddr, srcAddr, size, cudaMemcpyHostToDevice)!=cudaSuccess) CUWfail("F41LUR3!-333")
	} else {
		int streamSize=size/sizeof(numtype)/MAX_STREAMS;
		size_t streamBytes=streamSize*sizeof(numtype);
		for (int s=0; s<MAX_STREAMS; s++) {
			int offset=s*streamSize;
			if (cudaMemcpyAsync(&destAddr[offset], &srcAddr[offset], streamBytes, cudaMemcpyHostToDevice, (*(cudaStream_t*)cuStream[s]))!=cudaSuccess) {
				CUWfail("s=%d ; CUDA error %d\n", s, cudaGetLastError());
			}
		}
	}
}
EXPORT void d2h_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]) {
	if(MAX_STREAMS==0) {
		if(cudaMemcpy(destAddr, srcAddr, size, cudaMemcpyDeviceToHost)!=cudaSuccess) CUWfail("F41LUR3!-444")
	} else {
		int streamSize=size/sizeof(numtype)/MAX_STREAMS;
		size_t streamBytes=streamSize*sizeof(numtype);
		for (int s=0; s<MAX_STREAMS; s++) {
			int offset=s*streamSize;
			if (cudaMemcpyAsync(&destAddr[offset], &srcAddr[offset], streamBytes, cudaMemcpyDeviceToHost, (*(cudaStream_t*)cuStream[s]))!=cudaSuccess) {
				CUWfail("s=%d ; CUDA error %d\n", s, cudaGetLastError());
			}
		}
	}
}
*/
EXPORT void h2d_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]) {
	if (cuStream==nullptr) {
		if (!(cudaMemcpy(destAddr, srcAddr, size, cudaMemcpyHostToDevice)==cudaSuccess)) CUWfail("CUDA error %d", cudaGetLastError());
	} else {
		int streamSize=size/sizeof(numtype)/MAX_STREAMS;
		size_t streamBytes=streamSize*sizeof(numtype);
		for (int s=0; s<MAX_STREAMS; s++) {
			int offset=s*streamSize;
			if (cudaMemcpyAsync(&destAddr[offset], &srcAddr[offset], streamBytes, cudaMemcpyHostToDevice, (*(cudaStream_t*)cuStream[s]))!=cudaSuccess) {
				CUWfail("s=%d ; CUDA error %d\n", s, cudaGetLastError());
			}
		}
	}
}
EXPORT void d2h_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]) {
	if (cuStream==nullptr) {
		if (!(cudaMemcpy(destAddr, srcAddr, size, cudaMemcpyDeviceToHost)==cudaSuccess))  CUWfail("CUDA error %d", cudaGetLastError());
	} else {
		int streamSize=size/sizeof(numtype)/MAX_STREAMS;
		size_t streamBytes=streamSize*sizeof(numtype);
		for (int s=0; s<MAX_STREAMS; s++) {
			int offset=s*streamSize;
			if (cudaMemcpyAsync(&destAddr[offset], &srcAddr[offset], streamBytes, cudaMemcpyDeviceToHost, (*(cudaStream_t*)cuStream[s]))!=cudaSuccess) {
				CUWfail("s=%d ; CUDA error %d\n", s, cudaGetLastError());
			}
		}
	}
}

//==================================
__global__	void initGPUData_ker(float *data, int numElements, float value) {
	int tid = blockIdx.x * blockDim.x+threadIdx.x;
	if (tid < numElements) {
		data[tid] = value;
	}
}
EXPORT		void initGPUData(float *data, int numElements, float value) {
	dim3 gridDim;
	dim3 blockDim;

	blockDim.x = 1024;
	gridDim.x = (numElements+blockDim.x-1)/blockDim.x;

	initGPUData_ker<<< gridDim, blockDim>>> (data, numElements, value);
}

EXPORT bool loadBatchData_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]) {
	int streamSize=size/sizeof(numtype)/MAX_STREAMS;
	size_t streamBytes=streamSize*sizeof(numtype);
	for (int s=0; s<MAX_STREAMS; s++) {
		int offset=s*streamSize;
		if (cudaMemcpyAsync(&destAddr[offset], &srcAddr[offset], streamBytes, cudaMemcpyHostToDevice, (*(cudaStream_t*)cuStream[s]))!=cudaSuccess) {
			printf("s=%d ; CUDA error %d\n", s, cudaGetLastError());
			return false;
		}
	}
	return true;
}
EXPORT bool dumpArray_cu(int vlen, numtype* v, const char* fname) {
	numtype* hw=(numtype*)malloc(vlen*sizeof(numtype));
	if (cudaMemcpy(hw, v, vlen*sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) return false;
	FILE* f=fopen(fname, "w");
	if (f==nullptr) return false;
	for (int i=0; i<vlen; i++) fprintf(f, "%f\n", hw[i]);
	free(hw);
	fclose(f);
	return true;
}
EXPORT bool loadArray_cu(int vlen, numtype* v, const char* fname){
	numtype fh;
	numtype* vh=(numtype*)malloc(vlen*sizeof(numtype));
	FILE* f=fopen(fname, "r");
	if (f==nullptr) return false;
	for (int i=0; i<vlen; i++) {
		if(fscanf(f, "%f\n", &fh)==0) return false;
		vh[i]=fh;
	}
	if (cudaMemcpy(v, vh, vlen*sizeof(numtype), cudaMemcpyHostToDevice)!=cudaSuccess) return false;
	fclose(f);
	free(vh);
	return true;
}

//-- matrix functions
EXPORT bool cuMtr_cublas(void* cublasH, int my, int mx, numtype* m, numtype* otm) {
	float alpha=1;
	float beta=0;
	if (cublasSgeam((*(cublasHandle_t*)cublasH), CUBLAS_OP_T, CUBLAS_OP_T, my, mx, &alpha, m, mx, &beta, m, mx, otm, my)!=CUBLAS_STATUS_SUCCESS) return false;
	return true;
}

EXPORT bool MbyM_cu(void* cublasH, int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C) {

	float *alpha = &Ascale;
	float *beta = &Bscale;

	cublasOperation_t Aop=CUBLAS_OP_N;
	cublasOperation_t Bop=CUBLAS_OP_N;
	int m=Bx;
	int n=Ay;
	int k=Ax;
	int ldA=Ax;
	int ldB=Bx;
	int ldC=Bx;

	numtype* vA = A;
	numtype* vB = B;

	if (Atr) {
		Aop=CUBLAS_OP_T;
		n=Ax; k=Ay;
	}
	if (Btr) {
		Bop=CUBLAS_OP_T;
		m=By;
		ldC=By;
	}

	if (!Vinit_cu(m*n, C, 0, 0)) return false;
	if (cublasSgemm((*(cublasHandle_t*)cublasH), Bop, Aop, m, n, k, alpha, vB, ldB, vA, ldA, beta, C, ldC)!=CUBLAS_STATUS_SUCCESS) throw(new std::exception("call to cublasSgem()"));

	return true;
}

__global__ void VdotV_ker(int n, float x[], float y[], float* dot_p) {
	float tmp;
	int i = blockDim.x * blockIdx.x+threadIdx.x;

	if (i < n) {
		tmp = x[i]*y[i];
		atomicAdd(dot_p, tmp);
	}
}
__global__ void cuSadd(const numtype* s1, const numtype* s2, numtype* ssum) {
	ssum[0]=s1[0]+s2[0];
}
__global__ void cuVscale_ker(const int vlen, numtype *v, const numtype s) {
	int tid = blockIdx.x * blockDim.x+threadIdx.x;
	if (tid < vlen) v[tid] *= s;
}
__global__ void cuVcopy_ker(const int vlen, const numtype *v1, numtype *v2) {
	int tid = blockIdx.x * blockDim.x+threadIdx.x;
	if (tid < vlen) v2[tid] = v1[tid];
}
__global__ void cuVminusV_ker(const int vlen, const numtype *a, const numtype sa, const numtype *b, const numtype sb, numtype* c) {
	int tid = blockIdx.x * blockDim.x+threadIdx.x;
	if (tid < vlen) c[tid] = a[tid]*sa-b[tid]*sb;
}
__global__ void cuVplusV_ker(const int vlen, const numtype *a, const numtype sa, const numtype *b, const numtype sb, numtype* c) {
	int tid = blockIdx.x * blockDim.x+threadIdx.x;
	if (tid < vlen) c[tid] = a[tid]*sa+b[tid]*sb;
}
__global__ void cuVsum_ker(const int vlen, const numtype *v, numtype* osum) {

	//@@ Load a segment of the input vector into shared memory
	__shared__ float partialSum[2*CUDA_BLOCK_SIZE];
	unsigned int t = threadIdx.x, start = 2*blockIdx.x * CUDA_BLOCK_SIZE;
	if (start+t < vlen)
		partialSum[t] = v[start+t];
	else
		partialSum[t] = 0;
	if (start+CUDA_BLOCK_SIZE+t < vlen)
		partialSum[CUDA_BLOCK_SIZE+t] = v[start+CUDA_BLOCK_SIZE+t];
	else
		partialSum[CUDA_BLOCK_SIZE+t] = 0;
	//@@ Traverse the reduction tree
	for (unsigned int stride = CUDA_BLOCK_SIZE; stride>=1; stride >>= 1) {
		__syncthreads();
		if (t < stride)
			partialSum[t] += partialSum[t+stride];
	}
	//@@ Write the computed sum of the block to the output vector at the 
	//@@ correct index
	if (t==0)
		osum[blockIdx.x] = partialSum[0];

}
__global__ void cuVssum_ker(const int vlen, const numtype *v, numtype* ossum) {

	//@@ Load a segment of the input vector into shared memory
	__shared__ float partialSum[2*CUDA_BLOCK_SIZE];
	unsigned int t = threadIdx.x, start = 2*blockIdx.x * CUDA_BLOCK_SIZE;
	if (start+t < vlen)
		partialSum[t] = v[start+t]*v[start+t];
	else
		partialSum[t] = 0;
	if (start+CUDA_BLOCK_SIZE+t < vlen)
		partialSum[CUDA_BLOCK_SIZE+t] = v[start+CUDA_BLOCK_SIZE+t]*v[start+CUDA_BLOCK_SIZE+t];
	else
		partialSum[CUDA_BLOCK_SIZE+t] = 0;
	//@@ Traverse the reduction tree
	for (unsigned int stride = CUDA_BLOCK_SIZE; stride>=1; stride >>= 1) {
		__syncthreads();
		if (t < stride)
			partialSum[t] += partialSum[t+stride];
	}
	//@@ Write the computed sum of the block to the output vector at the 
	//@@ correct index
	if (t==0)
		ossum[blockIdx.x] = partialSum[0];

}
__global__ void Vscale(int vlen, numtype* v, numtype scaleM, numtype scaleP) {
	int i = blockIdx.x*blockDim.x+threadIdx.x;
	if (i<vlen) v[i] = scaleM*v[i]+scaleP;
}
__global__ void Vinit_ker(int vlen, numtype* v, numtype start, numtype inc) {
	int i = blockIdx.x*blockDim.x+threadIdx.x;
	if (i<vlen) v[i] = start+i*inc;
}
__global__ void VbyV2V_ker(int vlen, numtype* v1, numtype* v2, numtype* ov) {
	int i = blockIdx.x*blockDim.x+threadIdx.x;
	if (i<vlen) ov[i]=v1[i]*v2[i];
}

//-- scalar functions
EXPORT bool Sadd_cu(numtype* s1, numtype* s2, numtype* ssum) {
	cuSadd<<< 1, 1>>>(s1, s2, ssum);
	return ((cudaGetLastError()==cudaSuccess));
}

//-- vector functions;
EXPORT bool getMcol_cu(void* cublasH, int Ay, int Ax, numtype* A, int col, numtype* oCol) {
	cublasStatus_t err=cublasScopy((*((cublasHandle_t*)cublasH)), Ax, A, Ax, oCol, 1);
	if (err!=CUBLAS_STATUS_SUCCESS) {
		printf("getMcol_cu() CUBLAS error %d: %s\n", err, cudaGetErrorEnum(err));
		return false;
	}
	return true;
}
EXPORT bool Vscale_cu(int vlen, numtype* v, numtype s){
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuVscale_ker<<< gridDim, blockDim>>> (vlen, v, s);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Vcopy_cu(int vlen, numtype* v1, numtype* v2) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuVcopy_ker<<< gridDim, blockDim>>> (vlen, v1, v2);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Vadd_cu(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuVplusV_ker<<< gridDim, blockDim>>> (vlen, v1, scale1, v2, scale2, ov);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Vdiff_cu(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuVminusV_ker<<< gridDim, blockDim>>> (vlen, v1, scale1, v2, scale2, ov);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Vsum_cu(int vlen, numtype* v, numtype* ovsum, numtype* ss_d) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuVsum_ker<<< gridDim, blockDim>>> (vlen, v, ss_d );

	if (cudaMemcpy(ovsum, ss_d, sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) return false;

	return ((cudaGetLastError()==cudaSuccess));
}

EXPORT void VdotV_cu(int n, float x_d[], float y_d[], float* dot_d, int blocks, int threads) {

	cudaMemset(dot_d, 0, sizeof(float));

	/* Invoke kernel */
	VdotV_ker<<<blocks, threads>>>(n, x_d, y_d, dot_d);

	//cudaMemcpy(oVdotVh, dot_d, sizeof(float), cudaMemcpyDeviceToHost);

}

/*EXPORT bool VdotV_cu(int vlen, numtype* v1, numtype* v2, numtype* ovdotv) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	VdotV_ker<<< gridDim, blockDim>>> (vlen, v1, v2, ovdotv);

	return ((cudaGetLastError()==cudaSuccess));
}
*/
EXPORT bool Vssum_cu(int vlen, numtype* v, numtype* ovssum) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuVssum_ker<<< gridDim, blockDim>>> (vlen, v, ovssum);

	return ((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Vssum_cu_cublas(void* cublasH, int Vlen, numtype* V, numtype* oVssum, numtype* ss_d) {
	if (cublasSnrm2((*(cublasHandle_t*)cublasH), Vlen, V, 1, oVssum)!=CUBLAS_STATUS_SUCCESS) return false;
	(*oVssum)=(*oVssum)*(*oVssum);
	return true;
}

EXPORT bool Vnorm_cu(void* cublasH, int Vlen, numtype* V,  numtype* oVnorm, numtype* ss_d) {
	if (cublasSnrm2_v2((*(cublasHandle_t*)cublasH), Vlen, V, 1, oVnorm)!=CUBLAS_STATUS_SUCCESS) return false;
	//if (cudaMemcpy(oVnorm, ss_d, sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) return false;
	return true;
}
EXPORT bool Vinit_cu(int vlen, numtype* v, numtype start, numtype inc) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	Vinit_ker<<< gridDim, blockDim>>> (vlen, v, start, inc);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool VbyV2V_cu(int vlen, numtype* v1, numtype* v2, numtype* ov) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	VbyV2V_ker<<< gridDim, blockDim>>> (vlen, v1, v2, ov);

	return((cudaGetLastError()==cudaSuccess));
}

EXPORT bool VinitRnd_cu(int vlen, numtype* v, numtype rndmin, numtype rndmax, void* cuRandH) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	//-- Generate n floats on device, with  values between 0.0 and 1.0, where 0.0 is excluded and 1.0 is included
	if(curandGenerateUniform((*(curandGenerator_t*)cuRandH), v, vlen) !=CURAND_STATUS_SUCCESS) return false;
	//-- need to scale to rndmin<->rndmax
	Vscale<<< gridDim, blockDim>>>(vlen, v, (rndmax-rndmin), rndmax-(rndmax-rndmin)*1);

	/*/-- !!!!!!!!!!!!! REMOVE !!!!!!!!!!
	numtype* hw=(numtype*)malloc(vlen*sizeof(numtype));
	if (cudaMemcpy(hw, v, vlen*sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) return false;
	char* fname = "C:/temp/rndw.txt";
	FILE* f=fopen(fname, "w");
	for (int i=0; i<vlen; i++) fprintf(f, "%f\n", hw[i]);
	free(hw);
	fclose(f);
	//--
	*/
	return((cudaGetLastError()==cudaSuccess));
}

__global__ void cuTanh_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = tanhf(in[i]);
}
__global__ void cudTanh_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = 1-tanhf(in[i])*tanhf(in[i]);
}
__global__ void ORIG_cuTanh_ker(int vlen, numtype* in, numtype* out) {
	int i = blockIdx.x*blockDim.x+threadIdx.x;
	if (i<vlen) out[i] = tanhf(in[i]);
}
__global__ void ORIG_cudTanh_ker(int vlen, numtype* in, numtype* out) {
	int i = blockIdx.x*blockDim.x+threadIdx.x;
	if (i<vlen) out[i] = 1-tanhf(in[i])*tanhf(in[i]);
}
__global__ void cuExp4_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = 1/(1+exp(-4*in[i]));
}
__global__ void cudExp4_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = 4*exp(4*in[i])/(pow(exp(4*in[i])+1, 2));
}
__global__ void cuRelu_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = ((in[i] > 0) ? 1 : 0);
}
__global__ void cudRelu_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = ((in[i] > 0) ? in[i] : 0);
}
__global__ void cuSoftPlus_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = log(1+exp(in[i]));
}
__global__ void cudSoftPlus_ker(int vlen, numtype* in, numtype* out) {
	int i = threadIdx.x+blockIdx.x * blockDim.x;
	out[i] = 1/(1+exp(-in[i]));
}

EXPORT bool Tanh_cu(int vlen, numtype* in, numtype* out) {
	/*	int blockSize=64; // The launch configurator returned block size
	int minGridSize; // The minimum grid size needed to achieve the // maximum occupancy for a full device
	int gridSize; // The actual grid size needed, based on input // size
	cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, (void*)cudTanh_ker, 0, vlen);
	// Round up according to array size
	gridSize = (vlen+blockSize-1)/blockSize;
	cudTanh_ker<<< gridSize, blockSize>>> (vlen, in, out);
	*/
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuTanh_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool dTanh_cu(int vlen, numtype* in, numtype* out) {
/*	int blockSize=64; // The launch configurator returned block size
	int minGridSize; // The minimum grid size needed to achieve the // maximum occupancy for a full device 
	int gridSize; // The actual grid size needed, based on input // size 
	cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, (void*)cudTanh_ker, 0, vlen);
	// Round up according to array size 
	gridSize = (vlen+blockSize-1)/blockSize;
	cudTanh_ker<<< gridSize, blockSize>>> (vlen, in, out);
*/
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cudTanh_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Exp4_cu(int vlen, numtype* in, numtype* out) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;
	cuExp4_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool dExp4_cu(int vlen, numtype* in, numtype* out) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cudExp4_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool Relu_cu(int vlen, numtype* in, numtype* out) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuRelu_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool dRelu_cu(int vlen, numtype* in, numtype* out) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cudRelu_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool SoftPlus_cu(int vlen, numtype* in, numtype* out) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cuSoftPlus_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
EXPORT bool dSoftPlus_cu(int vlen, numtype* in, numtype* out) {
	dim3 gridDim;
	dim3 blockDim;
	blockDim.x = CUDA_BLOCK_SIZE;
	gridDim.x = (vlen+blockDim.x-1)/blockDim.x;

	cudSoftPlus_ker<<< gridDim, blockDim>>> (vlen, in, out);

	return((cudaGetLastError()==cudaSuccess));
}
