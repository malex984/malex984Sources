/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    mpsr_sl.h
 *  Purpose: declaration of sl_link routines for MP
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 12/00
 *  Version: $Id: mpsr_sl.h,v 1.1 2000-12-12 08:44:49 obachman Exp $
 *******************************************************************/
LINKAGE BOOLEAN slOpenMPFile(si_link l, short flag);
LINKAGE MP_Link_pt slOpenMPConnect(int n_argc, char **n_argv);
LINKAGE MP_Link_pt slOpenMPListen(int n_argc, char **n_argv);
LINKAGE MP_Link_pt slOpenMPLaunch(int n_argc, char **n_argv);
LINKAGE MP_Link_pt slOpenMPFork(si_link l, int n_argc, char **n_argv);
LINKAGE BOOLEAN slOpenMPTcp(si_link l, short flag);
LINKAGE BOOLEAN slWriteMP(si_link l, leftv v);
LINKAGE leftv slReadMP(si_link l);
LINKAGE BOOLEAN slCloseMP(si_link l);
LINKAGE BOOLEAN slKillMP(si_link l);
LINKAGE BOOLEAN slDumpMP(si_link l);
LINKAGE BOOLEAN slGetDumpMP(si_link l);
LINKAGE char* slStatusMP(si_link l, char* request);
LINKAGE int Batch_do(const char* port, const char* host);

