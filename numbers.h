#ifndef NUMBERS_H
#define NUMBERS_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: numbers.h,v 1.17 2000-11-16 16:51:05 Singular Exp $ */
/*
* ABSTRACT: interface to coefficient aritmetics
*/
#include "structs.h"

#define n_Copy(n, r)          nCopy(n)
//#define n_Delete(n, r)        (r)->cf->nDelete(n)
#define n_Delete(n, r)        nDelete(n)
#define n_Mult(n1, n2, r)     nMult(n1, n2)
#define n_Add(n1, n2, r)      nAdd(n1, n2)
#define n_IsZero(n, r)	      nIsZero(n)
#define n_Equal(n1, n2, r)    nEqual(n1, n2)
#define n_Neg(n, r)	          nNeg(n)
#define n_Sub(n1, n2, r)      nSub(n1, n2)
#define n_GetChar(r)          nGetChar()
#define n_Init(i, r)          nInit(i)
#define n_IsOne(n, r)         nIsOne(n)

/* variables */
extern short fftable[];

/* prototypes */
extern numberfunc nMult, nSub ,nAdd ,nDiv, nIntDiv, nIntMod, nExactDiv;
extern void    (*nNew)(number * a);
extern number  (*nInit)(int i);
extern number  (*nPar)(int i);
extern int     (*nParDeg)(number n);
extern int     (*nSize)(number n);
extern int     (*nInt)(number &n);
extern number  (*nNeg)(number a);
extern number  (*nInvers)(number a);
extern number  (*nCopy)(number a);
extern void    (*nWrite)(number &a);
extern char *  (*nRead)(char * s, number * a);
extern void    (*nNormalize)(number &a);
extern BOOLEAN (*nGreater)(number a,number b),
               (*nEqual)(number a,number b),
               (*nIsZero)(number a),
               (*nIsOne)(number a),
               (*nIsMOne)(number a),
               (*nGreaterZero)(number a);
extern void    (*nPower)(number a, int i, number * result);
extern number  (*nGetDenom)(number &n);
extern numberfunc nGcd, nLcm;

extern number nNULL; /* the 0 as constant */

#ifdef LDEBUG
extern BOOLEAN (*nDBTest)(number a, char *f, int l);
#define nTest(a) nDBTest(a,__FILE__,__LINE__)
extern void    (*nDBDelete)(number * a,char *f, int l);
#define nDelete(A) nDBDelete(A,__FILE__,__LINE__)
#else
#define nTest(a) (1)
extern void    (*nDelete)(number * a);
#endif

extern BOOLEAN (*nSetMap)(ring r);
extern number  (*nMap)(number from);
extern char *  (*nName)(number n);

void nDummy1(number* d);
void nDummy2(number &d);
number ndGcd(number a, number b);
number ndCopy(number a);

#ifdef LDEBUG
void nDBDummy1(number* d,char *f, int l);
#endif
int  nGetChar();
void nInitChar(ring r);
void nKillChar(ring r);
void nSetChar(ring r, BOOLEAN complete);

#endif
