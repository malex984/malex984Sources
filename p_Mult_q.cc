/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    p_Mult_q.cc
 *  Purpose: multiplication of polynomials
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 8/00
 *  Version: $Id: p_Mult_q.cc,v 1.6 2000-11-23 17:34:11 obachman Exp $
 *******************************************************************/
#include "mod2.h"

/***************************************************************
 *
 * Returns:  p * q, 
 * Destroys: if !copy then p, q
 * Assumes: pLength(p) >= 2 pLength(q) >=2
 ***************************************************************/
#include "p_polys.h"
#include "p_Procs.h"
#include "p_Numbers.h"


poly _p_Mult_q(poly p, poly q, const int copy, const ring r)
{
  assume(p != NULL && pNext(p) != NULL && q != NULL && pNext(q) != NULL);
  pAssume1(! pHaveCommonMonoms(p, q));
  p_Test(p, r);
  p_Test(q, r);

  poly res = pp_Mult_mm(p,q,r);     // holds initially q1*p
  poly qq = pNext(q);               // we iter of this
  poly qn = pp_Mult_mm(qq, p,r);    // holds p1*qi
  poly pp = pNext(p);               // used for Lm(qq)*pp
  poly rr = res;                    // last monom which is surely not NULL
  poly rn = pNext(res);             // pNext(rr)
  number n, n1;
  
  // now the main loop
  Top:
  if (rn == NULL) goto Smaller;
  p_LmCmpAction(rn, qn, r, goto Equal, goto Greater, goto Smaller);
  
  Greater:
  // rn > qn, so iter
  rr = rn;
  pIter(rn);
  goto Top;
  
  // rn < qn, append qn to rr, and compute next Lm(qq)*pp
  Smaller:
  pNext(rr) = qn;
  rr = qn;
  pIter(qn);
  Work: // compute res + Lm(qq)*pp
  if (rn == NULL)
    pNext(rr) = pp_Mult_mm(pp, qq, r);
  else
  {
    pNext(rr) = p_Plus_mm_Mult_qq(rn, qq, pp, r);
  }
  
  pIter(qq);
  if (qq == NULL) goto Finish;
  rn = pNext(rr);
  goto Top;
  
  Equal:
  n1 = pGetCoeff(rn);
  n = n_Add(n1, pGetCoeff(qn), r);
  n_Delete(&n1, r);
  if (n_IsZero(n, r))
  {
    n_Delete(&n, r);
    rn = p_LmFreeAndNext(rn, r);
  }
  else
  {
    pSetCoeff0(rn, n);
    rr = rn;
    pIter(rn);
  }
  n_Delete(&pGetCoeff(qn),r);
  qn = p_LmFreeAndNext(qn, r);
  goto Work;
  
  Finish:
  if (!copy)
  {
    p_Delete(&p, r);
    p_Delete(&q, r);
  }
  p_Test(res, r);
  return res;
}

