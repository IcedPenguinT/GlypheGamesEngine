#include "Memory.h"

#include "core/Logger.h"
#include "core/String.h"
#include "platform/Platform.h"

#include <string.h>
#include <stdio.h>

struct MemoryStats {
    u64 TotalAllocated;
    u64 TaggedAllocations[MEMORY_TAG_COUNT];
};

static const char* MemoryTagStrings[MEMORY_TAG_COUNT] = {
    "UNKOWN     ",
    "ARRAY      ",
    "LINEAR_ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      ",
};

typedef struct MemorySystemState {
    struct MemoryStats stats;
    u64 allocCount;
} MemorySystemState;

static MemorySystemState* statePtr;

void MemorySystemInitialize(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(MemorySystemState);
    if (state == 0)
        return;
    
    statePtr = state;
    statePtr->allocCount = 0;
    PlatformZeroMemory(&statePtr->stats, sizeof(statePtr->stats));
}

void MemorySystemShutdown(void* state) {
    statePtr = 0;
}

void* Allocate(u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKOWN)
        KWARNING("Allocate called uwing MEMORY_TAG_UNKOWN. Re-class this allocation.");

    if (statePtr) {
        statePtr->stats.TotalAllocated += size;
        statePtr->stats.TaggedAllocations[tag] += size;
        statePtr->allocCount++;
    }

    // TODO: Memory allignment
    void* block = PlatformAllocate(size, false);
    PlatformZeroMemory(block, size);
    return block;
}

void Free(void* block, u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKOWN)
        KWARNING("Allocate called uwing MEMORY_TAG_UNKOWN. Re-class this allocation.");
    
    if (statePtr) { 
        statePtr->stats.TotalAllocated -= size;
        statePtr->stats.TaggedAllocations[tag] -= size;
    }

    // TODO: Memory allignment
    PlatformFree(block, false);
}

void* ZeroMemory(void* block, u64 size) {
    return PlatformZeroMemory(block, size);
}

void* CopyMemory(void* dest, const void* source, u64 size) {
    return PlatformCopyMemory(dest, source, size);
}

void* SetMemory(void* dest, i32 value, u64 size) {
    return PlatformSetMemory(dest, value, size);
}

char* GetMemoryUsageStr() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for(u32 i = 0; i < MEMORY_TAG_COUNT; ++i){
        char unit[4] = "XiB";
        float amount = 1.0f;

        if (statePtr->stats.TaggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = statePtr->stats.TaggedAllocations[i] / (float)gib;
        } else if (statePtr->stats.TaggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = statePtr->stats.TaggedAllocations[i] / (float)mib;
        } else if (statePtr->stats.TaggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = statePtr->stats.TaggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)statePtr->stats.TaggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", MemoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* outString = StringDuplicate(buffer);
    return outString;
}

u64 GetMemoryAllocCount() {
    if (statePtr) 
        return statePtr->allocCount;
    return 0;
}