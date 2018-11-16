#pragma once
#include "../common.h"

//-- generic swap function
template <typename T> EXPORT void swap(T* v1, T* v2) {
	T tmp=(*v1);
	(*v1)=(*v2);
	(*v2)=tmp;
}


struct sMatrix {
	int my;
	int mx;
	numtype* m;

	sMatrix(int my_, int mx_, bool init_=false, numtype val0=0, numtype inc=0) {
		my=my_; mx=mx_;
		m=(numtype*)malloc(my*mx*sizeof(numtype));
		if (init_) { for (int i=0; i<(my*mx); i++) m[i]=val0+i*inc; }


	}
	~sMatrix() {
		free(m);
	}

	void transpose();
	int transposeTo(sMatrix* otm);
	void fill(numtype start, numtype inc);
	int setDiag(int diag, numtype val);
	void scale(float s);
	void print(const char* msg=nullptr, int smy0=-1, int smx0=-1, int smy=-1, int smx=-1);
	int copyTo(sMatrix* tom);
	int copySubTo(int y0=0, int x0=0, sMatrix* osm=nullptr);
	int X(sMatrix* B, sMatrix* C, bool trA, bool trB, float Ascale=1, float Bscale=1);
};
