/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    pInline2.cc
 *  Purpose: implementation of poly procs which are of constant time
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 8/00
 *  Version: $Id: pInline2.cc,v 1.3 2000-10-30 13:40:21 obachman Exp $
 *******************************************************************/
#ifndef PINLINE2_CC
#define PINLINE2_CC
#include "mod2.h"

#ifdef PDEBUG

#if PDEBUG < 2
#undef PDEBUG
#define PDEBUG 2
#endif

#undef NO_INLINE2
#define NO_INLINE2 1

#endif // PDEBUG

#include "p_polys.h"

#endif
