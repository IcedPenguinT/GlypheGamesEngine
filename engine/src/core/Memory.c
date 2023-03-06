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

static struct MemoryStats stats;

void InitializeMemory() {
    PlatformZeroMemory(&stats, sizeof(stats));
}

void ShutdownMemory() {

}

void* Allocate(u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKOWN)
        KWARNING("Allocate called uwing MEMORY_TAG_UNKOWN. Re-class this allocation.");

    stats.TotalAllocated += size;
    stats.TaggedAllocations[tag] += size;

    // TODO: Memory allignment
    void* block = PlatformAllocate(size, FALSE);
    PlatformZeroMemory(block, size);
    return block;
}

void Free(void* block, u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKOWN)
        KWARNING("Allocate called uwing MEMORY_TAG_UNKOWN. Re-class this allocation.");
    
    stats.TotalAllocated -= size;
    stats.TaggedAllocations[tag] -= size;

    // TODO: Memory allignment
    PlatformFree(block, FALSE);
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

        if (stats.TaggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.TaggedAllocations[i] / (float)gib;
        } else if (stats.TaggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.TaggedAllocations[i] / (float)mib;
        } else if (stats.TaggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.TaggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.TaggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", MemoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* outString = StringDuplicate(buffer);
    return outString;
}