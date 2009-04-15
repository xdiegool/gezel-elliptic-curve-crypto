#include <time.h>
#include <string>

#include <gmp.h>

#include "pairings.h"

#include "element.h"
#include "extension.h"

int main () {
	// Start measuring time
	clock_t start = clock();
		
	// Initialize P & Q values
	mpz_t tmp;
	mpz_init(tmp);
	
	mpz_set_str(tmp, "0x18239FA96047C449DBF0D78A2A596E5A7D67D209F", 0);
	Element Xp = Element(tmp);
	
	mpz_set_str(tmp, "0x421655A1B0BA910F5A1AB8E0C46C3B45F9EE48FC2", 0);
	Element Yp = Element(tmp);
	
	mpz_set_str(tmp, "0x18B05983CBE4AD54B2EA7597DD441ABD8CFD5B3D3", 0);
	Element Xq = Element(tmp);
	
	mpz_set_str(tmp, "0x3D6CB2B7F2C7CDE41B82A65E92D105C2269D1CC75", 0);
	Element Yq = Element(tmp);
	
	
	/*mpz_set_str(tmp, "0x219c4dd67bd0153f2ef5657515304e5140ae767d6", 0);
	Element test = Element(tmp);
	Element test2 = Element(tmp);
	Element test3 = Element(tmp);
	
	printf("[F2m Inv]\n\tA =    ");
	test.print();
	test.inverse(test);
	printf("\tA^-1 = ");
	test.print();
	printf("\tA*A^-1 = ");
	test.multiply(test, test2);
	test.print();
	
	printf("\n\nGEZEL result:\n\tG =   ");
	mpz_set_str(tmp, "0x2ade1d7a4dbacde234f2a2672638e5dd699180dfe", 0);
	test2.set(tmp);
	test2.print();
		
	mpz_set_str(tmp, "0x318a4c6e93e748f6c83d3eb7bd34f0849af8ec503", 0);
	test3.set(tmp);
	printf("\tA =   ");
	test3.print();
	
	test.multiply(test2, test3);
	printf("\tA*G = ");
	test.print();
	
	exit(0);*/
	
	// Set modulus
	// Reduction polynomial: x^163 +  x^7  +  x^6  +  x^3  +   1
	// Ignore the x^163, numbers are only 163 bits long
	mpz_t mod;
	mpz_init(mod);
	mpz_set_str(mod, "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011001001", 2);

	// Length of modulus in base 2
	size_t mod_len = 163;
	
	// Calculate distortion map
	Element Xfa = Element(Xq);
	
	Element Yfa = Element();
	Element Yfb = Element(Xq);
	
	Yfa.add(Xq, Yq);
	
	// Init f & V
	Element One = Element((unsigned long) 1);
	Element Zero = Element();
	
	Extension F = Extension(One, Zero, Zero, Zero);
		
	Element Xv = Element(Xp);
	Element Yv = Element(Yp);
	
	// Create G
	Extension G = Extension();
	
	// Start Miller loop
	for (unsigned long i = (unsigned long) mod_len; i > 0; i--) {
		F.square(F);
		calcCoordDouble(G, Xv, Yv, Xfa, Yfa, Yfb);
		F.multiply(F, G);
		
		printf("[Miller]  i: %d\n   Xv: ", i);
		Xv.print();
		printf("   Yv: ");
		Yv.print();
		/*printf("   F: ");
		F.print();*/
		printf("\n");
		
		if (i == 83) {
			calcCoordAdd(G, Xv, Yv, Xp, Yp, Xfa, Yfa, Yfb);
			F.multiply(F, G);
		}
	}
	
	printf("F (before final exp):\n");
	F.print();
		
	// Calculate final exponentiation
	calcFinalExp(F);
	
	printf("F:\n");
	F.print();
		
	printf("\nTime elapsed: %.3fs", ((double)clock() - start) / CLOCKS_PER_SEC);

	exit(0);
}

