#ifndef MMEMORY_H
#define MMEMORY_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: mmalloc.h,v 1.2 2000-08-14 16:05:43 obachman Exp $ */
/*
* ABSTRACT: declaration of routines for memory stuff
*/

#include <stdlib.h>

int mmInit(void);
#undef reallocSize
#undef freeSize

#ifdef __cplusplus
extern "C" 
{
#endif
  void* reallocSize(void* old_addr, size_t old_size, size_t new_size);
  void  freeSize(void* addr, size_t size);
#ifdef __cplusplus
}
#endif
#endif
