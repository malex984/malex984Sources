// ----------------------------------------------------------------------------
//  GMPrat.cc
//  begin of file
//  originally written by Gerd Sussner, sussner@mi.uni-erlangen.de
//  copied by Stephan Endrass, endrass@mathematik.uni-mainz.de
//  23.7.99
// ----------------------------------------------------------------------------

#define  GMPRAT_CC

#include "mod2.h"

#ifdef HAVE_SPECTRUM

#ifdef   GMPRAT_PRINT
#include <iostream.h>
#ifndef  GMPRAT_IOSTREAM
#include <stdio.h>
#endif
#endif

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "GMPrat.h"

// ----------------------------------------------------------------------------
//  Miscellaneous
// ----------------------------------------------------------------------------

Rational Rational::save;    // dummy variable
Rational RAT_EPS("1e-15");  // epsilon used in transcendental functions

// ----------------------------------------------------------------------------
//  the number Pi up to 25, 50, 75 and 100 digits
// ----------------------------------------------------------------------------

Rational RAT_PI_25("3.141592653589793238462643");
Rational RAT_PI_50("3.1415926535897932384626433832795028841971693993751");
Rational RAT_PI_75("3.141592653589793238462643383279502884197169399375105820974944592307816406286");
Rational RAT_PI_100("3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068");
Rational RAT_PI(RAT_PI_25);


// ----------------------------------------------------------------------------
//  disconnect a rational from its reference
// ----------------------------------------------------------------------------

void    Rational::disconnect( )
{
    if( p->n>1)
    {
        p->n--;
        p = new rep;
    }
    else
    {
        mpq_clear(p->rat);
    }
    mpq_init(p->rat);
}

// ----------------------------------------------------------------------------
//  Constructors
// ----------------------------------------------------------------------------

Rational::Rational( )
{
    p = new rep;
    mpq_init( p->rat );
}

Rational::Rational( long int a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_si( p->rat,a,1 );
}

Rational::Rational( unsigned long int a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_ui( p->rat,a,1 );
}

Rational::Rational( int a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_si( p->rat,(long int)a,1 );
}

Rational::Rational( unsigned int a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_ui( p->rat,(unsigned long)a,1 );
}

Rational::Rational( short int a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_si( p->rat,(long int)a,1 );
}

Rational::Rational( unsigned short int a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_ui( p->rat,(unsigned long int)a,1 );
}

Rational::Rational( char a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_si( p->rat,(long int)a,1 );
}

Rational::Rational( unsigned char a )
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_ui( p->rat,(unsigned long int)a,1 );
}

Rational::Rational( const Rational& a )
{
    a.p->n++;
    p=a.p;
}

Rational::Rational( double a )
{
    mpz_t   h1,h2;
    int     i=0;
    double  aa=a;

    p = new rep;
    mpq_init( p->rat );
    mpz_init_set_ui( h1,1 );

    while( fabs( 10.0*aa ) < DBL_MAX && i<DBL_DIG )
    {
        aa *= 10.;
        mpz_mul_ui( h1,h1,10 );
        i++;
    }
    mpz_init_set_d( h2,aa );
    mpq_set_num( p->rat,h2 );
    mpq_set_den( p->rat,h1 );
    mpq_canonicalize( p->rat );
    mpz_clear( h1 );
    mpz_clear( h2 );
}

Rational::Rational(float a)
{
    mpz_t h1,h2;
    int i=0;
    double aa=a;

    p=new rep;
    mpq_init(p->rat);
    mpz_init_set_ui(h1,1);
    while(fabs(10.*aa) < FLT_MAX && i<FLT_DIG){
        aa*=10.;
        mpz_mul_ui(h1,h1,10);
        i++;
    }
    mpz_init_set_d(h2,aa);
    mpq_set_num(p->rat,h2);
    mpq_set_den(p->rat,h1);
    mpq_canonicalize(p->rat);
    mpz_clear(h1);
    mpz_clear(h2);
}

// ----------------------------------------------------------------------------
//  Create a Rational from a string like "+1234.5678e-1234"
// ----------------------------------------------------------------------------

