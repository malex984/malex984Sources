/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Log: not supported by cvs2svn $
*/

/*
* ABSTRACT: SINGULAR shell grammatik
*/
%{

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "mod2.h"
#include "tok.h"
#include "stype.h"
#include "ipid.h"
#include "intvec.h"
#include "febase.h"
#include "matpol.h"
#include "ring.h"
#include "kstd1.h"
#include "mmemory.h"
#include "subexpr.h"
#include "ipshell.h"
#include "ipconv.h"
#include "ideals.h"
#include "numbers.h"
#include "polys.h"
#include "weight.h"
#include "stairc.h"
#include "timer.h"
#include "cntrlc.h"
#include "maps.h"
#include "syz.h"
#include "lists.h"


extern int   yylineno;
extern FILE* yyin;

char       my_yylinebuf[YY_READ_BUF_SIZE];
char *     currid;
BOOLEAN    noringvars=FALSE;
BOOLEAN    expected_parms;
int        cmdtok;
int        inerror = 0;

#define TESTSETINT(a,i)                                \
   if ((a).Typ() != INT_CMD)                           \
   {                                                   \
     WerrorS("no int expression");                     \
     YYERROR;                                          \
   }                                                   \
   (i) = (int)(a).Data();

#define MYYERROR(a) { WerrorS(a); YYERROR; }

void yyerror(char * fmt)
{
  int oldInError=inerror;

  inerror = 1;
  BOOLEAN old_errorreported=errorreported;
  errorreported = TRUE;
  if (currid!=NULL)
  {
    killid(currid,&idroot);
    currid = NULL;
  }
  if(!oldInError)
  {
    Werror( "error occurred in %s line %d: `%s`"
           ,VoiceName(), yylineno-1, my_yylinebuf);
    if (cmdtok!=0)
    {
      char *s=Tok2Cmdname(cmdtok);
      if (expected_parms)
      {
        Werror("expected %s-expression. type \'help %s;\'",s,s);
      }
      else
      {
        Werror("wrong type declaration. type \'help %s;\'",s);
      }
    }
    if (!old_errorreported && (lastreserved!=NULL))
    {
      Werror("last reserved name was `%s`",lastreserved);
    }
  }
  else
    inerror=oldInError;
  if ((voice>0)&&(myynest>0))
  {
    Werror("leaving %s",VoiceName());
    exitBuffer(BT_proc);
  }
}

#ifndef alloca
#define alloca malloc
#endif

%}

/* %expect 22 */
%pure_parser

/* special symbols */
%token DOTDOT
%token EQUAL_EQUAL
%token GE
%token LE
/*%token MINUSEQUAL*/
%token MINUSMINUS
%token NOT
%token NOTEQUAL
/*%token PLUSEQUAL*/
%token PLUSPLUS

/* types, part 1 (ring indep.)*/
%token <i> DRING_CMD
%token <i> INTMAT_CMD
%token <i> PROC_CMD
%token <i> RING_CMD

/* valid when ring defined ! */
%token <i> BEGIN_RING
/* types, part 2 */
%token <i> IDEAL_CMD
%token <i> MAP_CMD
%token <i> MATRIX_CMD
%token <i> MODUL_CMD
%token <i> NUMBER_CMD
%token <i> POLY_CMD
%token <i> VECTOR_CMD
/* end types */

/* ring dependent cmd:*/
%token <i> BAREISS_CMD
%token <i> BETTI_CMD
%token <i> COEFFS_CMD
%token <i> COEF_CMD
%token <i> CONTRACT_CMD
%token <i> DEGREE_CMD
%token <i> DEG_CMD
%token <i> DET_CMD
%token <i> DIFF_CMD
%token <i> DIM_CMD
%token <i> ELIMINATION_CMD
%token <i> E_CMD
%token <i> FETCH_CMD
%token <i> FREEMODULE_CMD
%token <i> KEEPRING_CMD
%token <i> HILBERT_CMD
%token <i> HOMOG_CMD
%token <i> IMAP_CMD
%token <i> INDEPSET_CMD
%token <i> INTERRED_CMD
%token <i> INTERSECT_CMD
%token <i> JACOB_CMD
%token <i> JET_CMD
%token <i> KBASE_CMD
%token <i> KOSZUL_CMD
%token <i> LEADCOEF_CMD
%token <i> LEADEXP_CMD
%token <i> LEAD_CMD
%token <i> LIFTSTD_CMD
%token <i> LIFT_CMD
%token <i> MAXID_CMD
%token <i> MINBASE_CMD
%token <i> MINOR_CMD
%token <i> MINRES_CMD
%token <i> MODULO_CMD
%token <i> MRES_CMD
%token <i> MULTIPLICITY_CMD
%token <i> ORD_CMD
%token <i> PAR_CMD
%token <i> PARDEG_CMD
%token <i> PREIMAGE_CMD
%token <i> QUOTIENT_CMD
%token <i> QHWEIGHT_CMD
%token <i> REDUCE_CMD
%token <i> REGULARITY_CMD
%token <i> RES_CMD
%token <i> SIMPLIFY_CMD
%token <i> SORTVEC_CMD
%token <i> SRES_CMD
%token <i> STD_CMD
%token <i> SUBST_CMD
%token <i> SYZYGY_CMD
%token <i> VAR_CMD
%token <i> VDIM_CMD
%token <i> WEDGE_CMD
%token <i> WEIGHT_CMD

/*system variables in ring block*/
%token <i> VALTVARS
%token <i> VMAXDEG
%token <i> VMAXMULT
%token <i> VNOETHER
%token <i> VMINPOLY

