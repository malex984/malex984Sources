/*****************************************
*  Computer Algebra System SINGULAR      *
*****************************************/
/* $Id: numbers.cc,v 1.49 2003-01-31 09:10:04 Singular Exp $ */

/*
* ABSTRACT: interface to coefficient aritmetics
*/

#include <string.h>
#include <stdlib.h>
#include "mod2.h"
#include "tok.h"
#include "febase.h"
#include "kstd1.h"
#include "numbers.h"
#include "longrat.h"
#include "longalg.h"
#include "modulop.h"
#include "gnumpfl.h"
#include "gnumpc.h"
#include "ring.h"
#include "ffields.h"
#include "shortfl.h"

//static int characteristic = 0;
extern int IsPrime(int p);

void   (*nNew)(number *a);
number (*nInit)(int i);
number (*nPar)(int i);
int    (*nParDeg)(number n);
int    (*nSize)(number n);
int    (*nInt)(number &n);
numberfunc nMult, nSub, nAdd, nDiv, nIntDiv, nIntMod, nExactDiv;
number (*nNeg)(number a);
number (*nInvers)(number a);
void   (*nNormalize)(number &a);
number (*nCopy)(number a);
number (*nRePart)(number a);
number (*nImPart)(number a);
BOOLEAN (*nGreater)(number a,number b);
BOOLEAN (*nEqual)(number a,number b);
BOOLEAN (*nIsZero)(number a);
BOOLEAN (*nIsOne)(number a);
BOOLEAN (*nIsMOne)(number a);
BOOLEAN (*nGreaterZero)(number a);
void    (*nWrite)(number &a);
char *  (*nRead)(char *s,number *a);
void    (*nPower)(number a, int i, number * result);
number  (*nGcd)(number a, number b, const ring r);
number  (*nLcm)(number a, number b, const ring r);
char * (*nName)(number n);
void   (*n__Delete)(number *a, const ring r);

/*0 implementation*/
number nNULL; /* the 0 as constant */


n_Procs_s *cf_root=NULL;

void   nDummy1(number* d) { *d=NULL; }
void   ndDelete(number* d, const ring r) { *d=NULL; }
void   ndInpMult(number &a, number b, const ring r)
{
  number n=n_Mult(a,b,r);
  n_Delete(&a,r);
  a=n;
}

#ifdef LDEBUG
void   nDBDummy1(number* d,char *f, int l) { *d=NULL; }
#endif

void   nDummy2(number& d) { }

char * ndName(number n) { return NULL; }

number ndPar(int i) { return nInit(0); }

number ndReturn0(number n) { return nInit(0); }

int    ndParDeg(number n) { return 0; }

number ndGcd(number a, number b, const ring r) { return r->cf->nInit(1); }

number ndIntMod(number a, number b) { return nInit(0); }

number ndGetDenom(number &n, const ring r) { return n_Init(1,r); }

int ndSize(number a) { return (int)nIsZero(a)==FALSE; }

number ndCopy(number a) { return a; }
number nd_Copy(number a,const ring r) { return r->cf->nCopy(a); }

/*2
* init operations for characteristic c (complete==TRUE)
* init nDelete    for characteristic c (complete==FALSE)
*/
void nSetChar(ring r)
{
  int c=rInternalChar(r);

  n__Delete= r->cf->cfDelete;
  if (rField_is_Extension(r))
  {
    naSetChar(c,r);
  }
  else if (rField_is_Zp(r))
  /*----------------------char. p----------------*/
  {
    npSetChar(c, r);
  }
  /* -------------- GF(p^m) -----------------------*/
  else if (rField_is_GF(r))
  {
    nfSetChar(c,r->parameter);
  }
  /* -------------- R -----------------------*/
  //if (c==(-1))
  else if (rField_is_R(r))
  {
  }
  /* -------------- long R -----------------------*/
  /* -------------- long C -----------------------*/
  else if ((rField_is_long_R(r))
  || (rField_is_long_C(r)))
  {
    setGMPFloatDigits(r->float_len,r->float_len2);
  }
#ifdef TEST
  /* -------------- R -----------------------*/
  //if (c==(-1))
  else if (!rField_is_R(r) && !rField_is_Q(r))
  {
    WerrorS("unknown field");
  }
#endif
  nNew   = r->cf->nNew;
  nNormalize=r->cf->nNormalize;
  nInit  = r->cf->nInit;
  nPar   = r->cf->nPar;
  nParDeg= r->cf->nParDeg;
  nInt   = r->cf->nInt;
  nAdd   = r->cf->nAdd;
  nSub   = r->cf->nSub;
  nMult  = r->cf->nMult;
  nDiv   = r->cf->nDiv;
  nExactDiv= r->cf->nExactDiv;
  nIntDiv= r->cf->nIntDiv;
  nIntMod= r->cf->nIntMod;
  nNeg   = r->cf->nNeg;
  nInvers= r->cf->nInvers;
  nCopy  = r->cf->nCopy;
  nGreater = r->cf->nGreater;
  nEqual = r->cf->nEqual;
  nIsZero = r->cf->nIsZero;
  nIsOne = r->cf->nIsOne;
  nIsMOne = r->cf->nIsMOne;
  nGreaterZero = r->cf->nGreaterZero;
  nWrite = r->cf->nWrite;
  nRead = r->cf->nRead;
  nPower = r->cf->nPower;
  nGcd  = r->cf->nGcd;
  nLcm  = r->cf->nLcm;
  nName= r->cf->nName;
  nSize  = r->cf->nSize;
  nRePart = r->cf->nRePart;
  nImPart = r->cf->nImPart;
  if (!errorreported) nNULL=r->cf->nNULL;
}

