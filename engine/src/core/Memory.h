#pragma once

#include "Defines.h"

typedef enum MemoryTag {
    MEMORY_TAG_UNKOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_COUNT
} MemoryTag;

KAPI void InitializeMemory();

KAPI void ShutdownMemory();

KAPI void* Allocate(u64 size, MemoryTag tag);

KAPI void Free(void* block, u64 size, MemoryTag tag);

KAPI void* ZeroMemory(void* block, u64 size);

KAPI void* CopyMemory(void* dest, const void* source, u64 size);

KAPI void* SetMemory(void* dest, i32 value, u64 size);

KAPI char* GetMemoryUsageStr();