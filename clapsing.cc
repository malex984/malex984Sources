// emacs edit mode for this file is -*- C++ -*-
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
// $Id: clapsing.cc,v 1.51 1999-06-14 16:25:42 Singular Exp $
/*
* ABSTRACT: interface between Singular and factory
*/

#include "mod2.h"
#ifdef HAVE_FACTORY
#define SI_DONT_HAVE_GLOBAL_VARS
#include "tok.h"
#include "clapsing.h"
#include "ipid.h"
#include "numbers.h"
#include "subexpr.h"
#include "ipshell.h"
#include "ring.h"
#include <factory.h>
#include "clapconv.h"
#ifdef HAVE_LIBFAC_P
#include <factor.h>
#endif
#include "ring.h"

//
// FACTORY_GCD_TEST: use new gcd instead of old one.  Does not work
//   without new gcd-implementation which is not publicly available.
//
// FACTORY_GCD_STAT: print statistics on polynomials.  Works only
//   with the file `gcd_stat.cc' and `gcd_stat.h which may be found
//   in the repository, module `factory-devel'.
//   Overall statistics may printed using `system("gcdstat");'.
//
// FACTORY_GCD_TIMING: accumulate time used for gcd calculations.
//   Time may be printed (and reset) with `system("gcdtime");'.
//   For this define, `timing.h' from the factory source directory
//   has to be copied to the Singular source directory.
//   Note: for better readability, the macros `TIMING_START()' and
//   `TIMING_END()' are used in any case.  However, they expand to
//   nothing if `FACTORY_GCD_TIMING' is off.
//
// FACTORY_GCD_DEBOUT: print polynomials involved in gcd calculations.
//   The polynomials are printed by means of the macros
//   `FACTORY_*OUT_POLY' which are defined to be empty if
//   `FACTORY_GCD_DEBOUT' is off.
//
// FACTORY_GCD_DEBOUT_PATTERN: print degree patterns of polynomials
//   involved in gcd calculations.
//   The patterns are printed by means of the macros
//   `FACTORY_*OUT_PAT' which are defined to be empty if
//   `FACTORY_GCD_DEBOUT_PATTERN' is off.
//
//   A degree pattern looks like this:
//
//   totDeg  size    deg(v1) deg(v2) ...
//
//   where "totDeg" means total degree, "size" the number of terms,
//   and "deg(vi)" is the degree with respect to variable i.
//   In univariate case, the "deg(vi)" are missing.  For this feature
//   you need the files `gcd_stat.cc' and `gcd_stat.h'.
//
//
// And here is what the functions print if `FACTORY_GCD_DEBOUT' (1),
// `FACTORY_GCD_STAT' (2), or `FACTORY_GCD_DEBOUT_PATTERN' (3) is on:
//
// sinclap_divide_content:
// (1) G = <firstCoeff>
// (3) G#= <firstCoeff, pattern>
// (1) h = <nextCoeff>
// (3) h#= <nextCoeff, pattern>
// (2) gcnt: <statistics on gcd as explained above>
// (1) g = <intermediateResult>
// (3) g#= <intermediateResult, pattern>
// (1) h = <nextCoeff>
// (3) h#= <nextCoeff, pattern>
// (2) gcnt: <statistics on gcd as explained above>
//  ...
// (1) h = <lastCoeff>
// (3) h#= <lastCoeff, pattern>
// (1) g = <finalResult>
// (3) g#= <finalResult, pattern>
// (2) gcnt: <statistics on gcd as explained above>
// (2) cont: <statistics on content as explained above>
//
// singclap_alglcm:
// (1) f = <inputPolyF>
// (3) f#= <inputPolyF, pattern>
// (1) g = <inputPolyG>
// (3) g#= <inputPolyG, pattern>
// (1) d = <its gcd>
// (3) d#= <its gcd, pattern>
// (2) alcm: <statistics as explained above>
//
// singclap_algdividecontent:
// (1) f = <inputPolyF>
// (3) f#= <inputPolyF, pattern>
// (1) g = <inputPolyG>
// (3) g#= <inputPolyG, pattern>
// (1) d = <its gcd>
// (3) d#= <its gcd, pattern>
// (2) acnt: <statistics as explained above>
//

#ifdef FACTORY_GCD_STAT
#include "gcd_stat.h"
#define FACTORY_GCDSTAT( tag, f, g, d ) \
  printGcdStat( tag, f, g, d )
#define FACTORY_CONTSTAT( tag, f ) \
  printContStat( tag, f )
#else
#define FACTORY_GCDSTAT( tag, f, g, d )
#define FACTORY_CONTSTAT( tag, f )
#endif