/*2
* init operations for ring r
*/
void nInitChar(ring r)
{
  int c=rInternalChar(r);
  n_coeffType t=rFieldType(r);

  if (rField_is_Extension(r))
  {
    if (r->algring==NULL)
    {
      short ch=-c;
      if (c==1) ch=0;
      r->algring=(ring) rDefault(ch,r->P,r->parameter);
      //r->algring->ShortOut=r->ShortOut;
      // includes: nInitChar(r->algring);
    }
  }

  n_Procs_s *n=cf_root;
  while((n!=NULL)
    && ((n->nChar!=c) || (n->type!=t)))
      n=n->next;
  if (n==NULL)
  {
    n=(n_Procs_s*)omAlloc0(sizeof(n_Procs_s));
    n->next=cf_root;
    n->ref=1;
    n->nChar=c;
    n->type=t;
    cf_root=n;
  }
  else if ((n->nChar==c) && (n->type==t))
  {
    n->ref++;
    r->cf=n;
    return;
  }
  else
  {
    WerrorS("nInitChar failed");
    return;
  }
  r->cf=n;
  n->nChar = c;
  n->nPar  = ndPar;
  n->nParDeg=ndParDeg;
  n->nSize = ndSize;
  n->n_GetDenom= ndGetDenom;
  n->nName =  ndName;
  n->nImPart=ndReturn0;
  n->cfDelete= ndDelete;
  n->nNew=nDummy1;
  n->nInpMult=ndInpMult;
  n->cfCopy=nd_Copy;
  n->nIntMod=ndIntMod; /* dummy !! */
  n->nNormalize=nDummy2;
  n->nGcd  = ndGcd;
  n->nLcm  = ndGcd; /* tricky, isn't it ?*/
  if (rField_is_Extension(r))
  {
    //naInitChar(c,TRUE,r);
    n->cfDelete = naDelete;
    n-> nNew       = naNew;
    n-> nNormalize = naNormalize;
    n->nInit       = naInit;
    n->nPar        = naPar;
    n->nParDeg     = naParDeg;
    n->nInt        = naInt;
    n->nAdd        = naAdd;
    n->nSub        = naSub;
    n->nMult       = naMult;
    n->nDiv        = naDiv;
    n->nExactDiv   = naDiv;
    n->nIntDiv     = naIntDiv;
    n->nNeg        = naNeg;
    n->nInvers     = naInvers;
    n->nCopy       = naCopy;
    n->cfCopy      = na_Copy;
    n->nGreater    = naGreater;
    n->nEqual      = naEqual;
    n->nIsZero     = naIsZero;
    n->nIsOne      = naIsOne;
    n->nIsMOne     = naIsMOne;
    n->nGreaterZero= naGreaterZero;
    n->nWrite      = naWrite;
    n->nRead       = naRead;
    n->nPower      = naPower;
    n->nGcd        = naGcd;
    n->nLcm        = naLcm;
    n->cfSetMap    = naSetMap;
    n->nName       = naName;
    n->nSize       = naSize;
    n->n_GetDenom   = naGetDenom;
#ifdef LDEBUG
    //n->nDBTest     = naDBTest;
#endif
  }
  else if (rField_is_Q(r))
  {
    n->cfDelete= nlDelete;
    n->nNew   = nlNew;
    n->nNormalize=nlNormalize;
    n->nInit  = nlInit;
    n->nInt   = nlInt;
    n->nAdd   = nlAdd;
    n->nSub   = nlSub;
    n->nMult  = nlMult;
    n->nInpMult=nlInpMult;
    n->nDiv   = nlDiv;
    n->nExactDiv= nlExactDiv;
    n->nIntDiv= nlIntDiv;
    n->nIntMod= nlIntMod;
    n->nNeg   = nlNeg;
    n->nInvers= nlInvers;
    n->nCopy  = nlCopy;
    n->nGreater = nlGreater;
    n->nEqual = nlEqual;
    n->nIsZero = nlIsZero;
    n->nIsOne = nlIsOne;
    n->nIsMOne = nlIsMOne;
    n->nGreaterZero = nlGreaterZero;
    n->nWrite = nlWrite;
    n->nRead = nlRead;
    n->nPower = nlPower;
    n->nGcd  = nlGcd;
    n->nLcm  = nlLcm;
    n->cfSetMap = nlSetMap;
    n->nSize  = nlSize;
    n->n_GetDenom = nlGetDenom;
#ifdef LDEBUG
    //n->nDBTest=nlDBTest;
#endif
  }
  else if (rField_is_Zp(r))
  /*----------------------char. p----------------*/
  {
    npInitChar(c,r);
    n->nInit  = npInit;
    n->nInt   = npInt;
    n->nAdd   = npAdd;
    n->nSub   = npSub;
    n->nMult  = npMult;
    n->nDiv   = npDiv;
    n->nExactDiv= npDiv;
    n->nNeg   = npNeg;
    n->nInvers= npInvers;
    n->nCopy  = ndCopy;
    n->nGreater = npGreater;
    n->nEqual = npEqual;
    n->nIsZero = npIsZero;
    n->nIsOne = npIsOne;
    n->nIsMOne = npIsMOne;
    n->nGreaterZero = npGreaterZero;
    n->nWrite = npWrite;
    n->nRead = npRead;
    n->nPower = npPower;
    n->cfSetMap = npSetMap;
    /* nName= ndName; */
    /*nSize  = ndSize;*/
#ifdef LDEBUG
    //n->nDBTest=npDBTest;
#endif
#ifdef NV_OPS
    if (c>NV_MAX_PRIME)
    {
      n->nMult  = nvMult;
      n->nDiv   = nvDiv;
      n->nExactDiv= nvDiv;
      n->nInvers= nvInvers;
    }
#endif
  }
  /* -------------- GF(p^m) -----------------------*/
  else if (rField_is_GF(r))
  {
    //nfSetChar(c,r->parameter);
    n->nInit  = nfInit;
    n->nPar   = nfPar;
    n->nParDeg= nfParDeg;
    n->nInt   = nfInt;
    n->nAdd   = nfAdd;
    n->nSub   = nfSub;
    n->nMult  = nfMult;
    n->nDiv   = nfDiv;
    n->nExactDiv= nfDiv;
    n->nNeg   = nfNeg;
    n->nInvers= nfInvers;
    n->nCopy  = ndCopy;
    n->nGreater = nfGreater;
    n->nEqual = nfEqual;
    n->nIsZero = nfIsZero;
    n->nIsOne = nfIsOne;
    n->nIsMOne = nfIsMOne;
    n->nGreaterZero = nfGreaterZero;
    n->nWrite = nfWrite;
    n->nRead = nfRead;
    n->nPower = nfPower;
    n->cfSetMap = nfSetMap;
    n->nName= nfName;
    /*nSize  = ndSize;*/
#ifdef LDEBUG
    //n->nDBTest=nfDBTest;
#endif
  }
  /* -------------- R -----------------------*/
  //if (c==(-1))
  else if (rField_is_R(r))
  {
    n->nInit  = nrInit;
    n->nInt   = nrInt;
    n->nAdd   = nrAdd;
    n->nSub   = nrSub;
    n->nMult  = nrMult;
    n->nDiv   = nrDiv;
    n->nExactDiv= nrDiv;
    n->nNeg   = nrNeg;
    n->nInvers= nrInvers;
    n->nCopy  = ndCopy;
    n->nGreater = nrGreater;
    n->nEqual = nrEqual;
    n->nIsZero = nrIsZero;
    n->nIsOne = nrIsOne;
    n->nIsMOne = nrIsMOne;
    n->nGreaterZero = nrGreaterZero;
    n->nWrite = nrWrite;
    n->nRead = nrRead;
    n->nPower = nrPower;
    n->cfSetMap=nrSetMap;
    /* nName= ndName; */
    /*nSize  = ndSize;*/
#ifdef LDEBUG
    //n->nDBTest=nrDBTest;
#endif
  }
  /* -------------- long R -----------------------*/
  else if (rField_is_long_R(r))
  {
    n->cfDelete= ngfDelete;
    n->nNew=ngfNew;
    n->nInit  = ngfInit;
    n->nInt   = ngfInt;
    n->nAdd   = ngfAdd;
    n->nSub   = ngfSub;
    n->nMult  = ngfMult;
    n->nDiv   = ngfDiv;
    n->nExactDiv= ngfDiv;
    n->nNeg   = ngfNeg;
    n->nInvers= ngfInvers;
    n->nCopy  = ngfCopy;
    n->nGreater = ngfGreater;
    n->nEqual = ngfEqual;
    n->nIsZero = ngfIsZero;
    n->nIsOne = ngfIsOne;
    n->nIsMOne = ngfIsMOne;
    n->nGreaterZero = ngfGreaterZero;
    n->nWrite = ngfWrite;
    n->nRead = ngfRead;
    n->nPower = ngfPower;
    n->cfSetMap=ngfSetMap;
    n->nName= ndName;
    n->nSize  = ndSize;
#ifdef LDEBUG
    //n->nDBTest=ngfDBTest;
#endif
  }
  /* -------------- long C -----------------------*/
  else if (rField_is_long_C(r))
  {
    n->cfDelete= ngcDelete;
    n->nNew=ngcNew;
    n->nNormalize=nDummy2;
    n->nInit  = ngcInit;
    n->nInt   = ngcInt;
    n->nAdd   = ngcAdd;
    n->nSub   = ngcSub;
    n->nMult  = ngcMult;
    n->nDiv   = ngcDiv;
    n->nExactDiv= ngcDiv;
    n->nNeg   = ngcNeg;
    n->nInvers= ngcInvers;
    n->nCopy  = ngcCopy;
    n->nGreater = ngcGreater;
    n->nEqual = ngcEqual;
    n->nIsZero = ngcIsZero;
    n->nIsOne = ngcIsOne;
    n->nIsMOne = ngcIsMOne;
    n->nGreaterZero = ngcGreaterZero;
    n->nWrite = ngcWrite;
    n->nRead = ngcRead;
    n->nPower = ngcPower;
    n->cfSetMap=ngcSetMap;
    n->nPar=ngcPar;
    n->nRePart=ngcRePart;
    n->nImPart=ngcImPart;
    /*nSize  = ndSize;*/
#ifdef LDEBUG
    //n->nDBTest=ngcDBTest;
#endif
  }
#ifdef TEST
  else
  {
    WerrorS("unknown field");
  }
#endif
  if (!errorreported)
  {
    n->nNULL=n->nInit(0);
    if (n->nRePart==NULL)
      n->nRePart=n->nCopy;
    if (n->nIntDiv==NULL)
      n->nIntDiv=n->nDiv;
  }
}

