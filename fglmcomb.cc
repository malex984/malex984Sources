// emacs edit mode for this file is -*- C++ -*-
// $Id: fglmcomb.cc,v 1.10 1998-05-15 14:31:20 wichmann Exp $

/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT -
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
#include "spolys.h"
#include "ipid.h"
#include "ipshell.h"
#include "febase.h"
#include "maps.h"
#include "mmemory.h"
#include "fglmvec.h"
#include "fglmgauss.h"
#include "kstd1.h"
#include "fglm.h"
#include <templates/ftmpl_list.h>

// nur fuer debug-Ausgaben:
static int
pSize( poly p )
{
    int count = 0;
    while ( p != NULL ) {
        count+= nSize( pGetCoeff( p ) );
        pIter( p );
    }
    return count;
}

static void
fglmEliminateMonomials( poly * pptr, fglmVector & v, polyset monomials, int numMonoms )
{
    poly temp = *pptr;
    poly pretemp = NULL;
    int point = 0;
    int state;

    while ( (temp != NULL) && (point < numMonoms) ) {
        state= pComp( temp, monomials[point] );
        if ( state == 0 ) {
            // Eliminate this monomial
            poly todelete;
            if ( pretemp == NULL ) {
                todelete = temp;
                pIter( *pptr );
                temp= *pptr;
            }
            else {
                todelete= temp;
                pIter( temp );
                pretemp->next= temp;
            }
            pGetCoeff( todelete )= nNeg( pGetCoeff( todelete ) );
            number newelem = nAdd( pGetCoeff( todelete ), v.getconstelem( point+1 ) );
            v.setelem( point+1, newelem );
            nDelete( & pGetCoeff( todelete ) );
            pFree1( todelete );
            point++;
        }
        else if ( state < 0 )
            point++;
        else {
            pretemp= temp;
            pIter( temp );
        }
    }
}

static BOOLEAN
fglmReductionStep( poly * pptr, ideal source, int * w )
{
// returns TRUE if the leading monomial was reduced
    if ( *pptr == NULL ) return FALSE;
    int k;
    int best = 0;
    for ( k= IDELEMS( source ) - 1; k >= 0; k-- ) {
        if ( pDivisibleBy( (source->m)[k], *pptr ) ) {
            if ( best == 0 ) {
                best= k + 1;
            }
            else {
                if ( w[k] < w[best-1] ) {
                    best= k + 1;
                }
            }
        }
    }
    if ( best > 0 ) {
        // OwnSPoly
        poly p2 = (source->m)[best-1];
        int i, diff;

        poly m = pOne();
        for ( i= pVariables; i > 0; i-- ) {
            diff= pGetExp( *pptr, i ) - pGetExp( p2, i );
            pSetExp( m, i, diff );
        }
        pSetm( m );
        number n1 = nCopy( pGetCoeff( *pptr ) );
        number n2 = pGetCoeff( p2 );

        p2= pCopy( p2 );
        pDelete1( pptr );
        pDelete1( & p2 );
        p2= pMult( m, p2 );

        number temp = nDiv( n1, n2 );
        nNormalize( temp );
        nDelete( & n1 );
        n1= temp;
        n1= nNeg( n1 );
        pMultN( p2, n1 );
//         pNormalize( p2 );
        nDelete( & n1 );
        *pptr= pAdd( *pptr, p2 );
    }
    return ( (best > 0) );
}

static void
fglmReduce( poly * pptr, fglmVector & v, polyset m, int numMonoms, ideal source, int * w )
{
    BOOLEAN reduced = FALSE;
    reduced= fglmReductionStep( pptr, source, w );
    while ( reduced == TRUE ) {
        fglmEliminateMonomials( pptr, v, m, numMonoms );
        reduced= fglmReductionStep( pptr, source, w );
    }
    STICKYPROT( "<" );
    poly temp = * pptr;
    if ( temp != NULL ) {
        while ( pNext( temp ) != NULL ) {
            STICKYPROT( ">" );
            reduced= fglmReductionStep( & pNext( temp ), source, w );
            while ( reduced == TRUE ) {
                fglmEliminateMonomials( & pNext( temp ), v, m, numMonoms );
                reduced= fglmReductionStep( & pNext( temp ), source, w );
            }
            if ( pNext( temp ) != NULL ) {
                pIter( temp );
            }
        }
    }
}

