/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: kspoly.cc,v 1.4 1999-10-25 08:32:15 obachman Exp $ */
/*
*  ABSTRACT -  Routines for Spoly creation and reductions
*/
#include "mod2.h"
#include "kutil.h"
#include "polys.h"
#include "pProcs.h"
#include "numbers.h"

// Define to enable tests in this file 
#define DEBUG_THIS

#if ! (defined(DEBUG_THIS) || (defined(KDEBUG) && (KDEBUG > 1)))
#undef assume
#define assume(x) 
#endif


/***************************************************************
 *
 * Reduces PR with PW
 * Assumes PR != NULL, PW != NULL, Lm(PR) divides Lm(PW)
 * 
 ***************************************************************/
void ksReducePoly(LObject* PR,
                  TObject* PW,
                  poly spNoether,
                  number *coef)
{
  assume(kTest_L(PR));
  assume(kTest_T(PW));

  poly p1 = PR->p;
  poly p2 = PW->p;
  assume(p2 != NULL && p1 != NULL && pDivisibleBy(p2,  p1));
  poly a2 = pNext(p2), lm = p1;

  p1 = pNext(p1);

  BOOLEAN reset_vec=FALSE;
  
  if (a2==NULL)
  {
    pDelete1(&lm);
    PR->p = p1;
    if (coef != NULL) *coef = nInit(1);
    return;
  }

  if (! nIsOne(pGetCoeff(p2)))
  {
    number bn = pGetCoeff(lm);
    number an = pGetCoeff(p2);
    int ct = ksCheckCoeff(&an, &bn);
    pSetCoeff(lm, bn);
    if ((ct == 0) || (ct == 2)) 
      p1 = p_Mult_n(p1, an);
    if (coef != NULL) *coef = an;
    else nDelete(&an);
  }
  else
  {
    if (coef != NULL)
      *coef = nInit(1);
  }
  
  
  if (pGetComp(p2) != pGetComp(lm))
  {
    pSetCompP(a2, pGetComp(lm));
    reset_vec = TRUE;
    // need to make sure that Comp of lm is same as comp of p2
    pSetComp(lm, pGetComp(p2));
    pSetmComp(lm);
  }

  pMonSubFrom(lm, p2);
  assume(pGetComp(lm) == 0);

  int l1, l2;
  PR->p = p_Minus_m_Mult_q(p1, lm, a2, spNoether);
  
  pDelete1(&lm);
  if (reset_vec) pSetCompP(a2, 0);
}

/***************************************************************
 *
 * Creates S-Poly of p1 and p2
 * 
 *
 ***************************************************************/