void nKillChar(ring r)
{
  if (r!=NULL)
  {
    if (r->cf!=NULL)
    {
      r->cf->ref--;
      if (r->cf->ref<=0)
      {
        n_Procs_s tmp;
        n_Procs_s* n=&tmp;
        tmp.next=cf_root;
        while((n->next!=NULL) && (n->next!=r->cf)) n=n->next;
        if (n->next==r->cf)
        {
          n->next=n->next->next;
          cf_root=tmp.next;
          r->cf->cfDelete(&(r->cf->nNULL),r);
          switch(r->cf->type)
          {
            case n_Zp:
                 #ifdef HAVE_DIV_MOD
		 if (r->cf->npInvTable!=NULL)
                 omFreeSize( (ADDRESS)r->cf->npInvTable,
                             r->cf->npPrimeM*sizeof(CARDINAL) );
                 #else
		 if (r->cf->npExpTable!=NULL)
		 {
                   omFreeSize( (ADDRESS)r->cf->npExpTable,
                               r->cf->npPrimeM*sizeof(CARDINAL) );
                   omFreeSize( (ADDRESS)r->cf->npLogTable,
                               r->cf->npPrimeM*sizeof(CARDINAL) );
	         }
                 #endif
                 break;

            default:
                 break;
          }
          omFreeSize((ADDRESS)r->cf, sizeof(n_Procs_s));
          r->cf=NULL;
        }
        else
        {
          WarnS("cf_root list destroyed");
        }
      }
    }
    if (r->algring!=NULL)
    {
      rKill(r->algring);
      r->algring=NULL;
    }
  }
}
