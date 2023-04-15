#include "Logger.h"
#include "Asserts.h"
#include "platform/Platform.h"

#include "platform/Filesystem.h"
#include "core/String.h"
#include "core/Memory.h"

// TODO: temporary
#include <stdarg.h>

typedef struct LoggerSystemState {
    FileHandle logFileHandle;
} LoggerSystemState;

static LoggerSystemState* statePtr;

void AppendToLogFile(const char* message) {
    if (statePtr && statePtr->logFileHandle.isValid) {
        // Since the message already contains a '\n', just write the bytes directly.
        u64 length = StringLength(message);
        u64 written = 0;
        if (!FilesystemWrite(&statePtr->logFileHandle, length, message, &written)) {
            PlatformConsoleWriteError("ERROR writing to console.log.", LOG_LEVEL_ERROR);
        }
    }
}

b8 InitializeLogging(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(LoggerSystemState);
    if (state == 0)
        return true;

    statePtr = state;
    // Create new/wipe existing log file, then open it.
    if (!FilesystemOpen("console.log", FILE_MODE_WRITE, false, &statePtr->logFileHandle)) {
        PlatformConsoleWriteError("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    KFATAL("A test message: %f", 3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARNING("A test message: %f", 3.14f);
    KINFO("A test message: %f", 3.14f);
    KDEBUG("A test message: %f", 3.14f);
    KTRACE("A test message: %f", 3.14f);

    return true;
}

void ShutdownLogging(void* state) {
    statePtr = 0;
}

void LogOutput(LogLevel level, const char* message, ...) {
    const char* LevelStrings[6] = {"[FATAL]:   ", "[ERROR]:   ", "[WARNING]: ", "[INFO]:    ", "[DEBUG]:   ", "[TRACE]:   "};
    b8 isError = level < LOG_LEVEL_WARNING;

    char outMessage[32000];
    ZeroMemory(outMessage, sizeof(outMessage));

    __builtin_va_list argPtr;
    va_start(argPtr, message);
    StringFormatV(outMessage, message, argPtr);
    va_end(argPtr);

    StringFormat(outMessage, "%s%s\n", LevelStrings[level], outMessage);

    if (isError) {
        PlatformConsoleWriteError(outMessage, level);
    } else {
        PlatformConsoleWrite(outMessage, level);
    }

    AppendToLogFile(outMessage);
}

void ReportAssertionFailed(const char* expression, const char* message, const char* file, i32 line) {
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: %s, in file: %s, line: %d\n", expression, message, file, line);
}