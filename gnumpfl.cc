/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: gnumpfl.cc,v 1.7 1999-07-02 16:14:39 wenk Exp $ */
/*
* ABSTRACT: computations with GMP floating-point numbers
*
* ngf == number gnu floats
*/

#include "mod2.h"
#include "tok.h"
#include "febase.h"
#include "mmemory.h"
#include "numbers.h"
#include "modulop.h"
#include "longrat.h"

#include "gnumpfl.h"
#include "mpr_complex.h"

extern size_t gmp_output_digits;

static int ngfPrimeM;
static number ngfMapP(number from)
{
  number to;
  int save=npPrimeM;
  npPrimeM=ngfPrimeM;
  to = ngfInit(npInt(from));
  npPrimeM=save;
  return to;
}
static number ngfMapQ(number from)
{
  gmp_float *res= new gmp_float();
  *res= numberToFloat(from);
  return (number)res;
}

BOOLEAN ngfSetMap(int c, char ** par, int nop, number minpol)
{
  if (c == 0)
  {                      /* Q -> R      */
    nMap = ngfMapQ;
    return TRUE;
  }
  if (c>1)
  {
    if (par==NULL)
    {                    /* Z/p -> R    */
      nMap = ngfMapP;
      return TRUE;
    }
    else
    {                    /* GF(q) -> R  */
      return FALSE;
    }
  }
  else if (c<0)
     return FALSE;       /* Z/p(a) -> R */
  else if (c==1)
     return FALSE;       /* Q(a) -> R   */
  return FALSE;
}

void ngfNew (number * r)
{
  *r= NULL;
}

/*2
* n := i
*/
number ngfInit (int i)
{
  gmp_float* n= NULL;
  if ( i != 0 )
  {
    n= new gmp_float( i );
  }
  return (number)n;
}

/*2
* convert number to int
*/
int ngfInt(number &i)
{
  if ( i == NULL ) return 0;
  return (int)*(gmp_float*)i;
}

/*2
* delete a
*/
#ifdef LDEBUG
void ngfDBDelete (number * a,char *f, int l)
#else
void ngfDelete (number * a)
#endif
{
  if ( *a != NULL )
  {
    delete *(gmp_float**)a;
    *a=NULL;
  }
}

/*2
* copy a to b
*/
number ngfCopy(number a)
{
  gmp_float* b= NULL;
  if ( a !=  NULL )
  {
    b= new gmp_float( *(gmp_float*)a );
  }
  return (number)b;
}

/*2
* za:= - za
*/
number ngfNeg (number a)
{
  if ( a == NULL ) return NULL;
  *(gmp_float*)a= -(*(gmp_float*)a);
  return (number)a;
}

/*
* 1/a
*/
number ngfInvers(number a)
{
  gmp_float* r= NULL;
  if ( (a==NULL) /*|| ((gmp_float*)a)->isZero()*/ )
  {
    WerrorS("div. 1/0");
  }
  else
  {
    r= new gmp_float( (gmp_float)1 / (*(gmp_float*)a) );
  }
  return (number)r;
}

/*2
* u:= a + b
*/
number ngfAdd (number a, number b)
{
  gmp_float* r= NULL;
  if ( a==NULL && b==NULL )
  {
    return NULL;
  }
  else if ( a == NULL )
  {
    r= new gmp_float( *(gmp_float*)b );
  }
  else if ( b == NULL )
  {
    r= new gmp_float( *(gmp_float*)a );
  }
  else
  {
    r= new gmp_float( (*(gmp_float*)a) + (*(gmp_float*)b) );
  }
  return (number)r;
}

/*2
* u:= a - b
*/
number ngfSub (number a, number b)
{
  gmp_float* r= NULL;
  if ( a==NULL && b==NULL )
  {
    return NULL;
  }
  else if ( a == NULL )
  {
    r= new gmp_float( -(*(gmp_float*)b) );
  }
  else if ( b == NULL )
  {
    r= new gmp_float( *(gmp_float*)a );
  }
  else
  {
    r= new gmp_float( (*(gmp_float*)a) - (*(gmp_float*)b) );
  }
  return (number)r;
}

/*2
* u := a * b
*/
number ngfMult (number a, number b)
{
  gmp_float* r= NULL;
  if ( a==NULL || b==NULL )
  {
    return NULL;
  }
  else
  {
    r= new gmp_float( (*(gmp_float*)a) * (*(gmp_float*)b) );
  }
  return (number)r;
}

/*2
* u := a / b
*/
number ngfDiv (number a, number b)
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
  gmp_float* r= new gmp_float( (*(gmp_float*)a) / (*(gmp_float*)b) );
  return (number)r;
}

