/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: polys.cc,v 1.66 2000-10-23 15:21:14 Singular Exp $ */

/*
* ABSTRACT - all basic methods to manipulate polynomials
*/

/* includes */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mod2.h"
#include "tok.h"
#include "omalloc.h"
#include "febase.h"
#include "numbers.h"
#include "polys.h"
#include "ring.h"

/* ----------- global variables, set by pSetGlobals --------------------- */
/* computes length and maximal degree of a POLYnomial */
pLDegProc pLDeg;
/* computes the degree of the initial term, used for std */
pFDegProc pFDeg;
/* the monomial ordering of the head monomials a and b */
/* returns -1 if a comes before b, 0 if a=b, 1 otherwise */

int pVariables;     // number of variables

/* 1 for polynomial ring, -1 otherwise */
int     pOrdSgn;
// it is of type int, not BOOLEAN because it is also in ip
/* TRUE if the monomial ordering is not compatible with pFDeg */
BOOLEAN pLexOrder;

/* ----------- global variables, set by procedures from hecke/kstd1 ----- */
/* the highest monomial below pHEdge */
poly      ppNoether = NULL;


void rSetmS(poly p, int* Components, long* ShiftedComponents)
{
  int pos=0;
  assume(Components != NULL && ShiftedComponents != NULL);
  if (currRing->typ!=NULL)
  {
    loop
    {
      long ord=0;
      sro_ord* o=&(currRing->typ[pos]);
      switch(o->ord_typ)
      {
        case ro_dp:
        {
          int a,e;
          a=o->data.dp.start;
          e=o->data.dp.end;
          for(int i=a;i<=e;i++) ord+=pGetExp(p,i);
          p->exp[o->data.dp.place]=ord;
          break;
        }
	case ro_wp_neg:
	  ord=POLY_NEGWEIGHT_OFFSET;
	  // no break;
        case ro_wp:
        {
          int a,e;
          a=o->data.wp.start;
          e=o->data.wp.end;
          int *w=o->data.wp.weights;
          for(int i=a;i<=e;i++) ord+=pGetExp(p,i)*w[i-a];
          p->exp[o->data.wp.place]=ord;
          break;
        }
        case ro_cp:
        {
          int a,e;
          a=o->data.cp.start;
          e=o->data.cp.end;
          int pl=o->data.cp.place;
          for(int i=a;i<=e;i++) { p->exp[pl]=pGetExp(p,i); pl++; }
          break;
        }
        case ro_syzcomp:
        {
          int c=pGetComp(p);
          long sc  = ShiftedComponents[Components[c]];
          assume(c == 0 || Components[c] != 0);
          assume(c == 0 || sc != 0);
          p->exp[o->data.syzcomp.place]=sc;
          break;
        }
        default:
          Print("wrong ord in rSetm:%d\n",o->ord_typ);
          return;
      }
      pos++;
      if(pos==currRing->OrdSize) return;
    }
  }
}

/* -------------------------------------------------------- */
/*2
* change all global variables to fit the description of the new ring
*/


void pSetGlobals(ring r, BOOLEAN complete)
{
  int i;
  if (ppNoether!=NULL) pDelete(&ppNoether);
  pVariables = r->N;
  pOrdSgn = r->OrdSgn;
  pFDeg=r->pFDeg;
  pLDeg=r->pLDeg;
  pLexOrder=r->LexOrder;
  
  if (complete)
  {
    if ((r->LexOrder) || (r->OrdSgn==-1))
      test &= ~Sy_bit(OPT_REDTAIL); /* noredTail */
  }
}


/*2
* assumes that the head term of b is a multiple of the head term of a
* and return the multiplicant *m
*/
poly pDivide(poly a, poly b)
{
  int i;
  poly result = pInit();

  for(i=(int)pVariables; i; i--)
    pSetExp(result,i, pGetExp(a,i)- pGetExp(b,i));
  pSetComp(result, pGetComp(a) - pGetComp(b));
  pSetm(result);
  return result;
}