%token <i> END_RING
/* end of ring definitions */

%token <i> CMD_1
%token <i> CMD_2
%token <i> CMD_3
%token <i> CMD_12
%token <i> CMD_23
%token <i> CMD_123
%token <i> CMD_M
%token <i> ROOT_DECL
        /* put variables of this type into the idroot list */
%token <i> ROOT_DECL_LIST
        /* put variables of this type into the idroot list */
%token <i> RING_DECL
        /* put variables of this type into the currRing list */
%token <i> EXAMPLE_CMD
%token <i> EXECUTE_CMD
%token <i> EXPORT_CMD
%token <i> HELP_CMD
%token <i> KILL_CMD
%token <i> LIB_CMD
%token <i> LISTVAR_CMD
%token <i> SETRING_CMD
%token <i> TYPE_CMD

%token <name> STRINGTOK BLOCKTOK INT_CONST
%token <name> UNKNOWN_IDENT RINGVAR PROC_DEF

/* control */
%token <i> BREAK_CMD
%token <i> CONTINUE_CMD
%token <i> ELSE_CMD
%token <i> EVAL
%token <i> QUOTE
%token <i> EXIT_CMD
%token <i> FOR_CMD
%token <i> IF_CMD
%token <i> SYS_BREAK
%token <i> WHILE_CMD
%token <i> RETURN
%token <i> END_PROC
%token <i> PARAMETER

/* system variables */
%token <i> SYSVAR

%type <i>    intexpr

%type <name> extendedid
%type <lv>   rlist ordering OrderingList orderelem
%type <name> stringexpr
%type <lv>   expr elemexpr exprlist expr_arithmetic
%type <lv>   declare_ip_variable left_value
%type <i>    ordername
%type <i>    cmdeq
%type <i>    currring_lists
%type <i>    setrings
%type <i>    ringcmd1

%type <i>    '=' '<' '>' '+' '-'
%type <i>    '*' '/' '[' ']' '.' '^' ',' ';'


/*%nonassoc '=' PLUSEQUAL DOTDOT*/
%nonassoc '=' DOTDOT
%left '|' '&'
%left EQUAL_EQUAL NOTEQUAL
%left '<' '>' GE LE
%left '+' '-'
%left '*' '/' '%'
%left UMINUS
%left  '^'
%left '[' ']'
%left '(' ')'
%left PLUSPLUS
%left MINUSMINUS

%%
lines:
        /**/
        | lines pprompt
          {
            if (timerv
            #ifdef SIC
            && (!sic)
            #endif
            )
            {
              writeTime("used time:");
              startTimer();
            }
            prompt_char = '>';
            currid = NULL;
            if (errorreported)
            {
              yyerror("");
            }
            if (inerror==2) PrintLn();
            errorreported = inerror = cmdtok = 0;
            lastreserved=NULL;
            expected_parms=FALSE;
            noringvars = FALSE;
            test_int();
          }
        ;

pprompt:
        flowctrl                       /* if, while, for etc */
        | command ';'                  /* commands returning no value */
          {ifswitch[voice]=0;}
        | assign  ';'                  /* general assignment */
          {ifswitch[voice]=0;}
        | parametercmd ';'             /* proc parameter assignments */
          {yylineno--;ifswitch[voice]=0;}
        | declare_ip_variable ';'      /* default initialization */
          { $1.CleanUp(); ifswitch[voice]=0;}
        | filecmd
          {ifswitch[voice]=0;}
        | returncmd
          {
            #ifdef SIC
            if (!sic)
            #endif
            YYACCEPT;
          }
        | EXIT_CMD
          {
            ifswitch[voice]=0;
            Print("exit found\n");
            YYACCEPT;
          }
        | SYS_BREAK
          {
            #ifdef SIC
            if (sic)
              emStoreOp(SYS_BREAK,0);
            else
            #endif
            {
              ifswitch[voice]=0;
              iiDebug();
            }
          }
        | ';'                    /* ignore empty statements */
          {ifswitch[voice]=0;}
        | error ';'
          {
            #ifdef SIQ
            siq=0;
            #endif
            ifswitch[voice]=0;
            if (inerror&&($1.i<UMINUS) && ($1.i>' '))
            {
              // 1: yyerror called
              // 2: scanner put actual string
              // 3: error rule put token+\n
              inerror=3;
              Print(" error at token `%s`\n",iiTwoOps($1.i));
            }
            if (!errorreported) WerrorS("...parse error");
            yyerror("");
            yyerrok;
            #ifdef SIC
            if (sic) YYABORT;
            #endif
            if (myynest>0)
            {
              //WerrorS("leaving yyparse");
              YYABORT;
            }
            else if (voice>0)
            {
              //WerrorS("leaving voice");
              exitBuffer(BT_break);
            }
            errorreported = FALSE;
            lastreserved=NULL;
          }
        ;

flowctrl: ifcmd
          | whilecmd
          | forcmd
          | proccmd
            {ifswitch[voice]=0;}
        ;

command: executecmd | helpcmd | killcmd | exportcmd | listcmd
         | ringcmd | scriptcmd | setringcmd | typecmd
         ;

assign: left_value exprlist
          {
            if(iiAssign(&$1,&$2)) YYERROR;
          }
        ;

