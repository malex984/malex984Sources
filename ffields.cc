/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: ffields.cc,v 1.27 2000-10-19 15:00:11 obachman Exp $ */
/*
* ABSTRACT: finite fields with a none-prime number of elements (via tables)
*/

#include <limits.h>
#include <string.h>
#include "mod2.h"
#include "tok.h"
#include "febase.h"
#include "omalloc.h"
#include "numbers.h"
#include "ring.h"
#include "ffields.h"

int nfCharQ=0;  /* the number of elemts: q*/
int nfM1;       /*representation of -1*/
static int nfCharP=0;  /* the characteristic: p*/
static int nfCharQ1=0; /* q-1 */
CARDINAL *nfPlus1Table=NULL; /* the table i=log(z^i) -> log(z^i+1) */
char * nfParameter;          /*  the name of the primitive element */
/* the q's from the table 'fftable' */
short fftable[]={
    4,  8, 16, 32, 64, 128, 256, 512,1024,2048,4096,8192,16384,
/*2^2 2^3 2^4 2^5 2^6  2^7  2^8  2^9 2^10 2^11 2^12 2^13  2^14*/
    9, 27, 81,243,729,2187, 6561,19683,
/*3^2 3^3 3^4 3^5 3^6  3^7  3^8   3^9*/
   25,125,625,3125,15625,
/*5^2 5^3 5^4 5^5  5^6*/
   49,343,2401,16807,
/*7^2 7^3  7^4 7^5*/
   121,1331, 14641,
/*11^2 11^3  11^4*/
  169, 2197, 28561,
/*13^2 13^3  13^4*/
  289, 4913,
/*17^2 17^3*/
  361, 6859,
/*19^2 19^3*/
  529, 12167,
/*23^2 23^3*/
  841, 24389,
/*29^2 29^3*/
  961, 29791,
/*31^2 31^3*/
  1369,
/*37^2*/
  1681,
/*41^2*/
  1849,
/*43^2*/
  2209,
/*47^2*/
  2809,
/*53^2*/
  3481,
/*59^2*/
  3721,
/*61^2*/
  4489,
/*67^2*/
  5041,
/*71^2*/
  5329,
/*73^2*/
  6241,
/*79^2*/
  6889,
/*83^2*/
  7921,
/*89^2*/
  9409,
/*97^2*/
  10201,
/*101^2*/
  10609,
/*103^2*/
  11449,
/*107^2*/
  11881,
/*109^2*/
  12769,
/*113^2*/
  16129,
/*127^2*/
  17161,
/*131^2*/
  18769,
/*137^2*/
  19321,
/*139^2*/
  22201,
/*149^2*/
  22801,
/*151^2*/
  24649,
/*157^2*/
  26569,
/*163^2*/
  27889,
/*167^2*/
  29929,
/*173^2*/
  32041,
/*179^2*/
  32761,
/*181^2*/
  0 };

/*1
* numbers in GF(p^n):
* let nfCharQ=q=nfCharP^n=p^n
* GF(q)\{0} will be generated by powers of an element Z
* Z^i will be represented by the int i, 1 by the int 0, 0 by the int q=nfChar
*/

#ifdef LDEBUG
/*2
* debugging: is a a valid representation of a number ?
*/
BOOLEAN nfDBTest (number a, char *f, int l)
{
  if (((int)a<0) || ((int)a>nfCharQ))
  {
    Print("wrong %d in %s:%d\n",(int)a,f,l);
    return FALSE;
  }
  int i=0;
  do
  {
    if (nfPlus1Table[i]>nfCharQ)
    {
      Print("wrong table %d=%d in %s:%d\n",i,nfPlus1Table[i],f,l);
      return FALSE;
    }
    i++;
  } while (i<nfCharQ);
  return TRUE;
}
#define nfTest(N) nfDBTest(N,__FILE__,__LINE__)
#endif

/*2
* k >= 0 ?
*/
BOOLEAN nfGreaterZero (number k)
{
#ifdef LDEBUG
  nfTest(k);
#endif
  return !nfIsZero(k) && !nfIsMOne(k);
}

