/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    slInit_Static.cc
 *  Purpose: link initialization for static linking
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 12/00
 *  Version: $Id: slInit_Static.cc,v 1.1 2000-12-12 08:44:55 obachman Exp $
 *******************************************************************/

/***************************************************************
 *
 * MP link Extension inits
 *
 ***************************************************************/
#include "mod2.h"
#include "silink.h"
#include "slInit.h"

#ifdef HAVE_MPSR

#include "MP.h"
#include "mpsr_sl.h"

extern void mpsr_Init();

si_link_extension slInitMPFileExtension(si_link_extension s)
{
  mpsr_Init();
  s->Open=slOpenMPFile;
  s->Close=slCloseMP;
  s->Kill=slCloseMP;
  s->Read=slReadMP;
  s->Dump=slDumpMP;
  s->GetDump=slGetDumpMP;
  s->Write=slWriteMP;
  s->Status=slStatusMP;
  s->type="MPfile";
  return s;
}

si_link_extension slInitMPTcpExtension(si_link_extension s)
{
  mpsr_Init();
  s->Open=slOpenMPTcp;
  s->Close=slCloseMP;
  s->Kill=slKillMP;
  s->Read=slReadMP;
  s->Dump=slDumpMP;
  s->GetDump=slGetDumpMP;
  s->Write=slWriteMP;
  s->Status=slStatusMP;
  s->type="MPtcp";
  return s;
}

BatchDoProc slInitMPBatchDo()
{
  mpsr_Init();
  return Batch_do;
}
#endif

#ifdef HAVE_DBM
#include "dbm_sl.h"
si_link_extension slInitDBMExtension(si_link_extension s)
{
  s->Open=dbOpen;
  s->Close=dbClose;
  s->Kill=dbClose;
  s->Read=dbRead1;
  s->Read2=dbRead2;
  s->Write=dbWrite;
  s->Status=slStatusAscii;
  s->type="DBM";
  return s;
}
#endif