elemexpr:
        RINGVAR
          {
            #ifdef SIC
            if (!sic)
            #endif
              if (currRing==NULL) MYYERROR("no ring active");
            syMake(&$$,$1);
          }
        | extendedid
          {
            syMake(&$$,$1);
          }
        | elemexpr '('  ')'
          {
            if(iiExprArith1(&$$,&$1,'(')) YYERROR;
          }
        | elemexpr '(' exprlist ')'
          {
            $1.next=(leftv)Alloc(sizeof(sleftv));
            memcpy($1.next,&$3,sizeof(sleftv));
            if(iiExprArithM(&$$,&$1,'(')) YYERROR;
          }
        | elemexpr '.' extendedid
          {
            idhdl r=idroot;
            if ($1.Typ()!=PACKAGE_CMD) MYYERROR("<package>.<id> expected");
            idroot=IDPACKAGE((idhdl)$1.data)->idroot;
            syMake(&$$,$3);
            idroot=r;
          }
        | '[' exprlist ']'
          {
            if (currRingHdl==NULL) MYYERROR("no ring active");
            int j = 0;
            memset(&$$,0,sizeof(sleftv));
            $$.rtyp=VECTOR_CMD;
            leftv v = &$2;
            while (v!=NULL)
            {
              poly p = pOne();
              pSetCompP(p,++j);
              int k = (int)(v->Data());
              if (k!=0)
              {
                if (v->Typ() == INT_CMD)
                {
                  $$.data = (void *)pAdd((poly)$$.data,pMult(p,pISet(k)));
                }
                else if (v->Typ() == POLY_CMD)
                {
                  $$.data = (void *)pAdd((poly)$$.data,pMult(p, (poly)(v->CopyD())) );
                }
                else
                  MYYERROR("expected '[poly,...'");
              }
              else
                pDelete1(&p);
              v=v->next;
            }
            $2.CleanUp();
          }
        | intexpr
          {
            memset(&$$,0,sizeof($$));
            $$.rtyp  = INT_CMD;
            $$.data = (void *)$1;
          }
        | SYSVAR
          {
            memset(&$$,0,sizeof($$));
            $$.rtyp = $1;
            $$.data = $$.Data();
          }
        | stringexpr
          {
            memset(&$$,0,sizeof($$));
            $$.rtyp  = STRING_CMD;
            $$.data = $1;
          }
        ;

exprlist:
        expr ',' exprlist
          {
            leftv v = &$1;
            while (v->next!=NULL)
            {
              v=v->next;
            }
            v->next = (leftv)Alloc(sizeof(sleftv));
            memcpy(v->next,&($3),sizeof(sleftv));
            $$ = $1;
          }
        | expr
          {
            $$ = $1;
          }
        | '(' expr ',' exprlist ')'
          {
            leftv v = &$2;
            while (v->next!=NULL)
            {
              v=v->next;
            }
            v->next = (leftv)Alloc(sizeof(sleftv));
            memcpy(v->next,&($4),sizeof(sleftv));
            $$ = $2;
          }
        ;

expr:   expr_arithmetic
          {
            /*if ($1.typ == eunknown) YYERROR;*/
            $$ = $1;
          }
        | elemexpr        { $$ = $1; }
        | '(' expr ')'    { $$ = $2; }
        | expr '[' expr ',' expr ']'
          {
            if(iiExprArith3(&$$,'[',&$1,&$3,&$5)) YYERROR;
          }
        | expr '[' expr ']'
          {
            if(iiExprArith2(&$$,&$1,'[',&$3)) YYERROR;
          }
        | ROOT_DECL '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,$1)) YYERROR;
          }
        | ROOT_DECL_LIST '(' exprlist ')'
          {
            if(iiExprArithM(&$$,&$3,$1)) YYERROR;
          }
        | ROOT_DECL_LIST '(' ')'
          {
            if(iiExprArithM(&$$,NULL,$1)) YYERROR;
          }
        | RING_DECL '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,$1)) YYERROR;
          }
        | currring_lists '(' exprlist ')'
          {
            if(iiExprArithM(&$$,&$3,$1)) YYERROR;
          }
        | currring_lists '(' ')'
          {
            if(iiExprArithM(&$$,NULL,$1)) YYERROR;
          }
        | CMD_1 '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,$1)) YYERROR;
          }
        | CMD_2 '(' expr ',' expr ')'
          {
            if(iiExprArith2(&$$,&$3,$1,&$5,TRUE)) YYERROR;
          }
        | CMD_3 '(' expr ',' expr ',' expr ')'
          {
            if(iiExprArith3(&$$,$1,&$3,&$5,&$7)) YYERROR;
          }
        | CMD_M '(' ')'
          {
            if(iiExprArithM(&$$,NULL,$1)) YYERROR;
          }
        | CMD_23 '(' expr ',' expr ')'
          {
            if(iiExprArith2(&$$,&$3,$1,&$5,TRUE)) YYERROR;
          }
        | CMD_23 '(' expr ',' expr ',' expr ')'
          {
            if(iiExprArith3(&$$,$1,&$3,&$5,&$7)) YYERROR;
          }
        | CMD_12 '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,$1)) YYERROR;
          }
        | CMD_12 '(' expr ',' expr ')'
          {
            if(iiExprArith2(&$$,&$3,$1,&$5,TRUE)) YYERROR;
          }
        | CMD_123 '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,$1)) YYERROR;
          }
        | CMD_123 '(' expr ',' expr ')'
          {
            if(iiExprArith2(&$$,&$3,$1,&$5,TRUE)) YYERROR;
          }
        | CMD_123 '(' expr ',' expr ',' expr ')'
          {
            if(iiExprArith3(&$$,$1,&$3,&$5,&$7)) YYERROR;
          }
        | MATRIX_CMD '(' expr ',' expr ',' expr ')'
          {
            if(iiExprArith3(&$$,MATRIX_CMD,&$3,&$5,&$7)) YYERROR;
          }
        | MATRIX_CMD '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,MATRIX_CMD)) YYERROR;
          }
        | INTMAT_CMD '(' expr ')'
          {
            if(iiExprArith1(&$$,&$3,INTMAT_CMD)) YYERROR;
          }
        | CMD_M '(' exprlist ')'
          {
            if(iiExprArithM(&$$,&$3,$1)) YYERROR;
          }
        | quote_start expr quote_end
          {
            $$=$2;
          }
        | quote_start expr '=' expr quote_end
          {
            #ifdef SIQ
            siq++;
            if (siq>0)
            { if (iiExprArith2(&$$,&$2,'=',&$4)) YYERROR; }
            else
            #endif
            {
              memset(&$$,0,sizeof($$));
              $$.rtyp=NONE;
              if (iiAssign(&$2,&$4)) YYERROR;
            }
            #ifdef SIQ
            siq--;
            #endif
          }
        | EVAL  '('
          {
            #ifdef SIQ
            siq--;
            #endif
          }
          expr ')'
          {
            #ifdef SIQ
            if (siq<=0) $4.Eval();
            #endif
            $$=$4;
            #ifdef SIQ
            siq++;
            #endif
          }
          ;

