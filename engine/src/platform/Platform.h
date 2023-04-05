#pragma once
#include "Defines.h"

b8 PlatformSystemStartup(
    u64* memoryRequirment,
    void* state,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height
);

void PlatformSystemShutdown(void* platState);

b8 PlatformPumpMessages();

void* PlatformAllocate(u64 size, b8 aligned);
void PlatformFree(void* block, b8 aligned);
void* PlatformZeroMemory(void* block, u64 size);
void* PlatformCopyMemory(void* dest, const void* source, u64 size);
void* PlatformSetMemory(void* dest, i32 value, u64 size);

void PlatformConsoleWrite(const char* message, u8 colour);
void PlatformConsoleWriteError(const char* message, u8 colour);

f64 PlatformGetAbsoluteTime();

void PlatformSleep(u64 ms);