#include "sMatrix.h"

void sMatrix::transpose() {
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
int sMatrix::transposeTo(sMatrix* otm) {
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
void sMatrix::fill(numtype start, numtype inc) {
	for (int i=0; i<(my*mx); i++) m[i]=start+i*inc;
}
int sMatrix::setDiag(int diag, numtype val) {
	// diag=0 -> [0,0] to [my,mx]
	// diag=1 -> [0,mx] to [my,0]
	if (my!=mx) return -1;
	int i=0;
	for (int y=0; y<my; y++) {
		for (int x=0; x<mx; x++) {
			if (diag==0) {
				m[i]=((y==x) ? 1.0f : 0.0f);
			} else {
				m[i]=((y==(mx-x-1)) ? 1.0f : 0.0f);
			}
			i++;
		}
	}
	return 0;
}
void sMatrix::scale(float s) {
	for (int i=0; i<(my*mx); i++) m[i]*=s;
}
void sMatrix::print(const char* msg, int smy0, int smx0, int smy, int smx) {
	if (smy==-1) smy=my;
	if (smx==-1) smx=mx;

	int idx;
	if (msg!=nullptr) printf("%s [%dx%d] - from [%d,%d] to [%d,%d]\n", msg, my, mx, (smy0==-1) ? 0 : smy0, (smx0==-1) ? 0 : smx0, smy0+smy, smx0+smx);
	for (int y=0; y<smy; y++) {
		for (int x=0; x<smx; x++) {
			idx= y*mx+x;
			printf("|%4.1f", m[idx]);
		}
		printf("|\n");
	}
}
int sMatrix::copyTo(sMatrix* tom) {
	if (tom->my!=my||tom->mx!=mx) {
		printf("copyTo() can only work with same-sized matrices!\n");
		return -1;
	}
	for (int i=0; i<(my*mx); i++) tom->m[i]=m[i];
	return 0;
}
int sMatrix::copySubTo(int y0, int x0, sMatrix* osm) {
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
int sMatrix::X(sMatrix* B, sMatrix* C, bool trA, bool trB, float Ascale, float Bscale) {
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
