#ifndef POLYS_IMPL_H
#define POLYS_IMPL_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: polys-impl.h,v 1.32 1999-09-27 15:05:29 obachman Exp $ */

/***************************************************************
 *
 * File:       polys-impl.h
 * Purpose:    low-level definition and declarations for polys
 *
 * If you touch anything here, you better know what you are doing.
 * What is here should not be used directly from other routines -- the
 * encapsulations in polys.h should be used, instead.
 *
 ***************************************************************/
#include "structs.h"
#include "mmemory.h"
#include "mmheap.h"

/***************************************************************
 *
 * definition of the poly structure and its fields
 *
 ***************************************************************/

#ifndef NDEBUG
#define VARS (10)   /*max. number of variables as constant*/
#else
#define VARS (0)
#endif
union s_exp
{
   Exponent_t e[VARS +1];
   long       l[(VARS +1)/2];
};

//typedef Exponent_t  monomial[VARS + 1];
typedef s_exp  monomial;
typedef Exponent_t* Exponent_pt;

typedef long Order_t;
struct  spolyrec
{
  poly      next; // next needs to be the first field
  number    coef; // and coef the second --- do not change this !!!
  monomial  exp; // make sure that exp is aligned
};

/***************************************************************
 *
 * variables/defines used for managment of monomials
 *
 ***************************************************************/

#define POLYSIZE (sizeof(poly) + sizeof(number))
#define POLYSIZEW (POLYSIZE / sizeof(long))
#define MAX_EXPONENT_NUMBER ((MAX_BLOCK_SIZE - POLYSIZE) / SIZEOF_EXPONENT)

// number of Variables
extern int pVariables;
// size of a monom in bytes - always a multiple of sizeof(void*)
extern int pMonomSize;
// size of a monom in units of sizeof(void*) -- i.e. in words
extern int pMonomSizeW;
// Ceiling((pVariables+1) / sizeof(void*)) == length of exp-vector in words
// extern int pVariables1W;
// Ceiling((pVariables) / sizeof(void*))
// extern int pVariablesW;
extern int *pVarOffset;
// extern int pVarLowIndex;
// extern int pVarHighIndex;
// extern int pVarCompIndex;

/***************************************************************
 *
 * Primitives for determening/setting  the way exponents are arranged
 *
 ***************************************************************/
#define _pExpIndex(i) (currRing->VarOffset[(i)])
#define _pRingExpIndex(r, i)  (r)->VarOffset[(i)]

#define _pCompIndex        (currRing->pCompIndex)
#define _pRingCompIndex(r)  ((r)->pCompIndex)

/***************************************************************
 *
 * Primitives for accessing and setting fields of a poly
 *
 ***************************************************************/
#define _pNext(p)           ((p)->next)
#define _pIter(p)           ((p) = (p)->next)

#define _pGetCoeff(p)       ((p)->coef)
#define _pSetCoeff(p,n)     {nDelete(&((p)->coef));(p)->coef=n;}
#define _pSetCoeff0(p,n)    (p)->coef=n

#define _pGetOrder(p)       ((p)->exp.l[currRing->pOrdIndex])

#if defined(PDEBUG) && PDEBUG != 0
extern Exponent_t pPDSetExp(poly p, int v, Exponent_t e, char* f, int l);
extern Exponent_t pPDGetExp(poly p, int v, char* f, int l);
extern Exponent_t pPDIncrExp(poly p, int v, char* f, int l);
extern Exponent_t pPDDecrExp(poly p, int v, char* f, int l);
extern Exponent_t pPDAddExp(poly p, int v, Exponent_t e, char* f, int l);
extern Exponent_t pPDMultExp(poly p, int v, Exponent_t e, char* f, int l);
extern Exponent_t pPDSubExp(poly p, int v, Exponent_t e, char* f, int l);

extern Exponent_t pPDRingSetExp(ring r,poly p,int v,Exponent_t e,char* f,int l);
extern Exponent_t pPDRingGetExp(ring r,poly p, int v, char* f, int l);

