/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: gnumpfl.cc,v 1.20 2001-01-30 11:45:47 pohl Exp $ */
/*
* ABSTRACT: computations with GMP floating-point numbers
*
* ngf == number gnu floats
*/

#include "mod2.h"
#include "tok.h"
#include "febase.h"
#include "omalloc.h"
#include "numbers.h"
#include "modulop.h"
#include "longrat.h"

#include "gnumpfl.h"
#include "mpr_complex.h"

extern size_t gmp_output_digits;

static number ngfMapP(number from)
{
  if ( from != NULL)
    return ngfInit(npInt(from));
  else
    return NULL;
}
static number ngfMapQ(number from)
{
  if ( from != NULL )
  {
    gmp_float *res=new gmp_float(numberFieldToFloat(from,QTOF));
    return (number)res;
  }
  else
    return NULL;
}
union nf
{
  float _f;
  number _n;
  nf(float f) {_f = f;}
  nf(number n) {_n = n;}
  float F() const {return _f;}
  number N() const {return _n;}
};
static number ngfMapR(number from)
{
  if ( from != NULL )
  {
    gmp_float *res=new gmp_float((double)nf(from).F());
    return (number)res;
  }
  else
    return NULL;
}
static number ngfMapC(number from)
{
  if ( (from != NULL) || ((gmp_complex*)from)->real().isZero() )
  {
    gmp_float *res=new gmp_float(((gmp_complex*)from)->real());
    return (number)res;
  }
  else
    return NULL;
}

nMapFunc ngfSetMap(ring src, ring dst)
{
  if (rField_is_Q(src))
  {
    return ngfMapQ;
  }
  if (rField_is_long_R(src))
  {
    return ngfCopy;
  }
  if (rField_is_R(src))
  {
    return ngfMapR;
  }
  if (rField_is_Zp(src))
  {
    return ngfMapP;
  }
  if (rField_is_long_C(src))
  {
    return ngfMapC;
  }
  return NULL;
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
    n= new gmp_float( (double)i );
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
void ngfDelete (number * a, const ring r)
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
  delete (gmp_float*)n;
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
    return (((gmp_float*)a)->sign() > 0);
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
    gmp_float divisor(1.0);
    char *start2=s;
    if ( *s == '/' )
    {
      s++;
      s= ngfEatFloatNExp( s );
      if (s!= start2+1)
      {
        char tmp_c=*s;
        *s='\0';
        divisor.setFromStr(start2+1);
        *s=tmp_c;
      }
      else
      {
        Werror("wrong long real format: %s",start2);
      }
    }
    char c=*start2;
    *start2='\0';
    if ( *(gmp_float**)a == NULL ) (*(gmp_float**)a)= new gmp_float();
    (*(gmp_float**)a)->setFromStr(start);
    *start2=c;
    (**(gmp_float**)a) /= divisor;
  }

  return s;
}

/*2
* write a floating point number
*/
void ngfWrite (number &a)
{
  char *out;
  if ( a != NULL )
  {
    out= floatToStr(*(gmp_float*)a,gmp_output_digits);
    StringAppend(out);
    //omFreeSize((ADDRESS)out, (strlen(out)+1)* sizeof(char) );
    omFree( (ADDRESS)out );
  }
  else
  {
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
