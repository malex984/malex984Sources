/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    slInit.h
 *  Purpose: declarations of link initialization functions
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 12/00
 *  Version: $Id: slInit.h,v 1.2 2002-04-26 15:56:18 Singular Exp $
 *******************************************************************/
#ifndef SL_INIT_H
#define SL_INIT_H

#include "silink.h"

#ifdef HAVE_MPSR
si_link_extension slInitMPFileExtension(si_link_extension s);
si_link_extension slInitMPTcpExtension(si_link_extension s);

typedef int (*BatchDoProc)(const char* port, const char* host);
BatchDoProc slInitMPBatchDo();
#endif // HAVE_MPSR

#ifdef HAVE_DBM
si_link_extension slInitDBMExtension(si_link_extension s);
#endif

#endif // SL_INIT_H