#ifdef FACTORY_GCD_TIMING
#define TIMING
#include "timing.h"
TIMING_DEFINE_PRINT( contentTimer );
TIMING_DEFINE_PRINT( algContentTimer );
TIMING_DEFINE_PRINT( algLcmTimer );
#else
#define TIMING_START( timer )
#define TIMING_END( timer )
#endif

#ifdef FACTORY_GCD_DEBOUT
#include "longalg.h"
#include "febase.h"
// alg f
#define FACTORY_ALGOUT_POLY( tag, f ) \
  StringSetS( tag ); \
  napWrite( f ); \
  PrintS(StringAppendS("\n"));
// CanonicalForm f, represents transcendent extension
#define FACTORY_CFTROUT_POLY( tag, f ) \
  { \
    alg F=convClapPSingTr( f ); \
    StringSetS( tag ); \
    napWrite( F ); \
    PrintS(StringAppendS("\n")); \
    napDelete( &F ); \
  }
// CanonicalForm f, represents algebraic extension
#define FACTORY_CFAOUT_POLY( tag, f ) \
  { \
    alg F=convClapASingA( f ); \
    StringSetS( tag ); \
    napWrite( F ); \
    PrintS(StringAppendS("\n")); \
    napDelete( &F ); \
  }
#else /* ! FACTORY_GCD_DEBOUT */
#define FACTORY_ALGOUT_POLY( tag, f )
#define FACTORY_CFTROUT_POLY( tag, f )
#define FACTORY_CFAOUT_POLY( tag, f )
#endif /* ! FACTORY_GCD_DEBOUT */

#ifdef FACTORY_GCD_DEBOUT_PATTERN
// alg f
#define FACTORY_ALGOUT_PAT( tag, f ) \
  if (currRing->minpoly!=NULL) \
  { \
    CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z); \
    Variable a=rootOf(mipo); \
    printPolyPattern( tag, convSingAClapA( f,a ), rPar( currRing ) ); \
  } \
  else \
  { \
    printPolyPattern( tag, convSingTrClapP( f ), rPar( currRing ) ); \
  }
// CanonicalForm f, represents transcendent extension
#define FACTORY_CFTROUT_PAT( tag, f ) printPolyPattern( tag, f, rPar( currRing ) )
// CanonicalForm f, represents algebraic extension
#define FACTORY_CFAOUT_PAT( tag, f ) printPolyPattern( tag, f, rPar( currRing ) )
#else /* ! FACTORY_GCD_DEBOUT_PATTERN */
#define FACTORY_ALGOUT_PAT( tag, f )
#define FACTORY_CFTROUT_PAT( tag, f )
#define FACTORY_CFAOUT_PAT( tag, f )
#endif /* ! FACTORY_GCD_DEBOUT_PATTERN */

// these macors combine both print macros
#define FACTORY_ALGOUT( tag, f ) \
  FACTORY_ALGOUT_POLY( tag " = ", f ); \
  FACTORY_ALGOUT_PAT( tag "#= ", f )
#define FACTORY_CFTROUT( tag, f ) \
  FACTORY_CFTROUT_POLY( tag " = ", f ); \
  FACTORY_CFTROUT_PAT( tag "#= ", f )
#define FACTORY_CFAOUT( tag, f ) \
  FACTORY_CFAOUT_POLY( tag " = ", f ); \
  FACTORY_CFAOUT_PAT( tag "#= ", f )





poly singclap_gcd ( poly f, poly g )
{
  poly res=NULL;

  if (f!=NULL) pCleardenom(f);
  if (g!=NULL) pCleardenom(g);
  else         return pCopy(f); // g==0 => gcd=f (but do a pCleardenom)
  if (f==NULL) return pCopy(g); // f==0 => gcd=g (but do a pCleardenom)

  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPClapP( f ) ), G( convSingPClapP( g ) );
    res=convClapPSingP( gcd( F, G ) );
    Off(SW_RATIONAL);
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    if (currRing->minpoly!=NULL)
    {
      if ( nGetChar()==1 ) /* Q(a) */
      {
        WerrorS( feNotImplemented );
      }
      else
      {
        CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
        Variable a=rootOf(mipo);
        CanonicalForm F( convSingAPClapAP( f,a ) ), G( convSingAPClapAP( g,a ) );
        res= convClapAPSingAP( gcd( F, G ) );
      }
    }
    else
    {
      CanonicalForm F( convSingTrPClapP( f ) ), G( convSingTrPClapP( g ) );
      res= convClapPSingTrP( gcd( F, G ) );
    }
    Off(SW_RATIONAL);
  }
  #if 0
  else if (( nGetChar()>1 )&&(currRing->parameter!=NULL)) /* GF(q) */
  {
    int p=rChar(currRing);
    int n=2;
    int t=p*p;
    while (t!=nChar) { t*=p;n++; }
    setCharacteristic(p,n,'a');
    CanonicalForm F( convSingGFClapGF( f ) ), G( convSingGFClapGF( g ) );
    res= convClapGFSingGF( gcd( F, G ) );
  }
  #endif
  else
    WerrorS( feNotImplemented );

  pDelete(&f);
  pDelete(&g);
  pTest(res);
  return res;
}

