/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    pInline1.cc
 *  Purpose: implementation of poly procs which iter over ExpVector
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 8/00
 *  Version: $Id: pInline1.cc,v 1.2 2000-09-14 13:04:38 obachman Exp $
 *******************************************************************/
#ifndef PINLINE1_CC
#define PINLINE1_CC

#include "mod2.h"

#ifdef PDEBUG

#if PDEBUG < 1
#undef PDEBUG
#define PDEBUG 1
#endif

#undef NO_INLINE1
#define NO_INLINE1 

#endif // PDEBUG

#include "pInline1.h"

#endif // PINLINE1_CC