extern Exponent_t pDBSetComp(poly p, Exponent_t k, int l, char* f, int l);
extern Exponent_t pDBDecrComp(poly p, char* f, int l);
extern Exponent_t pDBAddComp(poly p, Exponent_t k, int l, char* f, int l);
extern Exponent_t pDBSubComp(poly p, Exponent_t k, char* f, int l);
extern Exponent_t pDBRingSetComp(ring r, poly p, Exponent_t k, char* f, int l);


#define _pSetExp(p,v,e)     pPDSetExp(p,v,e,__FILE__,__LINE__)
#define _pGetExp(p,v)       pPDGetExp(p,v,__FILE__,__LINE__)
#define _pIncrExp(p,v)      pPDIncrExp(p,v,__FILE__,__LINE__)
#define _pDecrExp(p,v)      pPDDecrExp(p,v,__FILE__,__LINE__)
#define _pAddExp(p,i,v)     pPDAddExp(p,i,v,__FILE__,__LINE__)
#define _pSubExp(p,i,v)     pPDSubExp(p,i,v,__FILE__,__LINE__)
#define _pMultExp(p,i,v)    pPDMultExp(p,i,v,__FILE__,__LINE__)

#define _pRingSetExp(r,p,v,e)     pPDRingSetExp(r,p,v,e,__FILE__,__LINE__)
#define _pRingGetExp(r,p,v)       pPDRingGetExp(r,p,v,__FILE__,__LINE__)

#define _pSetComp(p,k)      pDBSetComp(p, k, 0, __FILE__, __LINE__)
#define _pDecrComp(p)       pDBDecrComp(p, __FILE__, __LINE__)
#define _pAddComp(p,v)      pDBAddComp(p,v, 0, __FILE__, __LINE__)
#define _pSubComp(p,v)      pDBSubComp(p,v, __FILE__, __LINE__)
#define _pRingSetComp(r,p,k)  pDBRingSetComp(r, p, k, __FILE__, __LINE__)

#define pSetCompS(p, k, l) pDBSetComp(p, k, l, __FILE__, __LINE__)

#else  // ! (defined(PDEBUG) && PDEBUG != 0)

#define _pSetExp(p,v,E)     (p)->exp.e[_pExpIndex(v)]=(E)
#define _pGetExp(p,v)       (p)->exp.e[_pExpIndex(v)]
#define _pIncrExp(p,v)      ((p)->exp.e[_pExpIndex(v)])++
#define _pDecrExp(p,v)      ((p)->exp.e[_pExpIndex(v)])--
#define _pAddExp(p,i,v)     ((p)->exp.e[_pExpIndex(i)]) += (v)
#define _pSubExp(p,i,v)     ((p)->exp.e[_pExpIndex(i)]) -= (v)
#define _pMultExp(p,i,v)    ((p)->exp.e[_pExpIndex(i)]) *= (v)

#define _pRingSetExp(r,p,v,e)     (p)->exp.e[_pRingExpIndex(r,v)]=(e)
#define _pRingGetExp(r,p,v)       (p)->exp.e[_pRingExpIndex(r,v)]

#define _pSetComp(p,k)      _pGetComp(p) = (k)
#define _pDecrComp(p)       _pGetComp(p)--
#define _pAddComp(p,v)      _pGetComp(p) += (v)
#define _pSubComp(p,v)      _pGetComp(p) -= (v)
#define _pRingSetComp(r,p,k)      (_pRingGetComp(r, p) = (k))
#define pSetCompS(p, k,l)     _pSetComp(p, k)

#endif // defined(PDEBUG) && PDEBUG != 0

#define _pGetComp(p)        ((p)->exp.e[_pCompIndex])
#define _pIncrComp(p)       _pGetComp(p)++
#define _pRingGetComp(r,p)        ((p)->exp.e[_pRingCompIndex(r)])

inline Exponent_t _pGetExpSum(poly p1, poly p2, int i)
{
  int index = _pExpIndex(i);
  return p1->exp.e[index] + p2->exp.e[index];
}
inline Exponent_t _pGetExpDiff(poly p1, poly p2, int i)
{
  int index = _pExpIndex(i);
  return p1->exp.e[index] - p2->exp.e[index];
}