Rational::Rational( char *s )
{
    mpz_t   h1;
    int     exp=0,i=0;
    char    *s1,*s2,*ss;

    ss = new char[strlen(s)+1];
    strcpy( ss,s );
    s1 = ss;
    p = new rep;
    mpq_init( p->rat );
    if( isdigit(s1[0]) || s1[0]=='-' || s1[0]=='+' )
    {
        if (s1[0]=='+') ++s1;
        if (s1[0]=='-') ++i;

        while( isdigit( s1[i] ) )
        {
            ++i;
        }
        if (s1[i]=='.')
        {
            ++i;
            while( isdigit( s1[i] ) )
            {
                s1[i-1]=s1[i];
                ++i;
                --exp;
            }
            s1[i-1]='\0';
        }
        if (s1[i]=='e' || s1[i]=='E')
        {
            s2=s1+i+1;
        }
        else
            s2="";

        s1[i]='\0';
        i=exp+atoi(s2);
        mpz_init_set_str(h1,s1,10);
        delete[] ss;
        mpq_set_z(p->rat,h1);
        mpq_set_ui(save.p->rat,10,1);
        if (i>0)
        {
            for(int j=0;j<i;j++)
                mpq_mul(p->rat,p->rat,save.p->rat);
        }
        else if (i<0)
        {
            for(int j=0;j>i;j--)
                mpq_div(p->rat,p->rat,save.p->rat);
        }
        mpq_canonicalize(p->rat);
        mpz_clear(h1);
    }
}

// ----------------------------------------------------------------------------
//  Constructors with two arguments: numerator and denominator
// ----------------------------------------------------------------------------

Rational::Rational(long int a, unsigned long int b)
{
    p = new rep;
    mpq_init( p->rat );
    mpq_set_si( p->rat,a,b );
    mpq_canonicalize( p->rat );
}

