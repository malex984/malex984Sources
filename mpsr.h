/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: mpsr.h,v 1.12 1998-11-09 15:43:02 obachman Exp $ */
/***************************************************************
 *
 * File:       mpsr.h
 * Purpose:    Global Header file for MP connection to Singular
 * Author:     Olaf Bachmann (10/95)
 *
 * Change History (most recent first):
 *
 ***************************************************************/
#ifdef HAVE_MPSR

#ifndef __MPSR__
#define __MPSR__

// now the MP include stuff (is surrounded by ifndef there)
extern "C"
{
#include"MP.h"
}

#include"MPT.h"
#include"structs.h"
#include"mmemory.h"
#include"ipid.h"
#include"polys.h"
#include"numbers.h"

/***************************************************************
 *
 * prototype declarations of routines we provide for the outer world
 *
 *
 ***************************************************************/
// from mpsr_Error.c
typedef enum mpsr_Status_t
{
  mpsr_Failure,
  mpsr_Success,
  mpsr_MP_Failure,
  mpsr_MPT_Failure,
  mpsr_UnknownLeftvType,
  mpsr_WrongLeftvType,
  mpsr_UnknownSingularToken,
  mpsr_UnknownDictionary,
  mpsr_UnkownOperator,
  mpsr_UnknownMPNodeType,
  mpsr_CanNotHandlePrototype,
  mpsr_WrongNumofArgs,
  mpsr_WrongArgumentType,
  mpsr_WrongNodeType,
  mpsr_ReqAnnotSkip,
  mpsr_WrongUnionDiscriminator,
  mpsr_UnknownCoeffDomain,
  mpsr_MaxError
} mpsr_Status_t;

extern mpsr_Status_t mpsr_SetError(mpsr_Status_t error);
extern mpsr_Status_t mpsr_SetError(MP_Link_pt link);
extern void mpsr_PrintError(mpsr_Status_t error);
extern void mpsr_PrintError();
extern void mpsr_PrintError(MP_Link_pt link);
extern void mpsr_PrintError(mpsr_Status_t error, MP_Link_pt link);
extern mpsr_Status_t mpsr_GetError();
extern void mpsr_ClearError();

// from mpsr_Get.cc
extern mpsr_Status_t mpsr_GetMsg(MP_Link_pt link, leftv &lv);
extern mpsr_Status_t mpsr_GetDump(MP_Link_pt link);
// from mpsr_Put.cc
extern mpsr_Status_t mpsr_PutMsg(MP_Link_pt link, leftv lv);
extern mpsr_Status_t mpsr_PutDump(MP_Link_pt link);


/***************************************************************
 *
 * Inline's
 *
 ***************************************************************/
inline leftv mpsr_InitLeftv(short tok, void *data)
{
  leftv lv = (leftv) Alloc0(sizeof(sleftv));
  lv->data = data;
  lv->rtyp = tok;
  return lv;
}

// this is only for intermediate ring changes by mpsr
// a "proper" setting of the global ring is done at the end of all the
// getting
extern BOOLEAN currComplete;

inline void mpsr_SetCurrRing(ring rg, BOOLEAN complete)
{
  if (currRing != rg || (complete && ! currComplete))
  {
#ifdef PDEBUG
    nSetChar(rg->ch, TRUE, rg->parameter,rg->P);
#else
    nSetChar(rg->ch, complete, rg->parameter,rg->P);
#endif
    pChangeRing(rg->N, rg->OrdSgn, rg->order, rg->block0, rg->block1,
                rg->wvhdl);
    currRing = rg;
    currComplete = complete;
  }
}

extern MP_Sint32_t *gTa;
extern MP_Sint32_t gTa_Length;


inline void mpsr_InitTempArray(int length)
{
  if (gTa_Length < length)
    {
      if (gTa != NULL)
        Free(gTa, gTa_Length*sizeof(MP_Sint32_t));
      gTa = (MP_Sint32_t *) Alloc((length)*sizeof(MP_Sint32_t));
      gTa_Length = length;
    }
}

/***************************************************************
 *
 * Macros
 *
 ***************************************************************/

// is application specific Dictionary
#define MP_SingularDict 129
// remove this, once the Galois field stuff is done properly
#define MP_AnnotSingularGalois  1
// String used to indicate the end of a communication
#define MPSR_QUIT_STRING    "MPtcp:quit"