static poly
fglmNewLinearCombination( ideal source, poly monset )
{
    polyset m = NULL;
    polyset nf = NULL;
    fglmVector * mv = NULL;

    fglmVector * v = NULL;
    polyset basis = NULL;
    int basisSize = 0;
    int basisBS = 16;
    int basisMax = basisBS;

    int * weights = NULL;
    int * lengthes = NULL;
    int * order = NULL;

    int numMonoms = 0;
    int k;

    // get number of monoms
    numMonoms= pLength( monset );
    STICKYPROT2( "%i monoms\n", numMonoms );

    // Allcoate Memory and initialize sets
    m= (polyset)Alloc( numMonoms * sizeof( poly ) );
    poly temp= monset;
    for ( k= 0; k < numMonoms; k++ ) {
//         m[k]= pOne();
//         pSetExpV( m[k], temp->exp );
//         pSetm( m[k] );
	m[k]= pNew();
	pCopy2( m[k], temp );
	pSetCoeff( m[k], nInit(1) );
        pIter( temp );
    }

    nf= (polyset)Alloc( numMonoms * sizeof( poly ) );
    mv= (fglmVector *)Alloc( numMonoms * sizeof( fglmVector ) );

    v= (fglmVector *)Alloc( numMonoms * sizeof( fglmVector ) );
    basisMax= basisBS;
    basis= (polyset)Alloc( basisMax * sizeof( poly ) );

    weights= (int *)Alloc( IDELEMS( source ) * sizeof( int ) );
    STICKYPROT( "weights: " );
    for ( k= 0; k < IDELEMS( source ); k++ ) {
        poly temp= (source->m)[k];
        int w = 0;
        while ( temp != NULL ) {
            w+= nSize( pGetCoeff( temp ) );
            pIter( temp );
        }
        weights[k]= w;
        STICKYPROT2( "%i ", w );
    }
    STICKYPROT( "\n" );
    lengthes= (int *)Alloc( numMonoms * sizeof( int ) );
    order= (int *)Alloc( numMonoms * sizeof( int ) );

    // calculate the NormalForm in a special way
    for ( k= 0; k < numMonoms; k++ )
    {
        STICKYPROT( "#" );
        poly current = pCopy( m[k] );
        fglmVector currV( numMonoms, k+1 );

        fglmReduce( & current, currV, m, numMonoms, source, weights );
        poly temp = current;
        int b;
        while ( temp != NULL )
	{
            BOOLEAN found = FALSE;
            for ( b= 0; (b < basisSize) && (found == FALSE); b++ )
	    {
                if ( pEqual( temp, basis[b] ) )
		{
                    found= TRUE;
                }
            }
            if ( found == FALSE )
	    {
                if ( basisSize == basisMax )
		{
                    // Expand the basis
                    basis= (polyset)ReAlloc( basis, basisMax * sizeof( poly ), (basisMax + basisBS ) * sizeof( poly ) );
                    basisMax+= basisBS;
                }
//                 basis[basisSize]= pOne();
//                 pSetExpV( basis[basisSize], temp->exp );
//                 pSetm( basis[basisSize] );
		basis[basisSize]= pNew();
		pCopy2( basis[basisSize], temp );
		pSetCoeff( basis[basisSize], nInit(1) );
                basisSize++;
            }
            pIter( temp );
        }
        nf[k]= current;
#ifdef __MWERKS__
        mv[k].mac_constr( currV );
#else
        mv[k].fglmVector( currV );
#endif
        STICKYPROT( "\n" );
    }
    // get the vector representation
    for ( k= 0; k < numMonoms; k++ ) {
        STICKYPROT( "." );
 
#ifdef __MWERKS__
        v[k].mac_constr_i( basisSize );
#else
        v[k].fglmVector( basisSize );
#endif
        poly mon= nf[k];
        while ( mon != NULL ) {
            BOOLEAN found = FALSE;
            int b= 0;
            while ( found == FALSE ) {
                if ( pEqual( mon, basis[b] ) ) {
                    found= TRUE;
                }
                else {
                    b++;
                }
            }
            number coeff = nCopy( pGetCoeff( mon ) );
            v[k].setelem( b+1, coeff );
            pIter( mon );
        }
        pDelete( nf + k );
    }
    // Free Memory not needed anymore
    Free( (ADDRESS)nf, numMonoms * sizeof( poly ) );
    Free( (ADDRESS)weights, IDELEMS( source ) * sizeof( int ) );

    STICKYPROT2( "\nbasis size: %i\n", basisSize );
    STICKYPROT( "(clear basis" );
    for ( k= 0; k < basisSize; k++ )
        pDelete( basis + k );
    STICKYPROT( ")\n" );
    // gauss reduce
    gaussReducer gauss( basisSize );
    BOOLEAN isZero = FALSE;
    fglmVector p;

    STICKYPROT( "sizes: " );
    for ( k= 0; k < numMonoms; k++ ) {
        lengthes[k]= v[k].numNonZeroElems();
        STICKYPROT2( "%i ", lengthes[k] );
    }
    STICKYPROT( "\n" );

    int act = 0;
    while ( (isZero == FALSE) && (act < numMonoms) ) {
        int best = 0;
        for ( k= numMonoms - 1; k >= 0; k-- ) {
            if ( lengthes[k] > 0 ) {
                if ( best == 0 ) {
                    best= k+1;
                }
                else {
                    if ( lengthes[k] < lengthes[best-1] ) {
                        best= k+1;
                    }
                }
            }
        }
        lengthes[best-1]= 0;
        order[act]= best-1;
        STICKYPROT2( " (%i) ", best );
        if ( ( isZero= gauss.reduce( v[best-1] )) == TRUE ) {
            p= gauss.getDependence();
        }
        else {
            STICKYPROT( "+" );
            gauss.store();
            act++;
        }
        v[best-1].~fglmVector();
    }
    poly result = NULL;
    if ( isZero == TRUE ) {
        number gcd = p.gcd();
        if ( ! nIsZero( gcd ) && ! ( nIsOne( gcd ) ) ) {
            p/= gcd;
        }
        nDelete( & gcd );
        fglmVector temp( numMonoms );
        for ( k= 0; k < p.size(); k++ ) {
            if ( ! p.elemIsZero( k+1 ) ) {
                temp+= p.getconstelem( k+1 ) * mv[order[k]];
            }
        }
        number denom = temp.clearDenom();
        nDelete( & denom );
        gcd = temp.gcd();
        if ( ! nIsZero( gcd ) && ! nIsOne( gcd ) ) {
            temp/= gcd;
        }
        nDelete( & gcd );

        poly sum = NULL;
        for ( k= 1; k <= numMonoms; k++ ) {
            if ( ! temp.elemIsZero( k ) ) {
                if ( result == NULL ) {
                    result= pCopy( m[k-1] );
                    sum= result;
                }
                else {
                    sum->next= pCopy( m[k-1] );
                    pIter( sum );
                }
                pSetCoeff( sum, nCopy( temp.getconstelem( k ) ) );
            }
        }
        pContent( result );
        if ( ! nGreaterZero( pGetCoeff( result ) ) ) result= pNeg( result );
    }
    // Free Memory
    Free( (ADDRESS)lengthes, numMonoms * sizeof( int ) );
    Free( (ADDRESS)order, numMonoms * sizeof( int ) );
//     for ( k= 0; k < numMonoms; k++ )
//         v[k].~fglmVector();
    Free( (ADDRESS)v, numMonoms * sizeof( fglmVector ) );
    for ( k= 0; k < basisSize; k++ )
        pDelete( basis + k );
    Free( (ADDRESS)basis, basisMax * sizeof( poly ) );
    for ( k= 0; k < numMonoms; k++ )
        mv[k].~fglmVector();
    Free( (ADDRESS)mv, numMonoms * sizeof( fglmVector ) );

    for ( k= 0; k < numMonoms; k++ )
        pDelete( m + k );
    Free( (ADDRESS)m, numMonoms * sizeof( poly ) );

    STICKYPROT( "\n" );
    return result;
}