poly singclap_resultant ( poly f, poly g , poly x)
{
  int i=pIsPurePower(x);
  if (i==0)
  {
    WerrorS("3rd argument must be a ring variable");
    return NULL;
  }
  if ((f==NULL) || (g==NULL))
    return NULL;
  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    Variable X(i);
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPClapP( f ) ), G( convSingPClapP( g ) );
    poly res=convClapPSingP( resultant( F, G, X ) );
    Off(SW_RATIONAL);
    return res;
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    poly res;
    if (currRing->minpoly!=NULL)
    {
      Variable X(i);
      CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPClapAP( f,a ) ), G( convSingAPClapAP( g,a ) );
      res= convClapAPSingAP( resultant( F, G, X ) );
    }
    else
    {
      Variable X(i+rPar(currRing));
      CanonicalForm F( convSingTrPClapP( f ) ), G( convSingTrPClapP( g ) );
      res= convClapPSingTrP( resultant( F, G, X ) );
    }
    Off(SW_RATIONAL);
    return res;
  }
  else
    WerrorS( feNotImplemented );
  return NULL;
}
//poly singclap_resultant ( poly f, poly g , poly x)
//{
//  int i=pVar(x);
//  if (i==0)
//  {
//    WerrorS("ringvar expected");
//    return NULL;
//  }
//  ideal I=idInit(1,1);
//
//  // get the coeffs von f wrt. x:
//  I->m[0]=pCopy(f);
//  matrix ffi=mpCoeffs(I,i);
//  ffi->rank=1;
//  ffi->ncols=ffi->nrows;
//  ffi->nrows=1;
//  ideal fi=(ideal)ffi;
//
//  // get the coeffs von g wrt. x:
//  I->m[0]=pCopy(g);
//  matrix ggi=mpCoeffs(I,i);
//  ggi->rank=1;
//  ggi->ncols=ggi->nrows;
//  ggi->nrows=1;
//  ideal gi=(ideal)ggi;
//
//  // contruct the matrix:
//  int fn=IDELEMS(fi); //= deg(f,x)+1
//  int gn=IDELEMS(gi); //= deg(g,x)+1
//  matrix m=mpNew(fn+gn-2,fn+gn-2);
//  if(m==NULL)
//  {
//    return NULL;
//  }
//
//  // enter the coeffs into m:
//  int j;
//  for(i=0;i<gn-1;i++)
//  {
//    for(j=0;j<fn;j++)
//    {
//      MATELEM(m,i+1,fn-j+i)=pCopy(fi->m[j]);
//    }
//  }
//  for(i=0;i<fn-1;i++)
//  {
//    for(j=0;j<gn;j++)
//    {
//      MATELEM(m,gn+i,gn-j+i)=pCopy(gi->m[j]);
//    }
//  }
//
//  poly r=mpDet(m);
//
//  idDelete(&fi);
//  idDelete(&gi);
//  idDelete((ideal *)&m);
//  return r;
//}

