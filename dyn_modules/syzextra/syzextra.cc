// -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
/*****************************************************************************\
 * Computer Algebra System SINGULAR    
\*****************************************************************************/
/** @file syzextra.cc
 * 
 * Here we implement the Computation of Syzygies
 *
 * ABSTRACT: Computation of Syzygies due to Schreyer
 *
 * @author Oleksandr Motsak
 *
 **/
/*****************************************************************************/

// include header file
#include <kernel/mod2.h>

#include "syzextra.h"

#include "DebugPrint.h"

#include <omalloc/omalloc.h>

#include <misc/intvec.h>
#include <misc/options.h>

#include <coeffs/coeffs.h>

#include <polys/monomials/p_polys.h>
#include <polys/monomials/ring.h>

#include <kernel/kstd1.h>
#include <kernel/polys.h>
#include <kernel/syz.h>
#include <kernel/ideals.h>



#include <Singular/tok.h>
#include <Singular/ipid.h>
#include <Singular/lists.h>
#include <Singular/attrib.h>

#include <Singular/ipid.h> 
#include <Singular/ipshell.h> // For iiAddCproc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// USING_NAMESPACE_SINGULARXX;
USING_NAMESPACE( SINGULARXXNAME :: DEBUG )


BEGIN_NAMESPACE_SINGULARXX     BEGIN_NAMESPACE(SYZEXTRA)


BEGIN_NAMESPACE(SORT_c_ds)