Rational::Rational(unsigned long int a, unsigned long int b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_ui(p->rat,a,b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(int a, unsigned int b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,(long int) a,(unsigned long int) b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(unsigned int a, unsigned int b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_ui(p->rat,(unsigned long) a,(unsigned long int) b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(short int a, unsigned short int b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,(long int) a,(unsigned long int) b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(unsigned short int a, unsigned short int b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_ui(p->rat,(unsigned long int) a,(unsigned long int) b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(char a, unsigned char b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,(long int) a,(unsigned long int) b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(unsigned char a, unsigned char b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_set_ui(p->rat,(unsigned long int) a,(unsigned long int) b);
    mpq_canonicalize(p->rat);
}

Rational::Rational(const Rational& a, const Rational& b)
{
    p=new rep;
    mpq_init(p->rat);
    mpq_div(p->rat, a.p->rat, b.p->rat);
}

Rational::Rational(long int a, long int b)
{
    if (b<0) a=-a;
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,a,abs(b));
    mpq_canonicalize(p->rat);
}

Rational::Rational(int a, int b)
{
    if (b<0) a=-a;
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,(long int) a,(unsigned long int) abs(b));
    mpq_canonicalize(p->rat);
}

Rational::Rational(short int a, short int b)
{
    if (b<0) a=-a;
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,(long int) a,(unsigned long int) abs(b));
    mpq_canonicalize(p->rat);
}

Rational::Rational(char a, char b)
{
    if (b<0) a=-a;
    p=new rep;
    mpq_init(p->rat);
    mpq_set_si(p->rat,(long int) a,(unsigned long int) abs(b));
    mpq_canonicalize(p->rat);
}

Rational::Rational(char *sn, char *sd)
{
  Rational
    h=sd;

  p=new rep;
  mpq_init(p->rat);
  *this=sn;
  mpq_div(p->rat,p->rat,h.p->rat);
}

// ----------------------------------------------------------------------------
//  Destructor
// ----------------------------------------------------------------------------

Rational::~Rational()
{
  if (--p->n==0){
    mpq_clear(p->rat);
    delete p;
  }
}

// ----------------------------------------------------------------------------
//  Assignment operators
// ----------------------------------------------------------------------------

Rational& Rational::operator=(long int a)
{
  disconnect();
  mpq_set_si(p->rat,a,1);
  return *this;
}

Rational& Rational::operator=(unsigned long int a)
{
  disconnect();
  mpq_set_ui(p->rat,a,1);
  return *this;
}

Rational& Rational::operator=(int a)
{
  disconnect();
  mpq_set_si(p->rat,(long int) a,1);
  return *this;
}

Rational& Rational::operator=(unsigned int a)
{
  disconnect();
  mpq_set_ui(p->rat,(unsigned long int) a,1);
  return *this;
}

Rational& Rational::operator=(short int a)
{
  disconnect();
  mpq_set_si(p->rat,(long int) a,1);
  return *this;
}

Rational& Rational::operator=(unsigned short int a)
{
  disconnect();
  mpq_set_ui(p->rat,(unsigned long int) a,1);
  return *this;
}

Rational& Rational::operator=(char a)
{
  disconnect();
  mpq_set_si(p->rat,(long int) a,1);
  return *this;
}

Rational& Rational::operator=(unsigned char a)
{
  disconnect();
  mpq_set_ui(p->rat,(unsigned long int) a,1);
  return *this;
}

Rational& Rational::operator=(double a)
{
  mpz_t
    h1,
    h2;
  int
    i=0;
  double
    aa=a;

  disconnect();
  mpz_init_set_ui(h1,1);
  while(fabs(10.*aa) < DBL_MAX && i<DBL_DIG){
    aa*=10.;
    mpz_mul_ui(h1,h1,10);
    i++;
  }
  mpz_init_set_d(h2,aa);
  mpq_set_num(p->rat,h2);
  mpq_set_den(p->rat,h1);
  mpq_canonicalize(p->rat);
  mpz_clear(h1);
  mpz_clear(h2);
  return *this;
}

Rational& Rational::operator=(float a)
{
  mpz_t
    h1,
    h2;
  int
    i=0;
  double
    aa=a;

  disconnect();
  mpz_init_set_ui(h1,1);
  while(fabs(10.*aa) < FLT_MAX && i<FLT_DIG){
    aa*=10.;
    mpz_mul_ui(h1,h1,10);
    i++;
  }
  mpz_init_set_d(h2,aa);
  mpq_set_num(p->rat,h2);
  mpq_set_den(p->rat,h1);
  mpq_canonicalize(p->rat);
  mpz_clear(h1);
  mpz_clear(h2);
  return *this;
}

Rational& Rational::operator=(char *s)
{
  mpz_t
    h1;
  int
    exp=0,
    i=0;
  char
    *s1=s,
    *s2,
    *ss;

  ss=new char[strlen(s)+1];
  strcpy(ss,s);
  s1=ss;
  disconnect();
  if (isdigit(s1[0]) || s1[0]=='-' || s1[0]=='+'){
    if (s1[0]=='+') ++s1;
    if (s1[0]=='-') ++i;
    while(isdigit(s1[i]))
      ++i;
    if (s1[i]=='.'){
      ++i;
      while(isdigit(s1[i])){
        s1[i-1]=s1[i];
        ++i;
        --exp;
      }
      s1[i-1]='\0';
    }
    if (s1[i]=='e' || s1[i]=='E'){
      s2=s1+i+1;
    }
    else
      s2="";
    s1[i]='\0';
    i=exp+atoi(s2);
    mpz_init_set_str(h1,s1,10);
    delete[] ss;
    mpq_set_z(p->rat,h1);
    mpq_set_ui(save.p->rat,10,1);
    if (i>0){
      for(int j=0;j<i;j++)
        mpq_mul(p->rat,p->rat,save.p->rat);
    }
    else if (i<0){
      for(int j=0;j>i;j--)
        mpq_div(p->rat,p->rat,save.p->rat);
    }
    mpq_canonicalize(p->rat);
    mpz_clear(h1);
  }
  else
    mpq_set_ui(p->rat,0,1);
  return *this;
}

Rational& Rational::operator=(const Rational& a)
{
  a.p->n++;
  if (--p->n==0){
    mpq_clear(p->rat);
    delete p;
  }
  p=a.p;
  return *this;
}

// ----------------------------------------------------------------------------
//  Numerator and denominator
// ----------------------------------------------------------------------------

Rational Rational::get_num( )
{
    Rational erg;

    mpq_set_num( erg.p->rat,mpq_numref( p->rat ) );

    return  erg;
}

int Rational::get_num_si( )
{
    return  mpz_get_si( mpq_numref( p->rat ) );
}

Rational Rational::get_den( )
{
    Rational erg;

    mpq_set_num( erg.p->rat,mpq_denref( p->rat ) );

    return  erg;
}

int Rational::get_den_si( )
{
    return  mpz_get_si( mpq_denref( p->rat ) );
}

// ----------------------------------------------------------------------------
//  Casting
// ----------------------------------------------------------------------------

Rational::operator bool()
{
    if (mpq_sgn(p->rat)) return true;
    return false;
}

Rational::operator long int()
{
  mpz_t
    h;
  long int
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_si(h);
  mpz_clear(h);

  return ret_val;
}

Rational::operator unsigned long int()
{
  mpz_t
    h;
  unsigned long int
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_ui(h);
  mpz_clear(h);
  return ret_val;
}

Rational::operator int()
{
  mpz_t
    h;
  long int
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_si(h);
  mpz_clear(h);

  return ret_val;
}

Rational::operator unsigned int()
{
  mpz_t
    h;
  unsigned long int
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_ui(h);
  mpz_clear(h);
  return ret_val;
}

Rational::operator short int()
{
  mpz_t
    h;
  short int
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_si(h);
  mpz_clear(h);
  return ret_val;
}

Rational::operator unsigned short int()
{
  mpz_t
    h;
  unsigned short int
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_ui(h);
  mpz_clear(h);
  return ret_val;
}

Rational::operator char()
{
  mpz_t
    h;
  char
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_si(h);
  mpz_clear(h);
  return ret_val;
}

Rational::operator unsigned char()
{
  mpz_t
    h;
  unsigned char
    ret_val;

  mpz_init(h);
  mpz_tdiv_q(h,mpq_numref(p->rat),mpq_denref(p->rat));
  ret_val=mpz_get_ui(h);
  mpz_clear(h);
  return ret_val;
}

Rational::operator double()
{
  return mpq_get_d(p->rat);
}

Rational::operator float()
{
  return mpq_get_d(p->rat);
}

// ----------------------------------------------------------------------------
//  Unary minus
// ----------------------------------------------------------------------------

Rational
Rational::operator-()
{
  Rational
    erg;

  mpq_neg(erg.p->rat,p->rat);
  return erg;
}

Rational operator - ( const Rational &r )
{
  Rational
    erg;

  mpq_neg(erg.p->rat,r.p->rat);
  return erg;
}

// ----------------------------------------------------------------------------
//  Inverse
// ----------------------------------------------------------------------------

Rational
Rational::operator~()
{
  Rational
    erg;

  mpq_inv(erg.p->rat,p->rat);
  return erg;
}

// ----------------------------------------------------------------------------
//  +=, -= ...
// ----------------------------------------------------------------------------

Rational&
Rational::operator+=(const Rational &a)
{
  mpq_set(save.p->rat,p->rat);
  disconnect();
  mpq_add(p->rat,save.p->rat,a.p->rat);
  return *this;
}

Rational&
Rational::operator-=(const Rational &a)
{
  mpq_set(save.p->rat,p->rat);
  disconnect();
  mpq_sub(p->rat,save.p->rat,a.p->rat);
  return *this;
}

Rational&
Rational::operator*=(const Rational &a)
{
  mpq_set(save.p->rat,p->rat);
  disconnect();
  mpq_mul(p->rat,save.p->rat,a.p->rat);
  return *this;
}

Rational&
Rational::operator/=(const Rational &a)
{
  mpq_set(save.p->rat,p->rat);
  disconnect();
  mpq_div(p->rat,save.p->rat,a.p->rat);
  return *this;
}

// ----------------------------------------------------------------------------
//  Increment and decrement
// ----------------------------------------------------------------------------

Rational&
Rational::operator++()
{
  mpq_set(save.p->rat,p->rat);
  *this=1;
  mpq_add(p->rat,p->rat,save.p->rat);
  return *this;
}

Rational
Rational::operator++(int)
{
  Rational
    erg(*this);

  mpq_set(save.p->rat,p->rat);
  *this=1;
  mpq_add(p->rat,p->rat,save.p->rat);
  return erg;
}

Rational&
Rational::operator--()
{
  mpq_set(save.p->rat,p->rat);
  *this=1;
  mpq_sub(p->rat,save.p->rat,p->rat);
  return *this;
}

Rational
Rational::operator--(int)
{
  Rational
    erg(*this);

  mpq_set(save.p->rat,p->rat);
  *this=1;
  mpq_sub(p->rat,save.p->rat,p->rat);
  return erg;
}

// ----------------------------------------------------------------------------
//  Relational operators
// ----------------------------------------------------------------------------

bool operator<(const Rational& a,const Rational& b)
{
  if (mpq_cmp(a.p->rat,b.p->rat)<0) return true;
  return false;
}

bool operator<=(const Rational& a,const Rational& b)
{
  if (mpq_cmp(a.p->rat,b.p->rat)>0) return false;
  return true;
}

bool operator>(const Rational& a,const Rational& b)
{
  if (mpq_cmp(a.p->rat,b.p->rat)>0) return true;
  return false;
}

bool operator>=(const Rational& a,const Rational& b)
{
  if (mpq_cmp(a.p->rat,b.p->rat)<0) return false;
  return true;
}

bool operator==(const Rational& a,const Rational& b)
{
  if (mpq_equal(a.p->rat,b.p->rat)) return true;
  return false;
}

bool operator!=(const Rational& a,const Rational& b)
{
  if (mpq_equal(a.p->rat,b.p->rat)) return false;
  return true;
}

// ----------------------------------------------------------------------------
//  Ostream
// ----------------------------------------------------------------------------

#ifdef GMPRAT_PRINT
ostream& operator<< (ostream& s,const Rational& a)
{
    char *snum,*sdenom;

    snum   = mpz_get_str( NULL,10,mpq_numref(a.p->rat) );
    sdenom = mpz_get_str( NULL,10,mpq_denref(a.p->rat) );

    if( sdenom[0] == '1' && sdenom[1] == '\0' )
    {
        #ifdef GMPRAT_IOSTREAM
            s << snum;
        #else
            fprintf( stdout,snum );
        #endif
    }
    else
    {
        #ifdef GMPRAT_IOSTREAM
            s << snum << "/" << sdenom;
        #else
            fprintf( stdout,snum );
            fprintf( stdout,"/" );
            fprintf( stdout,sdenom );
        #endif
    }

    //free( snum );
    //free( sdenom );

    return s;
}
#endif

unsigned int Rational::length( ) const
{
    char *snum = (char*)NULL;
    char *sden = (char*)NULL;

    snum = mpz_get_str( snum,10,mpq_numref( p->rat ) );
    sden = mpz_get_str( sden,10,mpq_denref( p->rat ) );

    int length = strlen( snum );

    if( sden[0] != '1' || sden[1] != '\0' ) length += strlen( sden ) + 1;

    free( snum );
    free( sden );

    return  length;
}

// ----------------------------------------------------------------------------
//  Operators
// ----------------------------------------------------------------------------

Rational
operator+(const Rational& a,const Rational &b)
{
  Rational
    erg(a);

  return erg+=b;
}

Rational
operator-(const Rational& a,const Rational &b)
{
  Rational
    erg(a);

  return erg-=b;
}

Rational
operator*(const Rational& a,const Rational &b)
{
  Rational
    erg(a);

  return erg*=b;
}

Rational pow( const Rational& a,int e )
{
    Rational erg(1);

    for( int i=0; i<e; i++ )
    {
        erg *= a;
    }
    return erg;
}

Rational operator/(const Rational& a,const Rational &b)
{
  Rational
    erg(a);

  return erg/=b;
}

int sgn(const Rational& a)
{
  return mpq_sgn(a.p->rat);
}

Rational
abs(const Rational& a)
{
  Rational
    erg;

  if (mpq_sgn(a.p->rat)<0)
    mpq_neg(erg.p->rat,a.p->rat);
  else
    mpq_set(erg.p->rat,a.p->rat);
  return erg;
}

Rational
sqrt(const Rational& a)
{
  Rational
    erg;
  mpz_t
    h1,
    h2;

  mpz_init_set(h1,mpq_numref(a.p->rat));
  mpz_init(h2);
  mpz_set_ui(h2,1);
  mpz_mul(h1,h1,mpq_denref(a.p->rat));
  mpz_mul(h1,h1,mpq_denref(RAT_EPS.p->rat));
  mpz_mul(h1,h1,mpq_denref(RAT_EPS.p->rat));
  mpz_div(h1,h1,mpq_numref(RAT_EPS.p->rat));
  mpz_div(h1,h1,mpq_numref(RAT_EPS.p->rat));
  mpz_sqrt(h1,h1);
  mpz_mul(h2,mpq_denref(a.p->rat),mpq_denref(RAT_EPS.p->rat));
  mpz_div(h2,h2,mpq_numref(RAT_EPS.p->rat));
  mpq_set_num(erg.p->rat,h1);
  mpq_set_den(erg.p->rat,h2);
  mpq_canonicalize(erg.p->rat);
  mpz_clear(h1);
  mpz_clear(h2);
  return erg;
}

Rational
exp(const Rational& a)
{
  Rational
    erg(1),
    pow(a),
    fak(1),
    rem,
    i(2);

  do{
    erg+=pow/fak;
    fak*=i++;
    pow*=a;
    rem=(((Rational)2)*abs(pow))/fak;
    if (rem<RAT_EPS) break;
  } while (true);
  return erg;
}

Rational
sin(const Rational& a)
{
  Rational
    erg(a),
    pow(a),
    fak(6),
    rem,
    i(4),
    aq(a);

  aq*=a;
  do {
    pow*=-aq;
    erg+=pow/fak;
    fak*=i++;
    fak*=i++;
    rem=(abs(pow*aq))/fak;
    if (rem<RAT_EPS) break;
  } while(true);
  return erg;
}

Rational
cos(const Rational& a)
{
  Rational
    erg(1),
    pow(1),
    fak(2),
    rem,
    i(3),
    aq(a);

  aq*=a;
  do {
    pow*=-aq;
    erg+=pow/fak;
    fak*=i++;
    fak*=i++;
    rem=(abs(pow*aq))/fak;
    if (rem<RAT_EPS) break;
  } while(true);
  return erg;
}

Rational
tan(const Rational& a)
{
  Rational
    ergsin(a),
    ergcos(1),
    pow(a),
    fak(2),
    i(3),
    rem,
    eps2(((Rational)2)*RAT_EPS);

  do{
    pow*=a*(-1);
    ergcos+=pow/fak;
    pow*=a;
    fak*=i++;
    ergsin+=pow/fak;
    fak*=i++;
    rem=(abs(pow*a*a))/fak;
    if (rem<eps2) break;
  } while(true);
  return ergsin/ergcos;
}

Rational gcd( const Rational &a,const Rational &b )
{
    if( a == 0 )
    {
        if( b == 0 )
        {
            return  (Rational)1;
        }
        else
        {
            return  abs( b );
        }
    }
    else if( b == 0 )
    {
        return  abs( a );
    }

    Rational erg;

    mpz_gcd( mpq_numref( erg.p->rat ),
            mpq_numref( a.p->rat ),mpq_numref( b.p->rat ) );
    mpz_gcd( mpq_denref( erg.p->rat ),
            mpq_denref( a.p->rat ),mpq_denref( b.p->rat ) );

    //mpq_canonicalize( erg.p->rat );

    return  abs( erg );
}

Rational gcd( Rational *a,int n )
{
    if( n == 1 )
    {
        return  a[0];
    }

    Rational g = gcd( a[0],a[1] );

    for( int i=2; i<n; i++ )
    {
        g = gcd( g,a[i] );
    }

    return  g;
}

Rational lcm( const Rational &a,const Rational &b )
{
    if( a == 0 )
    {
        return b;
    }
    else if( b == 0 )
    {
        return a;
    }

    return a*b/gcd(a,b);
}

Rational lcm( Rational *a,int n )
{
    if( n == 1 )
    {
        return  a[0];
    }

    Rational g = lcm( a[0],a[1] );

    for( int i=2; i<n; i++ )
    {
        g = lcm( g,a[i] );
    }

    return  g;
}

double  Rational::complexity( ) const
{
    double num = mpz_get_d( mpq_numref( p->rat ) );
    double den = mpz_get_d( mpq_denref( p->rat ) );

    if( num < 0 ) num = -num;
    if( den < 0 ) den = -den;

    return  ( num > den ? num : den );
}

#endif /* HAVE_SPECTRUM */
// ----------------------------------------------------------------------------
//  GMPrat.cc
//  end of file
// ----------------------------------------------------------------------------