lists singclap_extgcd ( poly f, poly g )
{
  // for now there is only the possibility to handle univariate
  // polynomials over
  // Q and Fp ...
  poly res=NULL,pa=NULL,pb=NULL;
  On(SW_SYMMETRIC_FF);
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPClapP( f ) ), G( convSingPClapP( g ) );
    if (!F.isUnivariate() || !G.isUnivariate() || F.mvar()!=G.mvar())
    {
      Off(SW_RATIONAL);
      WerrorS("not univariate");
      return NULL;
    }
    CanonicalForm Fa,Gb;
    On(SW_RATIONAL);
    res=convClapPSingP( extgcd( F, G, Fa, Gb ) );
    pa=convClapPSingP(Fa);
    pb=convClapPSingP(Gb);
    Off(SW_RATIONAL);
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    CanonicalForm Fa,Gb;
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPClapAP( f,a ) ), G( convSingAPClapAP( g,a ) );
      if (!F.isUnivariate() || !G.isUnivariate() || F.mvar()!=G.mvar())
      {
        WerrorS("not univariate");
        return NULL;
      }
      res= convClapAPSingAP( extgcd( F, G, Fa, Gb ) );
      pa=convClapAPSingAP(Fa);
      pb=convClapAPSingAP(Gb);
    }
    else
    {
      CanonicalForm F( convSingTrPClapP( f ) ), G( convSingTrPClapP( g ) );
      if (!F.isUnivariate() || !G.isUnivariate() || F.mvar()!=G.mvar())
      {
        Off(SW_RATIONAL);
        WerrorS("not univariate");
        return NULL;
      }
      res= convClapPSingTrP( extgcd( F, G, Fa, Gb ) );
      pa=convClapPSingTrP(Fa);
      pb=convClapPSingTrP(Gb);
    }
    Off(SW_RATIONAL);
  }
  else
  {
    WerrorS( feNotImplemented );
    return NULL;
  }
  lists L=(lists)Alloc(sizeof(slists));
  L->Init(3);
  L->m[0].rtyp=POLY_CMD;
  L->m[0].data=(void *)res;
  L->m[1].rtyp=POLY_CMD;
  L->m[1].data=(void *)pa;
  L->m[2].rtyp=POLY_CMD;
  L->m[2].data=(void *)pb;
  return L;
}

poly singclap_pdivide ( poly f, poly g )
{
  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  poly res=NULL;
  On(SW_RATIONAL);
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPClapP( f ) ), G( convSingPClapP( g ) );
    res = convClapPSingP( F / G );
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPClapAP( f,a ) ), G( convSingAPClapAP( g,a ) );
      res= convClapAPSingAP(  F / G  );
    }
    else
    {
      CanonicalForm F( convSingTrPClapP( f ) ), G( convSingTrPClapP( g ) );
      res= convClapPSingTrP(  F / G  );
    }
  }
  else
    WerrorS( feNotImplemented );
  Off(SW_RATIONAL);
  return res;
}

void singclap_divide_content ( poly f )
{
  if ( nGetChar() == 1 )
    setCharacteristic( 0 );
  else  if ( nGetChar() == -1 )
    return; /* not implemented for R */
  else  if ( nGetChar() < 0 )
    setCharacteristic( -nGetChar() );
  else if (currRing->parameter==NULL) /* not GF(q) */
    setCharacteristic( nGetChar() );
  else
    return; /* not implemented*/
  if ( f==NULL )
  {
    return;
  }
  else  if ( pNext( f ) == NULL )
  {
    pSetCoeff( f, nInit( 1 ) );
    return;
  }
  else
  {
    CFList L;
    CanonicalForm g, h;
    poly p = pNext(f);

    // first attemp: find 2 smallest g:

    number g1=pGetCoeff(f);
    number g2=pGetCoeff(p); // p==pNext(f);
    pIter(p);
    int sz1=nSize(g1);
    int sz2=nSize(g2);
    if (sz1>sz2)
    {
      number gg=g1;
      g1=g2; g2=gg;
      int sz=sz1;
      sz1=sz2; sz2=sz;
    }
    while (p!=NULL)
    {
      int n_sz=nSize(pGetCoeff(p));
      if (n_sz<sz1)
      {
        sz2=sz1;
        g2=g1;
        g1=pGetCoeff(p);
        sz1=n_sz;
        if (sz1<=3) break;
      }
      else if(n_sz<sz2)
      {
        sz2=n_sz;
        g2=pGetCoeff(p);
        sz2=n_sz;
      }
      pIter(p);
    }
    FACTORY_ALGOUT( "G", ((lnumber)g1)->z );
    g = convSingTrClapP( ((lnumber)g1)->z );
    g = gcd( g, convSingTrClapP( ((lnumber)g2)->z ));

    // second run: gcd's

    p = f;
    TIMING_START( contentTimer );
    while ( (p != NULL) && (g != 1) )
    {
      FACTORY_ALGOUT( "h", (((lnumber)pGetCoeff(p))->z) );
      h = convSingTrClapP( ((lnumber)pGetCoeff(p))->z );
      pIter( p );
#ifdef FACTORY_GCD_STAT
      // save g
      CanonicalForm gOld = g;
#endif

#ifdef FACTORY_GCD_TEST
      g = CFPrimitiveGcdUtil::gcd( g, h );
#else
      g = gcd( g, h );
#endif

      FACTORY_GCDSTAT( "gcnt:", gOld, h, g );
      FACTORY_CFTROUT( "g", g );
      L.append( h );
    }
    TIMING_END( contentTimer );
    FACTORY_CONTSTAT( "cont:", g );
    if ( g == 1 )
    {
      pTest(f);
      return;
    }
    #ifdef LDEBUG
    else if ( g == 0 )
    {
      pTest(f);
      pWrite(f);
      PrintS("=> gcd 0 in divide_content\n");
      return;
    }
    #endif
    else
    {
      CFListIterator i;
      for ( i = L, p = f; i.hasItem(); i++, p=pNext(p) )
      {
        lnumber c=(lnumber)pGetCoeff(p);
        napDelete(&c->z);
        #ifdef LDEBUG
        number nt=(number)Alloc0(sizeof(rnumber));
        lnumber nnt=(lnumber)nt;
        nnt->z=convClapPSingTr( i.getItem());
        nTest(nt);
        #endif
        c->z=convClapPSingTr( i.getItem() / g );
        //nTest((number)c);
        //#ifdef LDEBUG
        //number cn=(number)c;
        //StringSetS(""); nWrite(nt); StringAppend(" ==> ");
        //nWrite(cn);PrintS(StringAppend("\n"));
        //#endif
      }
    }
    pTest(f);
  }
}