static poly
fglmLinearCombination( ideal source, poly monset )
{
    int k;
    poly temp;

    polyset m;
    polyset nf;
    fglmVector * v;

    polyset basis;
    int basisSize = 0;
    int basisBS = 16;
    int basisMax;
    // get number of monomials in monset
    int numMonoms = 0;
    temp = monset;
    while ( temp != NULL ) {
        numMonoms++;
        pIter( temp );
    }
    // Allocate Memory
    m= (polyset)Alloc( numMonoms * sizeof( poly ) );
    nf= (polyset)Alloc( numMonoms * sizeof( poly ) );
    // Warning: The fglmVectors in v are yet not initialized
    v= (fglmVector *)Alloc( numMonoms * sizeof( fglmVector ) );
    basisMax= basisBS;
    basis= (polyset)Alloc( basisMax * sizeof( poly ) );

    // get the NormalForm and the basis monomials
    temp= monset;
    for ( k= 0; k < numMonoms; k++ ) {
        poly mon= pHead( temp );
        if ( ! nIsOne( pGetCoeff( mon ) ) ) {
            nDelete( & pGetCoeff( mon ) );
            pSetCoeff( mon, nInit( 1 ) );
        }
        STICKYPROT( "(" );
        nf[k]= kNF( source, currRing->qideal, mon );
        STICKYPROT( ")" );

        // search through basis
        STICKYPROT( "[" );
        poly sm = nf[k];
        while ( sm != NULL ) {
            BOOLEAN found = FALSE;
            int b;
            for ( b= 0; (b < basisSize) && (found == FALSE); b++ )
                if ( pEqual( sm, basis[b] ) ) found= TRUE;
            if ( found == FALSE ) {
                // Expand the basis
                if ( basisSize == basisMax ) {
                    basis= (polyset)ReAlloc( basis, basisMax * sizeof( poly ), (basisMax + basisBS ) * sizeof( poly ) );
                    basisMax+= basisBS;
                }
                basis[basisSize]= pHead( sm );
                nDelete( & pGetCoeff( basis[basisSize] ) );
                pSetCoeff( basis[basisSize], nInit( 1 ) );
                basisSize++;
            }
            pIter( sm );
        }
        STICKYPROT( "]" );
        m[k]= mon;
        pIter( temp );
    }
    // get the vector representation
    STICKYPROT2( "(%i)", basisSize );
    for ( k= 0; k < numMonoms; k++ ) {
#ifdef __MWERKS__
        v[k].mac_constr_i( basisSize );
#else
        v[k].fglmVector( basisSize );
#endif
        STICKYPROT( "(+" );
        poly mon= nf[k];
        while ( mon != NULL ) {
            BOOLEAN found = FALSE;
            int b= 0;
            while ( found == FALSE ) {
                if ( pEqual( mon, basis[b] ) )
                    found= TRUE;
                else
                    b++;
            }
            number coeff = nCopy( pGetCoeff( mon ) );
            v[k].setelem( b+1, coeff );
            pIter( mon );
        }
        STICKYPROT( ")" );
    }
    // gauss reduce
    gaussReducer gauss( basisSize );
    BOOLEAN isZero = FALSE;
    fglmVector p;
    for ( k= 0; (k < numMonoms) && (isZero == FALSE); k++ ) {
        STICKYPROT( "(-" );
        if ( ( isZero= gauss.reduce( v[k] )) == TRUE )
            p= gauss.getDependence();
        else
            gauss.store();
        STICKYPROT( ")" );
    }
    poly comb = NULL;
    if ( isZero == TRUE ) {
        number gcd = p.gcd();
        if ( ! nIsZero( gcd ) && ! ( nIsOne( gcd ) ) )
            p/= gcd;
        nDelete( & gcd );
        for ( k= 1; k <= p.size(); k++ ) {
            if ( ! p.elemIsZero( k ) ) {
                poly temp = pCopy( m[k-1] );
                pSetCoeff( temp, nCopy( p.getconstelem( k ) ) );
                comb= pAdd( comb, temp );
            }
        }
        if ( ! nGreaterZero( pGetCoeff( comb ) ) ) comb= pNeg( comb );
    }

    // Free Memory
    for ( k= 0; k < numMonoms; k++ ) {
        pDelete( m + k );
        pDelete( nf + k );
    }
    Free( (ADDRESS)m, numMonoms * sizeof( poly ) );
    Free( (ADDRESS)nf, numMonoms * sizeof( poly ) );
    // Warning: At this point all Vectors in v have to be initialized
    for ( k= numMonoms - 1; k >= 0; k-- ) v[k].~fglmVector();
    Free( (ADDRESS)v, numMonoms * sizeof( fglmVector ) );
    for ( k= 0; k < basisSize; k++ )
        pDelete( basis + k );
    Free( (ADDRESS)basis, basisMax * sizeof( poly ) );
    STICKYPROT( "\n" );
    return comb;
}

#endif
// Local Variables: ***
// compile-command: "make Singular" ***
// page-delimiter: "^\\(\\|//!\\)" ***
// fold-internal-margins: nil ***
// End: ***
