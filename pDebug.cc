/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    pDebug.h
 *  Purpose: implementation of debug related poly routines
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 8/00
 *  Version: $Id: pDebug.cc,v 1.3 2000-09-14 14:20:44 obachman Exp $
 *******************************************************************/

#ifndef PDEBUG_CC
#define PDEBUG_CC
#include <stdarg.h>
#include <stdio.h>

#include "mod2.h"
#ifdef PDEBUG

#include "polys.h"
#include "febase.h"
#include "omalloc.h"
#include "ring.h"


/***************************************************************
 *
 * Error reporting
 *
 ***************************************************************/
extern void wrp(poly p);
BOOLEAN dPolyReportError(poly p, ring r, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  
  fprintf(stderr, "// ***dPolyError: ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n occured at\n");
  omPrintCurrentBackTraceMax(stderr, 8);
  if (p != NULL)
  {
    if (r == currRing)
    {
      fprintf(stderr, " occured for poly: ");
      wrp(p);
      omPrintAddrInfo(stderr, p, " ");
    }
    else
    {
      omPrintAddrInfo(stderr, p, " occured for poly: ");
    }
  }
  dErrorBreak();
  return FALSE;
}

/***************************************************************
 *
 * checking for ring stuff
 *
 ***************************************************************/
BOOLEAN p_CheckIsFromRing(poly p, ring r)
{
  if (p != NULL)
  {
    void* custom = omGetCustomOfAddr(p);
    if (custom != NULL)
    {
      pPolyAssumeReturn(custom == r);
      return TRUE;
    }
    else
    {
      pPolyAssumeReturn(omIsBinPageAddr(p) && omSizeWOfAddr(p)==r->PolyBin->sizeW);
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}

BOOLEAN p_CheckPolyRing(poly p, ring r)
{
  pAssumeReturn(r != NULL && r->PolyBin != NULL);
  pAssumeReturn(p != NULL);
  return p_CheckIsFromRing(p, r);
}
BOOLEAN p_CheckRing(ring r)
{
  pAssumeReturn(r != NULL && r->PolyBin != NULL);
  return TRUE;
}

/***************************************************************
 *
 * Debugging/statistics of pDivisibleBy
 *
 ***************************************************************/
static int pDivisibleBy_number = 1;
static int pDivisibleBy_FALSE = 1;
static int pDivisibleBy_ShortFalse = 1;
BOOLEAN pDebugShortDivisibleBy(poly p1, unsigned long sev_1, ring r_1,
                               poly p2, unsigned long not_sev_2, ring r_2)
{
  _pPolyAssume(p_GetShortExpVector(p1, r_1) == sev_1, p1, r_1);
  _pPolyAssume(p_GetShortExpVector(p2, r_2) == ~ not_sev_2, p2, r_2);

  pDivisibleBy_number++;
  BOOLEAN ret = _p_DivisibleBy1(p1, r_1, p2, r_2);
  if (! ret) pDivisibleBy_FALSE++;
  if (sev_1 & not_sev_2)
  {
    pDivisibleBy_ShortFalse++;
    if (ret)
      dReportError("p1 divides p2, but sev's are wrong");
  }
  return ret;
}

void pPrintDivisbleByStat()
{
  Print("#Tests: %d; #FALSE %d(%d); #SHORT %d(%d)\n",
        pDivisibleBy_number,
        pDivisibleBy_FALSE, pDivisibleBy_FALSE*100/pDivisibleBy_number,
        pDivisibleBy_ShortFalse, pDivisibleBy_ShortFalse*100/pDivisibleBy_FALSE);
}

/***************************************************************
 *
 * Misc things helpful for debugging
 *
 ***************************************************************/
BOOLEAN pIsMonomOf(poly p, poly m)
{
  if (m == NULL) return TRUE;
  while (p != NULL)
  {
    if (p == m) return TRUE;
    pIter(p);
  }
  return FALSE;
}
BOOLEAN pHaveCommonMonoms(poly p, poly q)
{
  while (p != NULL)
  {
    if (pIsMonomOf(q, p))
    {
      return TRUE;
    }
    pIter(p);
  }
  return FALSE;
}

/***************************************************************
 *
 * Testing of polys
 *
 ***************************************************************/
static poly p_DebugInit(poly p, ring dest_ring, ring src_ring)
{
  poly d_p = p_Init(dest_ring);
  int i;
  assume(dest_ring->N == src_ring->N);
  
  for (i=1; i<= src_ring->N; i++)
  {
    p_SetExp(d_p, i, p_GetExp(p, i, src_ring), dest_ring);
  }
  if (rRing_has_Comp(dest_ring))
    p_SetComp(d_p, p_GetComp(p, src_ring), dest_ring);
  p_Setm(d_p, dest_ring);
  return d_p;
}

BOOLEAN _p_Test(poly p, ring r, int level)
{
  if (level < 0 || p == NULL) return TRUE;
  
  poly p_prev = NULL;
  
  // check addr with level+1 so as to check bin/page of addr
  if (level > 0)
    pPolyAssumeReturnMsg(omTestBinAddrSize(p, (r->PolyBin->sizeW)*SIZEOF_LONG, level+1)
                        == omError_NoError, "memory error");

  pFalseReturn(p_CheckRing(r));
  int ismod = p_GetComp(p, r) > 0;
  
  while (p != NULL)
  {
    // ring check
    pFalseReturn(p_CheckPolyRing(p, r));
    // omAddr check
    pPolyAssumeReturnMsg(omTestBinAddrSize(p, (r->PolyBin->sizeW)*SIZEOF_LONG, 1) 
                     == omError_NoError, "memory error");
    // number/coef check
    pPolyAssumeReturnMsg(p->coef != NULL || (n_GetChar(r) >= 2), "NULL coef");
    pPolyAssumeReturnMsg(!n_IsZero(p->coef, r), "Zero coef");

    // check for mix poly/vec representation 
    pPolyAssumeReturnMsg(ismod == (p_GetComp(p, r) > 0), "mixed poly/vector");

    // special check for ringorder_s/S
    if (currRing->order[1] == ringorder_S)
    {
      long c1, cc1, ccc1, ec1;
      sro_ord* o = &(currRing->typ[1]);

      c1 = pGetComp(p);
      cc1 = o->data.syzcomp.Components[c1];
      ccc1 = o->data.syzcomp.ShiftedComponents[cc1];
      pPolyAssumeReturnMsg(c1 == 0 || cc1 != 0, "Component <-> TrueComponent zero mismatch");
      pPolyAssumeReturnMsg(c1 == 0 || ccc1 != 0,"Component <-> ShiftedComponent zero mismatch");
      ec1 = p->exp[currRing->typ[1].data.syzcomp.place];
      pPolyAssumeReturnMsg(ec1 == ccc1, "Shifted comp out of sync. should %d, is %d");
    }
    
    // check that p_Setm works ok
    if (level > 0)
    {
      poly p_should_equal = p_DebugInit(p, r, r);
      pPolyAssumeReturnMsg(p_ExpVectorEqual(p, p_should_equal, r), "p_Setm field(s) out of sync");
      p_Free(p_should_equal, r);
    }
    
    // check order
    if (p_prev != NULL)
    {
      int cmp = p_LmCmp(p_prev, p, r);
      if (cmp == 0)
      {
        _pPolyAssumeReturnMsg(0, "monoms p and p->next are equal", p_prev, r);
      }
      else 
        _pPolyAssumeReturnMsg(p_LmCmp(p_prev, p, r) == 1, "wrong order", p_prev, r);

      // check that compare worked sensibly
      if (level > 1 && p_GetComp(p_prev, r) == p_GetComp(p, r))
      {
        int i;
        for (i=r->N; i>0; i--)
        {
          if (p_GetExp(p_prev, i, r) != p_GetExp(p, i, r)) break;
        }
        _pPolyAssumeReturnMsg(i > 0, "Exponents equal but compare different", p_prev, r);
      }
    }
    p_prev = p;
    pIter(p);
  }
  return TRUE;
}

BOOLEAN _p_LmTest(poly p, ring r, int level)
{
  if (level < 0 || p == NULL) return TRUE;
  poly pnext = pNext(p);
  pNext(p) = NULL;
  BOOLEAN test_res = _p_Test(p, r, level);
  pNext(p) = pnext;
  return test_res;
}

BOOLEAN _pp_Test(poly p, ring lmRing, ring tailRing, int level)
{
  if (level < 0 || p == NULL) return TRUE;
  if (pNext(p) == NULL || lmRing == tailRing) return _p_Test(p, lmRing, level);

  pFalseReturn(_p_LmTest(p, lmRing, level));
  pFalseReturn(_p_Test(pNext(p), tailRing, level));

  // check that lm > Lm(tail)
  if (level > 1)
  {
    poly lm = p;
    poly tail = p_DebugInit(pNext(p), lmRing, tailRing);
    poly pnext = pNext(lm);
    pNext(lm) = tail;
    BOOLEAN cmp = p_LmCmp(lm, tail);
    if (cmp != 1)
      dPolyReportError(lm, lmRing, "wrong order: lm <= Lm(tail)");
    pNext(lm) = pnext;
    return (cmp == 1);
  }
  return TRUE;
}
  
#endif // PDEBUG
#endif // PDEBUG_CC