/*2
* divides a by the monomial b, ignores monomials wihich are not divisible
* assumes that b is not NULL
*/
poly pDivideM(poly a, poly b)
{
  if (a==NULL) return NULL;
  poly result=a;
  poly prev=NULL;
  int i;
  number inv=nInvers(pGetCoeff(b));

  while (a!=NULL)
  {
    if (pDivisibleBy(b,a))
    {
      for(i=(int)pVariables; i; i--)
         pSubExp(a,i, pGetExp(b,i));
      pSubComp(a, pGetComp(b));
      pSetm(a);
      prev=a;
      pIter(a);
    }
    else
    {
      if (prev==NULL)
      {
        pDeleteLm(&result);
        a=result;
      }
      else
      {
        pDeleteLm(&pNext(prev));
        a=pNext(prev);
      }
    }
  }
  pMult_nn(result,inv);
  nDelete(&inv);
  pDelete(&b);
  return result;
}

/*2
* returns the LCM of the head terms of a and b in *m
*/
void pLcm(poly a, poly b, poly m)
{
  int i;
  for (i=pVariables; i; i--)
  {
    pSetExp(m,i, max( pGetExp(a,i), pGetExp(b,i)));
  }
  pSetComp(m, max(pGetComp(a), pGetComp(b)));
  /* Don't do a pSetm here, otherwise hres/lres chockes */
}

/*2
* convert monomial given as string to poly, e.g. 1x3y5z
*/
poly pmInit(char *st, BOOLEAN &ok)
{
  int i,j;
  ok=FALSE;
  BOOLEAN b=FALSE;
  poly rc = pInit();
  char *s = nRead(st,&(rc->coef));
  if (s==st)
  /* i.e. it does not start with a coeff: test if it is a ringvar*/
  {
    j = rIsRingVar(s);
    if (j >= 0)
    {
      pIncrExp(rc,1+j);
      goto done;
    }
  }
  else
    b=TRUE;
  while (*s!='\0')
  {
    char ss[2];
    ss[0] = *s++;
    ss[1] = '\0';
    j = rIsRingVar(ss);
    if (j >= 0)
    {
      s = eati(s,&i);
      pAddExp(rc,1+j, (Exponent_t)i);
    }
    else
    {
      if ((s!=st)&&isdigit(st[0]))
      {
        errorreported=TRUE;
      }
      pDelete(&rc);
      return NULL;
    }
  }
done:
  ok=!errorreported;
  if (nIsZero(pGetCoeff(rc))) pDeleteLm(&rc);
  else
  {
    pSetm(rc);
  }
  return rc;
}

/*2
*make p homgeneous by multiplying the monomials by powers of x_varnum
*/
poly pHomogen (poly p, int varnum)
{
  poly q=NULL;
  poly res;
  int  o,ii;

  if (p!=NULL)
  {
    if ((varnum < 1) || (varnum > pVariables))
    {
      return NULL;
    }
    o=pWTotaldegree(p);
    q=pNext(p);
    while (q != NULL)
    {
      ii=pWTotaldegree(q);
      if (ii>o) o=ii;
      pIter(q);
    }
    q = pCopy(p);
    res = q;
    while (q != NULL)
    {
      ii = o-pWTotaldegree(q);
      if (ii!=0)
      {
        pAddExp(q,varnum, (Exponent_t)ii);
        pSetm(q);
      }
      pIter(q);
    }
    q = pOrdPolyInsertSetm(res);
  }
  return q;
}


