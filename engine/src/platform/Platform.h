#pragma once
#include "Defines.h"

typedef struct PlatformState {
    void* InternalState;
} PlatformState;

KAPI b8 PlatformStartup(
    PlatformState* platState,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height
);

KAPI void PlatformShutdown(PlatformState* platState);

KAPI b8 PlatformPumpMessages(PlatformState* platState);

void* PlatformAllocate(u64 size, b8 aligned);
void PlatformFree(void* block, b8 aligned);
void* PlatformZeroMemory(void* block, u64 size);
void* PlatformCopyMemory(void* dest, const void* source, u64 size);
void* PlatformSetMemory(void* dest, i32 value, u64 size);

void PlatformConsoleWrite(const char* message, u8 colour);
void PlatformConsoleWriteError(const char* message, u8 colour);

f64 PlatformGetAbsoluteTime();

void PlatformSleep(u64 ms);