/*2
* u:= x ^ exp
*/
void ngfPower ( number x, int exp, number * u )
{
  if ( exp == 0 )
  { 
    gmp_float* n = new gmp_float(1);
    *u=(number)n; 
    return;
  }
  if ( exp == 1 )
  {
    nNew(u);
    if ( x == NULL )
    {
      gmp_float* n = new gmp_float();
      *u=(number)n;
    }
    else
    {
      gmp_float* n = new gmp_float();
      *n= *(gmp_float*)x;
      *u=(number)n; 
    }
    return;
  }

  ngfPower(x,exp-1,u);

  gmp_float *n=new gmp_float();
  *n=*(gmp_float*)x;
  *(gmp_float*)(*u) *= *(gmp_float*)n;

}

BOOLEAN ngfIsZero (number a)
{
  if ( a == NULL ) return TRUE;
  return ( ((gmp_float*)a)->isZero() );
}

/*2
* za >= 0 ?
*/
BOOLEAN ngfGreaterZero (number a)
{
  if ( a == NULL ) return TRUE;
  return ( (*(gmp_float*)a) >= (gmp_float)0.0 );
}

/*2
* a > b ?
*/
BOOLEAN ngfGreater (number a, number b)
{
  if ( a==NULL )
  {
    return (((gmp_float*)b)->sign() < 0);
  }
  if ( b==NULL )
  {
    return (((gmp_float*)a)->sign() < 0);
  }
  return ( (*(gmp_float*)a) > (*(gmp_float*)b) );
}

/*2
* a = b ?
*/
BOOLEAN ngfEqual (number a, number b)
{
  if ( a == NULL && b == NULL )
  {
    return TRUE;
  }
  else if ( a == NULL || b == NULL )
  {
    return FALSE;
  }
  return ( (*(gmp_float*)a) == (*(gmp_float*)b) );
}

/*2
* a == 1 ?
*/
BOOLEAN ngfIsOne (number a)
{
  if ( a == NULL ) return FALSE;
  return ((gmp_float*)a)->isOne();
}

/*2
* a == -1 ?
*/
BOOLEAN ngfIsMOne (number a)
{
  if ( a == NULL ) return FALSE;
  return ((gmp_float*)a)->isMOne();
}

/*2
* result =gcd(a,b)
* dummy, returns 1
*/
number ngfGcd(number a, number b)
{
  gmp_float *result= new gmp_float( 1 );
  return (number)result;
}

char * ngfEatFloatNExp( char * s )
{
  char *start= s;

  // eat floats (mantissa) like:
  //   0.394394993, 102.203003008,  .300303032
  while ((*s >= '0' && *s <= '9')||(*s == '.')) s++;

  // eat the exponent, starts with 'e' followed by '+', '-'
  // and digits, like:
  //   e-202, e+393
  if ( (s != start) && (*s == 'e') && ((*(s+1) == '+') || (*(s+1) == '-')) )
  {
    s=s+2; // eat e and sign
    while ((*s >= '0' && *s <= '9')) s++;
  }

  return s;
}

/*2
* extracts the number a from s, returns the rest
*/
char * ngfRead (char * s, number * a)
{
  char *start= s;

  //Print("%s\n",s);

  s= ngfEatFloatNExp( s );

  if (*s=='\0')  // 0
  {
    if ( *(gmp_float**)a == NULL ) (*(gmp_float**)a)= new gmp_float();
    (*(gmp_float**)a)->setFromStr(start);
  }
  else if (s==start)  // 1
  {
    if ( *(gmp_float**)a != NULL )  delete (*(gmp_float**)a);
    (*(gmp_float**)a)= new gmp_float(1);
  }
  else
  {
    if ( *s == '/' ) {
    }
    char c=*s;
    *s='\0';
    if ( *(gmp_float**)a == NULL ) (*(gmp_float**)a)= new gmp_float();
    (*(gmp_float**)a)->setFromStr(start);
    *s=c;
  }

 

  return s;
}

/*2
* write a floating point number
*/
void ngfWrite (number &a)
{
  char *out;
  if ( a ) {
    out= floatToStr(*(gmp_float*)a,gmp_output_digits);
    StringAppend(out);
    //Free((ADDRESS)out, (strlen(out)+1)* sizeof(char) );
    FreeL( (ADDRESS)out );
  } else {
    StringAppend("0");
  }
}

#ifdef LDEBUG
BOOLEAN ngfDBTest(number a, char *f, int l)
{
  return TRUE;
}
#endif

// local Variables: ***
// folded-file: t ***
// compile-command: "make installg" ***
// End: ***
