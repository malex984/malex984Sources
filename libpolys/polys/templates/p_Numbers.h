/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    p_Numbers.h
 *  Purpose: macros/inline functions for number operations
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 8/00
 *******************************************************************/
#ifndef P_NUMBERS_H
#define P_NUMBERS_H

#include <coeffs/coeffs.h>
#include <coeffs/numbers.h>
#include <polys/monomials/ring.h>

static inline number n_Copy_FieldGeneral(number n,    const ring r)
{ assume(r != NULL); return n_Copy(n,r->cf); }

static inline void   n_Delete_FieldGeneral(number* p, const ring r)
{ assume(r != NULL); n_Delete(p,r->cf); }

static inline number n_Mult_FieldGeneral(number n1, number n2, const ring r)
{ assume(r != NULL); return n_Mult(n1, n2, r->cf); }

static inline number n_Add_FieldGeneral(number n1, number n2, const ring r)
{ assume(r != NULL); return n_Add(n1, n2, r->cf); }

static inline BOOLEAN n_IsZero_FieldGeneral(number n, const ring r)
{ assume(r != NULL); return n_IsZero(n, r->cf); }

static inline BOOLEAN n_Equal_FieldGeneral(number n1, number n2, const ring r)
{ assume(r != NULL); return n_Equal(n1, n2, r->cf); }

static inline number n_Neg_FieldGeneral(number n,     const ring r)
{ assume(r != NULL); return n_InpNeg(n, r->cf); }

static inline number n_Sub_FieldGeneral(number n1, number n2, const ring r)
{ assume(r != NULL); return n_Sub(n1, n2, r->cf); }

static inline void n_InpMult_FieldGeneral(number &n1, number n2, const ring r)
{ assume(r != NULL); n_InpMult(n1, n2, r->cf); }

static inline void n_InpAdd_FieldGeneral(number &n1, number n2, const ring r)
{ assume(r != NULL); n_InpAdd(n1, n2, r->cf); }

#ifdef HAVE_RINGS
#define n_Copy_RingGeneral(n, r)           n_Copy_FieldGeneral(n, r)
#define n_Delete_RingGeneral(n, r)         n_Delete_FieldGeneral(n, r)
#define n_Mult_RingGeneral(n1, n2, r)      n_Mult_FieldGeneral(n1, n2, r)
#define n_Add_RingGeneral(n1, n2, r)       n_Add_FieldGeneral(n1, n2, r) 
#define n_IsZero_RingGeneral(n, r)         n_IsZero_FieldGeneral(n, r)
#define n_Equal_RingGeneral(n1, n2, r)     n_Equal_FieldGeneral(n1, n2, r)
#define n_Neg_RingGeneral(n, r)            n_Neg_FieldGeneral(n, r)
#define n_Sub_RingGeneral(n1, n2, r)       n_Sub_FieldGeneral(n1, n2, r)
//#define n_InpMult_RingGeneral(n1, n2, r)   n_InpMult_FieldGeneral(n1, n2, r)
#define n_InpMult_RingGeneral(n1, n2, r)   n_InpMult_FieldGeneral(n1, n2, r)

static inline void n_InpAdd_RingGeneral(number &n1, number n2, const ring r)
{ assume(r != NULL);  assume(rField_is_Ring(r)); n_InpAdd(n1, n2, r->cf); }
#endif

#include <coeffs/modulop.h>

#define n_Copy_FieldZp(n, r)        n
#define n_Delete_FieldZp(n, r)      do {} while (0)

static inline number n_Mult_FieldZp(number n1, number n2, const ring r)
{ STATISTIC(n_Mult); assume(r != NULL); return npMultM(n1, n2, r->cf); }

static inline number n_Add_FieldZp(number n1, number n2, const ring r)
{ STATISTIC(n_Add); assume(r != NULL); return npAddM(n1, n2, r->cf); }

static inline number n_Sub_FieldZp(number n1, number n2, const ring r)
{ STATISTIC(n_Sub); assume(r != NULL); return npSubM(n1, n2, r->cf); }

static inline BOOLEAN n_IsZero_FieldZp(number n, const ring r)
{ STATISTIC(n_IsZero); assume(r != NULL); return npIsZeroM(n, r->cf); }

static inline BOOLEAN n_Equal_FieldZp(number n1, number n2, const ring r)
{ STATISTIC(n_Equal); assume(r != NULL); return  npEqualM(n1, n2, r->cf); }

static inline number n_Neg_FieldZp(number n,     const ring r)
{ STATISTIC(n_InpNeg); assume(r != NULL); return npNegM(n, r->cf); }

static inline void n_InpMult_FieldZp(number &n1, number n2, const ring r)
{ STATISTIC(n_InpMult); assume(r != NULL); n1=npMultM(n1, n2, r->cf);  }

static inline void n_InpAdd_FieldZp(number &n1, number n2, const ring r)
{ STATISTIC(n_InpAdd); assume(r != NULL); assume(rField_is_Zp(r)); n1=npAddM(n1, n2, r->cf); }

#define DO_LINLINE
#include <coeffs/longrat.cc> // for inlining... TODO: fix this Uglyness?!!!

static inline number n_Copy_FieldQ(number n,    const ring r)
{ STATISTIC(n_Copy); assume(r != NULL); return nlCopy(n, r->cf); }

static inline void   n_Delete_FieldQ(number* n, const ring r)
{ STATISTIC(n_Delete); assume(r != NULL); nlDelete(n,r->cf); }

static inline number n_Mult_FieldQ(number n1, number n2, const ring r)
{ STATISTIC(n_Mult); assume(r != NULL); return nlMult(n1,n2, r->cf); }

static inline number n_Add_FieldQ(number n1, number n2, const ring r)
{ STATISTIC(n_Add); assume(r != NULL); return nlAdd(n1, n2, r->cf); }

static inline number n_Sub_FieldQ(number n1, number n2, const ring r)
{ STATISTIC(n_Sub); assume(r != NULL); return nlSub(n1, n2, r->cf); }

static inline BOOLEAN n_IsZero_FieldQ(number n, const ring r)
{ STATISTIC(n_IsZero); assume(r != NULL); return nlIsZero(n, r->cf); }

static inline BOOLEAN n_Equal_FieldQ(number n1, number n2, const ring r)
{ STATISTIC(n_Equal); assume(r != NULL); return  nlEqual(n1, n2, r->cf); }

static inline number n_Neg_FieldQ(number n,     const ring r)
{ STATISTIC(n_InpNeg); assume(r != NULL); return nlNeg(n, r->cf); }

static inline void n_InpMult_FieldQ(number &n1, number n2, const ring r)
{ STATISTIC(n_InpMult); assume(r != NULL); nlInpMult(n1, n2, r->cf); }

static inline void n_InpAdd_FieldQ(number &n1, number n2, const ring r)
{ STATISTIC(n_InpAdd); assume(r != NULL); assume(rField_is_Q(r)); nlInpAdd(n1, n2, r->cf); }
#endif