inline void _pGetExpV(poly p, Exponent_t *ev)
{
  for (int j = pVariables; j; j--)
      ev[j] = _pGetExp(p, j);

  ev[0] = _pGetComp(p);
}

extern pSetmProc pSetm;
inline void _pSetExpV(poly p, Exponent_t *ev)
{
  for (int j = pVariables; j; j--)
      _pSetExp(p, j, ev[j]);

  _pSetComp(p, ev[0]);
  pSetm(p);
}

/***************************************************************
 *
 * Storage Managament Routines
 *
 ***************************************************************/
#ifdef MDEBUG

poly    pDBInit(memHeap h, char *f, int l);
poly    pDBCopy(poly a, char *f, int l);
poly    pDBCopy(memHeap h, poly a, char *f, int l);
poly    pDBCopy1(poly a, char *f, int l);
poly    pDBHead(memHeap h, poly a, char *f, int l);
poly    pDBHead0(poly a, char *f, int l);
poly    pDBFetchCopy(ring r, poly a, char *f, int l);
poly    pDBFetchCopyDelete(ring r, poly a, char *f, int l);
poly    pDBFetchHead(ring r, poly a, char *f, int l);
poly    pDBFetchHeadDelete(ring r, poly a, char *f, int l);
poly    pDBShallowCopyDeleteHead(memHeap d_h,poly *s_p,memHeap s_h,
                                 char *f,int l);
poly    pDBShallowCopyDelete(memHeap d_h,poly *s_p,memHeap s_h, char *f,int l);


void    pDBDelete(poly * a, memHeap h, char * f, int l);
void    pDBDelete1(poly * a, memHeap h, char * f, int l);

#define pDBNew(h, f,l)  (poly) mmDBAllocHeap(h, f,l)
#define _pNew(h)        (poly) mmDBAllocHeap(h, __FILE__, __LINE__)
#define _pInit(h)       (poly) pDBInit(h, __FILE__,__LINE__)

#define pDBFree1(a,h,f,l)   mmDBFreeHeap((void*)a, h, f, l)
#define _pFree1(a, h)       mmDBFreeHeap((void*)a, h, __FILE__, __LINE__)

#define _pDelete(a, h)     pDBDelete((a),h, __FILE__,__LINE__)
#define _pDelete1(a, h)    pDBDelete1((a),h, __FILE__,__LINE__)

#define _pCopy(h, A)    pDBCopy(h,A,__FILE__,__LINE__)
#define _pCopy1(A)      pDBCopy1(A, __FILE__,__LINE__)
#define _pHead(h, A)    pDBHead(h, A,__FILE__,__LINE__)
#define _pHead0(A)      pDBHead0(A, __FILE__,__LINE__)

#define _pShallowCopyDeleteHead(dest_heap, source_p, source_heap) \
  pDBShallowCopyDeleteHead(dest_heap, source_p, source_heap,__FILE__,__LINE__)
#define _pShallowCopyDelete(dest_heap, source_p, source_heap) \
  pDBShallowCopyDelete(dest_heap, source_p, source_heap,__FILE__,__LINE__)

#define _pFetchCopy(r,A)        pDBFetchCopy(r, A,__FILE__,__LINE__)
#define _pFetchCopyDelete(r,A)  pDBFetchCopyDelete(r, A,__FILE__,__LINE__)
#define _pFetchHead(r,A)        pDBFetchHead(r, A,__FILE__,__LINE__)
#define _pFetchHeadDelete(r,A)  pDBFetchHeadDelete(r, A,__FILE__,__LINE__)

#define _pRingFree1(r, A)      pDBFreeHeap(A,r->mm_specHeap,__FILE__,__LINE__)
#define _pRingDelete1(r, A)     pDBDelete1(A,r->mm_specHeap,__FILE__,__LINE__)
#define _pRingDelete(r, A)      pDBDelete(A,r->mm_specHeap,__FILE__, __LINE__)

