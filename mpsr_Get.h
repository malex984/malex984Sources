/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: mpsr_Get.h,v 1.9 1999-11-15 17:20:33 obachman Exp $ */
/***************************************************************
 *
 * File:       mpsr_Get.h
 * Purpose:    declarations for all the Get routines
 * Author:     Olaf Bachmann (10/95)
 *
 * Change History (most recent first):
 *
 ***************************************************************/
#ifdef HAVE_MPSR

#ifndef __MPSR_GET__
#define __MPSR_GET__

#include "numbers.h"
#include "polys.h"
#include "febase.h"
#include "ipid.h"

#include"mpsr.h"
#if HAVE_ASO == 1
#include "mpsr_Get.aso"
#endif



/***************************************************************
 *
 * Get-specific data structures
 *
 ***************************************************************/

// the data structure which functions as external data
typedef struct mpsr_sleftv
{
  leftv lv;
  ring  r;
} mpsr_sleftv;

typedef mpsr_sleftv * mpsr_leftv;


/***************************************************************
 *
 * prototypes
 *
 ***************************************************************/
// from mpsr_GetMisc.cc
extern int mpsr_rDefault(short ch, char *name, ring &r);
extern ring mpsr_rDefault(short ch);
extern mpsr_Status_t mpsr_MergeLeftv(mpsr_leftv mlv1, mpsr_leftv mlv2);
extern void mpsr_MapLeftv(leftv l, ring from_ring, ring to_ring);
extern void mpsr_SetCurrRingHdl(ring r);
extern idhdl mpsr_FindIdhdl(char *name, ring &r);

// from mpsr_GetPoly.cc
extern mpsr_Status_t mpsr_GetRingAnnots(MPT_Node_pt node, ring &r,
                                        BOOLEAN &mv, BOOLEAN &IsUnOrdered);
extern mpsr_Status_t mpsr_GetPoly(MP_Link_pt link, poly &p, MP_Uint32_t nmon,
                                ring cring);
extern mpsr_Status_t mpsr_GetPolyVector(MP_Link_pt link, poly &p,
                                      MP_Uint32_t nmon, ring cring);
extern mpsr_Status_t mpsr_rSetOrdSgn(ring r);

// from mpsr_Get.cc
extern mpsr_Status_t mpsr_GetLeftv(MP_Link_pt link, mpsr_leftv mlv, short quote);

extern mpsr_Status_t mpsr_GetCommonOperatorLeftv(MP_Link_pt link,
                                               MPT_Node_pt node,
                                               mpsr_leftv mlv,
                                               short quote);
extern mpsr_Status_t mpsr_GetOperatorLeftv(MP_Link_pt link,
                                         MPT_Node_pt node,
                                         mpsr_leftv mlv,
                                         short quote);
extern MPT_Status_t mpsr_GetExternalData(MP_Link_pt link,
                                         MPT_Arg_t  *odata,
                                         MPT_Node_pt node);


/***************************************************************
 *
 * inlines
 *
 ***************************************************************/
inline BOOLEAN NodeCheck(MPT_Node_pt node, MP_NodeType_t type,
                        MP_DictTag_t dtag, MP_Common_t cv)
{
  mpsr_assume(node != NULL);
  return
    node->type == type &&
    MP_COMMON_T(node->nvalue) == cv &&
    node->dict == dtag;
}

inline BOOLEAN NodeCheck(MPT_Node_pt node, MP_DictTag_t dtag, MP_Common_t cv)
{
  mpsr_assume(node != NULL);
  return
    MP_COMMON_T(node->nvalue) == cv &&
    node->dict == dtag;
}


inline idhdl mpsr_InitIdhdl(short tok, void *data, char *name)
{
  idhdl h = (idhdl) Alloc0(sizeof(idrec));
  IDID(h) = mstrdup(name);
  IDTYP(h) = tok;
  IDDATA(h) = (char *) data;

  return h;
}


#endif
#endif