/*2
*replaces the maximal powers of the leading monomial of p2 in p1 by
*the same powers of n, utility for dehomogenization
*/
poly pDehomogen (poly p1,poly p2,number n)
{
  polyset P;
  int     SizeOfSet=5;
  int     i;
  poly    p;
  number  nn;

  P = (polyset)omAlloc0(5*sizeof(poly));
  //for (i=0; i<5; i++)
  //{
  //  P[i] = NULL;
  //}
  pCancelPolyByMonom(p1,p2,&P,&SizeOfSet);
  p = P[0];
  //P[0] = NULL ;// for safety, may be remoeved later
  for (i=1; i<SizeOfSet; i++)
  {
    if (P[i] != NULL)
    {
      nPower(n,i,&nn);
      pMult_nn(P[i],nn);
      p = pAdd(p,P[i]);
      //P[i] =NULL; // for safety, may be removed later
      nDelete(&nn);
    }
  }
  omFreeSize((ADDRESS)P,SizeOfSet*sizeof(poly));
  return p;
}

/*4
*Returns the exponent of the maximal power of the leading monomial of
*p2 in that of p1
*/
static int pGetMaxPower (poly p1,poly p2)
{
  int     i,k,res = 32000; /*a very large integer*/

  if (p1 == NULL) return 0;
  for (i=1; i<=pVariables; i++)
  {
    if ( pGetExp(p2,i) != 0)
    {
      k =  pGetExp(p1,i) /  pGetExp(p2,i);
      if (k < res) res = k;
    }
  }
  return res;
}

/*2
*Returns as i-th entry of P the coefficient of the (i-1) power of
*the leading monomial of p2 in p1
*/
void pCancelPolyByMonom (poly p1,poly p2,polyset * P,int * SizeOfSet)
{
  int   maxPow;
  poly  p,qp,Coeff;

  if (*P == NULL)
  {
    *P = (polyset) omAlloc(5*sizeof(poly));
    *SizeOfSet = 5;
  }
  p = pCopy(p1);
  while (p != NULL)
  {
    qp = p->next;
    p->next = NULL;
    maxPow = pGetMaxPower(p,p2);
    Coeff = pDivByMonom(p,p2);
    if (maxPow > *SizeOfSet)
    {
      pEnlargeSet(P,*SizeOfSet,maxPow+1-*SizeOfSet);
      *SizeOfSet = maxPow+1;
    }
    (*P)[maxPow] = pAdd((*P)[maxPow],Coeff);
    pDelete(&p);
    p = qp;
  }
}

/*2
*returns the leading monomial of p1 divided by the maximal power of that
*of p2
*/
poly pDivByMonom (poly p1,poly p2)
{
  int     k, i;

  if (p1 == NULL) return NULL;
  k = pGetMaxPower(p1,p2);
  if (k == 0)
    return pHead(p1);
  else
  {
    number n;
    poly p = pInit();

    p->next = NULL;
    for (i=1; i<=pVariables; i++)
    {
       pSetExp(p,i, pGetExp(p1,i)-k* pGetExp(p2,i));
    }
    nPower(p2->coef,k,&n);
    pSetCoeff0(p,nDiv(p1->coef,n));
    nDelete(&n);
    pSetm(p);
    return p;
  }
}
/*----------utilities for syzygies--------------*/
poly pTakeOutComp(poly * p, int k)
{
  poly q = *p,qq=NULL,result = NULL;

  if (q==NULL) return NULL;
  if (pGetComp(q)==k)
  {
    result = q;
    while ((q!=NULL) && (pGetComp(q)==k))
    {
      pSetComp(q,0);
      pSetmComp(q);
      qq = q;
      pIter(q);
    }
    *p = q;
    pNext(qq) = NULL;
  }
  if (q==NULL) return result;
  if (pGetComp(q) > k)
  {
    pDecrComp(q);
    pSetmComp(q);
  }
  poly pNext_q;
  while ((pNext_q=pNext(q))!=NULL)
  {
    if (pGetComp(pNext_q)==k)
    {
      if (result==NULL)
      {
        result = pNext_q;
        qq = result;
      }
      else
      {
        pNext(qq) = pNext_q;
        pIter(qq);
      }
      pNext(q) = pNext(pNext_q);
      pNext(qq) =NULL;
      pSetComp(qq,0);
      pSetmComp(qq);
    }
    else
    {
      /*pIter(q);*/ q=pNext_q;
      if (pGetComp(q) > k)
      {
        pDecrComp(q);
        pSetmComp(q);
      }
    }
  }
  return result;
}