/*2
* a*b
*/
number nfMult (number a,number b)
{
#ifdef LDEBUG
  nfTest(a);
  nfTest(b);
#endif
  if (((int)a == nfCharQ) || ((int)b == nfCharQ))
    return (number)nfCharQ;
  /*else*/
  int i=(int)a+(int)b;
  if (i>=nfCharQ1) i-=nfCharQ1;
#ifdef LDEBUG
  nfTest((number)i);
#endif
  return (number)i;
}

/*2
* int -> number
*/
number nfInit (int i)
{
  while (i <  0)    i += nfCharP;
  while (i >= nfCharP) i -= nfCharP;
  if (i==0) return (number)nfCharQ;
  int c=0;
  while (i>1)
  {
    c=nfPlus1Table[c];
    i--;
  }
#ifdef LDEBUG
  nfTest((number)c);
#endif
  return (number)c;
}

/*
* the generating element `z`
*/
number nfPar (int i)
{
  return (number)1;
}

/*2
* the degree of the "alg. number"
*/
int nfParDeg(number n)
{
#ifdef LDEBUG
  nfTest(n);
#endif
  if(nfCharQ == (int)n) return -1;
  return (int)n;
}

/*2
* number -> int
*/
int nfInt (number &n)
{
  return 0;
}

/*2
* a + b
*/
number nfAdd (number a, number b)
{
/*4 z^a+z^b=z^b*(z^(a-b)+1), if a>=b; *
*          =z^a*(z^(b-a)+1)  if a<b  */
#ifdef LDEBUG
  nfTest(a);
  nfTest(b);
#endif
  if (nfCharQ == (int)a) return b;
  if (nfCharQ == (int)b) return a;
  int zb,zab,r;
  if ((int)a >= (int)b)
  {
    zb = (int)b;
    zab = (int)a-(int)b;
  }
  else
  {
    zb = (int)a;
    zab = (int)b-(int)a;
  }
#ifdef LDEBUG
  nfTest((number)zab);
#endif
  if (nfPlus1Table[zab]==nfCharQ) r=nfCharQ; /*if z^(a-b)+1 =0*/
  else
  {
    r= zb+nfPlus1Table[zab];
    if(r>=nfCharQ1) r-=nfCharQ1;
  }
#ifdef LDEBUG
  nfTest((number)r);
#endif
  return (number)r;
}

/*2
* a - b
*/
number nfSub (number a, number b)
{
  number mb = nfNeg(b);
  return nfAdd(a,mb);
}

/*2
* a == 0 ?
*/
BOOLEAN nfIsZero (number  a)
{
#ifdef LDEBUG
  nfTest(a);
#endif
  return nfCharQ == (int)a;
}

/*2
* a == 1 ?
*/
BOOLEAN nfIsOne (number a)
{
#ifdef LDEBUG
  nfTest(a);
#endif
  return 0 == (int)a;
}

/*2
* a == -1 ?
*/
BOOLEAN nfIsMOne (number a)
{
#ifdef LDEBUG
  nfTest(a);
#endif
  if (0 == (int)a) return FALSE; /* special handling of char 2*/
  return nfM1 == (int)a;
}

/*2
* a / b
*/
number nfDiv (number a,number b)
{
#ifdef LDEBUG
  nfTest(b);
#endif
  if ((int)b==nfCharQ)
  {
    WerrorS("div. by 0");
    return (number)nfCharQ;
  }
#ifdef LDEBUG
  nfTest(a);
#endif
  if ((int)a==nfCharQ)
    return (number)nfCharQ;
  /*else*/
  int s = (int)a - (int)b;
  if (s < 0)
    s += nfCharQ1;
#ifdef LDEBUG
  nfTest((number)s);
#endif
  return (number)s;
}

/*2
* 1 / c
*/
number  nfInvers (number c)
{
#ifdef LDEBUG
  nfTest(c);
#endif
  if ((int)c==nfCharQ)
  {
    WerrorS("div. 1/0");
    return (number)nfCharQ;
  }
#ifdef LDEBUG
  nfTest(((number)(nfCharQ1-(int)c)));
#endif
  return (number)(nfCharQ1-(int)c);
}

/*2
* -c
*/
number nfNeg (number c)
{
/*4 -z^c=z^c*(-1)=z^c*nfM1*/
#ifdef LDEBUG
  nfTest(c);
#endif
  if (nfCharQ == (int)c) return c;
  int i=(int)c+nfM1;
  if (i>=nfCharQ1) i-=nfCharQ1;
#ifdef LDEBUG
  nfTest((number)i);
#endif
  return (number)i;
}