// some handy Macros for error handlings
#ifdef MPSR_DEBUG

#undef failr
#define failr(x)                                                    \
do                                                                  \
{                                                                   \
  mpsr_Status_t _status = x;                                        \
  if (_status != mpsr_Success)                                            \
    Werror("failr violation in %s line %d:",__FILE__, __LINE__);    \
  if (_status != mpsr_Success) return _status;                      \
}                                                                   \
while (0)

#undef mp_failr
#define mp_failr(x)                                                 \
do                                                                  \
{                                                                   \
  if (x != MP_Success)                                              \
  {                                                                 \
    Werror("mp_failr violation in %s line %d:",__FILE__, __LINE__); \
    return mpsr_SetError(link);                                     \
  }                                                                 \
}                                                                   \
while (0)

#undef mp_return
#define mp_return(x)                                                    \
do                                                                      \
{                                                                       \
  if (x != MP_Success)                                                  \
  {                                                                     \
   Werror("mp_return violation in %s line %d:",__FILE__, __LINE__);     \
   return mpsr_SetError(link);                                          \
  }                                                                     \
  else return mpsr_Success;                                             \
}                                                                       \
while (0)

#undef mpt_failr
#define mpt_failr(x)                                                 \
do                                                                  \
{                                                                   \
  if (x != MPT_Success)                                              \
  {                                                                 \
    Werror("mpt_failr violation in %s line %d:",__FILE__, __LINE__); \
    return mpsr_SetError(mpsr_MPT_Failure);                           \
  }                                                                 \
}                                                                   \
while (0)
#undef mpt_return
#define mpt_return(x)                                                    \
do                                                                      \
{                                                                       \
  if (x != MPT_Success)                                                  \
  {                                                                     \
   Werror("mpt_return violation in %s line %d:",__FILE__, __LINE__);     \
   return mpsr_SetError(mpsr_MPT_Failure);                                \
  }                                                                     \
  else return mpsr_Success;                                             \
}                                                                       \
while (0)

#undef mpsr_assume
#define mpsr_assume(cond)                                               \
do                                                                      \
{                                                                       \
  if ( ! (cond))                                                        \
    Werror("mpsr_assume violation in %s line %d",__FILE__, __LINE__);   \
} while (0)

#else
#undef failr
#define failr(x)                                \
do                                              \
{                                               \
  mpsr_Status_t _status = x;                    \
  if (_status != mpsr_Success) return _status;              \
}                                               \
while (0)
#undef mp_failr
#define mp_failr(x)                             \
do                                              \
{                                               \
  if (x != MP_Success)                          \
    return mpsr_SetError(link);                 \
}                                               \
while (0)
#undef mp_return
#define mp_return(x)                                \
do                                                  \
{                                                   \
  if (x != MP_Success) return mpsr_SetError(link);  \
  else return mpsr_Success;                         \
}                                                   \
while (0)
#undef mpt_failr
#define mpt_failr(x)                             \
do                                              \
{                                               \
  if (x != MPT_Success)                          \
    return mpsr_SetError(mpsr_MPT_Failure);                 \
}                                               \
while (0)
#undef mpt_return
#define mpt_return(x)                                \
do                                                  \
{                                                   \
  if (x != MPT_Success) return mpsr_SetError(mpsr_MPT_Failure);  \
  else return mpsr_Success;                         \
}                                                   \
while (0)
#undef mpsr_assume
#define mpsr_assume(cond) ((void) 0)

#endif // MPSR_DEBUG

// those defines are from longrat.cc
// It would be nice if we could include them, but they are not defined
// in a header file

#define SR_HDL(A) ((long)(A))
#define SR_INT    1
#define INT_TO_SR(INT)  ((number) (((long)INT << 2) + SR_INT))
#define SR_TO_INT(SR)   (((long)SR) >> 2)

union nf
{
  float _f;
  number _n;
  nf(float f) {_f = f;}
  nf(number n) {_n = n;}
  float F() const {return _f;}
  number N() const {return _n;}
};

#define Real32_2_Number(r) nf(nf(r).F()).N()
#define Number_2_Real32(n) nf(n).F()


#endif
#endif