quote_start:    QUOTE  '('
          {
            #ifdef SIQ
            siq++;
            #endif
          }
          ;

quote_end: ')'
          {
            #ifdef SIQ
            siq--;
            #endif
          }
          ;

expr_arithmetic:
          expr  PLUSPLUS     %prec PLUSPLUS
          {
            if(iiExprArith1(&$$,&$1,PLUSPLUS)) YYERROR;
          }
        | expr  MINUSMINUS   %prec MINUSMINUS
          {
            if(iiExprArith1(&$$,&$1,MINUSMINUS)) YYERROR;
          }
        | expr  '+' expr
          {
            if(iiExprArith2(&$$,&$1,'+',&$3)) YYERROR;
          }
        | expr  '-' expr
          {
            if(iiExprArith2(&$$,&$1,'-',&$3)) YYERROR;
          }
        | expr  '*' expr
          {
            if(iiExprArith2(&$$,&$1,'*',&$3)) YYERROR;
          }
        | expr  '/' expr
          {
            if(iiExprArith2(&$$,&$1,'/',&$3)) YYERROR;
          }
        | expr  '^' expr
          {
            if(iiExprArith2(&$$,&$1,'^',&$3)) YYERROR;
          }
        | expr  '%' expr
          {
            if(iiExprArith2(&$$,&$1,'%',&$3)) YYERROR;
          }
        | expr  '>' expr
          {
            if(iiExprArith2(&$$,&$1,'>',&$3)) YYERROR;
          }
        | expr  '<' expr
          {
            if(iiExprArith2(&$$,&$1,'<',&$3)) YYERROR;
          }
        | expr  '&' expr
          {
            if(iiExprArith2(&$$,&$1,'&',&$3)) YYERROR;
          }
        | expr  '|' expr
          {
            if(iiExprArith2(&$$,&$1,'|',&$3)) YYERROR;
          }
        | expr  NOTEQUAL expr
          {
            if(iiExprArith2(&$$,&$1,NOTEQUAL,&$3)) YYERROR;
          }
        | expr  EQUAL_EQUAL expr
          {
            if(iiExprArith2(&$$,&$1,EQUAL_EQUAL,&$3)) YYERROR;
          }
        | expr  GE  expr
          {
            if(iiExprArith2(&$$,&$1,GE,&$3)) YYERROR;
          }
        | expr  LE expr
          {
            if(iiExprArith2(&$$,&$1,LE,&$3)) YYERROR;
          }
        | expr DOTDOT expr
          {
            if(iiExprArith2(&$$,&$1,DOTDOT,&$3)) YYERROR;
          }
        | NOT '(' expr ')'
          {
            memset(&$$,0,sizeof($$));
            int i; TESTSETINT($3,i);
            $$.rtyp  = INT_CMD;
            $$.data = (void *)(i == 0 ? 1 : 0);
          }
        | '-' expr %prec UMINUS
          {
            if(iiExprArith1(&$$,&$2,'-')) YYERROR;
          }
        ;

left_value:
        declare_ip_variable cmdeq  { $$ = $1; }
        | exprlist '=' { $$ = $1; }
        ;

intexpr:
        INT_CONST
          {
            $$ = atoi($1);

            /* check: out of range input */
            int l = strlen($1)+2;
            if (l >= MAX_INT_LEN)
            {
              char tmp[100];
              sprintf(tmp,"%d",$$);
              if (strcmp(tmp,$1))
              {
                Werror("`%s` greater than %d(max. integer representation)"
                       ,$1,INT_MAX);
                YYERROR;
              }
            }
            /*remember not to FreeL($1)
            *because it is a part of the scanner buffer*/
          }
        ;

extendedid:
        UNKNOWN_IDENT
        | '`' expr '`'
          {
            if ($2.Typ()!=STRING_CMD)
            {
              MYYERROR("string expression expected");
            }
            $$ = (char *)$2.CopyD();
            $2.CleanUp();
          }
        ;

currring_lists:
        IDEAL_CMD | MODUL_CMD
        /* put variables into the current ring */
        ;