#else // ! MDEBUG

inline poly _pNew(memHeap h)
{
  poly p;
  AllocHeap(p, h);
  return p;
}

inline poly    _pInit(memHeap h)
{
  poly p;
  AllocHeap(p, h);
  memsetW((long *)p,0, pMonomSizeW);
  return p;
}

#define _pFree1(a, h)       FreeHeap(a, h)
#define _pRingFree1(r, a)   FreeHeap(a, r->mm_specHeap)
#define _pRingDelete1(r, A) _pDelete1(A, r->mm_specHeap)
#define _pRingDelete(r, A)  _pDelete(A, r->mm_specHeap)

extern void    _pDelete(poly * a, memHeap h);
extern void    _pDelete1(poly * a, memHeap h);

extern poly    _pCopy(poly a);
extern poly    _pCopy(memHeap h, poly a);
extern poly    _pCopy1(poly a);
extern poly    _pHead(memHeap h, poly a);
extern poly    _pHead0(poly a);
extern poly    _pShallowCopyDeleteHead(memHeap d_h, poly *s_p, memHeap s_h);
extern poly    _pShallowCopyDelete(memHeap d_h, poly *s_p, memHeap s_h);

extern poly    _pFetchCopy(ring r,poly a);
extern poly    _pFetchCopyDelete(ring r,poly a);
extern poly    _pFetchHead(ring r,poly a);
extern poly    _pFetchHeadDelete(ring r,poly a);

#endif // MDEBUG

#define _pCopy2(p1, p2)     memcpyW(p1, p2, pMonomSizeW)


/***************************************************************
 *
 * Routines which work on vectors instead of single exponents
 *
 ***************************************************************/
// Here is a handy Macro which disables inlining when run with
// profiling and enables it otherwise

#ifdef DO_DEEP_PROFILE

#ifndef POLYS_IMPL_CC

#define DECLARE(type, arglist) type arglist; \
   static type dummy_##arglist
#else
#define DECLARE(type, arglist) type arglist
#endif // POLYS_IMPL_CC

#else //! DO_DEEP_PROFILE

#define DECLARE(type, arglist ) inline type arglist

#endif // DO_DEEP_PROFILE


#if defined(PDEBUG) && PDEBUG == 1
#define _pMonAddFast(p1, p2)  pDBMonAddFast(p1, p2, __FILE__, __LINE__)
extern  void pDBMonAddFast(poly p1, poly p2, char* f, int l);
inline void __pMonAddFast(poly p1, poly p2)
#else
  DECLARE(void, _pMonAddFast(poly p1, poly p2))
#endif // defined(PDEBUG) && PDEBUG == 1
{
  int i = currRing->ExpLSize;
  long* s1 = &(p1->exp.l[0]);
  const long* s2 = &(p2->exp.l[0]);
  for (;;)
  {
    *s1 += *s2;
    i--;
    if (i==0) break;
    s1++;
    s2++;
  }
}

#if defined(PDEBUG) && PDEBUG == 1
#define _pMonSubFast(p1, p2)  pDBMonSubFast(p1, p2, __FILE__, __LINE__)
extern  void pDBMonSubFast(poly p1, poly p2, char* f, int l);
inline void __pMonSubFast(poly p1, poly p2)
#else
  DECLARE(void, _pMonSubFast(poly p1, poly p2))
#endif // defined(PDEBUG) && PDEBUG == 1
{
  int i = currRing->ExpLSize;
  long* s1 = &(p1->exp.l[0]);
  const long* s2 = &(p2->exp.l[0]);

  for (;;)
  {
    *s1 -= *s2;
    i--;
    if (i==0) break;
    s1++;
    s2++;
  }
}

// Makes p1 a copy of p2 and adds on exponents of p3
#if defined(PDEBUG) && PDEBUG == 1
#define _pCopyAddFast(p1, p2, p3)  pDBCopyAddFast(p1, p2, p3, __FILE__, __LINE__)
extern  void pDBCopyAddFast(poly p1, poly p2, poly p3, char* f, int l);
inline void __pCopyAddFast(poly p1, poly p2, poly p3)
#else
  DECLARE(void, _pCopyAddFast(poly p1, poly p2, poly p3))
