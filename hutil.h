#ifndef HUTIL_H
#define HUTIL_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT
*/

typedef short * scmon;
typedef scmon * scfmon;
typedef int * varset;
struct monrec;
typedef struct monrec monh;
typedef monh * monp;
typedef monp * monf;
struct monrec{  scfmon mo;
  int a;
};

#define LEN_MON (sizeof(scfmon) + sizeof(int))

extern scfmon hexist, hstc, hrad, hwork;
extern scmon hpure, hpur0;
extern varset hvar, hsel;
extern int hNexist, hNstc, hNrad, hNvar, hNpure;
extern short hisModule;
extern monf stcmem, radmem;

scfmon hInit(ideal S, ideal Q, int * Nexist);
void hComp(scfmon exist, int Nexist, short ak, scfmon stc, int * Nstc);
void hSupp(scfmon stc, int Nstc, varset var, int * Nvar);
void hOrdSupp(scfmon stc, int Nstc, varset var, int Nvar);
void hStaircase(scfmon stc, int * Nstc, varset var, int Nvar);
void hRadical(scfmon rad, int * Nrad, int Nvar);
void hLexS(scfmon stc, int Nstc, varset var, int Nvar);
void hLexR(scfmon rad, int Nrad, varset var, int Nvar);
void hPure(scfmon stc, int a, int *Nstc, varset var, int Nvar,
 scmon pure, int *Npure);
void hElimS(scfmon stc, int * e1, int a2, int e2,varset var, int Nvar);
void hElimR(scfmon rad, int * e1, int a2, int e2,varset var, int Nvar);
void hLex2S(scfmon stc, int e1, int a2, int e2,varset var,
 int Nvar, scfmon w);
void hLex2R(scfmon rad, int e1, int a2, int e2,varset var,
 int Nvar, scfmon w);
void hStepS(scfmon stc, int Nstc, varset var, int Nvar,int *a, short *x);
void hStepR(scfmon rad, int Nrad, varset var, int Nvar,int *a);
monf hCreate(int Nvar);
void hKill(monf xmem, int Nvar);
scfmon hGetmem(int lm, scfmon old, monp monmem);
scmon hGetpure(scmon p);

#endif
