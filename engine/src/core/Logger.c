#include "Logger.h"
#include "Asserts.h"

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
    const char* LevelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARNING]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    //b8 isError = level < 2;

    char outMessage[32000];
    memset(outMessage, 0, sizeof(outMessage));

    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, 32000, message, argPtr);
    va_end(argPtr);

    char outMessage2[32000];
    sprintf(outMessage2, "%s%s\n", LevelStrings[level], outMessage);

    //TODO: Platform specific output.
    printf("%s", outMessage2);
}

void ReportAssertionFailed(const char* expression, const char* message, const char* file, i32 line) {
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: %s, in file: %s, line: %d\n", expression, message, file, line);
}