#endif // defined(PDEBUG) && PDEBUG == 1
{
  long* s1 = &(p1->exp.l[0]);
  const long* s2 = &(p2->exp.l[0]);
  const long* s3 = &(p3->exp.l[0]);
  const long* const ub = s3 + currRing->ExpLSize;

  p1->next = p2->next;
  p1->coef = p2->coef;

  for (;;)
  {
    *s1 = *s2 + *s3;
    s3++;
    if (s3 == ub) break;
    s1++;
    s2++;
  }
  // we first are supposed to do a copy from p2 to p1 -- therefore,
  // component of p1 is set to comp of p2
  // _pSetComp(p1, _pGetComp(p2));
}

// Similar to pCopyAddFast, except that we do not care about the "next" field
#if defined(PDEBUG) && PDEBUG == 1
#define _pCopyAddFast0(p1, p2, p3)  pDBCopyAddFast(p1, p2, p3, __FILE__, __LINE__)
extern  void pDBCopyAddFast(poly p1, poly p2, poly p3, char* f, int l);
inline void __pCopyAddFast0(poly p1, poly p2, poly p3)
#else
  DECLARE(void, _pCopyAddFast0(poly p1, poly p2, poly p3))
#endif // defined(PDEBUG) && PDEBUG == 1
{
  long* s1 = &(p1->exp.l[0]);
  const long* s2 = &(p2->exp.l[0]);
  const long* s3 = &(p3->exp.l[0]);
  const long* const ub = s3 + currRing->ExpLSize;

  p1->coef = p2->coef;

  for (;;)
  {
    *s1 = *s2 + *s3;
    s3++;
    if (s3 == ub) break;
    s1++;
    s2++;
  }
  // _pSetComp(p1, _pGetComp(p2));
}

#if SIZEOF_LONG == 4

#if SIZEOF_EXPONENT == 1
#define P_DIV_MASK 0x80808080
#define EXPONENT_MAX     0x7f
#else // SIZEOF_EXPONENT == 2
#define P_DIV_MASK 0x80008000
#define EXPONENT_MAX   0x7fff
#endif

#else // SIZEOF_LONG == 8

#if SIZEOF_EXPONENT == 1
#define P_DIV_MASK 0x8080808080808080
#define EXPONENT_MAX             0x7f
#elif  SIZEOF_EXPONENT == 2
#define P_DIV_MASK 0x8000800080008000
#define EXPONENT_MAX           0x7fff
#else // SIZEOF_EXPONENT == 4
#define P_DIV_MASK 0x8000000080000000
#define EXPONENT_MAX       0x7fffffff
#endif

#endif

// #define LONG_MONOMS

#ifdef LONG_MONOMS
DECLARE(BOOLEAN, __pDivisibleBy(poly a, poly b))
{
  const unsigned long* const lb = (unsigned long*) &(a->exp.l[currRing->pDivLow]);
  const unsigned long* s1 = (unsigned long*) &(a->exp.l[currRing->pDivHigh]);
  const unsigned long* s2 = (unsigned long*) &(b->exp.l[currRing->pDivHigh]);

  for (;;)
  {
    // Yes, the following is correct, provided that the exponents do
    // not have their first bit set
    if ((*s2 - *s1) & P_DIV_MASK) return FALSE;
    if (s1 == lb) return TRUE;
    s1--;
    s2--;
  }
}
#else
DECLARE(BOOLEAN, __pDivisibleBy(poly a, poly b))
{
  int i=pVariables; // assume i>0

  for (;;)
  {
    if (_pGetExp(a,i) > _pGetExp(b,i))
      return FALSE;
    i--;
    if (i==0)
      return TRUE;
  }
}
#endif