// Splits *p into two polys: *q which consists of all monoms with
// component == comp and *p of all other monoms *lq == pLength(*q)
void pTakeOutComp(poly *r_p, Exponent_t comp, poly *r_q, int *lq)
{
  spolyrec pp, qq;
  poly p, q, p_prev;
  int l = 0;

#ifdef HAVE_ASSUME
  int lp = pLength(*r_p);
#endif

  pNext(&pp) = *r_p;
  p = *r_p;
  p_prev = &pp;
  q = &qq;

  while(p != NULL)
  {
    while (pGetComp(p) == comp)
    {
      pNext(q) = p;
      pIter(q);
      pSetComp(p, 0);
      pSetmComp(p);
      pIter(p);
      l++;
      if (p == NULL)
      {
        pNext(p_prev) = NULL;
        goto Finish;
      }
    }
    pNext(p_prev) = p;
    p_prev = p;
    pIter(p);
  }

  Finish:
  pNext(q) = NULL;
  *r_p = pNext(&pp);
  *r_q = pNext(&qq);
  *lq = l;
#ifdef HAVE_ASSUME
  assume(pLength(*r_p) + pLength(*r_q) == lp);
#endif
  pTest(*r_p);
  pTest(*r_q);
}

void pDecrOrdTakeOutComp(poly *r_p, Exponent_t comp, Order_t order,
                         poly *r_q, int *lq)
{
  spolyrec pp, qq;
  poly p, q, p_prev;
  int l = 0;

  pNext(&pp) = *r_p;
  p = *r_p;
  p_prev = &pp;
  q = &qq;

#ifdef HAVE_ASSUME
  if (p != NULL)
  {
    while (pNext(p) != NULL)
    {
      assume(pGetOrder(p) >= pGetOrder(pNext(p)));
      pIter(p);
    }
  }
  p = *r_p;
#endif

  while (p != NULL && pGetOrder(p) > order) pIter(p);

  while(p != NULL && pGetOrder(p) == order)
  {
    while (pGetComp(p) == comp)
    {
      pNext(q) = p;
      pIter(q);
      pIter(p);
      pSetComp(p, 0);
      pSetmComp(p);
      l++;
      if (p == NULL || pGetOrder(p) != order)
      {
        pNext(p_prev) = p;
        goto Finish;
      }
    }
    pNext(p_prev) = p;
    p_prev = p;
    pIter(p);
  }

  Finish:
  pNext(q) = NULL;
  *r_p = pNext(&pp);
  *r_q = pNext(&qq);
  *lq = l;
}

#if 1
poly pTakeOutComp1(poly * p, int k)
{
  poly q = *p;

  if (q==NULL) return NULL;

  poly qq=NULL,result = NULL;

  if (pGetComp(q)==k)
  {
    result = q; /* *p */
    while ((q!=NULL) && (pGetComp(q)==k))
    {
      pSetComp(q,0);
      pSetmComp(q);
      qq = q;
      pIter(q);
    }
    *p = q;
    pNext(qq) = NULL;
  }
  if (q==NULL) return result;
//  if (pGetComp(q) > k) pGetComp(q)--;
  while (pNext(q)!=NULL)
  {
    if (pGetComp(pNext(q))==k)
    {
      if (result==NULL)
      {
        result = pNext(q);
        qq = result;
      }
      else
      {
        pNext(qq) = pNext(q);
        pIter(qq);
      }
      pNext(q) = pNext(pNext(q));
      pNext(qq) =NULL;
      pSetComp(qq,0);
      pSetmComp(qq);
    }
    else
    {
      pIter(q);
//      if (pGetComp(q) > k) pGetComp(q)--;
    }
  }
  return result;
}
#endif

