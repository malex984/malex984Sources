/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: polys-impl.cc,v 1.1 1997-12-03 16:58:57 obachman Exp $ */

/***************************************************************
 *
 * File:       polys-impl.cc
 * Purpose:    low-level procuders for polys.
 *
 * If you touch anything here, you better know what you are doing.
 * What is here should not be used directly from other routines -- the
 * encapsulations in polys.[h,cc] should be used, instead.
 *
 ***************************************************************/
#ifndef POLYS_IMPL_CC
#define POLYS_IMPL_CC

#include <stdio.h>
#include <string.h>
#include "mod2.h"
#include "structs.h"
#include "mmemory.h"
#include "febase.h"
#include "numbers.h"
#include "polys.h"
#include "ring.h"
#include "ipid.h"

/***************************************************************
 *
 * definition of global variables
 *
 ***************************************************************/

int pVariables;
int pMonomSize;
int pMonomSizeW;

#ifdef COMP_FAST
int pVariables1W;
int pVariablesW;
int pVarOffset;
int pCompIndex;
int pVarLowIndex;
int pVarHighIndex;
#endif
int pLexSgn;

/***************************************************************
 *
 * Storage Managament Routines
 *
 ***************************************************************/

#ifdef MDEBUG
poly pDBNew(char *f, int l)
{
  poly p = (poly) mmDBAllocBlock(pMonomSize,f,l);
  memset(p,0,pMonomSize);
  return p;
}
#endif

/*2
* create a new monomial and init
*/
#ifdef MDEBUG
poly pDBInit(char * f, int l)
{
  poly p=pDBNew(f,l);
  //memset(p,0, pMonomSize);
  nNew(&(p->coef));
  return p;
}
#else
poly _pInit(void)
{
  poly p=(poly)mmAllocSpecialized();
  memset(p,0, pMonomSize);
  return p;
}
#endif

/*2
* delete a poly, resets pointer
* put the monomials in the freelist
*/
#ifdef MDEBUG
void pDBDelete(poly * p, char * f, int l)
{
  poly h = *p;

  while (h!=NULL)
  {
#ifdef LDEBUG
    nDBDelete(&(h->coef),f,l);
#else
    nDelete(&(h->coef));
#endif
    pIter(h);
    mmDBFreeBlock((ADDRESS)*p,pMonomSize,f,l);
    *p=h;
    if (l>0) l= -l;
  }
  *p = NULL;
}
#else
void _pDelete(poly* p)
{
  poly h = *p;
  poly pp;

  while (h!=NULL)
  {
    nDelete(&(h->coef));
    pp=h;
    pIter(h);
    mmFreeSpecialized((ADDRESS)pp);
  }
  *p = NULL;
}
#endif

/*2
* remove first monom
*/
#ifdef MDEBUG
void pDBDelete1(poly * p, char * f, int l)
{
  poly h = *p;

  if (h==NULL) return;
  nDelete(&(h->coef));
  *p = pNext(h);
  mmDBFreeBlock((ADDRESS)h,pMonomSize,f,l);
}
#else
void _pDelete1(poly* p)
{
  poly h = *p;

  if (h==NULL) return;
  nDelete(&(h->coef));
  *p = pNext(h);
  mmFreeSpecialized((ADDRESS)h);
}
#endif

void ppDelete(poly* p, ring rg)
{
  nSetChar(rg->ch,FALSE,rg->parameter,rg->P);
  pChangeRing(rg->N,rg->OrdSgn,rg->order,rg->block0,rg->block1,rg->wvhdl);
  pDelete(p);
  nSetChar(currRing->ch,FALSE,currRing->parameter,currRing->P);
  pChangeRing(currRing->N,currRing->OrdSgn,currRing->order,
    currRing->block0,currRing->block1,  currRing->wvhdl);
}

/*2
* remove first monom
*/
#ifdef MDEBUG
void pDBFree1(poly p, char * f, int l)
{
  if (p!=NULL)
  {
    p->coef=NULL;//nDelete(&(p->coef));
    mmDBFreeBlock((ADDRESS)p,pMonomSize,f,l);
  }
}
#endif

