/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: kspoly.cc,v 1.23 2000-12-14 16:38:50 obachman Exp $ */
/*
*  ABSTRACT -  Routines for Spoly creation and reductions
*/

// #define PDEBUG 2
#include "mod2.h"
#include "kutil.h"
#include "numbers.h"
#include "p_polys.h"
#include "p_Procs.h"


#ifdef KDEBUG
int red_count = 0;
int create_count = 0;
// define this if reductions are reported on TEST_OPT_DEBUG
// #define TEST_OPT_DEBUG_RED
#endif

/***************************************************************
 *
 * Reduces PR with PW
 * Assumes PR != NULL, PW != NULL, Lm(PR) divides Lm(PW)
 * 
 ***************************************************************/
int ksReducePoly(LObject* PR,
                 TObject* PW,
                 poly spNoether,
                 number *coef,
                 kStrategy strat)
{
#ifdef KDEBUG
  red_count++;
#ifdef TEST_OPT_DEBUG_RED
  if (TEST_OPT_DEBUG)
  {
    Print("Red %d:", red_count); PR->wrp(); Print(" with:");
    PW->wrp();
  }
#endif
#endif
  int ret = 0;
  ring tailRing = PR->tailRing;
  kTest_L(PR);
  kTest_T(PW);
  
  poly p1 = PR->GetLmTailRing();
  poly p2 = PW->GetLmTailRing();
  poly t2 = pNext(p2), lm = p1;
  assume(p1 != NULL && p2 != NULL);
  p_CheckPolyRing(p1, tailRing);
  p_CheckPolyRing(p2, tailRing);

  pAssume1(p2 != NULL && p1 != NULL && 
           p_DivisibleBy(p2,  p1, tailRing));

  pAssume1(p_GetComp(p1, tailRing) == p_GetComp(p2, tailRing) ||
           (p_GetComp(p2, tailRing) == 0 && 
            p_MaxComp(pNext(p2),tailRing) == 0));

  if (t2==NULL)
  {
    PR->LmDeleteAndIter();
    if (coef != NULL) *coef = n_Init(1, tailRing);
    return 0;
  }

  p_ExpVectorSub(lm, p2, tailRing);
  
  if (tailRing != currRing)
  {
    // check that reduction does not violate exp bound
    while (PW->max != NULL && !p_LmExpVectorAddIsOk(lm, PW->max, tailRing))
    {
      // undo changes of lm
      p_ExpVectorAdd(lm, p2, tailRing);
      if (strat == NULL) return 2;
      if (! kStratChangeTailRing(strat, PR, PW)) return -1;
      tailRing = strat->tailRing;
      p1 = PR->GetLmTailRing();
      p2 = PW->GetLmTailRing();
      t2 = pNext(p2);
      lm = p1;
      p_ExpVectorSub(lm, p2, tailRing);
      ret = 1;
    }
  }

  // take care of coef buisness
  if (! n_IsOne(pGetCoeff(p2), tailRing))
  {
    number bn = pGetCoeff(lm);
    number an = pGetCoeff(p2);
    int ct = ksCheckCoeff(&an, &bn);
    p_SetCoeff(lm, bn,tailRing);
    if ((ct == 0) || (ct == 2)) 
      PR->Tail_Mult_nn(an);
    if (coef != NULL) *coef = an;
    else n_Delete(&an, tailRing);
  }
  else
  {
    if (coef != NULL) *coef = n_Init(1, tailRing);
  }
  
  
  // and finally, 
  PR->Tail_Minus_mm_Mult_qq(lm, t2, PW->GetpLength() - 1, spNoether);
  PR->LmDeleteAndIter();
#if defined(KDEBUG) && defined(TEST_OPT_DEBUG_RED)
  if (TEST_OPT_DEBUG)
  {
    Print(" to: "); PR->wrp(); Print("\n");
  }
#endif
  return ret;
}

/***************************************************************
 *
 * Creates S-Poly of p1 and p2
 * 
 *
 ***************************************************************/
