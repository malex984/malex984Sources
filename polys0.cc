/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: polys0.cc,v 1.16 2000-10-23 12:02:18 obachman Exp $ */

/*
* ABSTRACT - all basic methods to convert polynomials to strings
*/

/* includes */
#include "mod2.h"
#include "tok.h"
#include "structs.h"
#include "numbers.h"
#include "ring.h"
#include "p_polys.h"
#include "febase.h"

/*2
* writes a monomial (p),
* uses form x*gen(.) if ko != coloumn number of p
*/
static void writemon(poly p, int ko, ring r)
{
  BOOLEAN wroteCoef=FALSE,writeGen=FALSE;

  if (pGetCoeff(p)!=NULL)
    nNormalize(pGetCoeff(p));

  if (((p_GetComp(p,r) == (short)ko)
    &&(p_LmIsConstantComp(p, r)))
  || ((!nIsOne(pGetCoeff(p)))
    && (!nIsMOne(pGetCoeff(p)))
  )
  )
  {
    nWrite(p->coef);
    wroteCoef=(rShortOut(r) == FALSE ||(r->parameter!=NULL));
    writeGen=TRUE;
  }
  else if (nIsMOne(pGetCoeff(p)))
  {
    if (nGreaterZero(pGetCoeff(p)))
    {
      nWrite(p->coef);
      wroteCoef=(rShortOut(r) == FALSE ||(r->parameter!=NULL));
      writeGen=TRUE;
    }
    else
      StringAppendS("-");
  }

  int i;
  for (i=0; i<r->N; i++)
  {
    {
      Exponent_t ee = p_GetExp(p,i+1,r);
      if (ee!=0)
      {
        if (wroteCoef)
          StringAppendS("*");
        //else
          wroteCoef=(rShortOut(r) == FALSE);
        writeGen=TRUE;
        StringAppendS(rRingVar(i, r));
        if (ee != 1)
        {
          if (rShortOut(r)==0) StringAppendS("^");
          StringAppend("%d", ee);
        }
      }
    }
  }
  //StringAppend("{%d}",p->Order);
  if (p_GetComp(p, r) != (Exponent_t)ko)
  {
    if (writeGen) StringAppendS("*");
    StringAppend("gen(%d)", p_GetComp(p, r));
  }
}

char* pString0(poly p, ring lmRing, ring tailRing)
{
  if (p == NULL)
  {
    return StringAppendS("0");
  }
  if ((p_GetComp(p, lmRing) == 0) || (!lmRing->VectorOut))
  {
    writemon(p,0, lmRing);
    p = pNext(p);
    while (p!=NULL)
    {
      if ((p->coef==NULL)||nGreaterZero(p->coef))
        StringAppendS("+");
      writemon(p,0, tailRing);
      p = pNext(p);
    }
    return StringAppendS("");
  }

  Exponent_t k = 1;
  StringAppendS("[");
  loop
  {
    while (k < p_GetComp(p,lmRing))
    {
      StringAppendS("0,");
      k++;
    }
    writemon(p,k,lmRing);
    pIter(p);
    while ((p!=NULL) && (k == p_GetComp(p, tailRing)))
    {
      if (nGreaterZero(p->coef)) StringAppendS("+");
      writemon(p,k,tailRing);
      pIter(p);
    }
    if (p == NULL) break;
    StringAppendS(",");
    k++;
  }
  return StringAppendS("]");
}

char* pString(poly p, ring lmRing, ring tailRing)
{
  StringSetS("");
  return pString0(p, lmRing, tailRing);
}

/*2
* writes a polynomial p to stdout
*/
void pWrite0(poly p, ring lmRing, ring tailRing)
{
  PrintS(pString(p, lmRing, tailRing));
}

/*2
* writes a polynomial p to stdout followed by \n
*/
void pWrite(poly p, ring lmRing, ring tailRing)
{
  pWrite0(p, lmRing, tailRing);
  PrintLn();
}

/*2
*the standard debugging output:
*print the first two monomials of the poly (wrp) or only the lead term (wrp0),
*possibly followed by the string "+..."
*/
void wrp0(poly p, ring ri)
{
  poly r;

  if (p==NULL) PrintS("NULL");
  else if (pNext(p)==NULL) pWrite0(p, ri);
  else
  {
    r = pNext(p);
    pNext(p) = NULL;
    pWrite0(p, ri);
    if (r!=NULL)
    {
      PrintS("+...");
      pNext(p) = r;
    }
  }
}
void wrp(poly p, ring lmRing, ring tailRing)
{
  poly r;

  if (p==NULL) PrintS("NULL");
  else if (pNext(p)==NULL) pWrite0(p, lmRing);
  else
  {
    r = pNext(pNext(p));
    pNext(pNext(p)) = NULL;
    pWrite0(p, lmRing, tailRing);
    if (r!=NULL)
    {
      PrintS("+...");
      pNext(pNext(p)) = r;
    }
  }
}