/*2
* a > b ?
*/
BOOLEAN nfGreater (number a,number b)
{
#ifdef LDEBUG
  nfTest(a);
  nfTest(b);
#endif
  return (int)a != (int)b;
}

/*2
* a == b ?
*/
BOOLEAN nfEqual (number a,number b)
{
#ifdef LDEBUG
  nfTest(a);
  nfTest(b);
#endif
  return (int)a == (int)b;
}

/*2
* write via StringAppend
*/
void nfWrite (number &a)
{
#ifdef LDEBUG
  nfTest(a);
#endif
  if ((int)a==nfCharQ)  StringAppendS("0");
  else if ((int)a==0)   StringAppendS("1");
  else if (nfIsMOne(a))   StringAppendS("-1");
  else
  {
    StringAppendS(nfParameter);
    if ((int)a!=1)
    {
      if(currRing->ShortOut==0)  StringAppendS("^");
      StringAppend("%d",(int)a);
    }
  }
}

/*2
*
*/
char * nfName(number a)
{
#ifdef LDEBUG
  nfTest(a);
#endif
  char *s;
  if (((int)a==nfCharQ) || ((int)a==0)) return NULL;
  else if ((int)a==1)
  {
    return omStrDup(nfParameter);
  }
  else
  {
    s=(char *)omAlloc(4+strlen(nfParameter));
    sprintf(s,"%s%d",nfParameter,(int)a);
  }
  return s;
}
/*2
* c ^ i with i>=0
*/
void nfPower (number a, int i, number * result)
{
#ifdef LDEBUG
  nfTest(a);
#endif
  if (i==0)
  {
    //*result=nfInit(1);
    *result = (number)0;
  }
  else if (i==1)
  {
    *result = a;
  }
  else
  {
    nfPower(a,i-1,result);
    *result = nfMult(a,*result);
  }
#ifdef LDEBUG
  nfTest(*result);
#endif
}

/*4
* read an integer (with reduction mod p)
*/
static char* nfEati(char *s, int *i)
{
  if (*s >= '0' && *s <= '9')
  {
    *i = 0;
    do
    {
      *i *= 10;
      *i += *s++ - '0';
      if (*i > (MAX_INT_VAL / 10)) *i = *i % nfCharP;
    }
    while (*s >= '0' && *s <= '9');
    if (*i >= nfCharP) *i = *i % nfCharP;
  }
  else *i = 1;
  return s;
}

/*2
* read a number
*/
char * nfRead (char *s, number *a)
{
  int i;
  number z;
  number n;

  s = nfEati(s, &i);
  z=nfInit(i);
  *a=z;
  if (*s == '/')
  {
    s++;
    s = nfEati(s, &i);
    n=nfInit(i);
    *a = nfDiv(z,n);
  }
  if (strncmp(s,nfParameter,strlen(nfParameter))==0)
  {
    s+=strlen(nfParameter);
    if ((*s >= '0') && (*s <= '9'))
    {
      s=eati(s,&i);
      while (i>=nfCharQ1) i-=nfCharQ1;
    }
    else
      i=1;
    z=(number)i;
    *a=nfMult(*a,z);
  }
#ifdef LDEBUG
  nfTest(*a);
#endif
  return s;
}

#ifdef HAVE_FACTORY
int gf_tab_numdigits62 ( int q );
int convertback62 ( char * p, int n );
#else
static int gf_tab_numdigits62 ( int q )
{
    if ( q < 62 )          return 1;
    else  if ( q < 62*62 ) return 2;
    /*else*/               return 3;
}

static int convback62 ( char c )
{
    if ( c >= '0' && c <= '9' )
        return int(c) - int('0');
    else  if ( c >= 'A' && c <= 'Z' )
        return int(c) - int('A') + 10;
    else
        return int(c) - int('a') + 36;
}

static int convertback62 ( char * p, int n )
{
    int r = 0;
    for ( int j = 0; j < n; j++ )
        r = r * 62 + convback62( p[j] );
    return r;
}
#endif

int nfMinPoly[16];

