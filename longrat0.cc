/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: longrat0.cc,v 1.12 2000-09-18 09:19:14 obachman Exp $ */
/*
* ABSTRACT -
* IO for long rational numbers (Hubert Grassmann)
*/

#include <stdio.h>
#include <string.h>
#include "mod2.h"
#include "tok.h"
#include "omalloc.h"
#include "febase.h"
#include "longrat.h"

#define SR_HDL(A) ((long)(A))
#define SR_INT    1
#define INT_TO_SR(INT)  ((number) (((long)INT << 2) + SR_INT))
#define SR_TO_INT(SR)   (((long)SR) >> 2)


/*2
* extracts a long integer from s, returns the rest
*/
char * nlEatLong(char *s, MP_INT *i)
{
  char * start=s;

  while (*s >= '0' && *s <= '9') s++;
  if (*s=='\0')
  {
    mpz_set_str(i,start,10);
  }
  else
  {
    char c=*s;
    *s='\0';
    mpz_set_str(i,start,10);
    *s=c;
  }
  return s;
}

/*2
* extracts the number a from s, returns the rest
*/
char * nlRead (char *s, number *a)
{
  if (*s<'0' || *s>'9')
  {
    *a = nlInit(1);
    return s;
  }
  *a=(number)omAllocBin(rnumber_bin);
  {
    (*a)->s = 3;
#if defined(LDEBUG)
    (*a)->debug=123456;
#endif
    MP_INT *z=&((*a)->z);
    MP_INT *n=&((*a)->n);
    mpz_init(z);
    s = nlEatLong(s, z);
    if (*s == '/')
    {
      mpz_init(n);
      (*a)->s = 0;
      s++;
      s = nlEatLong(s, n);
      if (mpz_cmp_si(n,(long)0)==0)
      {
        WerrorS("Zero Denominator");
        mpz_clear(n);
        (*a)->s = 3;
      }
      else if (mpz_cmp_si(n,(long)1)==0)
      {
        mpz_clear(n);
        (*a)->s=3;
      }
    }
    if (mpz_cmp_si(z,(long)0)==0)
    {
      *a=INT_TO_SR(0);
    }
    else
    if ((*a)->s==3)
    {
      int ui=(int)mpz_get_si(&(*a)->z);
      if ((((ui<<3)>>3)==ui)
      && (mpz_cmp_si(&(*a)->z,(long)ui)==0))
      {
        mpz_clear(&(*a)->z);
        omFreeBin((ADDRESS)(*a), rnumber_bin);
        (*a)=INT_TO_SR(ui);
      }
    }
    else
      nlNormalize(*a);
  }
  return s;
}

/*2
* write a rational number
*/
void nlWrite (number &a)
{
  char *s,*z;
  if (SR_HDL(a) & SR_INT)
  {
    StringAppend("%d",SR_TO_INT(a));
  }
  else if (a==NULL)
  {
    StringAppendS("o");
  }
  else
  {
    if (a->s==0)
    {
      nlNormalize(a);
      nlWrite(a);
      return;
    }
    int l=mpz_sizeinbase(&a->z,10);
    if (a->s<2) l=max(l,mpz_sizeinbase(&a->n,10));
    l+=2;
    s=(char*)omAlloc(l);
    z=mpz_get_str(s,10,&a->z);
    StringAppendS(z);
    if (a->s!=3)
    {
      StringAppendS("/");
      z=mpz_get_str(s,10,&a->n);
      StringAppendS(z);
    }
    omFreeSize((ADDRESS)s,l);
  }
}

