#ifndef STAIRC_H
#define STAIRC_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Header: /exports/cvsroot-2/cvsroot/Singular/stairc.h,v 1.1.1.1 1997-03-19 13:19:01 obachman Exp $ */
/* $Log: not supported by cvs2svn $
*/

#include "structs.h"

void scComputeHC(ideal s,int  k,poly &hEdge);

intvec * scIndIntvec(ideal S, ideal Q=NULL);
lists scIndIndset(ideal S, BOOLEAN all, ideal Q=NULL);
int scDimInt(ideal  s,ideal Q=NULL);
int scMultInt(ideal  s,ideal Q=NULL);
int scMult0Int(ideal  s,ideal Q=NULL);
void scDegree(ideal  s,ideal Q=NULL);

ideal scKBase(ideal  s, ideal Q=NULL);
//void scHilbertPoly(ideal  s,ideal Q=NULL);
//intvec *scHilbert(ideal  s,ideal Q=NULL);

intvec * hHstdSeries(ideal S, intvec *modulweight, ideal Q=NULL);
intvec * hFirstSeries(ideal S, intvec *modulweight, ideal Q=NULL);
intvec * hSecondSeries(intvec *hseries1);
intvec * hThirdSeries(intvec *hseries, int dim);
void hDegreeSeries(intvec *s1, intvec *s2, int *co, int *mu);
void hPrintHilb(intvec *hseries);

void hLookSeries(ideal S, intvec *modulweight, ideal Q=NULL);

#endif