void ksCreateSpoly(LObject* Pair,   poly spNoether, 
                   int use_buckets, ring tailRing, 
                   poly m1, poly m2, TObject** R)
{
#ifdef KDEBUG
  create_count++;
#endif
  kTest_L(Pair);
  poly p1 = Pair->p1;
  poly p2 = Pair->p2;
  poly last;
  Pair->tailRing = tailRing;
  
  assume(p1 != NULL);
  assume(p2 != NULL);
  assume(tailRing != NULL);

  poly a1 = pNext(p1), a2 = pNext(p2);
  number lc1 = pGetCoeff(p1), lc2 = pGetCoeff(p2);
  int co=0, ct = ksCheckCoeff(&lc1, &lc2);

  int l1=0, l2=0;
  
  if (p_GetComp(p1, currRing)!=p_GetComp(p2, currRing))
  {
    if (p_GetComp(p1, currRing)==0)
    {
      co=1;
      p_SetCompP(p1,p_GetComp(p2, currRing), currRing, tailRing);
    }
    else
    {
      co=2;
      p_SetCompP(p2, p_GetComp(p1, currRing), currRing, tailRing);
    }
  }

  // get m1 = LCM(LM(p1), LM(p2))/LM(p1)
  //     m2 = LCM(LM(p1), LM(p2))/LM(p2)
  if (m1 == NULL)
    k_GetLeadTerms(p1, p2, currRing, m1, m2, tailRing);
    
  pSetCoeff0(m1, lc2);
  pSetCoeff0(m2, lc1);  // and now, m1 * LT(p1) == m2 * LT(p2)

  if (R != NULL)
  {
    l1 = (R[Pair->i_r1])->GetpLength() - 1;
    l2 = (R[Pair->i_r2])->GetpLength() - 1;
  }
  
  // get m2 * a2
  if (spNoether != NULL)
  {
    l2 = -1;
    a2 = tailRing->p_Procs->pp_Mult_mm_Noether(a2, m2, spNoether, l2, tailRing,last);
    assume(l2 == pLength(a2));
  }
  else
    a2 = tailRing->p_Procs->pp_Mult_mm(a2, m2, tailRing,last);
  Pair->SetLmTail(m2, a2, l2, use_buckets, tailRing, last);

  // get m2*a2 - m1*a1
  Pair->Tail_Minus_mm_Mult_qq(m1, a1, l1, spNoether);
  
  // Clean-up time
  Pair->LmDeleteAndIter();
  p_LmDelete(m1, tailRing);
  
  if (co != 0)
  {
    if (co==1)
    {
      p_SetCompP(p1,0, currRing, tailRing);
    }
    else
    {
      p_SetCompP(p2,0, currRing, tailRing);
    }
  }
}