void ksCreateSpoly(LObject* Pair, 
                   poly spNoether)
{
  assume(kTest_L(Pair));
  poly p1 = Pair->p1;
  poly p2 = Pair->p2;

  assume(p1 != NULL);
  assume(p2 != NULL);

  poly a1 = pNext(p1), a2 = pNext(p2);
  number lc1 = pGetCoeff(p1), lc2 = pGetCoeff(p2);
  poly m1, m2;
  int co=0, ct = ksCheckCoeff(&lc1, &lc2);
  int x, l1;

  if (pGetComp(p1)!=pGetComp(p2))
  {
    if (pGetComp(p1)==0)
    {
      co=1;
      pSetCompP(p1,pGetComp(p2));
    }
    else
    {
      co=2;
      pSetCompP(p2,pGetComp(p1));
    }
  }

  // get m1 = LCM(LM(p1), LM(p2))/LM(p1)
  //     m2 = LCM(LM(p1), LM(p2))/LM(p2)
  m1 = pInit();
  m2 = pInit();
  for (int i = pVariables; i; i--)
  {
    x = pGetExpDiff(p1, p2, i);
    if (x > 0)
    {
      pSetExp(m2,i,x);
      pSetExp(m1,i,0);
    }
    else
    {
      pSetExp(m1,i,-x);
      pSetExp(m2,i,0);
    }
  }
  pSetm(m1);
  pSetm(m2);           // now we have m1 * LM(p1) == m2 * LM(p2)
  pSetCoeff0(m1, lc2);
  pSetCoeff0(m2, lc1); // and now, m1 * LT(p1) == m2 * LT(p2)

  // get m2 * a2
  a2 = p_Mult_m(a2, m2, spNoether);

  // and, finally, the spoly
  Pair->p = p_Minus_m_Mult_q(a2, m1, a1, spNoether);
  
  // Clean-up time
  pDelete1(&m1);
  pDelete1(&m2);
  
  if (co != 0)
  {
    if (co==1)
    {
      pSetCompP(p1,0);
    }
    else
    {
      pSetCompP(p2,0);
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Reduces PR at Current->next with PW
// Assumes PR != NULL, Current contained in PR 
//         Current->next != NULL, LM(PW) devides LM(Current->next)
// Changes: PR
// Const:   PW
void ksSpolyTail(LObject* PR, TObject* PW, poly Current, poly spNoether)
{
  poly Lp = PR->p;
  number coef;
  poly Save = PW->p;
  
  if (Lp == Save)
    PW->p = pCopy(Save);
    
  assume(Lp != NULL && Current != NULL && pNext(Current) != NULL);
  assume(pIsMonomOf(Lp, Current));
  
  PR->p = pNext(Current);
  ksReducePoly(PR, PW, spNoether, &coef);
  
  if (! nIsOne(coef))
  {
    pNext(Current) = NULL;
    p_Mult_n(Lp, coef);
  }
  nDelete(&coef);
  pNext(Current) = PR->p;
  PR->p = Lp;
  if (PW->p != Save)
  {
    pDelete(&(PW->p));
    PW->p = Save; // == Lp
  }
}


/***************************************************************
 *
 * Auxillary Routines
 * 
 * 
 ***************************************************************/

/*
* input - output: a, b
* returns:
*   a := a/gcd(a,b), b := b/gcd(a,b)
*   and return value
*       0  ->  a != 1,  b != 1
*       1  ->  a == 1,  b != 1
*       2  ->  a != 1,  b == 1
*       3  ->  a == 1,  b == 1
*   this value is used to control the spolys
*/
int ksCheckCoeff(number *a, number *b)
{
  int c = 0;
  number an = *a, bn = *b;
  nTest(an);
  nTest(bn);
  number cn = nGcd(an, bn);

  if(nIsOne(cn))
  {
    an = nCopy(an);
    bn = nCopy(bn);
  }
  else
  {
    an = nIntDiv(an, cn);
    bn = nIntDiv(bn, cn);
  }
  nDelete(&cn);
  if (nIsOne(an))
  {
    c = 1;
  }
  if (nIsOne(bn))
  {
    c += 2;
  }
  *a = an;
  *b = bn;
  return c;
}

/*2
* creates the leading term of the S-polynomial of p1 and p2
* do not destroy p1 and p2
* remarks:
*   1. the coefficient is 0 (nNew)
*   2. pNext is undefined
*/
//static void bbb() { int i=0; }
poly ksCreateShortSpoly(poly p1, poly p2)
{
  poly a1 = pNext(p1), a2 = pNext(p2);
  Exponent_t c1=pGetComp(p1),c2=pGetComp(p2);
  Exponent_t c;
  poly m1,m2;
  number t1,t2;
  int cm,i;
  BOOLEAN equal;

  if (a1==NULL)
  {
    if(a2!=NULL)
    {
      m2=pInit();
x2:
      for (i = pVariables; i; i--)
      {
        c = pGetExpDiff(p1, p2,i);
        if (c>0)
        {
          pSetExp(m2,i,(c+pGetExp(a2,i)));
        }
        else
        {
          pSetExp(m2,i,pGetExp(a2,i));
        }
      }
      if ((c1==c2)||(c2!=0))
      {
        pSetComp(m2,pGetComp(a2));
      }
      else
      {
        pSetComp(m2,c1);
      }
      pSetm(m2);
      nNew(&(pGetCoeff(m2)));
      return m2;
    }
    else
      return NULL;
  }
  if (a2==NULL)
  {
    m1=pInit();
x1:
    for (i = pVariables; i; i--)
    {
      c = pGetExpDiff(p2, p1,i);
      if (c>0)
      {
        pSetExp(m1,i,(c+pGetExp(a1,i)));
      }
      else
      {
        pSetExp(m1,i,pGetExp(a1,i));
      }
    }
    if ((c1==c2)||(c1!=0))
    {
      pSetComp(m1,pGetComp(a1));
    }
    else
    {
      pSetComp(m1,c2);
    }
    pSetm(m1);
    nNew(&(pGetCoeff(m1)));
    return m1;
  }
  m1 = pInit();
  m2 = pInit();
  for(;;)
  {
    for (i = pVariables; i; i--)
    {
      c = pGetExpDiff(p1, p2,i);
      if (c > 0)
      {
        pSetExp(m2,i,(c+pGetExp(a2,i)));
        pSetExp(m1,i,pGetExp(a1,i));
      }
      else
      {
        pSetExp(m1,i,(pGetExp(a1,i)-c));
        pSetExp(m2,i,pGetExp(a2,i));
      }
    }
    if(c1==c2)
    {
      pSetComp(m1,pGetComp(a1));
      pSetComp(m2,pGetComp(a2));
    }
    else
    {
      if(c1!=0)
      {
        pSetComp(m1,pGetComp(a1));
        pSetComp(m2,c1);
      }
      else
      {
        pSetComp(m2,pGetComp(a2));
        pSetComp(m1,c2);
      }
    }
    pSetm(m1);
    pSetm(m2);
    cm = pComp0(m1, m2);
    if (cm!=0)
    {
      if(cm==1)
      {
        pFree1(m2);
        nNew(&(pGetCoeff(m1)));
        return m1;
      }
      else
      {
        pFree1(m1);
        nNew(&(pGetCoeff(m2)));
        return m2;
      }
    }
    t1 = nMult(pGetCoeff(a2),pGetCoeff(p1));
    t2 = nMult(pGetCoeff(a1),pGetCoeff(p2));
    equal = nEqual(t1,t2);
    nDelete(&t2);
    nDelete(&t1);
    if (!equal)
    {
      pFree1(m2);
      nNew(&(pGetCoeff(m1)));
      return m1;
    }
    pIter(a1);
    pIter(a2);
    if (a2==NULL)
    {
      pFree1(m2);
      if (a1==NULL)
      {
        pFree1(m1);
        return NULL;
      }
      goto x1;
    }
    if (a1==NULL)
    {
      pFree1(m1);
      goto x2;
    }
  }
}


/***************************************************************
 *
 * Routines for backwards-Compatibility
 * 
 * 
 ***************************************************************/
poly ksOldSpolyRed(poly p1, poly p2, poly spNoether)
{
  LObject L;
  TObject T;

  L.p = p2;
  T.p = p1;
  
  ksReducePoly(&L, &T, spNoether);
  
  return L.p;
}

poly ksOldSpolyRedNew(poly p1, poly p2, poly spNoether)
{
  LObject L;
  TObject T;

  L.p = pCopy(p2);
  T.p = p1;
  
  ksReducePoly(&L, &T, spNoether);
  
  return L.p;
}

poly ksOldCreateSpoly(poly p1, poly p2, poly spNoether)
{
  LObject L;
  L.p1 = p1;
  L.p2 = p2;
  
  ksCreateSpoly(&L, spNoether);
  return L.p;
}

#if 1
void ksOldSpolyTail(poly p1, poly q, poly q2, poly spNoether)
{
  LObject L;
  TObject T;

  L.p = q;
  T.p = p1;
  
  ksSpolyTail(&L, &T, q2, spNoether);
  return;
}


#else
  
/*2
* reduction of tail(q) with p1
* lead(p1) divides lead(pNext(q2)) and pNext(q2) is reduced
* do not destroy p1, but tail(q)
*/
void ksOldSpolyTail(poly p1, poly q, poly q2, poly spNoether)
{
  number t;
  poly m, h;
  poly a1 = pNext(p1), p2 = pNext(q2), a2 = pNext(p2);
  number an = pGetCoeff(p1), bn = pGetCoeff(p2);
  int ct = ksCheckCoeff(&an, &bn);
  BOOLEAN reset_vec=FALSE;

  if(a1==NULL)
  {
    nDelete(&an);
    nDelete(&bn);
    nDelete(&pGetCoeff(p2));
    pFree1(p2);
    pNext(q2) = a2;
    return;
  }
  if (p1 != q)
  {
    m = p2;
    pSetCoeff(m,bn);
  }
  else
  {
    m = pHead0(p2);
    pSetCoeff0(m,bn);
  }
  if ((ct == 0) || (ct == 2))
  {
    pMultN(a2, an);
  }
  if ((pGetComp(p1) != pGetComp(p2))
  && (pGetComp(p1)==0))
  {
    pSetCompP(a1,pGetComp(p2));
    reset_vec=TRUE;
  }
  pMonSubFrom(m,p1);
  poly res = p_Minus_m_Mult_q(a2, m, a1);
  if ((ct == 0) || (ct == 2))
  {
    h = q;
    do
    {
      t = nMult(pGetCoeff(h),an);
      pSetCoeff(h,t);
      pIter(h);
    }
    while (h != p2);
  }
  h = res;
  nDelete(&an);
  nDelete(&bn);
  pFree1(m);
  pNext(q2) = h;
  if (reset_vec)
    pSetCompP(a1,0);
  if (p1 == q)
  {
    pDelete(&p2);
  }
}

#endif          

  
  
