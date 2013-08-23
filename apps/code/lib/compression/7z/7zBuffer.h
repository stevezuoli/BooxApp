/* 7zBuffer.h */

#ifndef __7Z_BUFFER_H
#define __7Z_BUFFER_H

#include <stddef.h>
#ifdef _SZ_ONE_DIRECTORY
#include "Types.h"
#else
#include "../../Types.h"
#endif

typedef struct _CSzByteBuffer
{
  size_t Capacity;
  Byte *Items;
}CSzByteBuffer;

void SzByteBufferInit(CSzByteBuffer *buffer);
int SzByteBufferCreate(CSzByteBuffer *buffer, size_t newCapacity, void * (*allocFunc)(size_t size));
void SzByteBufferFree(CSzByteBuffer *buffer, void (*freeFunc)(void *));

#endif