int ksReducePolyTail(LObject* PR, TObject* PW, poly Current, poly spNoether)
{
  BOOLEAN ret;
  number coef;
  poly Lp =     PR->GetLmCurrRing();
  poly Save =   PW->GetLmCurrRing();
  
  kTest_L(PR);
  kTest_T(PW);
  pAssume(pIsMonomOf(Lp, Current));
  
  assume(Lp != NULL && Current != NULL && pNext(Current) != NULL);
  assume(PR->bucket == NULL);

  LObject Red(pNext(Current), PR->tailRing);
  TObject With(PW, Lp == Save);

  pAssume(!pHaveCommonMonoms(Red.p, With.p));
  ret = ksReducePoly(&Red, &With, spNoether, &coef);
  
  if (!ret)
  {
    if (! n_IsOne(coef, currRing))
    {
      pNext(Current) = NULL;
      if (Current == PR->p && PR->t_p != NULL)
        pNext(PR->t_p) = NULL;
      PR->Mult_nn(coef);
    }

    n_Delete(&coef, currRing);
    pNext(Current) = Red.GetLmTailRing();
    if (Current == PR->p && PR->t_p != NULL)
      pNext(PR->t_p) = pNext(Current);
  }

  if (Lp == Save) 
    With.Delete();
  return ret;
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
poly ksCreateShortSpoly(poly p1, poly p2, ring tailRing)
{
  poly a1 = pNext(p1), a2 = pNext(p2);
  Exponent_t c1=p_GetComp(p1, currRing),c2=p_GetComp(p2, currRing);
  Exponent_t c;
  poly m1,m2;
  number t1,t2;
  int cm,i;
  BOOLEAN equal;

  if (a1==NULL)
  {
    if(a2!=NULL)
    {
      m2=p_Init(currRing);
x2:
      for (i = pVariables; i; i--)
      {
        c = p_GetExpDiff(p1, p2,i, currRing);
        if (c>0)
        {
          p_SetExp(m2,i,(c+p_GetExp(a2,i,tailRing)),currRing);
        }
        else
        {
          p_SetExp(m2,i,p_GetExp(a2,i,tailRing),currRing);
        }
      }
      if ((c1==c2)||(c2!=0))
      {
        p_SetComp(m2,p_GetComp(a2,tailRing), currRing);
      }
      else
      {
        p_SetComp(m2,c1,currRing);
      }
      p_Setm(m2, currRing);
      nNew(&(pGetCoeff(m2)));
      return m2;
    }
    else
      return NULL;
  }
  if (a2==NULL)
  {
    m1=p_Init(currRing);
x1:
    for (i = pVariables; i; i--)
    {
      c = p_GetExpDiff(p2, p1,i,currRing);
      if (c>0)
      {
        p_SetExp(m1,i,(c+p_GetExp(a1,i, tailRing)),currRing);
      }
      else
      {
        p_SetExp(m1,i,p_GetExp(a1,i, tailRing), currRing);
      }
    }
    if ((c1==c2)||(c1!=0))
    {
      p_SetComp(m1,p_GetComp(a1,tailRing),currRing);
    }
    else
    {
      p_SetComp(m1,c2,currRing);
    }
    p_Setm(m1, currRing);
    nNew(&(pGetCoeff(m1)));
    return m1;
  }
  m1 = p_Init(currRing);
  m2 = p_Init(currRing);
  for(;;)
  {
    for (i = pVariables; i; i--)
    {
      c = p_GetExpDiff(p1, p2,i,currRing);
      if (c > 0)
      {
        p_SetExp(m2,i,(c+p_GetExp(a2,i,tailRing)), currRing);
        p_SetExp(m1,i,p_GetExp(a1,i, tailRing), currRing);
      }
      else
      {
        p_SetExp(m1,i,(p_GetExp(a1,i,tailRing)-c), currRing);
        p_SetExp(m2,i,p_GetExp(a2,i, tailRing), currRing);
      }
    }
    if(c1==c2)
    {
      p_SetComp(m1,p_GetComp(a1, tailRing), currRing);
      p_SetComp(m2,p_GetComp(a2, tailRing), currRing);
    }
    else
    {
      if(c1!=0)
      {
        p_SetComp(m1,p_GetComp(a1, tailRing), currRing);
        p_SetComp(m2,c1, currRing);
      }
      else
      {
        p_SetComp(m2,p_GetComp(a2, tailRing), currRing);
        p_SetComp(m1,c2, currRing);
      }
    }
    p_Setm(m1,currRing);
    p_Setm(m2,currRing);
    cm = p_LmCmp(m1, m2,currRing);
    if (cm!=0)
    {
      if(cm==1)
      {
        p_LmFree(m2,currRing);
        nNew(&(pGetCoeff(m1)));
        return m1;
      }
      else
      {
        p_LmFree(m1,currRing);
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
      p_LmFree(m2,currRing);
      nNew(&(pGetCoeff(m1)));
      return m1;
    }
    pIter(a1);
    pIter(a2);
    if (a2==NULL)
    {
      p_LmFree(m2,currRing);
      if (a1==NULL)
      {
        p_LmFree(m1,currRing);
        return NULL;
      }
      goto x1;
    }
    if (a1==NULL)
    {
      p_LmFree(m1,currRing);
      goto x2;
    }
  }
}





  
  
