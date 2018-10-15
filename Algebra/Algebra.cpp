#include "Algebra.h"

#ifdef USE_GPU
#include "../MyCU/MyCU.h"
#endif


EXPORT void Mprint(int my, int mx, numtype* sm, const char* msg, int smy0, int smx0, int smy, int smx) {

	if (smy==-1) smy=my;
	if (smx==-1) smx=mx;

	int idx;
	if (msg!=nullptr) printf("%s [%dx%d] - from [%d,%d] to [%d,%d]\n", msg, my, mx, (smy0==-1) ? 0 : smy0, (smx0==-1) ? 0 : smx0, smy0+smy, smx0+smx);
	for (int y=0; y<smy; y++) {
		for (int x=0; x<smx; x++) {
			idx= y*mx+x;
			printf("|%2.5f", sm[idx]);
		}
		printf("|\n");
	}
}
EXPORT void Msub(int my, int mx, numtype* INm, numtype* OUTsm, int smy0, int smx0, int smy, int smx) {

	if (smy==0) smy=my;
	if (smx==0) smx=mx;

	int INidx=0; int OUTidx=0;
	for (int y=0; y<smy; y++) {
		for (int x=0; x<smx; x++) {
			INidx= y*mx+x;
			OUTsm[OUTidx]=INm[INidx];
			OUTidx++;
		}
	}

}
//-- int functions
EXPORT int Vsum(int Vlen, int* V) {
	int ret=0;
	for (int i=0; i<Vlen; i++) ret+=V[i];
	return ret;
}
EXPORT void Vscale(int Vlen, int* V, float s) {
	for (int i=0; i<Vlen; i++) V[i]=(int)(V[i]*s);
}

#ifdef USE_GPU
#else
#endif
//--

