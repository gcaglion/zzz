#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifdef USE_GPU
#include "../MyCU/MyCUparms.h"	//-- defines MAX_STREAMS
#else
#define MAX_STREAMS 1
#endif

//-- generic swap function
#ifdef __cplusplus
template <typename T> EXPORT void swap(T* v1, T* v2) {
	T tmp=(*v1);
	(*v1)=(*v2);
	(*v2)=tmp;
}
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

struct sMatrix {
	int my;
	int mx;
	numtype* m;

	sMatrix(int my_, int mx_, bool init_=false, numtype val0=0, numtype inc=0 ) {
		my=my_; mx=mx_;
		m=(numtype*)malloc(my*mx*sizeof(numtype));
		if(init_) { for (int i=0; i<(my*mx); i++) m[i]=val0+i*inc; }
		
		
	}
	~sMatrix() {
		free(m);
	}
	
	void transpose() {
		numtype** tm=(numtype**)malloc(mx*sizeof(numtype*)); for (int y=0; y<mx; y++) tm[y]=(numtype*)malloc(my*sizeof(numtype));
		for (int y = 0; y < my; y++) {
			for (int x = 0; x < mx; x++) {
				tm[x][y] = m[y*mx+x];
			}
		}
		for (int y = 0; y < my; y++) {
			for (int x = 0; x < mx; x++) {
				m[x*my+y]=tm[x][y];
			}
		}

		for (int y=0; y<mx; y++) free(tm[y]);
		free(tm);

		int tmp=my;	my=mx; mx=tmp;
	}
	int transposeTo(sMatrix* otm) {
		if (otm->mx!=my||otm->my!=mx) {
			printf("transposeTo() dimensions mismatch!\n");
			return -1;
		}
		for (int y = 0; y < my; y++) {
			for (int x = 0; x < mx; x++) {
				otm->m[x*my+y] = m[y*mx+x];
			}
		}
		return 0;
	}
	void fill(numtype start, numtype inc) {
		for (int i=0; i<(my*mx); i++) m[i]=start+i*inc;
	}
	int setDiag(int diag, numtype val) {
		// diag=0 -> [0,0] to [my,mx]
		// diag=1 -> [0,mx] to [my,0]
		if (my!=mx) return -1;
		int i=0;
		for (int y=0; y<my; y++) {
			for (int x=0; x<mx; x++) {
				if(diag==0){
					m[i]=((y==x)?1.0f:0.0f);
				} else {
					m[i]=((y==(mx-x-1))?1.0f:0.0f);
				}
				i++;
			}
		}
		return 0;
	}
	void scale(float s) {
		for (int i=0; i<(my*mx); i++) m[i]*=s;
	}
	void print(const char* msg=nullptr, int smy0=-1, int smx0=-1, int smy=-1, int smx=-1) {
		if (smy==-1) smy=my;
		if (smx==-1) smx=mx;

		int idx;
		if (msg!=nullptr) printf("%s [%dx%d] - from [%d,%d] to [%d,%d]\n", msg, my, mx, (smy0==-1)?0:smy0,(smx0==-1)?0:smx0,smy0+smy,smx0+smx);
		for (int y=0; y<smy; y++) {
			for (int x=0; x<smx; x++) {
				idx= y*mx+x;
				printf("|%4.1f", m[idx]);
			}
			printf("|\n");
		}
	}
	int copyTo(sMatrix* tom) {
		if(tom->my!=my || tom->mx!=mx) {
			printf("copyTo() can only work with same-sized matrices!\n");
			return -1;
		}
		for (int i=0; i<(my*mx); i++) tom->m[i]=m[i];
		return 0;
	}
	int copySubTo(int y0=0, int x0=0, sMatrix* osm=nullptr) {
		if (osm==nullptr) return -1;

		int idx;
		int odx=0;
		for (int y=y0; y<(y0+osm->my); y++) {
			for (int x=x0; x<(x0+osm->mx); x++) {
				idx= y*this->mx+x;
				osm->m[odx]=m[idx];
				odx++;
			}
		}

		return 0;
	}

	int X(sMatrix* B, sMatrix* C, bool trA, bool trB, float Ascale=1, float Bscale=1) {
		if (trA) swap(&mx, &my);
		if (trB) swap(&B->mx, &B->my);

		for (int y = 0; y < my; y++) {
			for (int x2 = 0; x2 < B->mx; x2++) {
				C->m[y*B->mx+x2] = 0;
				for (int x = 0; x < mx; x++) {
					C->m[y*B->mx+x2] += m[y*mx+x]*Ascale * B->m[x*B->mx+x2]*Bscale;
				}
			}
		}
		return 0;
	}
};
struct sAlgebra : public sObj {

	void* cublasH;
	void* cuRandH;
	void* cuStream[MAX_STREAMS];
	numtype* ss;	// shared scalar

					//-- class constructor/destructor
	EXPORT sAlgebra(sObjParmsDef);
	EXPORT ~sAlgebra();

	//-- class methods
	EXPORT void MbyM(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C, bool forceCPU=false);
	EXPORT void getMcol(int Ay, int Ax, numtype* A, int col, numtype* oCol, bool forceCPU);
	//-- CPU<->GPU transfer functions
	EXPORT void h2d(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);
	EXPORT void d2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);
	EXPORT void x2h(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);
	EXPORT void h2x(numtype* destAddr, numtype* srcAddr, int size, bool useStreams=false);

	bool myMalloc(numtype** var, int size);
	bool getMcol_cpu(int Ay, int Ax, numtype* A, int col, numtype* oCol);
	bool MbyM_std(int Ay, int Ax, numtype Ascale, bool Atr, numtype* A, int By, int Bx, numtype Bscale, bool Btr, numtype* B, numtype* C);
	bool Vinit(int size, numtype* v, numtype start, numtype inc);
	bool VbyV2V(int Vlen, numtype* V1, numtype* V2, numtype* oV);
	bool Vdiff(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);
	bool Vadd(int vlen, numtype* v1, numtype scale1, numtype* v2, numtype scale2, numtype* ov);

} ;

EXPORT void Mprint(int my, int mx, numtype* sm, const char* msg=nullptr, int smy0=-1, int smx0=-1, int smy=-1, int smx=-1);
EXPORT void Msub(int my, int mx, numtype* INm, numtype* OUTsm, int smy0, int smx0, int smy, int smx);
EXPORT int Vsum(int Vlen, int* V);

//-- TODO: CUDA VERSIONS !!!
EXPORT void Vscale(int Vlen, int* V, float s);
//--

//-- vector functions
EXPORT bool Vscale(int vlen, numtype* v, numtype s);
EXPORT bool Vcopy(int vlen, numtype* v1, numtype* v2);
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