declare_ip_variable:
        ROOT_DECL elemexpr
          {
            if (iiDeclCommand(&$$,&$2,myynest,$1,&idroot)) YYERROR;
          }
        | ROOT_DECL_LIST elemexpr
          {
            if (iiDeclCommand(&$$,&$2,myynest,$1,&idroot)) YYERROR;
          }
        | RING_DECL elemexpr
          {
            #ifdef SIC
            if ((sic) && (currRing==NULL))
            {
              if (iiDeclCommand(&$$,&$2,myynest,$1,NULL)) YYERROR;
            }
            else
            #endif
            {
              if (iiDeclCommand(&$$,&$2,myynest,$1,&(currRing->idroot))) YYERROR;
            }
          }
        | currring_lists elemexpr
          {
            #ifdef SIC
            if ((sic) && (currRing==NULL))
            {
              if (iiDeclCommand(&$$,&$2,myynest,$1,NULL)) YYERROR;
            }
            else
            #endif
            {
              if (iiDeclCommand(&$$,&$2,myynest,$1,&(currRing->idroot))) YYERROR;
            }
          }
        | MATRIX_CMD elemexpr '[' expr ']' '[' expr ']'
          {
            if (iiDeclCommand(&$$,&$2,myynest,$1,&(currRing->idroot))) YYERROR;
            #ifdef SIC
            if (sic)
            {
            }
            else
            #endif
            {
              int r; TESTSETINT($4,r);
              int c; TESTSETINT($7,c);
              if (r < 1)
                MYYERROR("rows must be greater than 0");
              if (c < 0)
                MYYERROR("cols must be greater than -1");
              leftv v=&$$;
              //while (v->next!=NULL) { v=v->next; }
              idhdl h=(idhdl)v->data;
              idDelete(&IDIDEAL(h));
              IDMATRIX(h) = mpNew(r,c);
              if (IDMATRIX(h)==NULL) YYERROR;
            }
          }
        | MATRIX_CMD elemexpr
          {
            if (iiDeclCommand(&$$,&$2,myynest,$1,&(currRing->idroot))) YYERROR;
          }
        | INTMAT_CMD elemexpr '[' expr ']' '[' expr ']'
          {
            if (iiDeclCommand(&$$,&$2,myynest,$1,&idroot)) YYERROR;
            #ifdef SIC
            if (sic)
            {
            }
            else
            #endif
            {
              int r; TESTSETINT($4,r);
              int c; TESTSETINT($7,c);
              if (r < 1)
                MYYERROR("rows must be greater than 0");
              if (c < 0)
                MYYERROR("cols must be greater than -1");
              leftv v=&$$;
              //while (v->next!=NULL) { v=v->next; }
              idhdl h=(idhdl)v->data;
              delete IDINTVEC(h);
              IDINTVEC(h) = new intvec(r,c,0);
              if (IDINTVEC(h)==NULL) YYERROR;
            }
          }
        | INTMAT_CMD elemexpr
          {
            if (iiDeclCommand(&$$,&$2,myynest,$1,&idroot)) YYERROR;
            leftv v=&$$;
            idhdl h;
            do
            {
               h=(idhdl)v->data;
               delete IDINTVEC(h);
               IDINTVEC(h) = new intvec(1,1,0);
               v=v->next;
            } while (v!=NULL);
          }
        | declare_ip_variable ',' elemexpr
          {
            int t=$1.Typ();
            sleftv r;
            memset(&r,0,sizeof(sleftv));
            if ((BEGIN_RING<t) && (t<END_RING))
            {
              if (iiDeclCommand(&r,&$3,myynest,t,&(currRing->idroot))) YYERROR;
            }
            else
            {
              if (iiDeclCommand(&r,&$3,myynest,t,&idroot)) YYERROR;
            }
            leftv v=&$1;
            while (v->next!=NULL) v=v->next;
            v->next=(leftv)Alloc(sizeof(sleftv));
            memcpy(v->next,&r,sizeof(sleftv));
            $$=$1;
          }
        ;

stringexpr:
        STRINGTOK
        | LIB_CMD
          {
            idhdl h = ggetid( "LIB" );
            if (h==NULL) YYERROR;
            $$ = mstrdup(IDSTRING(h));
          }
        ;

rlist:
        expr
        | '(' expr ',' exprlist ')'
          {
            leftv v = &$2;
            while (v->next!=NULL)
            {
              v=v->next;
            }
            v->next = (leftv)Alloc(sizeof(sleftv));
            memcpy(v->next,&($4),sizeof(sleftv));
            $$ = $2;
          }
        ;

ordername:
        UNKNOWN_IDENT
        {
          if (!($$=rOrderName($1)))
            YYERROR;
        }
        ;

orderelem:
        ordername
          {
            memset(&$$,0,sizeof($$));
            intvec *iv = new intvec(2);
            (*iv)[0] = 1;
            (*iv)[1] = $1;
            $$.rtyp = INTVEC_CMD;
            $$.data = (void *)iv;
          }
        | ordername '(' exprlist ')'
          {
            memset(&$$,0,sizeof($$));
            leftv sl = &$3;
            int slLength;
            {
              slLength =  exprlist_length(sl);
              int l = 2 +  slLength;
              intvec *iv = new intvec(l);
              (*iv)[0] = slLength;
              (*iv)[1] = $1;

              int i = 2;
              while ((i<l) && (sl!=NULL))
              {
                if (sl->Typ() == INT_CMD)
                {
                  (*iv)[i++] = (int)(sl->Data());
                }
                else if ((sl->Typ() == INTVEC_CMD)
                ||(sl->Typ() == INTMAT_CMD))
                {
                  intvec *ivv = (intvec *)(sl->Data());
                  int ll = 0,l = ivv->length();
                  for (; l>0; l--)
                  {
                    (*iv)[i++] = (*ivv)[ll++];
                  }
                }
                else
                {
                  delete iv;
                  $3.CleanUp();
                  MYYERROR("wrong type in ordering");
                }
                sl = sl->next;
              }
              $$.rtyp = INTVEC_CMD;
              $$.data = (void *)iv;
            }
            $3.CleanUp();
          }
        ;

