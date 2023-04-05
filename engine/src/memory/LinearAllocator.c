#include "LinearAllocator.h"

#include "core/Memory.h"
#include "core/Logger.h"

void LinearAllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator) {
    if (outAllocator) {
        outAllocator->totalSize = totalSize;
        outAllocator->allocated = 0;
        outAllocator->ownsMemory = memory == 0;
        if (memory) {
            outAllocator->memory = memory;
        } else {
            outAllocator->memory = Allocate(totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}
void LinearAllocatorDestroy(LinearAllocator* allocator) {
    if (allocator) {
        allocator->allocated = 0;
        if (allocator->ownsMemory && allocator->memory) {
            Free(allocator->memory, allocator->totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        } 
        allocator->memory = 0;
        allocator->totalSize = 0;
        allocator->ownsMemory = false;
    }
}

void* LinearAllocatorAllocate(LinearAllocator* allocator, u64 size) {
    if (allocator && allocator->memory) {
        if (allocator->allocated + size > allocator->totalSize) {
            u64 remaining = allocator->totalSize - allocator->allocated;
            KERROR("LinearAllocatorAllocate - Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return 0;
        }

        void* block = ((u8*)allocator->memory) + allocator->allocated;
        allocator->allocated += size;
        return block;
    }

    KERROR("LinearAllocatorAllocate - provided allocator not initialized.");
    return 0;
}

void LinearAllocatorFreeAll(LinearAllocator* allocator) {
    if (allocator && allocator->memory) {
        allocator->allocated = 0;
        ZeroMemory(allocator->memory, allocator->totalSize);
    }
}