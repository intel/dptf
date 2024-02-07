// ReSharper disable All
/* Alloc.h -- Memory allocation functions
2021-07-13 : Igor Pavlov : Public domain */

#ifndef __COMMON_ALLOC_H
#define __COMMON_ALLOC_H

#include "7zTypes.h"

EXTERN_C_BEGIN

void *MyAlloc(size_t size);
void MyFree(void *address);


#define MidAlloc(size) MyAlloc(size)
#define MidFree(address) MyFree(address)
#define BigAlloc(size) MyAlloc(size)
#define BigFree(address) MyFree(address)


extern const ISzAlloc g_Alloc;

#define g_BigAlloc g_AlignedAlloc
#define g_MidAlloc g_AlignedAlloc

extern const ISzAlloc g_AlignedAlloc;


typedef struct
{
  ISzAlloc vt;
  ISzAllocPtr baseAlloc;
  unsigned numAlignBits; /* ((1 << numAlignBits) >= sizeof(void *)) */
  size_t offset;         /* (offset == (k * sizeof(void *)) && offset < (1 << numAlignBits) */
} CAlignOffsetAlloc;

void AlignOffsetAlloc_CreateVTable(CAlignOffsetAlloc *p);


EXTERN_C_END

#endif