OrderingList:
        orderelem
        |  orderelem ',' OrderingList
          {
            $$ = $1;
            $$.next = (sleftv *)Alloc(sizeof(sleftv));
            memcpy($$.next,&$3,sizeof(sleftv));
          }
        ;

ordering:
        orderelem
        | '(' OrderingList ')'
          {
            $$ = $2;
          }
        ;

cmdeq:  '='
          {
            expected_parms = TRUE;
          }
        ;


/* --------------------------------------------------------------------*/
/* section of pure commands                                            */
/* --------------------------------------------------------------------*/

executecmd:
        EXECUTE_CMD expr
          {
            #ifdef SIC
            if (sic) MYYERROR("execute not supported");
            #endif
            if ($2.Typ() == STRING_CMD)
            {
              char * s = (char *)AllocL(strlen((char *)$2.Data()) + 4);
              strcpy( s, (char *)$2.Data());
              strcat( s, "\n;\n");
              $2.CleanUp();
              newBuffer(s,BT_execute);
            }
            else
            {
              MYYERROR("string expected");
            }
          }
        ;

filecmd:
        '<' stringexpr ';'
          { if (iiPStart(NULL,$2,NULL)) YYERROR; }
        ;

helpcmd:
        HELP_CMD STRINGTOK
          {
            singular_help($2,FALSE);
            FreeL((ADDRESS)$2);
          }
        | HELP_CMD
          {
#ifdef buildin_help
            char *s=(char *)Alloc(10);
            strcpy(s,"index");
            singular_help(s,FALSE);
            Free((ADDRESS)s,10);
#else
            system("info singular");
#endif
          }
        | EXAMPLE_CMD STRINGTOK
          {
            singular_help($2,TRUE);
            FreeL((ADDRESS)$2);
            if (inerror) YYERROR;
          }
        ;

exportcmd:
        EXPORT_CMD exprlist
        {
          if (iiExport(&$2,0)) YYERROR;
        }
        | EXPORT_CMD exprlist extendedid expr
        {
          if ((strcmp($3,"to")!=0) ||
          (($4.Typ()!=PACKAGE_CMD) && ($4.Typ()!=INT_CMD)))
            MYYERROR("export <id> to <package|int>");
          FreeL((ADDRESS)$3);
          if ($4.Typ()==INT_CMD)
          {
            if (iiExport(&$2,((int)$4.Data())-1)) YYERROR;
          }
          else
          {
            if (iiExport(&$2,0,IDPACKAGE((idhdl)$4.data)->idroot))
              YYERROR;
          }
        }
        ;

killcmd:
        KILL_CMD kill_any_identifier {;}
        ;

kill_any_identifier:
        elemexpr
          {
            leftv v=&$1;
            do
            {
              if (v->rtyp!=IDHDL)
              {
                if (v->name!=NULL) Werror("`%s` is undefined in kill",v->name);
                else               WerrorS("kill what ?");
              }
              else
              {
                killhdl((idhdl)v->data);
              }
              v=v->next;
            } while (v!=NULL);
            $1.CleanUp();
          }
        | kill_any_identifier ',' elemexpr
          {
            if ($3.rtyp!=IDHDL)
            {
              if ($3.name!=NULL) Werror("`%s` is undefined",$3.name);
              else               WerrorS("kill what ?");
            }
            else
            {
              killhdl((idhdl)$3.data);
            }
            $3.CleanUp();
          }
        ;

