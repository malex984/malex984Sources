#ifndef GRING_H
#define GRING_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: gring.h,v 1.22 2003-10-17 19:54:15 levandov Exp $ */
/*
* ABSTRACT additional defines etc for --with-plural
*/

#ifdef HAVE_PLURAL
#include "structs.h"

void ncKill(ring r);

#define UPMATELEM(i,j,nVar) ( (nVar * ((i)-1) - ((i) * ((i)-1))/2 + (j)-1)-(i) )
// poly functions defined in p_Procs :
poly nc_pp_Mult_mm(poly p, poly m, const ring r, poly &last);
poly nc_p_Mult_mm(poly p, poly m, const ring r);
poly nc_p_Minus_mm_Mult_qq(poly p, const poly m, poly q, const ring r);
// other routines we need in addition :
poly nc_p_Mult_mm_Common(poly p, const poly m, int side, const ring r);
poly nc_mm_Mult_p(const poly m, poly p, const ring r);
poly nc_mm_Mult_nn (int *F, int *G, const ring r); 
poly nc_mm_Mult_uu (int *F,int jG,int bG, const ring r);

/* #define nc_uu_Mult_ww nc_uu_Mult_ww_vert */
poly nc_uu_Mult_ww (int i, int a, int j, int b, const ring r);
/* poly nc_uu_Mult_ww_vert (int i, int a, int j, int b, const ring r); */
/* poly nc_uu_Mult_ww_horvert (int i, int a, int j, int b, const ring r); */
/* poly nc_uu_Mult_ww_hvdiag (int i, int a, int j, int b, const ring r); */
/* not written yet */

poly _nc_p_Mult_q(poly p, poly q, const int copy, const ring r);

//copy :
poly nc_p_CopyGet(poly a, ring r);
poly nc_p_CopyPut(poly a, ring r);

//syzygies :
// former nc_spGSpolyCreate
poly nc_CreateSpoly(poly p1, poly p2, poly spNoether, const ring r);
// former nc_spGSpolyRed
poly nc_ReduceSpoly(poly p1, poly p2, poly spNoether, const ring r);
// former nc_spGSpolyRedNew
poly nc_ReduceSpolyNew(poly p1, poly p2, poly spNoether, const ring r);
// former nc_spGSpolyRedTail
void nc_ReduceSpolyTail(poly p1, poly q, poly q2, poly spNoether, const ring r);
// former nc_spShort
poly nc_CreateShortSpoly(poly p1, poly p2, const ring r=currRing);

ideal gr_bba (ideal F, ideal Q,kStrategy strat);

// brackets:
poly nc_p_Bracket_qq(poly p, poly q);
poly nc_mm_Bracket_nn(poly m1, poly m2);

//twostd:

ideal twostd(ideal I);

// complete reduction routines

//void nc_kBucketPolyRed(kBucket_pt b, poly p);
void nc_kBucketPolyRed(kBucket_pt b, poly p, number *c);
void nc_PolyPolyRed(poly &b, poly p, number *c);

matrix nc_PrintMat(int a, int b, ring r, int metric);

int nc_CheckSubalgebra(poly PolyVar, ring r);

#else
// dummy definition to make gcc happy
#define nc_kBucketPolyRed(A,B,C) 0
#define nc_PolyPolyRed(A,B,C) 0

#endif /* HAVE_PLURAL */
#endif