/*2
* creates a copy of p
*/
#ifdef MDEBUG
poly pDBCopy(poly p,char *f,int l)
#else
poly _pCopy(poly p)
#endif
{
  poly w, a;

  if (p==NULL) return NULL;
  pDBTest(p,f,l);
#ifdef MDEBUG
  w = a = pDBNew(f,l);
#else
  w = a = pNew();
#endif
  memcpy(w,p,pMonomSize);
  w->coef=nCopy(p->coef);
  if (pNext(p)!=NULL)
  {
    pIter(p);
    do
    {
#ifdef MDEBUG
      a = pNext(a) = pDBNew(f,l);
#else
      a = pNext(a) = pNew();
#endif
      memcpy(a,p,pMonomSize);
      a->coef=nCopy(p->coef);
      pIter(p);
    }
    while (p!=NULL);
  }
  pNext(a) = NULL;
  return w;
}


/*2
* creates a copy of the initial monomial of p
* sets the coeff of the copy to a defined value
*/
#ifdef MDEBUG
poly pDBCopy1(poly p,char *f,int l)
#else  
poly _pCopy1(poly p)
#endif  
{
  poly w;
#ifdef MDEBUG
  w = pDBNew(f,l);
#else  
  w = pNew();
#endif  
  pCopy2(w,p);
  nNew(&(w->coef));
  pNext(w) = NULL;
  return w;
}

/*2
* returns (a copy of) the head term of a
*/
#ifdef MDEBUG
poly pDBHead(poly p,char *f, int l)
#else
poly _pHead(poly p)
#endif
{
  poly w=NULL;

  if (p!=NULL)
  {
#ifdef MDEBUG
    w = pDBNew(f,l);
#else
    w = pNew();
#endif
    pCopy2(w,p);
    pSetCoeff0(w,nCopy(pGetCoeff(p)));
    pNext(w) = NULL;
  }
  return w;
}

poly pHeadProc(poly p)
{
  return pHead(p);
}

/*2
* returns (a copy of) the head term of a without the coef
*/
#ifdef MDEBUG
poly pDBHead0(poly p,char *f, int l)
#else
poly _pHead0(poly p)
#endif
{
  poly w=NULL;

  if (p!=NULL)
  {
#ifdef MDEBUG
    w = pDBNew(f,l);
#else
    w = pNew();
#endif
    pCopy2(w,p);
    pSetCoeff0(w,NULL);
    pNext(w) = NULL;
  }
  return w;
}


/***************************************************************
 *
 * Routines for turned on debugging
 *
 ***************************************************************/

#ifdef PDEBUG

#if PDEBUG != 0
Exponent_t pPDSetExp(poly p, int v, Exponent_t e, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return (p)->exp[_pExpIndex(v)]=(e);
}

Exponent_t pPDGetExp(poly p, int v, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return (p)->exp[_pExpIndex(v)];
}

Exponent_t pPDIncrExp(poly p, int v, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return ((p)->exp[_pExpIndex(v)])++;
}

Exponent_t pPDDecrExp(poly p, int v, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return ((p)->exp[_pExpIndex(v)])--;
}

Exponent_t pPDAddExp(poly p, int v, Exponent_t e, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return ((p)->exp[_pExpIndex(v)]) += (e);
}

Exponent_t pPDSubExp(poly p, int v, Exponent_t e, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return ((p)->exp[_pExpIndex(v)]) -= (e);
}

Exponent_t pPDMultExp(poly p, int v, Exponent_t e, char* f, int l)
{
  if (v == 0)
  {
    Print("zero index to exponent in %s:%d\n", f, l);
  }
  if (v > pVariables)
  {
    Print("index %d to exponent too large in %s:%d\n", v, f, l);
  }
  return ((p)->exp[_pExpIndex(v)]) *= (e);
}