listcmd:
        LISTVAR_CMD '(' ROOT_DECL ')'
          {
            list_cmd($3,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' ROOT_DECL_LIST ')'
          {
            list_cmd($3,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' RING_DECL ')'
          {
            if ($3==QRING_CMD) $3=RING_CMD;
            list_cmd($3,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' currring_lists ')'
          {
            list_cmd($3,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' RING_CMD ')'
          {
            #ifdef SIC
            if (sic) MYYERROR("not implemented yet");
            #endif
            list_cmd(RING_CMD,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' MATRIX_CMD ')'
          {
            #ifdef SIC
            if (sic) MYYERROR("not implemented yet");
            #endif
            list_cmd(MATRIX_CMD,NULL,"// ",TRUE);
           }
        | LISTVAR_CMD '(' INTMAT_CMD ')'
          {
            #ifdef SIC
            if (sic) MYYERROR("not implemented yet");
            #endif
            list_cmd(INTMAT_CMD,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' PROC_CMD ')'
          {
            #ifdef SIC
            if (sic) MYYERROR("not implemented yet");
            #endif
            list_cmd(PROC_CMD,NULL,"// ",TRUE);
          }
        | LISTVAR_CMD '(' elemexpr ')'
          {
            #ifdef SIC
            if (sic) MYYERROR("not implemented yet");
            #endif
            list_cmd(0,$3.Name(),"// ",TRUE);
            $3.CleanUp();
          }
        | LISTVAR_CMD '(' ')'
          {
            #ifdef SIC
            if (sic) MYYERROR("not implemented yet");
            #endif
            list_cmd(-1,NULL,"// ",TRUE);
          }
        ;

ringcmd1:
       RING_CMD { noringvars = TRUE; }
       ;

ringcmd:
        ringcmd1
          elemexpr cmdeq
          rlist     ','      /* description of coeffs */
          rlist     ','      /* var names */
          ordering           /* list of (multiplier ordering (weight(s))) */
          {
            #ifdef SIC
            if (sic)
            {
            }
            #endif
            //noringvars = FALSE;
            if(!rInit($2.name,        /* ringname */
                     &$4,            /* characteristik and list of parameters*/
                     &$6,            /* names of ringvariables */
                     &$8,            /* ordering */
                     FALSE))         /* is not a dring */
            {
              MYYERROR("cannot make ring");
            }
            $4.CleanUp();
            $6.CleanUp();
            $8.CleanUp();
          }
        | ringcmd1 elemexpr
          {
            //noringvars = FALSE;
            if (!inerror) rDefault($2.name);
          }
        | DRING_CMD { noringvars = TRUE; }
          elemexpr cmdeq
          rlist     ','       /* description of coeffs */
          rlist     ','       /* var names */
          ordering           /* list of (multiplier ordering (weight(s))) */
          {
            #ifdef DRING
            idhdl h;
            //noringvars = FALSE;
            if(!(h=rInit($3.name,    /* ringname */
                     &$5,            /* characteristik and list of parameters*/
                     &$7,            /* names of ringvariables */
                     &$9,            /* ordering */
                     TRUE)))         /* is a dring */
            {
              YYERROR;
            }
            setFlag(h,FLAG_DRING);
            rDSet();
            $5.CleanUp();
            $7.CleanUp();
            $9.CleanUp();
            #endif
          }
        ;

scriptcmd:
         LIB_CMD stringexpr
          {
            if (iiLibCmd($2)) YYERROR;
          }
        ;

setrings:  SETRING_CMD | KEEPRING_CMD ;

setringcmd:
        setrings expr
          {
            if (($1==KEEPRING_CMD) && (myynest==0))
               MYYERROR("only inside a proc allowed");
            const char * n=$2.Name();
            if ((($2.Typ()==RING_CMD)||($2.Typ()==QRING_CMD))
            && ($2.rtyp==IDHDL))
            {
              idhdl h=(idhdl)$2.data;
              if ($2.e!=NULL) h=rFindHdl((ring)$2.Data(),NULL);
              if ($1==KEEPRING_CMD)
              {
                if (h!=NULL)
                {
                  if (IDLEV(h)!=0)
                  {
                    if (iiExport(&$2,myynest-1)) YYERROR;
                    //if (TEST_OPT_KEEPVARS)
                    //{
                      idhdl p=IDRING(h)->idroot;
                      int prevlev=myynest-1;
                      while (p!=NULL)
                      {
                        if (IDLEV(p)==myynest) IDLEV(p)=prevlev;
                        p=IDNEXT(p);
                      }
                    //}
                  }
                  iiLocalRing[myynest-1]=IDRING(h);
                }
                else
                {
                  Werror("%s is no identifier",n);
                  $2.CleanUp();
                  YYERROR;
                }
              }
              if (h!=NULL) rSetHdl(h,TRUE);
              else
              {
                Werror("cannot find the name of the basering %s",n);
                $2.CleanUp();
                YYERROR;
              }
              $2.CleanUp();
            }
            else
            {
              Werror("%s is no name of a ring/qring",n);
              $2.CleanUp();
              YYERROR;
            }
          }
        ;

typecmd:
        TYPE_CMD expr
          {
            if ($2.rtyp!=IDHDL) MYYERROR("identifier expected");
            #ifdef SIC
            if (sic)
            {
            }
            else
            #endif
            {
              idhdl h = (idhdl)$2.data;
              type_cmd(h);
            }
          }
        | exprlist
          {
            //Print("typ is %d, rtyp:%d\n",$1.Typ(),$1.rtyp);
            #ifdef SIQ
            if ($1.rtyp!=COMMAND)
            {
            #endif
              if ($1.Typ()==UNKNOWN)
              {
                if ($1.name!=NULL)
                {
                  Werror("`%s` is undefined",$1.name);
                  FreeL((ADDRESS)$1.name);
                }
                YYERROR;
              }
            #ifdef SIQ
            }
            #endif
            $1.Print(&sLastPrinted);
            $1.CleanUp();
          }
        ;

/* --------------------------------------------------------------------*/
/* section of flow control                                             */
/* --------------------------------------------------------------------*/

ifcmd: IF_CMD '(' expr ')' BLOCKTOK
          {
            #ifdef SIC
            if (sic)
            {
              //egPrint(IF_CMD,1,"if, 1\n");
              //egPrint(&$3);
              newBuffer( $5, BT_if);
            }
            else
            #endif
            {
              int i; TESTSETINT($3,i);
              if (i!=0)
              {
                newBuffer( $5, BT_if);
                //printf("if:new level %d\n",voice);
              }
              else
              {
                FreeL((ADDRESS)$5);
                ifswitch[voice]=1;
                //printf("if:(0):set ifsw=1 in voice %d\n",voice);
              }
            }
          }
        | ELSE_CMD BLOCKTOK
          {
            #ifdef SIC
            if (sic)
            {
            //egPrint(ELSE_CMD,0,"op: else, 0\n");
            newBuffer( $2, BT_else);
            }
            else
            #endif
            {
              if (ifswitch[voice]==1)
              {
                ifswitch[voice]=0;
                newBuffer( $2, BT_else);
              }
              else
              {
                if (ifswitch[voice]!=2)
                {
                  char *s=$2+strlen($2)-1;
                  while ((*s=='\0')||(*s=='\n')) s--;
                  s[1]='\0';
                  Warn("`else` without `if` in level %d",myynest);
                }
                FreeL((ADDRESS)$2);
              }
              ifswitch[voice]=0;
            }
          }
        | IF_CMD '(' expr ')' BREAK_CMD
          {
            #ifdef SIC
            if (sic)
            {
              //egPrint(IFBREAK,1,"ifbreak, 1\n");
              //egPrint(&$3);
            }
            else
            #endif
            {
              int i; TESTSETINT($3,i);
              if (i)
              {
                if (exitBuffer(BT_break)) YYERROR;
              }
              ifswitch[voice]=0;
            }
          }
        | BREAK_CMD
          {
            #ifdef SIC
            if (sic)
            {
            }
            else
            #endif
            {
              if (exitBuffer(BT_break)) YYERROR;
              ifswitch[voice]=0;
            }
          }
        | CONTINUE_CMD
          {
            #ifdef SIC
            if (sic)
            {
            }
            else
            #endif
            {
              contBuffer(BT_break);
              ifswitch[voice]=0;
            }
          }
      ;

whilecmd:
        WHILE_CMD STRINGTOK BLOCKTOK
          {
            /* -> if(!$2) break; $3; continue;*/
            char * s = (char *)AllocL( strlen($2) + strlen($3) + 36);
            sprintf(s,"whileif (!(%s)) break;\n%scontinue;\n " ,$2,$3);
            newBuffer(s,BT_break);
            FreeL((ADDRESS)$2);
            FreeL((ADDRESS)$3);
          }
        ;

forcmd:
        FOR_CMD STRINGTOK STRINGTOK STRINGTOK BLOCKTOK
          {
            /* $2 */
            /* if (!$3) break; $5; $4; continue; */
            char * s = (char *)AllocL( strlen($3)+strlen($4)+strlen($5)+36);
            sprintf(s,"forif (!(%s)) break;\n%s%s;\ncontinue;\n "
                   ,$3,$5,$4);
            FreeL((ADDRESS)$3);
            FreeL((ADDRESS)$4);
            FreeL((ADDRESS)$5);
            newBuffer(s,BT_break);
            s = (char *)AllocL( strlen($2) + 3);
            sprintf(s,"%s;\n",$2);
            FreeL((ADDRESS)$2);
            newBuffer(s,BT_if);
          }
        ;

proccmd:
        PROC_CMD extendedid BLOCKTOK
          {
            #ifdef SIC
            if (sic)
            {
              FreeL((ADDRESS)$2);
              FreeL((ADDRESS)$3);
              MYYERROR("not implemented");
            }
            #endif
            idhdl h = enterid($2,myynest,PROC_CMD,&idroot,FALSE);
            if (h==NULL) {FreeL((ADDRESS)$3); YYERROR;}
            IDSTRING(h) = (char *)AllocL(strlen($3)+31);
            sprintf(IDSTRING(h),"parameter list #;\n%s;return();\n\n",$3);
            FreeL((ADDRESS)$3);
          }
        | PROC_CMD extendedid '=' expr ';'
          {
            #ifdef SIC
            if (sic)
            {
              $4.CleanUp();
              FreeL((ADDRESS)$2);
              MYYERROR("not implemented");
            }
            #endif
            if ($4.Typ()!=STRING_CMD)
              MYYERROR("string expression expected");
            idhdl h = enterid($2,myynest,PROC_CMD,&idroot,FALSE);
            if (h==NULL) YYERROR;
            IDSTRING(h) = (char *)$4.CopyD();
            $4.CleanUp();
          }
        | PROC_DEF STRINGTOK BLOCKTOK
          {
            idhdl h = enterid($1,myynest,PROC_CMD,&idroot,FALSE);
            if (h==NULL)
            {
              FreeL((ADDRESS)$2);
              FreeL((ADDRESS)$3);
              YYERROR;
            }
            char *args=iiProcArgs($2,FALSE);
            FreeL((ADDRESS)$2);
            IDSTRING(h) = (char *)AllocL(strlen($3)+strlen(args)+14);
            sprintf(IDSTRING(h),"%s\n%s;RETURN();\n\n",args,$3);
            FreeL((ADDRESS)args);
            FreeL((ADDRESS)$3);
            //Print(">>%s<<\n",IDSTRING(h));
          }
        ;

parametercmd:
        PARAMETER declare_ip_variable
          {
            //Print("par:%s, %d\n",$2.Name(),$2.Typ());
            yylineno--;
            #ifdef SIC
            if (sic)
            {
              //egParameter(&$2);
            }
            else
            #endif
            {
              if (iiParameter(&$2)) YYERROR;
            }
          }
        | PARAMETER expr
          {
            //Print("par:%s, %d\n",$2.Name(),$2.Typ());
            sleftv tmp_expr;
            #ifdef SIC
            if (sic)
            {
              if (iiDeclCommand(&tmp_expr,&$2,myynest,DEF_CMD,&idroot))
                YYERROR;
              //egParameter(&$2);
            }
            else
            #endif
            {
              if ((iiDeclCommand(&tmp_expr,&$2,myynest,DEF_CMD,&idroot))
              || (iiParameter(&tmp_expr)))
                YYERROR;
            }
          }
        ;

returncmd:
        RETURN '(' exprlist ')'
          {
            iiRETURNEXPR[myynest].Copy(&$3);
            $3.CleanUp();
            if (exitBuffer(BT_proc)) YYERROR;
          }
        | RETURN '(' ')'
          {
            iiRETURNEXPR[myynest].Init();
            iiRETURNEXPR[myynest].rtyp=NONE;
            if (exitBuffer(BT_proc)&&($1!=END_PROC)) YYERROR;
            if ($1==END_PROC) YYACCEPT;
          }
        ;