static int primepower(int c)
{
  int p=1;
  int cc=c;
  while(cc!= rInternalChar(currRing)) { cc*=c; p++; }
  return p;
}

ideal singclap_factorize ( poly f, intvec ** v , int with_exps)
{
  // with_exps: 1 return only true factors
  //            2 return true factors and exponents
  //            0 return factors and exponents

  ideal res=NULL;
  if (f==NULL)
  {
    res=idInit(1,1);
    if (with_exps!=1)
    {
      (*v)=new intvec(1);
      (*v)[1]=1;
    }
    return res;
  }
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  CFFList L;
  number N=NULL;
  number NN=NULL;

  if (rField_is_Q() || rField_is_Zp())
  {
    setCharacteristic( nGetChar() );
    if (nGetChar()==0) /* Q */
    {
      if (f!=NULL)
      {
        number n0=nCopy(pGetCoeff(f));
        if (with_exps==0)
          N=nCopy(n0);
        pCleardenom(f);
        NN=nDiv(n0,pGetCoeff(f));
        nDelete(&n0);
        if (with_exps==0)
        {
          nDelete(&N);
          N=nCopy(NN);
        }
      }
    }
    CanonicalForm F( convSingPClapP( f ) );
    if (nGetChar()==0) /* Q */
    {
      L = factorize( F );
    }
    else /* Fp */
    {
#ifdef HAVE_LIBFAC_P
      L = Factorize( F );
#else
      goto notImpl;
#endif
    }
  }
  else if (rField_is_GF())
  {
    int c=rChar(currRing);
    setCharacteristic( c, primepower(c) );
    CanonicalForm F( convSingGFClapGF( f ) );
    if (F.isUnivariate())
    {
      L = factorize( F );
    }
    else
    {
      goto notImpl;
    }
  }
  // and over Q(a) / Fp(a)
  else if (rField_is_Extension())
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    if ((currRing->minpoly!=NULL)
    && (nGetChar()<(-1)))
    {
      CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPClapAP( f,a ) );
      L.insert(F);
      if (F.isUnivariate())
      {
        L = factorize( F, a );
      }
      else
      {
        WarnS("complete factorization only for univariate polynomials");
        CanonicalForm G( convSingTrPClapP( f ) );
        if (nGetChar()==1) /* Q(a) */
        {
          L = factorize( G );
        }
        else
        {
#ifdef HAVE_LIBFAC_P
          L = Factorize( G );
#else
          goto notImpl;
#endif
        }
      }
    }
    else
    {
      CanonicalForm F( convSingTrPClapP( f ) );
      if (nGetChar()==1) /* Q(a) */
      {
        L = factorize( F );
      }
      else /* Fp(a) */
      {
#ifdef HAVE_LIBFAC_P
        L = Factorize( F );
#else
        goto notImpl;
#endif
      }
    }
  }
  else
  {
    goto notImpl;
  }
  {
    // the first factor should be a constant
    if ( ! L.getFirst().factor().inCoeffDomain() )
      L.insert(CFFactor(1,1));
    // convert into ideal
    int n = L.length();
    CFFListIterator J=L;
    int j=0;
    if (with_exps!=1)
    {
      if ((with_exps==2)&&(n>1))
      {
        n--;
        J++;
      }
      *v = new intvec( n );
    }
    res = idInit( n ,1);
    for ( ; J.hasItem(); J++, j++ )
    {
      if (with_exps!=1) (**v)[j] = J.getItem().exp();
      if (rField_is_Zp() || rField_is_Q())           /* Q, Fp */
        res->m[j] = convClapPSingP( J.getItem().factor() );
      else if (rField_is_GF())
        res->m[j] = convClapGFSingGF( J.getItem().factor() );
      else if (rField_is_Extension())     /* Q(a), Fp(a) */
      {
        if (currRing->minpoly==NULL)
          res->m[j] = convClapPSingTrP( J.getItem().factor() );
        else
          res->m[j] = convClapAPSingAP( J.getItem().factor() );
      }
    }
    if (N!=NULL)
    {
      pMultN(res->m[0],N);
      nDelete(&N);
      N=NULL;
    }
    // delete constants
    if ((with_exps!=0) && (res!=NULL))
    {
      int i=IDELEMS(res)-1;
      int j=0;
      for(;i>=0;i--)
      {
        if ((res->m[i]!=NULL) && (pNext(res->m[i])==NULL) && (pIsConstant(res->m[i])))
        {
          pDelete(&(res->m[i]));
          if ((v!=NULL) && ((*v)!=NULL))
            (**v)[i]=0;
          j++;
        }
      }
      if (j>0)
      {
        idSkipZeroes(res);
        if ((v!=NULL) && ((*v)!=NULL))
        {
          intvec *w=*v;
          *v = new intvec( max(n-j,1) );
          for (i=0,j=0;i<w->length();i++)
          {
            if((*w)[i]!=0)
            {
              (**v)[j]=(*w)[i]; j++;
            }
          }
          delete w;
        }
      }
      if (res->m[0]==NULL)
      {
        res->m[0]=pOne();
      }
    }
  }
