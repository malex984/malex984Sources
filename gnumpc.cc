/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: gnumpc.cc,v 1.16 2000-12-15 18:49:28 Singular Exp $ */
/*
* ABSTRACT: computations with GMP complex floating-point numbers
*
* ngc == number gnu complex
*/

#include "mod2.h"
#include "tok.h"
#include "febase.h"
#include "omalloc.h"
#include "numbers.h"
#include "longrat.h"

#include "gnumpc.h"
#include "gnumpfl.h"
#include "mpr_complex.h"

extern size_t gmp_output_digits;


static number ngcMapQ(number from)
{
  if ( from != NULL )
  {
    gmp_complex *res=new gmp_complex(numberFieldToFloat(from,QTOF));
    return (number)res;
  }
  else
    return NULL;
}

nMapFunc ngcSetMap(ring src,ring dst)
{
  if (rField_is_long_C(src))
  {
    return ngcCopy;
  }
  if(rField_is_Q(src))
  {
    return ngcMapQ;
  }
  return NULL;
}

number   ngcPar(int i)
{
  gmp_complex* n= new gmp_complex( (long)0, (long)1 );
  return (number)n;
}

void ngcNew (number * r)
{
  *r=NULL;
}

/*2
* n := i
*/
number ngcInit (int i)
{
  gmp_complex* n= NULL;
  if ( i != 0 )
  {
    n= new gmp_complex( (long)i, (long)0 );
  }
  return (number)n;
}

/*2
* convert number to int
*/
int ngcInt(number &i)
{
  if ( i == NULL ) return 0;
  return (int)((gmp_complex*)i)->real();
}

/*2
* delete a
*/
#ifdef LDEBUG
void ngcDBDelete (number * a,char *f, int l)
#else
void ngcDelete (number * a)
#endif
{
  if ( *a != NULL ) {
    delete *(gmp_complex**)a;
    *a=NULL;
  }
}

/*2
* copy a to b
*/
number ngcCopy(number a)
{
  gmp_complex* b= NULL;
  if ( a !=  NULL )
  {
    b= new gmp_complex( *(gmp_complex*)a );
  }
  return (number)b;
}

/*2
* za:= - za
*/
number ngcNeg (number a)
{
  if ( a == NULL ) return NULL;
  number m1=nInit(-1);
  a=ngcMult(m1,a);
  return (number)a;
}

/*
* 1/a
*/
number ngcInvers(number a)
{
  gmp_complex* r= NULL;
  if ( (a==NULL) /*|| ((gmp_complex*)a)->isZero()*/ )
  {
    WerrorS("div. 1/0");
  }
  else
  {
    r= new gmp_complex( (gmp_complex)1 / (*(gmp_complex*)a) );
  }
  return (number)r;
}

/*2
* u:= a + b
*/
number ngcAdd (number a, number b)
{
  gmp_complex* r= NULL;
  if ( a==NULL && b==NULL )
  {
    return NULL;
  }
  else if ( a == NULL )
  {
    r= new gmp_complex( *(gmp_complex*)b );
  }
  else if ( b == NULL )
  {
    r= new gmp_complex( *(gmp_complex*)a );
  }
  else
  {
    r= new gmp_complex( (*(gmp_complex*)a) + (*(gmp_complex*)b) );
  }
  return (number)r;
}

/*2
* u:= a - b
*/
number ngcSub (number a, number b)
{
  gmp_complex* r= NULL;
  if ( a==NULL && b==NULL )
  {
    return NULL;
  }
  else if ( a == NULL )
  {
    r= new gmp_complex( (*(gmp_complex*)b) );
    r= (gmp_complex *)ngcNeg((number)r);
  }
  else if ( b == NULL )
  {
    r= new gmp_complex( *(gmp_complex*)a );
  }
  else
  {
    r= new gmp_complex( (*(gmp_complex*)a) - (*(gmp_complex*)b) );
  }
  return (number)r;
}

/*2
* u := a * b
*/
number ngcMult (number a, number b)
{
  gmp_complex* r= NULL;
  if ( a==NULL || b==NULL )
  {
    return NULL;
  }
  else
  {
    r= new gmp_complex( (*(gmp_complex*)a) * (*(gmp_complex*)b) );
  }
  return (number)r;
}

/*2
* u := a / b
*/
number ngcDiv (number a, number b)
{
  if ( b==NULL /*|| ((gmp_float*)b)->isZero()*/ )
  {
    // a/0 = error
    WerrorS("div. 1/0");
    return NULL;
  }
  else if ( a==NULL )
  {
    // 0/b = 0
    return NULL;
  }
  gmp_complex* r= new gmp_complex( (*(gmp_complex*)a) / (*(gmp_complex*)b) );
  return (number)r;
}

