// emacs edit mode for this file is -*- C++ -*-
// $Id: fglmhom.cc,v 1.5 1997-12-15 22:46:23 obachman Exp $

/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* 
* ABSTRACT - The FGLM-Algorithm extended for homogeneous ideals.
*   Calculates via the hilbert-function a groebner basis.
*/

#ifndef NOSTREAMIO
#include <iostream.h>
#endif

#include "mod2.h"
#ifdef HAVE_FGLM
#include "tok.h"
#include "structs.h"
#include "subexpr.h"  
#include "polys.h"
#include "ideals.h"
#include "ring.h"
#include "ipid.h"
#include "ipshell.h"
#include "febase.h"
#include "maps.h"
#include "mmemory.h"
#include "fglm.h"
#include "fglmvec.h"
#include "fglmgauss.h"
#include "intvec.h"
#include "kstd1.h"
#include "stairc.h"  // -> hHStdSeries, hFirstSeries usw.
#include <templates/list.h>

// obachman: Got rid off those "redefiende messages by includeing fglm.h
#include "fglm.h"
#if 0
#define PROT(msg) if (BTEST1(OPT_PROT)) Print(msg)
#define STICKYPROT(msg) if (BTEST1(OPT_PROT)) Print(msg)
#define PROT2(msg,arg) if (BTEST1(OPT_PROT)) Print(msg,arg)
#define STICKYPROT2(msg,arg) if (BTEST1(OPT_PROT)) Print(msg,arg)
#define fglmASSERT(ignore1,ignore2)
#endif

struct doublepoly
{
    poly sm;
    poly dm;
};

class homogElem
{
public:
    doublepoly mon;
    fglmVector v;
    fglmVector dv;
    int basis;
    int destbasis;
    BOOLEAN inDest;
    homogElem() : v(), dv(), basis(0), destbasis(0), inDest(FALSE) {}
    homogElem( poly m, int b, BOOLEAN ind ) :
	basis(b), inDest(ind) 
    {
	mon.dm= m;
	mon.sm= NULL;
    }
};

struct homogData 
{
    ideal sourceIdeal;
    doublepoly * sourceHeads;
    int numSourceHeads;
    ideal destIdeal;
    int numDestPolys;
    homogElem * monlist;
    int monlistmax;
    int monlistblock;
    int numMonoms;
    int basisSize;
    int overall;  // nur zum testen.
    int numberofdestbasismonoms;
//     homogData() : sourceHeads(NULL), numSourceHeads(0), monlist(NULL), 
// 	numMonoms(0), basisSize(0) {}
};

int
hfglmNextdegree( intvec * source, ideal current, int & deg ) 
{
    int numelems;
    intvec * newhilb = hHstdSeries( current, NULL, currQuotient );

    loop 
    {
	if ( deg < newhilb->length() ) 
	{
	    if ( deg < source->length() )
		numelems= (*newhilb)[deg]-(*source)[deg];
	    else
		numelems= (*newhilb)[deg];
	}
	else
	{
	    if (deg < source->length())
		numelems= -(*source)[deg];
	    else
	    {
		deg= 0;
		return 0;
	    }
	}
	if (numelems != 0) 
	    return numelems;
	deg++;
    }  
    delete newhilb;
}

void 
generateMonoms( poly m, int var, int deg, homogData * dat )
{
    if ( var == pVariables ) {
	BOOLEAN inSource = FALSE;
	BOOLEAN inDest = FALSE;
	poly mon = pCopy( m );
	pSetExp( mon, var, deg );
	pSetm( mon );
	++dat->overall;
	int i;
	for ( i= dat->numSourceHeads - 1; (i >= 0) && (inSource==FALSE); i-- ) {
	    if ( pDivisibleBy( dat->sourceHeads[i].dm, mon ) ) {
		inSource= TRUE;
	    }
	}
	for ( i= dat->numDestPolys - 1; (i >= 0) && (inDest==FALSE); i-- ) {
	    if ( pDivisibleBy( (dat->destIdeal->m)[i], mon ) ) {
		inDest= TRUE;
	    }
	}
	if ( (!inSource) || (!inDest) ) {
	    int basis = 0;
	    if ( !inSource )
		basis= ++(dat->basisSize);
	    if ( !inDest ) 
		++dat->numberofdestbasismonoms;
	    if ( dat->numMonoms == dat->monlistmax ) {
		dat->monlist= (homogElem * )ReAlloc( dat->monlist, (dat->monlistmax)*sizeof( homogElem ), (dat->monlistmax+dat->monlistblock) * sizeof( homogElem ) );
		int k;
		for ( k= dat->monlistmax; k < (dat->monlistmax+dat->monlistblock); k++ ) 
		    dat->monlist[k].homogElem();
		dat->monlistmax+= dat->monlistblock;
	    }
	    dat->monlist[dat->numMonoms]= homogElem( mon, basis, inDest );
	    dat->numMonoms++;
	    if ( inSource && ! inDest ) PROT( "\\" );
	    if ( ! inSource && inDest ) PROT( "/" );
	    if ( ! inSource && ! inDest ) PROT( "." );
	}
	else {
	    pDelete( & mon );
	}
	return;
    }
    else {
	poly newm = pCopy( m );
	while ( deg >= 0 ) {
	    generateMonoms( newm, var+1, deg, dat );
	    pIncrExp( newm, var );
	    pSetm( newm );
	    deg--;
	}
	pDelete( & newm );
    }
    return;
}

