#ifndef IPSHELL_H
#define IPSHELL_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: ipshell.h,v 1.55 2009-07-28 14:18:34 Singular Exp $ */
/*
* ABSTRACT
*/
#include <stdio.h>
#include "structs.h"


extern int  traceit ;
#define TRACE_SHOW_PROC   1
#define TRACE_SHOW_LINENO 2
#define TRACE_SHOW_LINE   4
#define TRACE_SHOW_RINGS  8
#define TRACE_SHOW_LINE1  16
#define TRACE_BREAKPOINT  32
#define TRACE_TMP_BREAKPOINT  64
extern leftv iiCurrArgs;
extern idhdl iiCurrProc;
extern int iiOp; /* the current operation*/
extern int  myynest;
extern const char *  currid;
extern int     iiRETURNEXPR_len;
extern sleftv *iiRETURNEXPR;
#ifdef USE_IILOCALRING
extern ring   *iiLocalRing;
#endif
extern cmdnames cmds[];
extern const char *lastreserved;
extern const char *singular_date;

extern BOOLEAN yyInRingConstruction; /* 1: during ring construction */

int     IsCmd(const char *n, int & tok);
short   IsCmdToken(short tok);

BOOLEAN iiPStart(idhdl pn, sleftv * sl);
BOOLEAN iiEStart(char* example, procinfo *pi);
void    type_cmd(idhdl h);
void    test_cmd(int i);
void    list_cmd(int typ, const char* what, const char * prefix,
                 BOOLEAN iterate, BOOLEAN fullname=FALSE);
//char *  iiStringMatrix(matrix im, int dim, char ch=',');
void    killlocals(int v);
int     exprlist_length(leftv v);
const char *  Tok2Cmdname(int i);
const char *  iiTwoOps(int t);
int     IsPrime(int i);

BOOLEAN iiWRITE(leftv res,leftv exprlist);
BOOLEAN iiExport(leftv v, int toLev);
BOOLEAN iiExport(leftv v, int toLev, idhdl roothdl);
BOOLEAN iiInternalExport (leftv v, int toLev, idhdl roothdl);
char *  iiGetLibName(procinfov v);
char *  iiGetLibProcBuffer( procinfov pi, int part=1 );
char *  iiProcName(char *buf, char & ct, char* &e);
char *  iiProcArgs(char *e,BOOLEAN withParenth);
BOOLEAN iiLibCmd( char *newlib, BOOLEAN autoexport, BOOLEAN tellerror, BOOLEAN force );
/* sees wheter library lib has already been loaded
   if yes, writes filename of lib into where and returns TRUE,
   if  no, returns FALSE
*/
BOOLEAN iiLocateLib(const char* lib, char* where);
leftv   iiMap(map theMap, const char * what);
void    iiMakeResolv(resolvente r, int length, int rlen, char * name, int typ0,
           intvec ** weights=NULL);
BOOLEAN jjMINRES(leftv res, leftv v);
BOOLEAN jjBETTI(leftv res, leftv v);
BOOLEAN jjBETTI2(leftv res, leftv u, leftv v);
BOOLEAN jjBETTI2_ID(leftv res, leftv u, leftv v);
BOOLEAN jjIMPORTFROM(leftv res, leftv u, leftv v);

BOOLEAN jjVARIABLES_P(leftv res, leftv u);
BOOLEAN jjVARIABLES_ID(leftv res, leftv u);

int     iiRegularity(lists L);
leftv   singular_system(sleftv h);
BOOLEAN jjSYSTEM(leftv res, leftv v);
void    iiDebug();
BOOLEAN iiCheckRing(int i);
poly    iiHighCorner(ideal i, int ak);
char *  iiConvName(const char *libname);
BOOLEAN iiLoadLIB(FILE *fp, char *libnamebuf, char *newlib,
                         idhdl pl, BOOLEAN autoexport, BOOLEAN tellerror);


/* ================================================================== */
/* Expressions : */
BOOLEAN iiExprArith1(leftv res, sleftv* a, int op);
BOOLEAN iiExprArith2(leftv res, sleftv* a, int op, sleftv* b,
                     BOOLEAN proccall=FALSE);
BOOLEAN iiExprArith3(leftv res, int op, leftv a, leftv b, leftv c);
BOOLEAN iiExprArithM(leftv res, sleftv* a, int op);

typedef BOOLEAN (*proc1)(leftv,leftv);

#ifdef __GNUC__
#if (__GNUC__ < 3)
#define INIT_BUG 1
void    jjInitTab1();
#endif
#endif

#ifdef GENTABLE
typedef char * (*Proc1)(char *);
struct sValCmd1
{
  proc1 p;
  short cmd;
  short res;
  short arg;
  short valid_for;
};

typedef BOOLEAN (*proc2)(leftv,leftv,leftv);
struct sValCmd2
{
  proc2 p;
  short cmd;
  short res;
  short arg1;
  short arg2;
  short valid_for;
};

typedef BOOLEAN (*proc3)(leftv,leftv,leftv,leftv);
struct sValCmd3
{
  proc3 p;
  short cmd;
  short res;
  short arg1;
  short arg2;
  short arg3;
  short valid_for;
};
struct sValCmdM
{
  proc1 p;
  short cmd;
  short res;
  short number_of_args; /* -1: any, -2: any >0, .. */
  short valid_for;
};
extern struct sValCmd2 dArith2[];
extern struct sValCmd1 dArith1[];
extern struct sValCmd3 dArith3[];
extern struct sValCmdM dArithM[];
#endif

/* ================================================================== */
/* Assigments : */
BOOLEAN iiAssign(leftv left, leftv right);

typedef BOOLEAN (*proci)(leftv,leftv,Subexpr);
struct sValAssign_sys
{
  proc1 p;
  short res;
  short arg;
};

struct sValAssign
{
  proci p;
  short res;
  short arg;
};

BOOLEAN iiParameter(leftv p);
/* ================================================================== */
int     iiDeclCommand(leftv sy, leftv name, int lev, int t, idhdl* root,
  BOOLEAN isring = FALSE, BOOLEAN init_b=TRUE);
sleftv * iiMake_proc(idhdl pn, package pack, sleftv* sl);
// from misc.cc:
char *  showOption();
BOOLEAN setOption(leftv res, leftv v);
/* ================================================================== */
char * versionString();
/* ================================================================== */
void  singular_example(char *str);

BOOLEAN iiTryLoadLib(leftv v, const char *id);

void listall(int showproc=1);
void iiCheckPack(package &p);
#ifndef NDEBUG
void checkall();
#endif
#endif