#ifdef COMP_FAST
// checks whether fast monom add did not overflow
void pDBMonAddFast(poly p1, poly p2, char* f, int l)
{
  poly ptemp = pNew();
  pCopy2(ptemp, p1);
  _pMonAddFast(p1, p2);

  for (int i=1; i<=pVariables; i++)
    pAddExp(ptemp, i, pGetExp(p2, i));
  pGetOrder(ptemp) += pGetOrder(p2);

  if (! pEqual(ptemp, p1))
    Print("Error inpMonAddFast in %s:%d\n", f, l);
  pFree1(ptemp);
}

void pDBCopyAddFast(poly p1, poly p2, poly p3, char* f, int l)
{
  if (p2 == p1 || p3 == p1)
  {
    Print("Error in CopyAddFast: Destination equals source in %s:%d\n", f, l);
  }
  poly ptemp = pNew();
  __pCopyAddFast(ptemp, p2, p3);

  pCopy2(p1, p2);
  pDBMonAddFast(p1, p3, f, l);

  if (! pEqual(ptemp, p1))
    Print("Error in CopyMonAddFast in %s:%d\n", f, l);
  pFree1(ptemp);
}
  
BOOLEAN pDBDivisibleBy(poly a, poly b, char* f, int l)
{
  BOOLEAN istrue = TRUE;
  BOOLEAN f_istrue = _pDivisibleBy_orig(a, b);

  if ((a!=NULL)&&((pGetComp(a)==0) || (pGetComp(a) == pGetComp(b))))
  {
    for (int i = 1; i<=pVariables; i++)
      if (pGetExp(a, i) > pGetExp(b,i))
      {
        istrue = FALSE;
        break;
      }
  }
  else istrue = FALSE;
  
  if (istrue != f_istrue)
  {
    Print("Error in pDivisibleBy in %s:%d\n", f, l);
    _pDivisibleBy_orig(a, b);
  }
  return f_istrue;
}

#endif // COMP_FAST

#endif // PDEBUG != 0 

    
BOOLEAN pDBTest(poly p, char *f, int l)
{
  poly old=NULL;
  BOOLEAN ismod=FALSE;
  while (p!=NULL)
  {
#ifdef MDEBUG
    if (!mmDBTestBlock(p,pMonomSize,f,l))
      return FALSE;
#endif
#ifdef LDEBUG
    if (!nDBTest(p->coef,f,l))
       return FALSE;
#endif
    if ((p->coef==NULL)&&(nGetChar()<2))
    {
      Print("NULL coef in poly in %s:%d\n",f,l);
      return FALSE;
    }
    if (nIsZero(p->coef))
    {
      Print("zero coef in poly in %s:%d\n",f,l);
      return FALSE;
    }
    int i=pVariables;
#ifndef DRING
    for(;i;i--)
    {
      if (pGetExp(p,i)<0)
      {
        Print("neg. Exponent in %s:%l\n",f,l);
        return FALSE;
      }
    }
    if (pGetComp(p)<0)
    {
      Print("neg Component in %s:%l\n",f,l);
      return FALSE;
    }
#endif
    if (ismod==0)
    {
      if (pGetComp(p)>0) ismod=1;
      else               ismod=2;
    }
    else if (ismod==1)
    {
      if (pGetComp(p)==0)
      {
        Print("mix vec./poly in %s:%d\n",f,l);
        return FALSE;
      }
    }
    else if (ismod==2)
    {
      if (pGetComp(p)!=0)
      {
        Print("mix poly/vec. in %s:%d\n",f,l);
        return FALSE;
      }
    }
    i=p->Order;
    pSetm(p);
    if(i!=p->Order)
    {
      Print("wrong ord-field in %s:%d\n",f,l);
      return FALSE;
    }
    old=p;
    pIter(p);
    if (pComp(old,p)!=1)
    {
      Print("wrong order in %s:%d\n",f,l);
      pComp(old, p);
      return FALSE;
    }
  }
  return TRUE;
}
#endif // PDEBUG


#endif // POLYS_IMPL_CC
