#ifndef IDEALS_H
#define IDEALS_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: ideals.h,v 1.16 1999-12-16 13:35:20 pohl Exp $ */
/*
* ABSTRACT - all basic methods to manipulate ideals
*/
#include "structs.h"

#ifdef PDEBUG
ideal idDBInit (int size, int rank, char *f, int l);
#define idInit(A,B) idDBInit(A,B,__FILE__,__LINE__)
void idDBDelete (ideal* h, char *f, int l);
#define idDelete(A) idDBDelete(A,__FILE__,__LINE__)
#else
ideal idInit (int size, int rank=1);
  /*- creates an ideal -*/
void idDelete (ideal* h);
  /*- deletes an ideal -*/
#endif
void idPrint(ideal id);
  /*- initialise an ideal -*/
ideal idMaxIdeal (int deg);
  /*- initialise the maximal ideal (at 0) -*/
void idSkipZeroes (ideal ide);
  /*gives an ideal the minimal possible size*/
void idNorm(ideal id);
void idDelMultiples(ideal id);
void idDelEquals(ideal id);
void idDelLmEquals(ideal id);
void idDelDiv(ideal id);

#ifdef PDEBUG
void idDBTest(ideal h1,char *f,int l);
#define idTest(A) idDBTest(A,__FILE__,__LINE__)
#else
#define idTest(A)  (TRUE)
#define idPrint(A) ((void)0)
#endif

#ifdef PDEBUG
ideal idDBCopy(ideal h1,char *f,int l);
#define idCopy(A) idDBCopy(A,__FILE__,__LINE__)
#else
ideal idCopy (ideal h1);
#endif
  /*adds two ideals without simplifying the result*/
ideal idSimpleAdd (ideal h1,ideal h2);
  /*adds the quotient ideal*/
ideal idAdd (ideal h1,ideal h2);
  /* h1 + h2 */
ideal idMult (ideal h1,ideal h2);
  /*hh := h1 * h2*/

BOOLEAN idIs0 (ideal h);

int     idRankFreeModule(ideal m);

BOOLEAN idHomIdeal (ideal id, ideal Q=NULL);
BOOLEAN idHomModule(ideal m, ideal Q,intvec **w);

ideal idMinBase (ideal h1);
  /*returns a minimized set of generators of h1*/
int pLowVar (poly p);
  /*-the minimal index of used variables - 1-*/
ideal idDehomogen (ideal id1,poly p,number n);
  /*dehomogenized the generators of the ideal id1 with the leading
  monomial of p replaced by n*/
void pShift (poly * p,int i);
  /*- verschiebt die Indizes der Modulerzeugenden um i -*/
void    idInitChoise (int r,int beg,int end,BOOLEAN *endch,int * choise);
void    idGetNextChoise (int r,int end,BOOLEAN *endch,int * choise);
int     idGetNumberOfChoise(int t, int d, int begin, int end, int * choise);

int     binom (int n,int r);

ideal   idFreeModule (int i);

ideal   idSect (ideal h1,ideal h2);
ideal   idMultSect(resolvente arg, int length);

//ideal   idSyzygies (ideal h1, tHomog h,intvec **w);
ideal   idSyzygies (ideal h1, tHomog h,intvec **w, BOOLEAN setSyzComp=TRUE, 
                    BOOLEAN setRegularity=FALSE, int *deg = NULL);
ideal   idLiftStd  (ideal h1, matrix *m, tHomog h=testHomog);

ideal   idLift (ideal mod, ideal sumod);
ideal   idLiftNonStB (ideal  mod, ideal submod,BOOLEAN goodShape=FALSE);

intvec * idMWLift(ideal mod,intvec * weights);

ideal   idQuot (ideal h1,ideal h2,
                BOOLEAN h1IsStb=FALSE, BOOLEAN resultIsIdeal=FALSE);

ideal   idPower(ideal gid,int deg);

//ideal   idElimination (ideal h1,poly delVar);
ideal   idElimination (ideal h1,poly delVar, intvec *hilb=NULL);

ideal   idMinors(matrix a, int ar, ideal R=NULL);

ideal   idCompactify(ideal id);

ideal idMinEmbedding(ideal arg,BOOLEAN inPlace=FALSE);

ideal   idHead(ideal h);

ideal   idHomogen(ideal h, int varnum);

BOOLEAN idIsSubModule(ideal id1,ideal id2);

ideal   idVec2Ideal(poly vec);

ideal   idMatrix2Module(matrix mat);

matrix  idModule2Matrix(ideal mod);

matrix  idModule2formatedMatrix(ideal mod,int rows, int cols);

ideal   idSubst(ideal i, int n, poly e);

ideal   idJet(ideal i,int d);
ideal   idJetW(ideal i,int d, intvec * iv);

matrix  idDiff(matrix i, int k);
matrix  idDiffOp(ideal I, ideal J,BOOLEAN multiply=TRUE);

intvec *idSort(ideal id,BOOLEAN nolex=TRUE);
ideal   idModulo (ideal h1,ideal h2);
int     idElem(ideal F);
matrix  idCoeffOfKBase(ideal arg, ideal kbase, poly how);
intvec *idQHomWeights(ideal id);
ideal   idTransp(ideal a);
#endif