void pDeleteComp(poly * p,int k)
{
  poly q;

  while ((*p!=NULL) && (pGetComp(*p)==k)) pDeleteLm(p);
  if (*p==NULL) return;
  q = *p;
  if (pGetComp(q)>k)
  {
    pDecrComp(q);
    pSetmComp(q);
  }
  while (pNext(q)!=NULL)
  {
    if (pGetComp(pNext(q))==k)
      pDeleteLm(&(pNext(q)));
    else
    {
      pIter(q);
      if (pGetComp(q)>k)
      {
        pDecrComp(q);
        pSetmComp(q);
      }
    }
  }
}
/*----------end of utilities for syzygies--------------*/

/*2
* pair has no common factor ? or is no polynomial
*/
BOOLEAN pHasNotCF(poly p1, poly p2)
{

  if (pGetComp(p1) > 0 || pGetComp(p2) > 0)
    return FALSE;
  int i = 1;
  loop
  {
    if ((pGetExp(p1, i) > 0) && (pGetExp(p2, i) > 0))   return FALSE;
    if (i == pVariables)                                return TRUE;
    i++;
  }
}


/*2
*divides p1 by its leading monomial
*/
void pNorm(poly p1)
{
  poly h;
  number k, c;

  if (p1!=NULL)
  {
    if (!nIsOne(pGetCoeff(p1)))
    {
      nNormalize(pGetCoeff(p1));
      k=pGetCoeff(p1);
      c = nInit(1);
      pSetCoeff0(p1,c);
      h = pNext(p1);
      while (h!=NULL)
      {
        c=nDiv(pGetCoeff(h),k);
        if (!nIsOne(c)) nNormalize(c);
        pSetCoeff(h,c);
        pIter(h);
      }
      nDelete(&k);
    }
    else
    {
      h = pNext(p1);
      while (h!=NULL)
      {
        nNormalize(pGetCoeff(h));
        pIter(h);
      }
    }
  }
}

/*2
*normalize all coeffizients
*/
void pNormalize(poly p)
{
  if (rField_has_simple_inverse()) return; /* Z/p, GF(p,n), R, long R/C */
  while (p!=NULL)
  {
    nTest(pGetCoeff(p));
    nNormalize(pGetCoeff(p));
    pIter(p);
  }
}

// splits p into polys with Exp(n) == 0 and Exp(n) != 0
// Poly with Exp(n) != 0 is reversed
static void pSplitAndReversePoly(poly p, int n, poly *non_zero, poly *zero)
{
  if (p == NULL)
  {
    *non_zero = NULL;
    *zero = NULL;
    return;
  }
  spolyrec sz;
  poly z, n_z, next;
  z = &sz;
  n_z = NULL;

  while(p != NULL)
  {
    next = pNext(p);
    if (pGetExp(p, n) == 0)
    {
      pNext(z) = p;
      pIter(z);
    }
    else
    {
      pNext(p) = n_z;
      n_z = p;
    }
    p = next;
  }
  pNext(z) = NULL;
  *zero = pNext(&sz);
  *non_zero = n_z;
  return;
}

/*3
* substitute the n-th variable by 1 in p
* destroy p
*/
static poly pSubst1 (poly p,int n)
{
  poly qq,result = NULL;
  poly zero, non_zero;

  // reverse, so that add is likely to be linear
  pSplitAndReversePoly(p, n, &non_zero, &zero);

  while (non_zero != NULL)
  {
    assume(pGetExp(non_zero, n) != 0);
    qq = non_zero;
    pIter(non_zero);
    qq->next = NULL;
    pSetExp(qq,n,0);
    pSetm(qq);
    result = pAdd(result,qq);
  }
  p = pAdd(result, zero);
  pTest(p);
  return p;
}

