#pragma once

#include "Defines.h"

// Holds a handle to a file.
typedef struct FileHandle {
    // Opaque handle to internal file handle.
    void* handle;
    b8 isValid;
} FileHandle;

typedef enum FileModes {
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} FileModes;

KAPI b8 FilesystemExists(const char* path);

KAPI b8 FilesystemOpen(const char* path, FileModes mode, b8 binary, FileHandle* outHandle);

KAPI void FilesystemClose(FileHandle* handle);

KAPI b8 FilesystemReadLine(FileHandle* handle, char** lineBuf);

KAPI b8 FilesystemWriteLine(FileHandle* handle, const char* text);

KAPI b8 FilesystemRead(FileHandle* handle, u64 dataSize, void* outData, u64* outBytesRead);

KAPI b8 FilesystemReadAllBytes(FileHandle* handle, u8** outBytes, u64* outBytesRead);

KAPI b8 FilesystemWrite(FileHandle* handle, u64 dataSize, const void* data, u64* outBytesWritten);