notImpl:
  if (res==NULL)
    WerrorS( feNotImplemented );
  if (NN!=NULL)
  {
    pMultN(f,NN);
    nDelete(&NN);
  }
  if (N!=NULL)
  {
    nDelete(&N);
  }
  return res;
}

matrix singclap_irrCharSeries ( ideal I)
{
#ifdef HAVE_LIBFAC_P
  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  matrix res=NULL;
  int i;
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  CFList L;
  ListCFList LL;
  if (((nGetChar() == 0) || (nGetChar() > 1) )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      if (I->m[i]!=NULL)
        L.append(convSingPClapP(I->m[i]));
    }
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      if (I->m[i]!=NULL)
        L.append(convSingTrPClapP(I->m[i]));
    }
  }
  else
  {
    WerrorS( feNotImplemented );
    return res;
  }

  LL=IrrCharSeries(L);
  int m= LL.length(); // Anzahl Zeilen
  int n=0;
  ListIterator<CFList> LLi;
  CFListIterator Li;
  for ( LLi = LL; LLi.hasItem(); LLi++ )
  {
    n = max(LLi.getItem().length(),n);
  }
  if ((m==0) || (n==0))
  {
    Warn("char_series returns %d x %d matrix from %d input polys (%d)\n",m,n,IDELEMS(I)+1,LL.length());
    iiWriteMatrix((matrix)I,"I",2,0);
    m=max(m,1);
    n=max(n,1);
  }
  res=mpNew(m,n);
  for ( m=1, LLi = LL; LLi.hasItem(); LLi++, m++ )
  {
    for (n=1, Li = LLi.getItem(); Li.hasItem(); Li++, n++)
    {
      if ( (nGetChar() == 0) || (nGetChar() > 1) )
        MATELEM(res,m,n)=convClapPSingP(Li.getItem());
      else
        MATELEM(res,m,n)=convClapPSingTrP(Li.getItem());
    }
  }
  Off(SW_RATIONAL);
  return res;
#else
  return NULL;
#endif
}

char* singclap_neworder ( ideal I)
{
#ifdef HAVE_LIBFAC_P
  int i;
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  CFList L;
  if (((nGetChar() == 0) || (nGetChar() > 1) )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      L.append(convSingPClapP(I->m[i]));
    }
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      L.append(convSingTrPClapP(I->m[i]));
    }
  }
  else
  {
    WerrorS( feNotImplemented );
    return NULL;
  }

  List<int> IL=neworderint(L);
  ListIterator<int> Li;
  StringSetS("");
  Li = IL;
  int offs=rPar(currRing);
  int* mark=(int*)Alloc0((pVariables+offs)*sizeof(int));
  int cnt=pVariables+offs;
  loop
  {
    i=Li.getItem()-1;
    mark[i]=1;
    if (i<offs)
    {
      StringAppendS(currRing->parameter[i]);
    }
    else
    {
      StringAppendS(currRing->names[i-offs]);
    }
    Li++;
    cnt--;
    if(cnt==0) break;
    StringAppendS(",");
    if(! Li.hasItem()) break;
  }
  for(i=0;i<pVariables+offs;i++)
  {
    if(mark[i]==0)
    {
      if (i<offs)
      {
        StringAppendS(currRing->parameter[i]);
      }
      else
      {
        StringAppendS(currRing->names[i-offs]);
      }
      cnt--;
      if(cnt==0) break;
      StringAppendS(",");
    }
  }
  return mstrdup(StringAppendS(""));