void calcCoordDouble(Extension& G, Element& Xv, Element& Yv, Element& Xfa, Element& Yfa, Element& Yfb) {
	Element lambda = Element();
	
	lambda.square(Xv);
	lambda.addOne();
	
	// Calculate new X & Y coords
	Element Xnew = Element();
	Element Ynew = Element();
	
	Xnew.square(lambda);
	
	Ynew.add(Xnew, Xv);
	Ynew.multiply(Ynew, lambda);
	Ynew.add(Ynew, Yv);
	Ynew.addOne();
	
	// Calculate G
	G.getA().add(Xv, Xfa);
	G.getA().multiply(G.getA(), lambda);
	G.getA().add(G.getA(), Yv);
	G.getA().add(G.getA(), Yfa);
	
	G.getB().add(lambda, Yfb);
	
	G.getC().set(0);
	
	G.getD().set(1);
	
	// Set new coords
	Xv.set(Xnew);
	Yv.set(Ynew);	
}

void calcCoordAdd(Extension& G, Element& Xv, Element& Yv, Element& Xp, Element& Yp, Element& Xfa, Element& Yfa, Element& Yfb) {
	Element lambda = Element();
	
	printf("[Coord Add]\n");
	
	lambda.add(Xv, Xp);
	
	printf("Xv =      ");
	Xv.print();
	printf("Xp =      ");
	Xp.print();
	printf("Xv + Xp = ");
	lambda.print();
	printf("\n");
	
	lambda.inverse(lambda);
	
	if (lambda.isZero() == true) {
		lambda.set(1);
	}
	
	Element temp = Element();
	temp.add(Yv, Yp);
	
	printf("Yv =      ");
	Yv.print();
	printf("Yp =      ");
	Yp.print();
	printf("Yv + Yp = ");
	temp.print();
	printf("\n");
	
	lambda.add(lambda, temp);
	
	// Calculate new X & Y coords
	Element Xnew = Element();
	Element Ynew = Element();
	
	Xnew.square(lambda);
	Xnew.add(Xnew, Xv);
	Xnew.add(Xnew, Xp);
	
	Ynew.add(Xnew, Xp);
	Ynew.multiply(Ynew, lambda);
	Ynew.add(Ynew, Yp);
	Ynew.addOne();
	
	// Calculate G
	G.getA().add(Xp, Xfa);
	G.getA().multiply(G.getA(), lambda);
	G.getA().add(G.getA(), Yp);
	G.getA().add(G.getA(), Yfa);
	
	G.getB().add(lambda, Yfb);
	
	G.getC().set(0);
	
	G.getD().set(1);
	
	// Set new coords
	Xv.set(Xnew);
	Yv.set(Ynew);
}

