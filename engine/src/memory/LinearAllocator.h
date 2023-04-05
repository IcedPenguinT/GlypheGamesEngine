#pragma once

#include "Defines.h"

typedef struct LinearAllocator {
    u64 totalSize;
    u64 allocated;
    void* memory;
    b8 ownsMemory;
} LinearAllocator;

KAPI void LinearAllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator);
KAPI void LinearAllocatorDestroy(LinearAllocator* allocator);

KAPI void* LinearAllocatorAllocate(LinearAllocator* allocator, u64 size);
KAPI void LinearAllocatorFreeAll(LinearAllocator* allocator);