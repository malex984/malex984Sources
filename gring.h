#ifndef GRING_H
#define GRING_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: gring.h,v 1.15 2003-01-29 16:04:18 levandov Exp $ */
/*
* ABSTRACT additional defines etc for --with-plural
*/

#ifdef HAVE_PLURAL
#include "structs.h"

#define UPMATELEM(i,j,nVar) ( (nVar * ((i)-1) - ((i) * ((i)-1))/2 + (j)-1)-(i) )
// poly functions defined in p_Procs :
poly nc_pp_Mult_mm(poly p, poly m, const ring r,poly &last);
poly nc_p_Mult_mm(poly p, poly m, const ring r);
poly nc_p_Minus_mm_Mult_qq(poly p, const poly m, poly q, const ring r);
// other routines we need in addition :
poly nc_mm_Mult_p(const poly m, poly p, const ring r);
poly nc_mm_Mult_nn (Exponent_t *F, Exponent_t *G, const ring r); 
poly nc_mm_Mult_uu (Exponent_t *F,int jG,int bG, const ring r);

#define nc_uu_Mult_ww nc_uu_Mult_ww_horvert
poly nc_uu_Mult_ww_vert (int i, int a, int j, int b, const ring r);
poly nc_uu_Mult_ww_horvert (int i, int a, int j, int b, const ring r);
poly nc_uu_Mult_ww_hvdiag (int i, int a, int j, int b, const ring r);


poly _nc_p_Mult_q(poly p, poly q, const int copy, const ring r);

//syzygies :
poly nc_spGSpolyCreate(poly p1, poly p2,poly spNoether, const ring r);
poly nc_spGSpolyRed(poly p1, poly p2,poly spNoether, const ring r);
poly nc_spGSpolyRedNew(poly p1, poly p2,poly spNoether, const ring r);
void nc_spGSpolyRedTail(poly p1, poly q, poly q2, poly spNoether, const ring r);
poly nc_spShort(poly p1, poly p2, const ring r=currRing);
void nc_kBucketPolyRed(kBucket_pt b, poly p);

ideal gr_bba (ideal F, ideal Q,kStrategy strat);

// brackets:
poly nc_p_Bracket_qq(poly p, poly q);
poly nc_mm_Bracket_nn(poly m1, poly m2);

//twostd:

ideal twostd(ideal I);

// complete reduction routines

void nc_kBucketPolyRed(kBucket_pt b, poly p, number *c);
void nc_PolyPolyRed(poly &b, poly p, number *c);

matrix nc_PrintMat(int a, int b, ring r, int metric);

#else
// dummy definition to make gcc happy
#define nc_kBucketPolyRed(A,B,C) 0
#define nc_PolyPolyRed(A,B,C) 0

#endif /* HAVE_PLURAL */
#endif

