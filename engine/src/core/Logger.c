#include "Logger.h"
#include "Asserts.h"
#include "platform/Platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 InitializeLogging() {
    // TODO: create log file.
    return TRUE;
}

void ShutdownLogging() {
    // TODO: cleanup loggin/write queued entries
}

void LogOutput(LogLevel level, const char* message, ...) {
    const char* LevelStrings[6] = {"[FATAL]:   ", "[ERROR]:   ", "[WARNING]: ", "[INFO]:    ", "[DEBUG]:   ", "[TRACE]:   "};
    b8 isError = level < LOG_LEVEL_WARNING;

    const i32 msgLength = 32000;
    char outMessage[msgLength];
    memset(outMessage, 0, sizeof(outMessage));

    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, 32000, message, argPtr);
    va_end(argPtr);

    char outMessage2[msgLength];
    sprintf(outMessage2, "%s%s\n", LevelStrings[level], outMessage);

    if (isError) {
        PlatformConsoleWriteError(outMessage2, level);
    } else {
        PlatformConsoleWrite(outMessage2, level);
    }
}

void ReportAssertionFailed(const char* expression, const char* message, const char* file, i32 line) {
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: %s, in file: %s, line: %d\n", expression, message, file, line);
}