void nfShowMipo()
{
  int i=nfMinPoly[0];
  int j=0;
  loop
  {
    j++;
    if (nfMinPoly[j]!=0)
      StringAppend("%d*%s^%d",nfMinPoly[j],nfParameter,i);
    i--;
    if(i<0) break;
    if (nfMinPoly[j]!=0)
      StringAppendS("+");
  }
}

static void nfReadMipo(char *s)
{
  const char *l=strchr(s,';')+1;
  char *n;
  int i=strtol(l,&n,10);
  l=n;
  int j=1;
  nfMinPoly[0]=i;
  while(i>=0)
  {
    nfMinPoly[j]=strtol(l,&n,10);
    if (l==n) break;
    l=n;
    j++;
    i--;
  }
  if (i>=0)
  {
    WerrorS("error in reading minpoly from gftables");
  }
}

/*2
* init global variables from files 'gftables/%d'
*/
void nfSetChar(int c, char **param)
{
  //Print("GF(%d)\n",c);
  nfParameter=param[0];
  if ((c==nfCharQ)||(c==-nfCharQ))
    /*this field is already set*/  return;
  int i=0;
  while ((fftable[i]!=c) && (fftable[i]!=0)) i++;
  if (fftable[i]==0)
    return;
  if (nfCharQ > 1)
  {
    omFreeSize( (ADDRESS)nfPlus1Table,nfCharQ*sizeof(CARDINAL) );
    nfPlus1Table=NULL;
  }
  if ((c>1) || (c<0))
  {
    if (c>1) nfCharQ = c;
    else     nfCharQ = -c;
    char buf[100];
#ifdef macintosh
    sprintf(buf,"gftables:%d",nfCharQ);
#else
    sprintf(buf,"gftables/%d",nfCharQ);
#endif
    FILE * fp = feFopen(buf,"r",NULL,TRUE);
    if (fp==NULL)
    {
      return;
    }
    if(!fgets( buf, sizeof(buf), fp)) return;
    if(strcmp(buf,"@@ factory GF(q) table @@\n")!=0)
    {
      goto err;
    }
    if(!fgets( buf, sizeof(buf), fp))
    {
      goto err;
    }
    int q;
    sscanf(buf,"%d %d",&nfCharP,&q);
    nfReadMipo(buf);
    nfCharQ1=nfCharQ-1;
    //Print("nfCharQ=%d,nfCharQ1=%d,mipo=>>%s<<\n",nfCharQ,nfCharQ1,buf);
    nfPlus1Table= (CARDINAL *)omAlloc( (nfCharQ)*sizeof(CARDINAL) );
    int digs = gf_tab_numdigits62( nfCharQ );
    char * bufptr;
    int i = 1;
    int k;
    while ( i < nfCharQ )
    {
      fgets( buf, sizeof(buf), fp);
      //( strlen( buffer ) == (size_t)digs * 30, "illegal table" );
      bufptr = buf;
      k = 0;
      while ( (i < nfCharQ) && (k < 30) )
      {
        nfPlus1Table[i] = convertback62( bufptr, digs );
        if(nfPlus1Table[i]>nfCharQ)
        {
          Print("wrong entry %d: %d(%c%c%c)\n",i,nfPlus1Table[i],bufptr[0],bufptr[1],bufptr[2]);
        }
        bufptr += digs;
        if (nfPlus1Table[i]==nfCharQ)
        {
          if(i==nfCharQ1)
          {
            nfM1=0;
          }
          else
          {
            nfM1=i;
          }
        }
        i++; k++;
      }
    }
    nfPlus1Table[0]=nfPlus1Table[nfCharQ1];
  }
  else
    nfCharQ=0;
#ifdef LDEBUG
  nfTest((number)0);
#endif
  return;
err:
  Werror("illegal GF-table %d",nfCharQ);
}

/*2
* map Z/p -> GF(p,n)
*/
number nfMapP(number c)
{
  return nfInit((int)c);
}

/*2
* set map function nMap ... -> GF(p,n)
*/
BOOLEAN nfSetMap(ring r)
{
  if (rField_is_GF(r,nfCharQ))
  {
    nMap=ndCopy;   /* GF(p,n) -> GF(p,n) */
    return TRUE;
  }
  if (rField_is_Zp(r,nfCharP))
  {
    nMap=nfMapP;    /* Z/p -> GF(p,n) */
    return TRUE;
  }
  return FALSE;     /* default */
}