void
mapMonoms( ring oldRing, homogData & dat ) 
{
    int * vperm = (int *)Alloc( (currRing->N + 1)*sizeof(int) );
    maFindPerm( oldRing->names, oldRing->N, NULL, 0, currRing->names, currRing->N, NULL, 0, vperm, NULL );
    nSetMap( oldRing->ch, oldRing->parameter, oldRing->P, oldRing->minpoly );
    int s;
    for ( s= dat.numMonoms - 1; s >= 0; s-- ) {
//	dat.monlist[s].mon.sm= pPermPoly( dat.monlist[s].mon.dm, vperm, currRing->N, NULL, 0 );
      // obachman: changed the folowing to reflect the new calling interface of
      // pPermPoly -- Tim please check whether this is correct!
	dat.monlist[s].mon.sm= pPermPoly( dat.monlist[s].mon.dm, vperm, oldRing, NULL, 0 );      
    }
}

void
getVectorRep( homogData & dat ) 
{
    // Calculate the NormalForms
    int s;
    for ( s= 0;  s < dat.numMonoms; s++ ) {
	if ( dat.monlist[s].inDest == FALSE ) {
	    fglmVector v;
	    if ( dat.monlist[s].basis == 0 ) {
		v= fglmVector( dat.basisSize );
		// now the monom is in L(source)
		PROT( "(" );
		poly nf = kNF( dat.sourceIdeal, NULL, dat.monlist[s].mon.sm );
		PROT( ")" );
		poly temp = nf;
		while (temp != NULL ) {
		    int t;
		    for ( t= dat.numMonoms - 1; t >= 0; t-- ) {
			if ( dat.monlist[t].basis > 0 ) {
			    if ( pEqual( dat.monlist[t].mon.sm, temp ) ) {
				number coeff= nCopy( pGetCoeff( temp ) );
				v.setelem( dat.monlist[t].basis, coeff );
			    }
			}
		    }
		    temp= pIter(temp);
		}
		pDelete( & nf );
	    }
	    else {
		PROT( "." );
		v= fglmVector( dat.basisSize, dat.monlist[s].basis );
	    }
	    dat.monlist[s].v= v;
	}
    }
}

void
remapVectors( ring oldring, homogData & dat ) 
{
    nSetMap( oldring->ch, oldring->parameter, oldring->P, oldring->minpoly );
    int s;
    for ( s= dat.numMonoms - 1; s >= 0; s-- ) {
	if ( dat.monlist[s].inDest == FALSE ) {
	    int k;
	    fglmVector newv( dat.basisSize );
	    for ( k= dat.basisSize; k > 0; k-- ){
		number newnum= nMap( dat.monlist[s].v.getelem( k ) );
		newv.setelem( k, newnum );
	    }
	    dat.monlist[s].dv= newv;
	}
    }
}

void
gaussreduce( homogData & dat, int maxnum, int BS ) 
{
    int s;
    int found= 0;

    int destbasisSize = 0;
    gaussReducer gauss( dat.basisSize );
    
    for ( s= 0; (s < dat.numMonoms) && (found < maxnum); s++ ) {
	if ( dat.monlist[s].inDest == FALSE ) {
	    if ( gauss.reduce( dat.monlist[s].dv ) == FALSE ) {
		destbasisSize++;
		dat.monlist[s].destbasis= destbasisSize;
		gauss.store();
		PROT( "." );
	    }
	    else {
		fglmVector p= gauss.getDependence();
		poly result = pCopy( dat.monlist[s].mon.dm );
		pSetCoeff( result, nCopy( p.getconstelem( p.size() ) ) );
		int l = 0;
		int k;
		for ( k= 1; k < p.size(); k++ ) {
		    if ( ! p.elemIsZero( k ) ) {
			while ( dat.monlist[l].destbasis != k ) 
			    l++;
			poly temp = pCopy( dat.monlist[l].mon.dm );
			pSetCoeff( temp, nCopy( p.getconstelem( k ) ) );
			result= pAdd( result, temp );
		    }
		}
		if ( ! nGreaterZero( pGetCoeff( result ) ) ) result= pNeg( result );
//		PROT2( "(%s)", pString( result ) );
		PROT( "+" );
		found++;
		(dat.destIdeal->m)[dat.numDestPolys]= result;
		dat.numDestPolys++;
		if ( IDELEMS(dat.destIdeal) == dat.numDestPolys ) {
		    pEnlargeSet( & dat.destIdeal->m, IDELEMS( dat.destIdeal ), BS );
		    IDELEMS( dat.destIdeal )+= BS;
		}
		
	    }
		
	}
    }
    PROT2( "(%i", s );
    PROT2( "/%i)", dat.numberofdestbasismonoms );
}