#ifdef _GNU_SOURCE
static int cmp_c_ds(const void *p1, const void *p2, void *R)
{
#else
static int cmp_c_ds(const void *p1, const void *p2)
{
  void *R = currRing; 
#endif

  const int YES = 1;
  const int NO = -1;

  const ring r =  (const ring) R; // TODO/NOTE: the structure is known: C, lp!!!

  assume( r == currRing );

  const poly a = *(const poly*)p1;
  const poly b = *(const poly*)p2;

  assume( a != NULL );
  assume( b != NULL );

  assume( p_LmTest(a, r) );
  assume( p_LmTest(b, r) );


  const signed long iCompDiff = p_GetComp(a, r) - p_GetComp(b, r);

  // TODO: test this!!!!!!!!!!!!!!!!

  //return -( compare (c, qsorts) )

#ifndef NDEBUG
  const int __DEBUG__ = 0;
  if( __DEBUG__ )
  {
    PrintS("cmp_c_ds: a, b: \np1: "); dPrint(a, r, r, 2);
    PrintS("b: "); dPrint(b, r, r, 2);
    PrintLn();
  }
#endif


  if( iCompDiff > 0 )
    return YES;

  if( iCompDiff < 0 )
    return  NO;

  assume( iCompDiff == 0 );

  const signed long iDegDiff = p_Totaldegree(a, r) - p_Totaldegree(b, r);

  if( iDegDiff > 0 )
    return YES;

  if( iDegDiff < 0 )
    return  NO;

  assume( iDegDiff == 0 );

#ifndef NDEBUG
  if( __DEBUG__ )
  {
    PrintS("cmp_c_ds: a & b have the same comp & deg! "); PrintLn();
  }
#endif

  for (int v = rVar(r); v > 0; v--)
  {
    assume( v > 0 );
    assume( v <= rVar(r) );

    const signed int d = p_GetExp(a, v, r) - p_GetExp(b, v, r);

    if( d > 0 )
      return YES;

    if( d < 0 )
      return NO;

    assume( d == 0 );
  }

  return 0;  
}

END_NAMESPACE
/* namespace SORT_c_ds */

/// return a new term: leading coeff * leading monomial of p
/// with 0 leading component!
poly leadmonom(const poly p, const ring r)
{
  poly m = NULL;

  if( p != NULL )
  {
    assume( p != NULL );
    assume( p_LmTest(p, r) );

    m = p_LmInit(p, r);
    p_SetCoeff0(m, n_Copy(p_GetCoeff(p, r), r), r);

    p_SetComp(m, 0, r);
    p_Setm(m, r);

    assume( p_GetComp(m, r) == 0 );
    assume( m != NULL );
    assume( pNext(m) == NULL );
    assume( p_LmTest(m, r) );
  }

  return m;
}


   
poly p_Tail(const poly p, const ring r)
{
  if( p == NULL)
    return NULL;
  else
    return p_Copy( pNext(p), r );
}


ideal id_Tail(const ideal id, const ring r)
{
  if( id == NULL)
    return NULL;

  const ideal newid = idInit(IDELEMS(id),id->rank);
  
  for (int i=IDELEMS(id) - 1; i >= 0; i--)
    newid->m[i] = p_Tail( id->m[i], r );

  newid->rank = id_RankFreeModule(newid, currRing);

  return newid;  
}



void Sort_c_ds(const ideal id, const ring r)
{
  const int sizeNew = IDELEMS(id);

#ifdef _GNU_SOURCE
#define qsort_my(m, s, ss, r, cmp) qsort_r(m, s, ss, cmp, r)
#else
#define qsort_my(m, s, ss, r, cmp) qsort_r(m, s, ss, cmp)
#endif
  
  if( sizeNew >= 2 )
    qsort_my(id->m, sizeNew, sizeof(poly), r, FROM_NAMESPACE(SORT_c_ds, cmp_c_ds));

#undef qsort_my

  id->rank = id_RankFreeModule(id, r);
}


ideal SchreyerSyzygyComputation::Compute1LeadingSyzygyTerms()
{
  const ideal& id = m_idLeads;
  const ring& r = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;


  //   const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
//   const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
  const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
//   const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
//   const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  assume(!__LEAD2SYZ__);

  // 1. set of components S?
  // 2. for each component c from S: set of indices of leading terms
  // with this component?
  // 3. short exp. vectors for each leading term?

  const int size = IDELEMS(id);

  if( size < 2 )
  {
    const ideal newid = idInit(1, 0); newid->m[0] = NULL; // zero ideal...        
    return newid;
  }


  // TODO/NOTE: input is supposed to be (reverse-) sorted wrt "(c,ds)"!??

  // components should come in groups: count elements in each group
  // && estimate the real size!!!


  // use just a vector instead???
  const ideal newid = idInit( (size * (size-1))/2, size); // maximal size: ideal case!

  int k = 0;

  for (int j = 0; j < size; j++)
  {
    const poly p = id->m[j];
    assume( p != NULL );
    const int  c = p_GetComp(p, r);

    for (int i = j - 1; i >= 0; i--)
    {
      const poly pp = id->m[i];
      assume( pp != NULL );
      const int  cc = p_GetComp(pp, r);

      if( c != cc )
        continue;

      const poly m = p_Init(r); // p_New???

      // m = LCM(p, pp) / p! // TODO: optimize: knowing the ring structure: (C/lp)!
      for (int v = rVar(r); v > 0; v--)
      {
        assume( v > 0 );
        assume( v <= rVar(r) );

        const short e1 = p_GetExp(p , v, r);
        const short e2 = p_GetExp(pp, v, r);

        if( e1 >= e2 )
          p_SetExp(m, v, 0, r);
        else
          p_SetExp(m, v, e2 - e1, r);

      }

      assume( (j > i) && (i >= 0) );

      p_SetComp(m, j + 1, r);
      pNext(m) = NULL;
      p_SetCoeff0(m, n_Init(1, r->cf), r); // for later...

      p_Setm(m, r); // should not do anything!!!

      newid->m[k++] = m;
    }
  }

//   if( __DEBUG__ && FALSE )
//   {
//     PrintS("ComputeLeadingSyzygyTerms::Temp0: \n");
//     dPrint(newid, r, r, 1);
//   }

  // the rest of newid is assumed to be zeroes...

  // simplify(newid, 2 + 32)??
  // sort(newid, "C,ds")[1]???
  id_DelDiv(newid, r); // #define SIMPL_LMDIV 32

//   if( __DEBUG__ && FALSE )
//   {
//     PrintS("ComputeLeadingSyzygyTerms::Temp1: \n");
//     dPrint(newid, r, r, 1);
//   }

  idSkipZeroes(newid); // #define SIMPL_NULL 2

//   if( __DEBUG__ )
//   {
//     PrintS("ComputeLeadingSyzygyTerms::Output: \n");
//     dPrint(newid, r, r, 1);
//   }

  Sort_c_ds(newid, r);

  return newid;
}

ideal SchreyerSyzygyComputation::Compute2LeadingSyzygyTerms()
{
  const ideal& id = m_idLeads;
  const ring& r = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;
  
//   const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
//   const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
//   const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
//   const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
  const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;
  

  // 1. set of components S?
  // 2. for each component c from S: set of indices of leading terms
  // with this component?
  // 3. short exp. vectors for each leading term?

  const int size = IDELEMS(id);

  if( size < 2 )
  {
    const ideal newid = idInit(1, 1); newid->m[0] = NULL; // zero module...   
    return newid;
  }


    // TODO/NOTE: input is supposed to be sorted wrt "C,ds"!??

    // components should come in groups: count elements in each group
    // && estimate the real size!!!


    // use just a vector instead???
  ideal newid = idInit( (size * (size-1))/2, size); // maximal size: ideal case!

  int k = 0;

  for (int j = 0; j < size; j++)
  {
    const poly p = id->m[j];
    assume( p != NULL );
    const int  c = p_GetComp(p, r);

    for (int i = j - 1; i >= 0; i--)
    {
      const poly pp = id->m[i];
      assume( pp != NULL );
      const int  cc = p_GetComp(pp, r);

      if( c != cc )
        continue;

        // allocate memory & zero it out!
      const poly m = p_Init(r); const poly mm = p_Init(r);


        // m = LCM(p, pp) / p! mm = LCM(p, pp) / pp!
        // TODO: optimize: knowing the ring structure: (C/lp)!

      for (int v = rVar(r); v > 0; v--)
      {
        assume( v > 0 );
        assume( v <= rVar(r) );

        const short e1 = p_GetExp(p , v, r);
        const short e2 = p_GetExp(pp, v, r);

        if( e1 >= e2 )
          p_SetExp(mm, v, e1 - e2, r); //            p_SetExp(m, v, 0, r);
        else
          p_SetExp(m, v, e2 - e1, r); //            p_SetExp(mm, v, 0, r);

      }

      assume( (j > i) && (i >= 0) );

      p_SetComp(m, j + 1, r);
      p_SetComp(mm, i + 1, r);

      const number& lc1 = p_GetCoeff(p , r);
      const number& lc2 = p_GetCoeff(pp, r);

      number g = n_Lcm( lc1, lc2, r );

      p_SetCoeff0(m ,       n_Div(g, lc1, r), r);
      p_SetCoeff0(mm, n_Neg(n_Div(g, lc2, r), r), r);

      n_Delete(&g, r);

      p_Setm(m, r); // should not do anything!!!
      p_Setm(mm, r); // should not do anything!!!

      pNext(m) = mm; //        pNext(mm) = NULL;

      newid->m[k++] = m;
    }
  }

//   if( __DEBUG__ && FALSE )
//   {
//     PrintS("Compute2LeadingSyzygyTerms::Temp0: \n");
//     dPrint(newid, r, r, 1);
//   }

  if( !__TAILREDSYZ__ )
  {
      // simplify(newid, 2 + 32)??
      // sort(newid, "C,ds")[1]???
    id_DelDiv(newid, r); // #define SIMPL_LMDIV 32

//     if( __DEBUG__ && FALSE )
//     {
//       PrintS("Compute2LeadingSyzygyTerms::Temp1 (deldiv): \n");
//       dPrint(newid, r, r, 1);
//     }
  }
  else
  {
      //      option(redSB); option(redTail);
      //      TEST_OPT_REDSB
      //      TEST_OPT_REDTAIL
    assume( r == currRing );
    BITSET _save_test = test;
    test |= (Sy_bit(OPT_REDTAIL) | Sy_bit(OPT_REDSB));

    intvec* w=new intvec(IDELEMS(newid));
    ideal tmp = kStd(newid, currQuotient, isHomog, &w);
    delete w;

    test = _save_test;

    id_Delete(&newid, r);
    newid = tmp;

//     if( __DEBUG__ && FALSE )
//     {
//       PrintS("Compute2LeadingSyzygyTerms::Temp1 (std): \n");
//       dPrint(newid, r, r, 1);
//     }

  }

  idSkipZeroes(newid);

  Sort_c_ds(newid, r);
  
  return newid;
}

void SchreyerSyzygyComputation::ComputeSyzygy()
{
//  FROM_NAMESPACE(INTERNAL, _ComputeSyzygy(m_idLeads, m_idTails, m_syzLeads, m_syzTails, m_rBaseRing, m_atttributes)); // TODO: just a wrapper for now :/

  assume( m_idLeads != NULL );
  assume( m_idTails != NULL );
  
  const ideal& L = m_idLeads;
  const ideal& T = m_idTails;

  ideal& TT = m_syzTails;
  const ring& R = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;

//  const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
//  const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
  const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
  const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
  const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  assume( R == currRing ); // For attributes :-/

  assume( IDELEMS(L) == IDELEMS(T) );

  ComputeLeadingSyzygyTerms( __LEAD2SYZ__ ); // 2 terms OR 1 term!

  ideal& LL = m_syzLeads;
  
  const int size = IDELEMS(LL);

  TT = idInit(size, 0);

  if( size == 1 && LL->m[0] == NULL )
    return;


  ideal LS = NULL;

  if( __TAILREDSYZ__ )
    LS = LL;

  for( int k = size - 1; k >= 0; k-- )
  {
    const poly a = LL->m[k]; assume( a != NULL );

    const int r = p_GetComp(a, R) - 1; 

    assume( r >= 0 && r < IDELEMS(T) );
    assume( r >= 0 && r < IDELEMS(L) );

    poly aa = leadmonom(a, R); assume( aa != NULL); // :(    

    poly a2 = pNext(a);    

    if( a2 != NULL )
    {
      TT->m[k] = a2; pNext(a) = NULL;
    }

    if( ! __HYBRIDNF__ )
    {
      poly t = TraverseTail(aa, T->m[r]);

      if( a2 != NULL )
      {
        assume( __LEAD2SYZ__ );

        const int r2 = p_GetComp(a2, R) - 1; poly aa2 = leadmonom(a2, R); // :(

        assume( r2 >= 0 && r2 < IDELEMS(T) );

        TT->m[k] = p_Add_q(a2, p_Add_q(t, TraverseTail(aa2, T->m[r2]), R), R);

        p_Delete(&aa2, R);
      } else
        TT->m[k] = p_Add_q(t, ReduceTerm(aa, L->m[r], a), R);

    } else
    {
      if( a2 == NULL )
      {
        aa = p_Mult_mm(aa, L->m[r], R);
        a2 = FindReducer(aa, a); 
      }
      assume( a2 != NULL );

      TT->m[k] = SchreyerSyzygyNF(a, a2); // will copy a2 :(

      p_Delete(&a2, R);
    }
    p_Delete(&aa, R);    
  }

  TT->rank = id_RankFreeModule(TT, R);
}

void SchreyerSyzygyComputation::ComputeLeadingSyzygyTerms(bool bComputeSecondTerms)
{
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;

  const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
  const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  if( bComputeSecondTerms )
//    m_syzLeads = FROM_NAMESPACE(INTERNAL, _Compute2LeadingSyzygyTerms(m_idLeads, m_rBaseRing, m_atttributes));
    m_syzLeads = Compute2LeadingSyzygyTerms();
  else
    m_syzLeads = Compute1LeadingSyzygyTerms();
//    m_syzLeads = FROM_NAMESPACE(INTERNAL, _ComputeLeadingSyzygyTerms(m_idLeads, m_rBaseRing, m_atttributes));
  
  // NOTE: set m_LS if tails are to be reduced!

  if (__TAILREDSYZ__)
    m_LS = m_syzLeads;
}

poly SchreyerSyzygyComputation::FindReducer(poly product, poly syzterm) const
{
//  return FROM_NAMESPACE(INTERNAL, _FindReducer(product, syzterm, m_idLeads, m_LS, m_rBaseRing, m_atttributes));
//  poly _FindReducer(poly product, poly syzterm,
//                     ideal L, ideal LS,
//                     const ring r,
//                     const SchreyerSyzygyComputationFlags attributes)

  const ideal& L = m_idLeads;
  const ideal& LS = m_LS;
  const ring& r = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;


  const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
  const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
//   const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
//   const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
  const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  assume( product != NULL );
  assume( L != NULL );

  int c = 0;

  if (syzterm != NULL)
    c = p_GetComp(syzterm, r) - 1;

  assume( c >= 0 && c < IDELEMS(L) );

  if (__SYZCHECK__ && syzterm != NULL)
  {
    const poly m = L->m[c];

    assume( m != NULL ); assume( pNext(m) == NULL );

    poly lm = p_Mult_mm(leadmonom(syzterm, r), m, r);
    assume( p_EqualPolys(lm, product, r) );

    //  def @@c = leadcomp(syzterm); int @@r = int(@@c);
    //  def @@product = leadmonomial(syzterm) * L[@@r];

    p_Delete(&lm, r);    
  }

  // looking for an appropriate diviser q = L[k]...
  for( int k = IDELEMS(L)-1; k>= 0; k-- )
  {
    const poly p = L->m[k];    

    // ... which divides the product, looking for the _1st_ appropriate one!
    if( !p_LmDivisibleBy(p, product, r) )
      continue;


    const poly q = p_New(r);
    pNext(q) = NULL;
    p_ExpVectorDiff(q, product, p, r); // (LM(product) / LM(L[k]))
    p_SetComp(q, k + 1, r);
    p_Setm(q, r);

    // cannot allow something like: a*gen(i) - a*gen(i)
    if (syzterm != NULL && (k == c))
      if (p_ExpVectorEqual(syzterm, q, r))
      {
        if( __DEBUG__ )
        {
          Print("_FindReducer::Test SYZTERM: q == syzterm !:((, syzterm is: ");
          dPrint(syzterm, r, r, 1);
        }    

        p_LmFree(q, r);
        continue;
      }

    // while the complement (the fraction) is not reducible by leading syzygies 
    if( LS != NULL )
    {
      assume( __TAILREDSYZ__ );
      BOOLEAN ok = TRUE;

      for(int kk = IDELEMS(LS)-1; kk>= 0; kk-- )
      {
        const poly pp = LS->m[kk];

        if( p_LmDivisibleBy(pp, q, r) )
        {

          if( __DEBUG__ )
          {
            Print("_FindReducer::Test LS: q is divisible by LS[%d] !:((, diviser is: ", kk+1);
            dPrint(pp, r, r, 1);
          }    

          ok = FALSE; // q in <LS> :((
          break;                 
        }
      }

      if(!ok)
      {
        p_LmFree(q, r);
        continue;
      }
    }

    p_SetCoeff0(q, n_Neg( n_Div( p_GetCoeff(product, r), p_GetCoeff(p, r), r), r), r);
    return q;

  }


  return NULL;
}


poly SchreyerSyzygyComputation::SchreyerSyzygyNF(poly syz_lead, poly syz_2) const
{
// return FROM_NAMESPACE(INTERNAL, _SchreyerSyzygyNF(syz_lead, syz_2, m_idLeads, m_idTails, m_LS, m_rBaseRing, m_atttributes));
// poly _SchreyerSyzygyNF(poly syz_lead, poly syz_2,
//                       ideal L, ideal T, ideal LS,
//                       const ring r,
//                       const SchreyerSyzygyComputationFlags attributes)
// {

  const ideal& L = m_idLeads;
  const ideal& T = m_idTails;
  const ideal& LS = m_LS;
  const ring& r = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;

  
//   const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
//   const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
//   const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
//   const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
//   const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  assume( syz_lead != NULL );
  assume( syz_2 != NULL );

  assume( L != NULL );
  assume( T != NULL );

  assume( IDELEMS(L) == IDELEMS(T) );

  int  c = p_GetComp(syz_lead, r) - 1;

  assume( c >= 0 && c < IDELEMS(T) );

  poly p = leadmonom(syz_lead, r); // :(  
  poly spoly = pp_Mult_qq(p, T->m[c], r);
  p_Delete(&p, r);


  c = p_GetComp(syz_2, r) - 1;
  assume( c >= 0 && c < IDELEMS(T) );

  p = leadmonom(syz_2, r); // :(
  spoly = p_Add_q(spoly, pp_Mult_qq(p, T->m[c], r), r);
  p_Delete(&p, r);

  poly tail = p_Copy(syz_2, r); // TODO: use bucket!?

  while (spoly != NULL)
  {
    poly t = FindReducer(spoly, NULL);

    p_LmDelete(&spoly, r);

    if( t != NULL )
    {
      p = leadmonom(t, r); // :(
      c = p_GetComp(t, r) - 1;

      assume( c >= 0 && c < IDELEMS(T) );

      spoly = p_Add_q(spoly, pp_Mult_qq(p, T->m[c], r), r);

      p_Delete(&p, r);

      tail = p_Add_q(tail, t, r);
    }    
  }

  return tail;
}


poly SchreyerSyzygyComputation::TraverseTail(poly multiplier, poly tail) const
{
  const ideal& L = m_idLeads;
  const ideal& T = m_idTails;
  const ideal& LS = m_LS;
  const ring& r = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;

//  return FROM_NAMESPACE(INTERNAL, _TraverseTail(multiplier, tail, m_idLeads, m_idTails, m_LS, m_rBaseRing, m_atttributes));
// poly _TraverseTail(poly multiplier, poly tail, 
//                    ideal L, ideal T, ideal LS,
//                    const ring r,
//                    const SchreyerSyzygyComputationFlags attributes)
// {


//   const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
//   const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
//   const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
//   const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
//   const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  assume( multiplier != NULL );

  assume( L != NULL );
  assume( T != NULL );

  poly s = NULL;

  // iterate over the tail
  for(poly p = tail; p != NULL; p = pNext(p))
    s = p_Add_q(s, ReduceTerm(multiplier, p, NULL), r);  

  return s;
}




poly SchreyerSyzygyComputation::ReduceTerm(poly multiplier, poly term4reduction, poly syztermCheck) const
{
  const ideal& L = m_idLeads;
  const ideal& T = m_idTails;
  const ideal& LS = m_LS;
  const ring& r = m_rBaseRing;
  const SchreyerSyzygyComputationFlags& attributes = m_atttributes;

//  return FROM_NAMESPACE(INTERNAL, _ReduceTerm(multiplier, term4reduction, syztermCheck, m_idLeads, m_idTails, m_LS, m_rBaseRing, m_atttributes));
// poly _ReduceTerm(poly multiplier, poly term4reduction, poly syztermCheck,
//                 ideal L, ideal T, ideal LS,
//                 const ring r,
//                 const SchreyerSyzygyComputationFlags attributes)



//   const BOOLEAN __DEBUG__      = attributes.__DEBUG__;
//   const BOOLEAN __SYZCHECK__   = attributes.__SYZCHECK__;
//   const BOOLEAN __LEAD2SYZ__   = attributes.__LEAD2SYZ__;
//   const BOOLEAN __HYBRIDNF__   = attributes.__HYBRIDNF__;
//   const BOOLEAN __TAILREDSYZ__ = attributes.__TAILREDSYZ__;

  assume( multiplier != NULL );
  assume( term4reduction != NULL );


  assume( L != NULL );
  assume( T != NULL );

  // assume(r == currRing); // ?

  // simple implementation with FindReducer:
  poly s = NULL;

  if(1)
  {
    // NOTE: only LT(term4reduction) should be used in the following:
    poly product = pp_Mult_mm(multiplier, term4reduction, r);
    s = FindReducer(product, syztermCheck);
    p_Delete(&product, r);
  }

  if( s == NULL ) // No Reducer?
    return s;

  poly b = leadmonom(s, r);

  const int c = p_GetComp(s, r) - 1;
  assume( c >= 0 && c < IDELEMS(T) );

  const poly tail = T->m[c];

  if( tail != NULL )
    s = p_Add_q(s, TraverseTail(b, tail), r);  

  return s;
}





BEGIN_NAMESPACE_NONAME
   
static inline int atGetInt(idhdl rootRingHdl, const char* attribute, int def)
{
  return ((int)(long)(atGet(rootRingHdl, attribute, INT_CMD, (void*)def)));
}

END_NAMESPACE   

SchreyerSyzygyComputationFlags::SchreyerSyzygyComputationFlags(idhdl rootRingHdl):
#ifndef NDEBUG
    __DEBUG__( (BOOLEAN)atGetInt(rootRingHdl,"DEBUG", TRUE) ),
#else
    __DEBUG__( (BOOLEAN)atGetInt(rootRingHdl,"DEBUG", FALSE) ),
#endif
    __SYZCHECK__( (BOOLEAN)atGetInt(rootRingHdl, "SYZCHECK", __DEBUG__) ),
    __LEAD2SYZ__( (BOOLEAN)atGetInt(rootRingHdl, "LEAD2SYZ", 1) ), 
    __TAILREDSYZ__( (BOOLEAN)atGetInt(rootRingHdl, "TAILREDSYZ", 1) ), 
    __HYBRIDNF__( (BOOLEAN)atGetInt(rootRingHdl, "HYBRIDNF", 0) ) 
{    
  if( __DEBUG__ )
  {
    PrintS("SchreyerSyzygyComputationFlags: \n");
    Print("   DEBUG     : \t%d\n", __DEBUG__);
    Print("   SYZCHECK  : \t%d\n", __SYZCHECK__);
    Print("   LEAD2SYZ  : \t%d\n", __LEAD2SYZ__);
    Print("   TAILREDSYZ: \t%d\n", __TAILREDSYZ__);
  }

  // TODO: just current setting!
  assume( rootRingHdl == currRingHdl );
  assume( rootRingHdl->typ == RING_CMD );
  assume( rootRingHdl->data.uring == currRing );
  // move the global ring here inside???
}

   


END_NAMESPACE               END_NAMESPACE_SINGULARXX


// Vi-modeline: vim: filetype=c:syntax:shiftwidth=2:tabstop=8:textwidth=0:expandtab