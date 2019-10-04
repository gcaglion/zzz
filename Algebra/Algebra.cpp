#include "Algebra.h"

//-- class constructor/destructor
sAlgebra::sAlgebra(sObjParmsDef) : sObj(sObjParmsVal) {

	//-- init CUDA/BLAS
	cublasH=new void*;
	cuRandH=new void*;	
	for (int i=0; i<MAX_STREAMS; i++) cuStream[i]=new void*;
#ifdef USE_GPU
	CUWsafecall(initCUDA);
	CUWsafecall(initCUBLAS, cublasH);
	CUWsafecall(initCURand, cuRandH);
	CUWsafecall(initCUstreams, cuStream);
#endif
}
sAlgebra::~sAlgebra() {
	//.....
	// destroy cublasH, cuRandH, streams, curanddestroygenerator...
	delete cublasH;
	delete cuRandH;
}
//-- class methods

//-- multi-threading
void sAlgebra::createGPUContext() {
#ifdef USE_GPU
	createGPUcontext_cu();
#endif
}
void sAlgebra::destroyGPUContext() {
#ifdef USE_GPU
	destroyGPUcontext_cu();
#endif
}
void sAlgebra::syncGPUContext() {
#ifdef USE_GPU
	syncGPUcontext_cu();
#endif
}

//-- device-level sync
void sAlgebra::devSync() {
#ifdef USE_GPU
	devSync_cu();
#endif
}

//-- memory initializatin
void sAlgebra::myMalloc(numtype** var, int size) {
#ifdef USE_GPU
	CUWsafecallSilent(Malloc_cu, var, size);
#else
	(*var) = (numtype*)malloc(size*sizeof(numtype));
#endif
}
EXPORT void sAlgebra::myFree(numtype* var) {
#ifdef USE_GPU
	CUWsafecallSilent(Free_cu, var);
#else
	free(var);
#endif
}