/*2
* u:= x ^ exp
*/
void ngcPower ( number x, int exp, number * u )
{
  if ( exp == 0 )
  {
    gmp_complex* n = new gmp_complex(1);
    *u=(number)n;
    return;
  }
  if ( exp == 1 )
  {
    nNew(u);
    if ( x == NULL )
    {
      gmp_complex* n = new gmp_complex();
      *u=(number)n;
    }
    else
    {
      gmp_complex* n = new gmp_complex();
      *n= *(gmp_complex*)x;
      *u=(number)n;
    }
    return;
  }
  ngcPower(x,exp-1,u);
  gmp_complex *n=new gmp_complex();
  *n=*(gmp_complex*)x;
  *(gmp_complex*)(*u) *= *(gmp_complex*)n;
  delete n;
}

BOOLEAN ngcIsZero (number a)
{
  if ( a == NULL ) return TRUE;
  return ( ((gmp_complex*)a)->real().isZero() && ((gmp_complex*)a)->imag().isZero());
}

number ngcRePart(number a)
{
  if (((gmp_complex*)a)->real().isZero()) return NULL;
  gmp_complex* n = new gmp_complex(((gmp_complex*)a)->real());
  return (number)n;
}  
  
number ngcImPart(number a)
{
  if (((gmp_complex*)a)->imag().isZero()) return NULL;
  gmp_complex* n = new gmp_complex(((gmp_complex*)a)->imag());
  return (number)n;
}  
  
/*2
* za >= 0 ?
*/
BOOLEAN ngcGreaterZero (number a)
{
  if ( a == NULL ) return TRUE;
  if ( ! ((gmp_complex*)a)->imag().isZero() )
    return ( abs( *(gmp_complex*)a).sign() >= 0 );
  else
    return ( ((gmp_complex*)a)->real().sign() >= 0 );
}

/*2
* a > b ?
*/
BOOLEAN ngcGreater (number a, number b)
{
  if ( a==NULL )
  {
    return (((gmp_complex*)b)->real().sign() < 0);
  }
  if ( b==NULL )
  {
    return (((gmp_complex*)a)->real().sign() < 0);
  }
  return FALSE;
}

/*2
* a = b ?
*/
BOOLEAN ngcEqual (number a, number b)
{
  if ( a == NULL && b == NULL )
  {
    return TRUE;
  }
  else if ( a == NULL || b == NULL )
  {
    return FALSE;
  }
  return ( (*(gmp_complex*)a) == (*(gmp_complex*)b) );
}

/*2
* a == 1 ?
*/
BOOLEAN ngcIsOne (number a)
{
  if ( a == NULL ) return FALSE;
  //return (((gmp_complex*)a)->real().isOne() && ((gmp_complex*)a)->imag().isZero());
  return (((gmp_complex*)a)->real().isOne());
}

/*2
* a == -1 ?
*/
BOOLEAN ngcIsMOne (number a)
{
  if ( a == NULL ) return FALSE;
  //  return (((gmp_complex*)a)->real().isMOne() && ((gmp_complex*)a)->imag().isZero());
  return (((gmp_complex*)a)->real().isMOne());
}

/*2
* extracts the number a from s, returns the rest
*/
char * ngcRead (char * s, number * a)
{
  char *start= s;
  if ((*s >= '0') && (*s <= '9'))
  {
    gmp_float *re=NULL;
    s=ngfRead(s,(number *)&re);
    gmp_complex *aa=new gmp_complex(*re);
    *a=(number)aa;
    delete re;
  }
  else if (strncmp(s,currRing->parameter[0],strlen(currRing->parameter[0]))==0)
  {
    s+=strlen(currRing->parameter[0]);
    gmp_complex *aa=new gmp_complex((long)0,(long)1);
    *a=(number)aa;
  }
  else
  {
    *a=(number) new gmp_complex((long)1);
  }
  return s;
}

/*2
* write a floating point number
*/
void ngcWrite (number &a)
{
  if (a==NULL)
    StringAppend("0");
  else
  {
    char *out;
    out= complexToStr(*(gmp_complex*)a,gmp_output_digits);
    StringAppend(out);
    //    omFreeSize((ADDRESS)out, (strlen(out)+1)* sizeof(char) );
    omFree( (ADDRESS)out );
  }
}

#ifdef LDEBUG
BOOLEAN ngcDBTest(number a, char *f, int l)
{
  return TRUE;
}
#endif

// local Variables: ***
// folded-file: t ***
// compile-command: "make installg" ***
// End: ***