#else
  return NULL;
#endif
}

BOOLEAN singclap_isSqrFree(poly f)
{
  BOOLEAN b=FALSE;
  Off(SW_RATIONAL);
  //  Q / Fp
  if (((nGetChar() == 0) || (nGetChar() > 1) )
  &&(currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPClapP( f ) );
    if((nGetChar()>1)&&(!F.isUnivariate()))
      goto err;
    b=(BOOLEAN)isSqrFree(F);
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    //if (currRing->minpoly!=NULL)
    //{
    //  CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
    //  Variable a=rootOf(mipo);
    //  CanonicalForm F( convSingAPClapAP( f,a ) );
    //  ...
    //}
    //else
    {
      CanonicalForm F( convSingTrPClapP( f ) );
      b=(BOOLEAN)isSqrFree(F);
    }
    Off(SW_RATIONAL);
  }
  else
  {
err:
    WerrorS( feNotImplemented );
  }
  return b;
}

poly singclap_det( const matrix m )
{
  int r=m->rows();
  if (r!=m->cols())
  {
    Werror("det of %d x %d matrix",r,m->cols());
    return NULL;
  }
  poly res=NULL;
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CFMatrix M(r,r);
    int i,j;
    for(i=r;i>0;i--)
    {
      for(j=r;j>0;j--)
      {
        M(i,j)=convSingPClapP(MATELEM(m,i,j));
      }
    }
    res= convClapPSingP( determinant(M,r) ) ;
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    CFMatrix M(r,r);
    poly res;
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      int i,j;
      for(i=r;i>0;i--)
      {
        for(j=r;j>0;j--)
        {
          M(i,j)=convSingAPClapAP(MATELEM(m,i,j),a);
        }
      }
      res= convClapAPSingAP( determinant(M,r) ) ;
    }
    else
    {
      int i,j;
      for(i=r;i>0;i--)
      {
        for(j=r;j>0;j--)
        {
          M(i,j)=convSingTrPClapP(MATELEM(m,i,j));
        }
      }
      res= convClapPSingTrP( determinant(M,r) );
    }
  }
  else
    WerrorS( feNotImplemented );
  Off(SW_RATIONAL);
  return res;
}

int singclap_det_i( intvec * m )
{
  setCharacteristic( 0 );
  CFMatrix M(m->rows(),m->cols());
  int i,j;
  for(i=m->rows();i>0;i--)
  {
    for(j=m->cols();j>0;j--)
    {
      M(i,j)=IMATELEM(*m,i,j);
    }
  }
  int res= convClapISingI( determinant(M,m->rows())) ;
  Off(SW_RATIONAL);
  return res;
}
/*==============================================================*/
/* interpreter interface : */
BOOLEAN jjGCD_P(leftv res, leftv u, leftv v)
{
  res->data=(void *)singclap_gcd((poly)(u->CopyD()),((poly)v->CopyD()));
  return FALSE;
}

BOOLEAN jjFAC_P(leftv res, leftv u)
{
  intvec *v=NULL;
  ideal f=singclap_factorize((poly)(u->Data()), &v, 0);
  if (f==NULL) return TRUE;
  lists l=(lists)Alloc(sizeof(slists));
  l->Init(2);
  l->m[0].rtyp=IDEAL_CMD;
  l->m[0].data=(void *)f;
  l->m[1].rtyp=INTVEC_CMD;
  l->m[1].data=(void *)v;
  res->data=(void *)l;
  return FALSE;
}

BOOLEAN jjSQR_FREE_DEC(leftv res, leftv u,leftv dummy)
{
  intvec *v=NULL;
  int sw=(int)dummy->Data();
  ideal f=singclap_factorize((poly)(u->Data()), &v, sw);
  if (f==NULL)
    return TRUE;
  switch(sw)
  {
    case 0:
    case 2:
    {
      lists l=(lists)Alloc(sizeof(slists));
      l->Init(2);
      l->m[0].rtyp=IDEAL_CMD;
      l->m[0].data=(void *)f;
      l->m[1].rtyp=INTVEC_CMD;
      l->m[1].data=(void *)v;
      res->data=(void *)l;
      res->rtyp=LIST_CMD;
      return FALSE;
    }
    case 1:
      res->data=(void *)f;
      return FALSE;
    case 3:
      {
        poly p=f->m[0];
        int i=IDELEMS(f);
        f->m[0]=NULL;
        while(i>1)
        {
          i--;
          p=pMult(p,f->m[i]);
          f->m[i]=NULL;
        }
        res->data=(void *)p;
        res->rtyp=POLY_CMD;
      }
      return FALSE;
  }
  WerrorS("invalid switch");
  return TRUE;
}