void calcFinalExp(Extension& F) {
	// Calculate some temporary variables
	Element temp = Element();
	
	Element m0, m1, m2, m3;
	
	m0.square(F.getA());
	m1.square(F.getB());
	m2.square(F.getC());
	m3.square(F.getD());
	
	Element T0a, T0b, T1a, T1b, T2a, T2b, T3a, T3b;
	
	T0a.add(m0, m1);
	T0b.set(m1);
	
	T1a.add(m2, m3);
	T1b.set(m3);
	
	T2a.set(m3);
	T2b.set(m2);
	
	// Use T3a & T3b as temp here
	T3a.add(F.getA(), F.getB());
	temp.add(F.getC(), F.getD());
	// T3a = a0
	T3a.multiply(T3a, temp);
	// T3b = a1
	T3b.multiply(F.getA(), F.getC());
	// temp = a2
	temp.multiply(F.getB(), F.getD());
	
	T3a.add(T3a, temp);
	T3b.add(T3b, temp);
	
	Element T4a, T4b;
	
	T4b.add(m1, m3);
	T4a.add(T4b, m0);
	T4a.add(T4a, m2);
	
	Element T6a, T6b, T7a, T7b, T8a, T8b;
	
	T6a.set(m2);
	T6b.add(m2, m3);
	
	T7a.set(T4a);
	T7b.set(T4b);
	
	T8a.set(T0a);
	T8b.set(T0b);
	
	Element Da, Db;
	
	Da.add(T3a, T4a);
	Db.add(T3b, T4b);
	
	/*printf("\nD:\n\tDa = ");
	Da.print();
	
	printf("\tDb = ");
	Db.print();

	printf("\n");*/
		
	// Calculate inverse of D
	calcF2mInverse(Da, Db);
	
	/*printf("\nD^-1:\n\tDa = ");
	Da.print();
	
	printf("\tDb = ");
	Db.print();

	printf("\n");*/
	
	// Calculate V & W
	
	Element V0a, V0b, W0a, W0b;
	Element V1a, V1b, W1a, W1b;
	
	V0a.add(T6a, T8a);
	V0b.add(T6b, T8b);
	
	V1a.set(T6a);
	V1b.set(T6b);
	
	W0a.set(T7a);
	W0b.set(T7b);
	
	W1a.set(T6a);
	W1b.set(T6b);
	
	calcF2mMult(V0a, V0b, V0a, V0b, Da, Db);
	calcF2mMult(V1a, V1b, V1a, V1b, Da, Db);
	
	calcF2mMult(W0a, W0b, W0a, W0b, Da, Db);
	calcF2mMult(W1a, W1b, W1a, W1b, Da, Db);
		
	Extension V = Extension(V0a, V0b, V1a, V1b);
	Extension W = Extension(W0a, W0b, W1a, W1b);
		
	// Calculate V^(2^m + 1)
	calcF4mFinalPow(V, V);
	
	// Calculate W^(2^82)
	for (int i = 0; i < 82; i++) {
		W.square(W);
	}
	
	// Calculate F = V*W
	F.multiply(V, W);
}

void calcF4mFinalPow(Extension& R, Extension& E) {
	// Calculate some temporary variables
	Element m0, m1, m2, m3, m4;
	Element s0, s1;
	
	s0.add(E.getC(), E.getD());
	s1.add(E.getA(), E.getB());
	
	m0.multiply(s0, s1);
	m1.multiply(E.getC(), E.getD());
	m2.multiply(E.getA(), E.getD());
	m3.multiply(E.getA(), E.getB());
	m4.multiply(E.getB(), E.getC());
	
	s0.square(s0);
	s1.square(s1);
	
	// Calculate result
	R.getC().add(m1, s0);
	
	R.getB().add(R.getC(), m0);
	
	R.getA().add(R.getB(), s1);
	R.getA().add(R.getA(), m2);
	R.getA().add(R.getA(), m3);
	
	R.getB().add(R.getB(), m4);
	
	R.getD().add(m0, m2);
}

void calcF2mMult(Element& Ra, Element& Rb, Element& Aa, Element& Ab, Element& Ba, Element& Bb) {
	// Calculate some temporary variables
	Element a0, a1, a2;
	
	// Use a1 as temp var
	a0.add(Aa, Ab);
	a1.add(Ba, Bb);
	a0.multiply(a0, a1);
	
	a1.multiply(Aa, Ba);
	
	a2.multiply(Ab, Bb);
	
	// Calculate result
	Ra.add(a1, a2);
	Rb.add(a0, a1);
}

void calcF2mInverse(Element& Ea, Element& Eb) {
	// Calculate some temporary variables
	Element w, temp;
	
	temp.square(Ea);

	w.add(Ea, Eb);
	w.multiply(w, Eb);
	w.add(w, temp);
	
	printf("[w inverse]\n");
	
	w.inverse(w);
	
	// Calculate result
	Element Ra, Rb;
	
	Ea.add(Ea, Eb);
	Ea.multiply(Ea, w);
	
	Eb.multiply(Eb, w);	
}