#if defined(PDEBUG) && PDEBUG == 1
#define _pDivisibleBy(a,b)   pDBDivisibleBy(a, b, __FILE__, __LINE__)
extern  BOOLEAN pDBDivisibleBy(poly p1, poly p2, char* f, int l);
inline BOOLEAN _pDivisibleBy_orig(poly a, poly b)
#else
inline BOOLEAN _pDivisibleBy(poly a, poly b)
#endif // defined(PDEBUG) && PDEBUG == 1
{
  if ((a!=NULL)&&((_pGetComp(a)==0) || (_pGetComp(a) == _pGetComp(b))))
  {
    return __pDivisibleBy(a,b);
  }
  return FALSE;
}

#if defined(PDEBUG) && PDEBUG == 1
#define _pDivisibleBy1(a,b)   pDBDivisibleBy1(a, b, __FILE__, __LINE__)
extern  BOOLEAN pDBDivisibleBy1(poly p1, poly p2, char* f, int l);
inline BOOLEAN _pDivisibleBy1_orig(poly a, poly b)
#else
inline BOOLEAN _pDivisibleBy1(poly a, poly b)
#endif // defined(PDEBUG) && PDEBUG == 1
{
  if (_pGetComp(a) == 0 || _pGetComp(a) == _pGetComp(b))
    return __pDivisibleBy(a,b);
  return FALSE;
}

#if defined(PDEBUG) && PDEBUG == 1
#define _pDivisibleBy2(a,b)   pDBDivisibleBy2(a, b, __FILE__, __LINE__)
extern  BOOLEAN pDBDivisibleBy2(poly p1, poly p2, char* f, int l);
#else
#define _pDivisibleBy2(a,b) __pDivisibleBy(a,b)
#endif // defined(PDEBUG) && PDEBUG == 1


DECLARE(BOOLEAN, _pEqual(poly p1, poly p2))
{
  const long *s1 = (long*) &(p1->exp.l[0]);
  const long *s2 = (long*) &(p2->exp.l[0]);
  const long* const lb = s1 + currRing->ExpLSize;

  for(;;)
  {
    if (*s1 != *s2) return FALSE;
    s1++;
    if (s1 == lb) return TRUE;
    s2++;
  }
}
/***************************************************************
 *
 * Misc. things
 * 
 *
 ***************************************************************/
// Divisiblity tests based on Short Exponent Vectors
#ifdef PDEBUG
#define _pShortDivisibleBy(a, sev_a, b, not_sev_b) \
  pDBShortDivisibleBy(a, sev_a, b, not_sev_b, __FILE__, __LINE__)
BOOLEAN pDBShortDivisibleBy(poly p1, unsigned long sev_1,
                            poly p2, unsigned long not_sev_2, 
                            char* f, int l);
#else
#define _pShortDivisibleBy(a, sev_a, b, not_sev_b) \
  ( ! (sev_a & not_sev_b) && pDivisibleBy(a, b))
#endif


/***************************************************************
 *
 * Routines which implement low-level manipulations/operations
 * on exponents and "are allowed" to access single exponetns
 *
 ***************************************************************/

#ifdef LONG_MONOMS
DECLARE(int, __pExpQuerSum2(poly p, int from, int to))
{
  int j = 0;
  int i = from ;

  for(;;)
  {
    if (i > to) break;
    j += p->exp.e[i];
    i++;
  }
  if (from <= _pCompIndex && to >= _pCompIndex)
    return j - _pGetComp(p);
  return j;
}

#define _pExpQuerSum(p)  __pExpQuerSum2(p, currRing->pVarLowIndex, currRing->pVarHighIndex)

inline int _pExpQuerSum2(poly p,int from,int to)
{
  int ei_to = _pExpIndex(to);
  int ei_from = _pExpIndex(from);

  if (ei_from > ei_to)
    return __pExpQuerSum2(p, ei_to, ei_from);
  else
    return __pExpQuerSum2(p, ei_from, ei_to);
}

#else
DECLARE(int, _pExpQuerSum(poly p))
{
  int s = 0;
  int i = pVariables;
  for (;;)
  {
    s += _pGetExp(p, i);
    i--;
    if (i==0) return s;
  }
}
#endif

#endif // POLYS_IMPL_H