//-- vector functions
EXPORT bool Vscale(int vlen, numtype* v, numtype s) {
#ifdef USE_GPU
	return(Vscale_cu(vlen, v, s));
#else
	for (int i=0; i<vlen; i++) v[i]*=s;
	return true;
#endif
}
EXPORT bool Vcopy(int vlen, numtype* v1, numtype* v2) {
#ifdef USE_GPU
	return(Vcopy_cu(vlen, v1, v2));
#else
	for (int i=0; i<vlen; i++) v2[i]=v1[i];
	return true;
#endif
}
EXPORT bool Vadd(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov) {
#ifdef USE_GPU
	return (Vadd_cu(vlen, v1, scale1, v2, scale2, ov));
#else
	for (int i=0; i<vlen; i++) ov[i]=v2[i]*scale2+v1[i]*scale1;
	return true;
#endif
}
EXPORT bool Vdiff(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov) {
#ifdef USE_GPU
	return (Vdiff_cu(vlen, v1, scale1, v2, scale2, ov));
#else
	for (int i=0; i<vlen; i++) ov[i]=v1[i]*scale1-v2[i]*scale2;
	return true;
#endif
}
EXPORT bool Vssum(int vlen, numtype* v, numtype* ovssum) {
	//-- if using GPU, the sum scalar also resides in GPU
#ifdef USE_GPU
	return(Vssum_cu(vlen, v, ovssum));
#else
	(*ovssum)=0;
	for (int i=0; i<vlen; i++) (*ovssum)+=v[i]*v[i];
	return true;
#endif
}
EXPORT bool Vinit(int size, numtype* v, numtype start, numtype inc) {
#ifdef USE_GPU
	return(Vinit_cu(size, v, start, inc));
#else
	for (int i=0; i<size; i++) v[i]=start+i*inc;
	return true;
#endif
}
EXPORT bool VinitRnd(int Vlen, numtype* V, numtype rndmin, numtype rndmax, void* cuRandH) {
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
EXPORT bool VbyV2V(int Vlen, numtype* V1, numtype* V2, numtype* oV) {
#ifdef USE_GPU
	return(VbyV2V_cu(Vlen, V1, V2, oV));
#else
	for (int i = 0; i < Vlen; i++) oV[i] = V1[i]*V2[i];
	return true;
#endif
}

EXPORT bool Mtranspose(void* cublasH, int my, int mx, numtype* m, numtype* otm) {
#ifdef USE_GPU
	return(cuMtr_cublas(cublasH, my, mx, m, otm));
#else
	for (int y = 0; y < my; y++) {
		for (int x = 0; x < mx; x++) {
			otm[x*my+y] = m[y*mx+x];
		}
	}
	return true;
#endif
}
EXPORT bool MbyM_std(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C) {

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


//-- memory initializatin
EXPORT bool myMalloc(numtype** var, int size) {
#ifdef USE_GPU
	return (Malloc_cu(var, size));
#else
	(*var) = (numtype*)malloc(size*sizeof(numtype));
	return true;
#endif
}
EXPORT bool myFree(numtype* var) {
	#ifdef USE_GPU
		return (Free_cu(var));
	#else
		free(var);
		return true;
	#endif
}

//-- read/write mem<->file
EXPORT bool dumpArray(int vlen, numtype* v, const char* fname) {
#ifdef USE_GPU
	return(dumpArray_cu(vlen, v, fname));
#else
	FILE* f=fopen(fname, "w");
	if (f==nullptr) return false;
	for (int i=0; i<vlen; i++) fprintf(f, "%f\n", v[i]);
	fclose(f);
	return true;
#endif
}
EXPORT bool dumpArrayH(int vlen, numtype* v, const char* fname) {
	FILE* f=fopen(fname, "w");
	if (f==nullptr) return false;
	for (int i=0; i<vlen; i++) fprintf(f, "%f\n", v[i]);
	fclose(f);
	return true;
}
EXPORT bool loadArray(int vlen, numtype* v, const char* fname) {
#ifdef USE_GPU
	return(loadArray_cu(vlen, v, fname));
#else
	numtype fh;
	FILE* f=fopen(fname, "r");
	if (f==nullptr) return false;
	for (int i=0; i<vlen; i++) {
		if (fscanf(f, "%f\n", &fh)==0) return false;
		v[i]=fh;
	}
	fclose(f);
	return true;
#endif
}

EXPORT bool Tanh(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(Tanh_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)tanh(in[i]);
	return true;
#endif 
}
EXPORT bool dTanh(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return (dTanh_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(1-pow(tanh(in[i]),2));
	return true;
#endif 
}
EXPORT bool Exp4(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(Exp4_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(1/(1+exp(-4*in[i])));
	return true;
#endif
}
EXPORT bool dExp4(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(dExp4_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(4*exp(4*in[i])/(pow(exp(4*in[i])+1, 2)));
	return true;
#endif
}
EXPORT bool Relu(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(Relu_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(((in[i] > 0) ? 1 : 0));
	return true;
#endif 
}
EXPORT bool dRelu(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(dRelu_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(((in[i] > 0) ? in[i] : 0));
	return true;
#endif 
}
EXPORT bool SoftPlus(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(SoftPlus_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(log(1+exp(in[i])));
	return true;
#endif 
}
EXPORT bool dSoftPlus(int Vlen, numtype* in, numtype* out){
#ifdef USE_GPU 
	return(dSoftPlus_cu(Vlen, in, out));
#else 
	for (int i=0; i<Vlen; i++) out[i]=(numtype)(1/(1+exp(-in[i])));
	return true;
#endif 
}


EXPORT bool VVVcomp(int Vlen, numtype* V1, numtype* V2, numtype* oV, bool usegpu) {
#ifdef USE_GPU	
	if (usegpu) {
		if (VbyV2V_cu(Vlen, V1, V2, oV)!=0) return false;
	} else {
		for (int i = 0; i<Vlen; i++) oV[i] = V1[i]*V2[i];
	}
#endif
	return true;
}
EXPORT bool Vdiffcomp(int Vlen, numtype* V1, numtype scale1, numtype* V2, numtype scale2, numtype* oV, bool usegpu) {
#ifdef USE_GPU	
	if (usegpu) {
		if (Vdiff_cu(Vlen, V1, scale1, V2, scale2, oV)!=0) return false;
	} else {
		for (int i = 0; i<Vlen; i++) oV[i] = V1[i]*scale1-V2[i]*scale2;
	}
#endif
	return true;
}
EXPORT bool MbyMcomp(void* cublasH, int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C, numtype* T, bool usegpu) {
#ifdef USE_GPU	
	if (usegpu) {
		return MbyM_cu(cublasH, Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C);
	} else {
		return MbyM_std(Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C);
	}
#endif
	return true;
}

bool Vcompare(int vlen, numtype* v1, numtype* v2) {
	bool ret=0;
	numtype diff;
	for (int i=0; i<vlen; i++) {
		diff=(numtype)fabs(v1[i]-v2[i]);
		if (diff>1e-5) {
			printf("diff at [%d] = %f \n", i, diff);
			ret=false;
		}
	}
	return ret;
}

bool MbyMcompare(void* cublasH, int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, int Cy, int Cx, numtype* C, numtype* T) {
#ifdef USE_GPU
	DWORD start;
	int Tsize=(Ay+By)*(Ax+Bx);

	//-- malloc host
	numtype* Ah=(numtype*)malloc(Ay*Ax*sizeof(numtype));
	numtype* Bh=(numtype*)malloc(By*Bx*sizeof(numtype));
	numtype* Ch=(numtype*)malloc(Cy*Cx*sizeof(numtype));
	numtype* Th=(numtype*)malloc(Tsize*sizeof(numtype));
	numtype* Cr=(numtype*)malloc(Cy*Cx*sizeof(numtype));	//-- gets copy of the results from device 

	//-- copy dev->host
	start=timeGetTime();
	if (cudaMemcpy(Ah, A, Ay*Ax*sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) return false;
	if (cudaMemcpy(Bh, B, By*Bx*sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) return false;
	printf("copy dev->host; elapsed time=%ld\n", (DWORD)(timeGetTime()-start));

	//-- cpu run
	start=timeGetTime();
	if (MbyM_std(Ay, Ax, Ascale, Atr, Ah, By, Bx, Bscale, Btr, Bh, Ch)) return false;
	printf("CPU run; elapsed time=%ld \n", (DWORD)(timeGetTime()-start));
	//mprint(Ay, Ax, Ah, "Ah"); mprint(By, Bx, Bh, "Bh"); mprint(Cy, Cx, Ch, "Ch");

	//-- gpu run
	start=timeGetTime();
	if (MbyM_cu(cublasH, Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C)!=0) return false;
	printf("GPU run; elapsed time=%ld \n", (DWORD)(timeGetTime()-start));

	//-- copy results dev->host
	start=timeGetTime();
	if (cudaMemcpy(Cr, C, Cy*Cx*sizeof(numtype), cudaMemcpyDeviceToHost)!=cudaSuccess) {
		printf("CUDA error %d\n", cudaGetLastError());
		return false;
	}

	//-- compare results
	bool ret=Vcompare(Cy*Cx, Cr, Ch);

	//-- free host
	free(Ah); free(Bh); free(Ch); free(Th); free(Cr);

	return ret;
#else
	return false;
#endif
}


//-- class constructor/destructor
sAlgebra::sAlgebra(sObjParmsDef) : sObj(sObjParmsVal) {

	//-- init CUDA/BLAS
	cublasH=new void*;
	cuRandH=new void*;
	for (int i=0; i<MAX_STREAMS; i++) cuStream[i]=new void*;

#ifdef USE_GPU
	safecall(initCUDA());
	safecall(initCUDA());
	safecall(initCUBLAS(cublasH));
	safecall(initCURand(cuRandH));
	safecall(initCUstreams(cuStream));
#endif
	//-- init shared scalar
	safecall(this, myMalloc, &ss, 1);
}
sAlgebra::~sAlgebra() {
	myFree(ss);
	//.....
	// destroy cublasH, cuRandH, streams, curanddestroygenerator...
}
//-- class methods
bool sAlgebra::getMcol_cpu(int Ay, int Ax, numtype* A, int col, numtype* oCol) {
	for (int y=0; y<Ay; y++) oCol[y]=A[y*Ax+col];
	return true;
}
void sAlgebra::getMcol(int Ay, int Ax, numtype* A, int col, numtype* oCol, bool forceCPU) {
#ifdef USE_GPU
	if (forceCPU) {
		safecall(getMcol_cpu(Ay, Ax, A, col, oCol));
	} else {
		safecall(getMcol_cu(cublasH, Ay, Ax, A, col, oCol));
	}
#else
	this->getMcol_cpu(Ay, Ax, A, col, oCol);
#endif
}
void sAlgebra::MbyM(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C, bool forceCPU) {
#ifdef USE_GPU
	if(forceCPU) {
		safecall(MbyM_std(Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C));
	} else {
		safecall(MbyM_cu(cublasH, Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C));
	}
#else
	MbyM_std(Ay, Ax, Ascale, Atr, A, By, Bx, Bscale, Btr, B, C);
#endif
}
void sAlgebra::h2d(numtype* destAddr, numtype* srcAddr, int size, bool useStreams) {
#ifdef USE_GPU
	safecall(h2d_cu(destAddr, srcAddr, size, ((useStreams)?cuStream:nullptr)) );
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
void sAlgebra::d2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams) {
#ifdef USE_GPU
	safecall(d2h_cu(destAddr, srcAddr, size, ((useStreams) ? cuStream : nullptr)));
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
void sAlgebra::x2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams) {
#ifdef USE_GPU
	safecall(d2h_cu(destAddr, srcAddr, size, ((useStreams) ? cuStream : nullptr)));
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
void sAlgebra::h2x(numtype* destAddr, numtype* srcAddr, int size, bool useStreams) {
#ifdef USE_GPU
	safecall(h2d_cu(destAddr, srcAddr, size, ((useStreams) ? cuStream : nullptr)));
#else
	memcpy_s(destAddr, size, srcAddr, size);
#endif
}
bool sAlgebra::myMalloc(numtype** var, int size) {
#ifdef USE_GPU
	return (Malloc_cu(var, size));
#else
	(*var) = (numtype*)malloc(size*sizeof(numtype));
	return true;
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
bool sAlgebra::Vinit(int size, numtype* v, numtype start, numtype inc) {
#ifdef USE_GPU
	return(Vinit_cu(size, v, start, inc));
#else
	for (int i=0; i<size; i++) v[i]=start+i*inc;
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
bool sAlgebra::Vdiff(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov) {
#ifdef USE_GPU
	return (Vdiff_cu(vlen, v1, scale1, v2, scale2, ov));
#else
	for (int i=0; i<vlen; i++) ov[i]=v1[i]*scale1-v2[i]*scale2;
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
bool sAlgebra::myFree(numtype* var) {
#ifdef USE_GPU
	return (Free_cu(var));
#else
	free(var);
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
	return(Vssum_cu(vlen, v, ovssum));
#else
	(*ovssum)=0;
	for (int i=0; i<vlen; i++) (*ovssum)+=v[i]*v[i];
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