/*3
* substitute the n-th variable by number e in p
* destroy p
*/
static poly pSubst2 (poly p,int n, number e)
{
  assume( ! nIsZero(e) );
  poly qq,result = NULL;
  number nn, nm;
  poly zero, non_zero;

  // reverse, so that add is likely to be linear
  pSplitAndReversePoly(p, n, &non_zero, &zero);

  while (non_zero != NULL)
  {
    assume(pGetExp(non_zero, n) != 0);
    qq = non_zero;
    pIter(non_zero);
    qq->next = NULL;
    nPower(e, pGetExp(qq, n), &nn);
    nm = nMult(nn, pGetCoeff(qq));
    pSetCoeff(qq, nm);
    nDelete(&nn);
    pSetExp(qq, n, 0);
    pSetm(qq);
    result = pAdd(result,qq);
  }
  p = pAdd(result, zero);
  pTest(p);
  return p;
}


/* delete monoms whose n-th exponent is different from zero */
poly pSubst0(poly p, int n)
{
  spolyrec res;
  poly h = &res;
  pNext(h) = p;

  while (pNext(h)!=NULL)
  {
    if (pGetExp(pNext(h),n)!=0)
    {
      pDeleteLm(&pNext(h));
    }
    else
    {
      pIter(h);
    }
  }
  pTest(pNext(&res));
  return pNext(&res);
}

/*2
* substitute the n-th variable by e in p
* destroy p
*/
poly pSubst(poly p, int n, poly e)
{
  if (e == NULL) return pSubst0(p, n);

  if (pIsConstant(e))
  {
    if (nIsOne(pGetCoeff(e))) return pSubst1(p,n);
    else return pSubst2(p, n, pGetCoeff(e));
  }

  int exponent,i;
  poly h, res, m;
  Exponent_t *me,*ee;
  number nu,nu1;

  me=(Exponent_t *)omAlloc((pVariables+1)*sizeof(Exponent_t));
  ee=(Exponent_t *)omAlloc((pVariables+1)*sizeof(Exponent_t));
  if (e!=NULL) pGetExpV(e,ee);
  res=NULL;
  h=p;
  while (h!=NULL)
  {
    if ((e!=NULL) || (pGetExp(h,n)==0))
    {
      m=pHead(h);
      pGetExpV(m,me);
      exponent=me[n];
      me[n]=0;
      for(i=pVariables;i>0;i--)
        me[i]+=exponent*ee[i];
      pSetExpV(m,me);
      if (e!=NULL)
      {
        nPower(pGetCoeff(e),exponent,&nu);
        nu1=nMult(pGetCoeff(m),nu);
        nDelete(&nu);
        pSetCoeff(m,nu1);
      }
      res=pAdd(res,m);
    }
    pDeleteLm(&h);
  }
  omFreeSize((ADDRESS)me,(pVariables+1)*sizeof(Exponent_t));
  omFreeSize((ADDRESS)ee,(pVariables+1)*sizeof(Exponent_t));
  return res;
}

BOOLEAN pCompareChain (poly p,poly p1,poly p2,poly lcm)
{
  int k, j;

  if (lcm==NULL) return FALSE;

  for (j=pVariables; j; j--)
    if ( pGetExp(p,j) >  pGetExp(lcm,j)) return FALSE;
  if ( pGetComp(p) !=  pGetComp(lcm)) return FALSE;
  for (j=pVariables; j; j--)
  {
    if (pGetExp(p1,j)!=pGetExp(lcm,j))
    {
      if (pGetExp(p,j)!=pGetExp(lcm,j))
      {
        for (k=pVariables; k>j; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p2,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        for (k=j-1; k; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p2,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        return FALSE;
      }
    }
    else if (pGetExp(p2,j)!=pGetExp(lcm,j))
    {
      if (pGetExp(p,j)!=pGetExp(lcm,j))
      {
        for (k=pVariables; k>j; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p1,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        for (k=j-1; k!=0 ; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p1,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        return FALSE;
      }
    }
  }
  return FALSE;
}