#if 0
BOOLEAN jjIS_SQR_FREE(leftv res, leftv u)
{
  BOOLEAN b=singclap_factorize((poly)(u->Data()), &v, 0);
  res->data=(void *)b;
}
#endif

BOOLEAN jjEXTGCD_P(leftv res, leftv u, leftv v)
{
  res->data=singclap_extgcd((poly)u->Data(),(poly)v->Data());
  return (res->data==NULL);
}
BOOLEAN jjRESULTANT(leftv res, leftv u, leftv v, leftv w)
{
  res->data=singclap_resultant((poly)u->Data(),(poly)v->Data(), (poly)w->Data());
  return errorreported;
}
BOOLEAN jjCHARSERIES(leftv res, leftv u)
{
  res->data=singclap_irrCharSeries((ideal)u->Data());
  return (res->data==NULL);
}

alg singclap_alglcm ( alg f, alg g )
{
 FACTORY_ALGOUT( "f", f );
 FACTORY_ALGOUT( "g", g );

 // over Q(a) / Fp(a)
 if (nGetChar()==1) setCharacteristic( 0 );
 else               setCharacteristic( -nGetChar() );
 alg res;

 if (currRing->minpoly!=NULL)
 {
   CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
   Variable a=rootOf(mipo);
   CanonicalForm F( convSingAClapA( f,a ) ), G( convSingAClapA( g,a ) );
   CanonicalForm GCD;

   TIMING_START( algLcmTimer );
   // calculate gcd
#ifdef FACTORY_GCD_TEST
   GCD = CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD = gcd( F, G );
#endif
   TIMING_END( algLcmTimer );

   FACTORY_CFAOUT( "d", GCD );
   FACTORY_GCDSTAT( "alcm:", F, G, GCD );

   // calculate lcm
   res= convClapASingA( (F/GCD)*G );
 }
 else
 {
   CanonicalForm F( convSingTrClapP( f ) ), G( convSingTrClapP( g ) );
   CanonicalForm GCD;
   TIMING_START( algLcmTimer );
   // calculate gcd
#ifdef FACTORY_GCD_TEST
   GCD = CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD = gcd( F, G );
#endif
   TIMING_END( algLcmTimer );

   FACTORY_CFTROUT( "d", GCD );
   FACTORY_GCDSTAT( "alcm:", F, G, GCD );

   // calculate lcm
   res= convClapPSingTr( (F/GCD)*G );
 }

 Off(SW_RATIONAL);
 return res;
}

void singclap_algdividecontent ( alg f, alg g, alg &ff, alg &gg )
{
 FACTORY_ALGOUT( "f", f );
 FACTORY_ALGOUT( "g", g );

 // over Q(a) / Fp(a)
 if (nGetChar()==1) setCharacteristic( 0 );
 else               setCharacteristic( -nGetChar() );
 ff=gg=NULL;

 if (currRing->minpoly!=NULL)
 {
   CanonicalForm mipo=convSingTrClapP(((lnumber)currRing->minpoly)->z);
   Variable a=rootOf(mipo);
   CanonicalForm F( convSingAClapA( f,a ) ), G( convSingAClapA( g,a ) );
   CanonicalForm GCD;

   TIMING_START( algContentTimer );
#ifdef FACTORY_GCD_TEST
   GCD=CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD=gcd( F, G );
#endif
   TIMING_END( algContentTimer );

   FACTORY_CFAOUT( "d", GCD );
   FACTORY_GCDSTAT( "acnt:", F, G, GCD );

   if (GCD!=1)
   {
     ff= convClapASingA( F/ GCD );
     gg= convClapASingA( G/ GCD );
   }
 }
 else
 {
   CanonicalForm F( convSingTrClapP( f ) ), G( convSingTrClapP( g ) );
   CanonicalForm GCD;

   TIMING_START( algContentTimer );
#ifdef FACTORY_GCD_TEST
   GCD=CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD=gcd( F, G );
#endif
   TIMING_END( algContentTimer );

   FACTORY_CFTROUT( "d", GCD );
   FACTORY_GCDSTAT( "acnt:", F, G, GCD );

   if (GCD!=1)
   {
     ff= convClapPSingTr( F/ GCD );
     gg= convClapPSingTr( G/ GCD );
   }
 }

 Off(SW_RATIONAL);
}
#endif