BOOLEAN
fglmhomog( idhdl sourceRingHdl, ideal sourceIdeal, idhdl destRingHdl, ideal & destIdeal )
{
#define groebnerBS 16
    int numGBelems;
    int deg = 0;

    homogData dat;

    // get the hilbert series and the leading monomials of the sourceIdeal:
    rSetHdl( sourceRingHdl, TRUE );
    ring sourceRing = currRing;

    intvec * hilb = hHstdSeries( sourceIdeal, NULL, currQuotient ); 
    int s;
    dat.sourceIdeal= sourceIdeal;
    dat.sourceHeads= (doublepoly *)Alloc( IDELEMS( sourceIdeal ) * sizeof( doublepoly ) );
    for ( s= IDELEMS( sourceIdeal ) - 1; s >= 0; s-- ) {
	dat.sourceHeads[s].sm= pHead( (sourceIdeal->m)[s] );
    }
    dat.numSourceHeads= IDELEMS( sourceIdeal );
    rSetHdl( destRingHdl, TRUE );
    ring destRing = currRing;

    // Map the sourceHeads to the destRing
    int * vperm = (int *)Alloc( (sourceRing->N + 1)*sizeof(int) );
    maFindPerm( sourceRing->names, sourceRing->N, NULL, 0, currRing->names, currRing->N, NULL, 0, vperm, NULL );
    nSetMap( sourceRing->ch, sourceRing->parameter, sourceRing->P, sourceRing->minpoly );
    for ( s= IDELEMS( sourceIdeal ) - 1; s >= 0; s-- ) {
	dat.sourceHeads[s].dm= pPermPoly( dat.sourceHeads[s].sm, vperm, sourceRing, NULL, 0 );
    }

    dat.destIdeal= idInit( groebnerBS, 1 );
    dat.numDestPolys= 0;

    while ( (numGBelems= hfglmNextdegree( hilb, dat.destIdeal, deg )) != 0 ) {
	int num = 0;  // the number of monoms of degree deg
	PROT2( "deg= %i ", deg );
	PROT2( "num= %i\ngen>", numGBelems );
	dat.monlistblock= 512;
	dat.monlistmax= dat.monlistblock;
	dat.monlist= (homogElem *)Alloc( dat.monlistmax*sizeof( homogElem ) );
	int j;
	for ( j= dat.monlistmax - 1; j >= 0; j-- ) dat.monlist[j].homogElem();
	dat.numMonoms= 0;
	dat.basisSize= 0;
	dat.overall= 0;
	dat.numberofdestbasismonoms= 0;
	
	poly start= pOne();
	generateMonoms( start, 1, deg, &dat );
	pDelete( & start );

	PROT2( "(%i/", dat.basisSize );
	PROT2( "%i)\nvec>", dat.overall );
	// switch to sourceRing and map monoms
	rSetHdl( sourceRingHdl, TRUE );
	mapMonoms( destRing, dat );
	getVectorRep( dat );
	
	// switch to destination Ring and remap the vectors
	rSetHdl( destRingHdl, TRUE );
	remapVectors( sourceRing, dat );
	
	PROT( "<\nred>" );
	// now do gaussian reduction
	gaussreduce( dat, numGBelems, groebnerBS );

	Free( (ADDRESS)dat.monlist, dat.monlistmax*sizeof( homogElem ) );
	PROT( "<\n" );
    }
    PROT( "\n" );
    destIdeal= dat.destIdeal;
    idSkipZeroes( destIdeal );
    return TRUE;
}

ideal
fglmhomProc(leftv first, leftv second)
{
    idhdl dest= currRingHdl;
    ideal result;
    // in den durch das erste Argument angegeben Ring schalten:
    rSetHdl( (idhdl)first->data, TRUE );
    idhdl ih= currRing->idroot->get( second->name, myynest );
    ASSERT( ih!=NULL, "Error: Can't find ideal in ring");
    rSetHdl( dest, TRUE );

    ideal i= IDIDEAL(ih);
    fglmhomog( (idhdl)first->data, i, dest, result );

    return( result );
}

#endif

// Questions:
// Muss ich einen intvec freigeben?
// ----------------------------------------------------------------------------
// Local Variables: ***
// compile-command: "make Singular" ***
// page-delimiter: "^\\(\\|//!\\)" ***
// fold-internal-margins: nil ***
// End: ***
