/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: mminit.cc,v 1.23 2001-05-31 09:11:38 Singular Exp $ */
/*
* ABSTRACT: init of memory management
*/

#include <stdio.h>
#include <stdlib.h>

#include "mod2.h"
#include "mmalloc.h"
#include "structs.h"
// this prevents the definition of malloc/free and macros
// by omalloc
#define OMALLOC_C
#include "omalloc.h"

extern "C"
{
#include <gmp.h>
}

int mmIsInitialized=mmInit();

extern "C"
{
  void omSingOutOfMemoryFunc()
  {
    fprintf(stderr, "\nerror: no more memory\n");
    omPrintStats(stderr);
    m2_end(14);
    /* should never get here */
    exit(1);
  }
}

int mmInit( void )
{
  if(mmIsInitialized==0)
  {
#ifdef OMALLOC_USES_MALLOC
    mp_set_memory_functions(omMallocFunc,omReallocSizeFunc,omFreeSizeFunc);
#else
    mp_set_memory_functions(malloc,reallocSize,freeSize);
#endif
    om_Opts.OutOfMemoryFunc = omSingOutOfMemoryFunc;
#ifndef OM_NDEBUG
    om_Opts.ErrorHook = dErrorBreak;
#endif    
    omInitInfo();
#ifdef OM_SING_KEEP
    om_Opts.Keep = OM_SING_KEEP;
#endif
  }
  mmIsInitialized=1;
  return 1;
}
