#ifndef MMPAGE_H
#define MMPAGE_H

#ifdef PAGE_TEST
#undef PALLOC
#define PALLOC mmPage_Create
#undef PFREE
#define PFREE mmPage_Delete
#endif

/***********************************************************************
 *
 * declarations of procedures
 */

void* mmGetPage();

void mmFreePage(void* page);

void mmReleaseFreePages();

int mmGetNumberOfFreePages();
int mmGetNumberOfUsedPages();
int mmGetNumberOfAllocatedPages();

/***********************************************************************
 *
 * Macros for page manipulations
 */

#define mmIsAddrPageAligned(addr) \
  (((long) (addr) & (SIZE_OF_SYSTEM_PAGE -1)) == 0)

#define mmGetPageOfAddr(addr) \
  ((void*) ((long) (addr) & ~(SIZE_OF_SYSTEM_PAGE -1)))

#define mmIsAddrOnPage(addr, page) (mmGetPageOfAddr(addr) == (void*) (page))

#define mmAreAddrOnSamePage(a1, a2) \
  (mmGetPageOfAddr(a1) == mmGetPageOfAddr(a2))

int mmIsAddrOnFreePage(void* addr);

#define mmIsNotAddrOnFreePage(addr) (!mmIsAddrOnFreePage(addr))

#endif /* MMPAGE_H */
