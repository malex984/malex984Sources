/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: mpsr_Tok.h,v 1.4 1999-11-15 17:20:35 obachman Exp $ */

/***************************************************************
 *
 * File:       mpsr_Tok.h
 * Purpose:    Routines which realize Singular CMD <-> MP (dict, cop) mappings
 * Author:     Olaf Bachmann (1/97)
 *
 * Change History (most recent first):
 *
 ***************************************************************/
#ifndef __MPSR_TOK_H__
#define __MPSR_TOK_H__
#include"mpsr.h"

extern mpsr_Status_t mpsr_tok2mp(short tok, MP_DictTag_t *dict,
                               MP_Common_t *cop);

extern mpsr_Status_t mpsr_mp2tok(MP_DictTag_t dict, MP_Common_t cop,
                               short *o_tok);

extern MP_Common_t mpsr_ord2mp(int sr_ord);
extern short mpsr_mp2ord(MP_Common_t mp_ord);

#define MP_AnnotSingularProcDef         1
#define MP_AnnotSingularPackageType     2

#endif  // __MPSR_TOK_H__
