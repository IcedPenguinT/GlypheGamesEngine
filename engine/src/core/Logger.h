#pragma once

#include "Defines.h"

#define LOG_WARNING_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for realease builds.

#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum LogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} LogLevel;

b8 InitializeLogging(u64* memoryRequirement, void* state);
void ShutdownLogging(void* state);

KAPI void LogOutput(LogLevel level, const char* message, ...);

//Logs a fatal-level message
#define KFATAL(message, ...) LogOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef KERROR
//Logs a error-level message
#define KERROR(message, ...) LogOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARNING_ENABLED == 1
//Logs a warning-level message
#define KWARNING(message, ...) LogOutput(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_WARNING_ENABLED != 1
#define KWARNING(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
//Logs a warning-level message
#define KINFO(message, ...) LogOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_WARNING_ENABLED != 1
#define KINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
//Logs a warning-level message
#define KDEBUG(message, ...) LogOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_WARNING_ENABLED != 1
#define KDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
//Logs a warning-level message
#define KTRACE(message, ...) LogOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
//Does nothing when LOG_WARNING_ENABLED != 1
#define KTRACE(message, ...)
#endif