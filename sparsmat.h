#ifndef SPARSEMAT_H
#define SPARSEMAT_H
/*******************************************************************
 *  Computer Algebra System SINGULAR
 *
 *  sparsmat.h: operations with sparse matrices
 *          (bareis, ...)
 *
 *******************************************************************/
/* $Id: sparsmat.h,v 1.2 1999-06-02 14:09:36 pohl Exp $ */


poly smMult(poly, poly);
void smPolyDiv(poly, poly);
poly smMultDiv(poly, poly, const poly);
void smSpecialPolyDiv(poly, poly);

poly smCallDet(ideal I);
lists smCallBareiss(ideal smat, int x, int y);
lists smCallNewBareiss(ideal smat, int x, int y);

#endif