bool sAlgebra::getMcol_cpu(int Ay, int Ax, numtype* A, int col, numtype* oCol) {
	for (int y=0; y<Ay; y++) oCol[y]=A[y*Ax+col];
	return true;
}
void sAlgebra::getMcol(int Ay, int Ax, numtype* A, int col, numtype* oCol, bool forceCPU) {
#ifdef USE_GPU
	if (forceCPU) {
		CUWsafecallSilent(getMcol_cpu, Ay, Ax, A, col, oCol);
	} else {
		CUWsafecallSilent(getMcol_cu, cublasH, Ay, Ax, A, col, oCol);
	}
#else
	this->getMcol_cpu(Ay, Ax, A, col, oCol);
#endif
}
void sAlgebra::MbyM(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C, bool forceCPU) {
#ifdef USE_GPU
	if(forceCPU) {
		CUWsafecallSilent(MbyM_std, Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C);
	} else {
		CUWsafecallSilent(MbyM_cu, cublasH, Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C);
	}
#else
	MbyM_std(Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C);
#endif
}
void sAlgebra::h2d(numtype* destAddr, numtype* srcAddr, int size, bool useStreams) {
#ifdef USE_GPU
	CUWsafecallSilent(h2d_cu, destAddr, srcAddr, size, ((useStreams&&MAX_STREAMS>1) ? cuStream : nullptr));
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
void sAlgebra::d2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams) {
#ifdef USE_GPU
	CUWsafecallSilent(d2h_cu, destAddr, srcAddr, size, ((useStreams&&MAX_STREAMS>1) ? cuStream : nullptr));
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
void sAlgebra::d2d(numtype* destAddr, numtype* srcAddr, int size) {
#ifdef USE_GPU
	CUWsafecallSilent(d2d_cu, destAddr, srcAddr, size);
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}

bool sAlgebra::MbyM_std(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C) {

	int m1y=Ay, m1x=Ax, m1i; numtype* m1=A;
	int m2y=By, m2x=Bx, m2i; numtype* m2=B;
	if (Atr) {
		m1y=Ax; m1x=Ay;
	}
	if (Btr) {
		m2y=Bx; m2x=By;
	}
	int mmi; numtype* mm=C;

	for (int y = 0; y < m1y; y++) {
		for (int x2 = 0; x2<m2x; x2++) {
			mmi=y*m2x+x2;
			mm[mmi]=0;
			for (int x = 0; x<m1x; x++) {
				m1i=(Atr) ? (x*m1y+y) : (y*m1x+x);
				m2i=(Btr) ? (x2*m2y+x) : (x*m2x+x2);
				mm[mmi]+=m1[m1i]*m2[m2i];
				//printf("C[%d] += A[%d] * B[%d] => %f * %f = %f\n", mmi, m1i, m2i, m1[m1i], m2[m2i], mm[mmi]);
			}
		}
	}
	//printf("\n");
	return true;
}
bool sAlgebra::Vinit(int Vlen, numtype* v, numtype start, numtype inc) {
#ifdef USE_GPU
	return(Vinit_cu(Vlen, v, start, inc));
#else
	for (int i=0; i<Vlen; i++) v[i]=start+i*inc;
	return true;
#endif
}
bool sAlgebra::VbyV2V(int Vlen, numtype* V1, numtype* V2, numtype* oV) {
#ifdef USE_GPU
	return(VbyV2V_cu(Vlen, V1, V2, oV));
#else
	for (int i = 0; i < Vlen; i++) oV[i] = V1[i]*V2[i];
	return true;
#endif
}
bool sAlgebra::Vadd(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov) {
#ifdef USE_GPU
	return (Vadd_cu(vlen, v1, scale1, v2, scale2, ov));
#else
	for (int i=0; i<vlen; i++) ov[i]=v2[i]*scale2+v1[i]*scale1;
	return true;
#endif
}
bool sAlgebra::Vcopy(int vlen, numtype* v1, numtype* v2) {
#ifdef USE_GPU
	return(Vcopy_cu(vlen, v1, v2));
#else
	for (int i=0; i<vlen; i++) v2[i]=v1[i];
	return true;
#endif
}
bool sAlgebra::Vscale(int vlen, numtype* v1, numtype scale, numtype* ov) {
#ifdef USE_GPU
	if (!Vcopy_cu(vlen, v1, ov)) return false;
	return(Vscale_cu(vlen, ov, scale));
#else
	for (int i=0; i<vlen; i++) ov[i]=v1[i]*scale;
	return true;
#endif
}

void sAlgebra::VdotV(int vlen, numtype* v1, numtype* v2, numtype* ohvdotv) {
	(*ohvdotv)=0;
#ifdef USE_GPU
	if (!VdotV_cu(cublasH, vlen, v1, v2, ohvdotv)) fail("salkaz!");
#else
	for (int i = 0; i < vlen; i++) (*ohvdotv) += v1[i]*v2[i];
#endif
}

//-- Activation Functions
bool sAlgebra::Tanh(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(Tanh_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)tanh(in[i]);
	return true;
#endif 
}
bool sAlgebra::dTanh(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return (dTanh_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(1-pow(tanh(in[i]), 2));
	return true;
#endif 
}
bool sAlgebra::Tanh2(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(Tanh2_cu(Vlen, in, out));
#else 
	fail("Not implemented!")
		return false;
#endif 
}
bool sAlgebra::dTanh2(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return (dTanh2_cu(Vlen, in, out));
#else 
	fail("Not implemented!")
	return false;
#endif 
}
bool sAlgebra::Exp4(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(Exp4_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(1/(1+exp(-4*in[i])));
	return true;
#endif
}
bool sAlgebra::dExp4(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(dExp4_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(4*exp(4*in[i])/(pow(exp(4*in[i])+1, 2)));
	return true;
#endif
}
bool sAlgebra::Relu(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(Relu_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(((in[i] > 0) ? 1 : 0));
	return true;
#endif 
}
bool sAlgebra::dRelu(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(dRelu_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(((in[i] > 0) ? in[i] : 0));
	return true;
#endif 
}
bool sAlgebra::SoftPlus(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(SoftPlus_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(log(1+exp(in[i])));
	return true;
#endif 
}
bool sAlgebra::dSoftPlus(int Vlen, numtype* in, numtype* out) {
#ifdef USE_GPU 
	return(dSoftPlus_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(1/(1+exp(-in[i])));
	return true;
#endif 
}

//-- Vector functions
bool sAlgebra::Vssum(int vlen, numtype* v, numtype* ovssum) {
	//-- if using GPU, the sum scalar also resides in GPU
#ifdef USE_GPU
	return(Vssum_cu(cublasH, vlen, v, ovssum));
#else
	(*ovssum)=0;
	for (int i=0; i<vlen; i++) (*ovssum)+=v[i]*v[i];
	return true;
#endif
}
bool sAlgebra::Vnorm(int vlen, numtype* v, numtype* ovnorm) {
#ifdef USE_GPU
	return(Vnorm_cu(cublasH, vlen, v, ovnorm));
#else
	numtype vssum=0;
	for (int i=0; i<vlen; i++) vssum+=v[i]*v[i];
	(*ovnorm)=sqrt(vssum);
	return true;
#endif
}
bool sAlgebra::VinitRnd(int Vlen, numtype* V, numtype rndmin, numtype rndmax, void* cuRandH) {
#ifdef USE_GPU
	return(VinitRnd_cu(Vlen, V, rndmin, rndmax, cuRandH));
#else
	time_t t;
	srand((unsigned)time(&t));

	//-- Print 5 random numbers from 0 to 49
	for (int i = 0; i < Vlen; i++) {
		V[i] = rndmin+(numtype)rand()/((numtype)RAND_MAX+1) * (rndmax-rndmin);
		//printf("rand[%d]=%f\n", i, V[i]);
	}

	unsigned int number=1234;
	int err;
	for (int i=0; i<Vlen; i++) {
		err = rand_s(&number);
		V[i] = rndmin+(numtype)number/((numtype)UINT_MAX+1) * (rndmax-rndmin);
	}

	return true;
#endif
}

//-- gateway calls to CUDA wrapper
EXPORT void CUWinitCUDA() {
#ifdef USE_GPU
	initCUDA();
#endif
}
EXPORT void CUWinitCUBLAS(void* cublasH) {
#ifdef USE_GPU
	initCUBLAS(cublasH);
#endif
}
EXPORT void CUWinitCURand(void* cuRandH) {
#ifdef USE_GPU
	initCURand(cuRandH);
#endif
}
EXPORT void CUWinitCUstreams(void* cuStream[]) {
#ifdef USE_GPU
	initCUstreams(cuStream);
#endif
}
//-- CPU<->GPU transfer functions
EXPORT void CUWh2d_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]){
#ifdef USE_GPU
	h2d_cu(destAddr, srcAddr, size, cuStream);
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
EXPORT void CUWd2h_cu(numtype* destAddr, numtype* srcAddr, int size, void* cuStream[]){
#ifdef USE_GPU
	d2h_cu(destAddr, srcAddr, size, cuStream);
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}


//-- read/write mem<->file
EXPORT bool dumpArray(int vlen, numtype* v, const char* fname) {
#ifdef USE_GPU
	return(dumpArray_cu(vlen, v, fname));
#else
	return(dumpArrayH(vlen, v, fname));
#endif
}
EXPORT bool loadArray(int vlen, numtype* v, const char* fname) {
#ifdef USE_GPU
	return(loadArray_cu(vlen, v, fname));
#else
	FILE* f=fopen(fname, "r");
	if (f==nullptr) return false;
	for (int i=0; i<vlen; i++) fscanf_s(f, "%f", &v[i]);
	fclose(f);
	return true;
#endif
}