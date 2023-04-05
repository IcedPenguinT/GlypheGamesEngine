#include "Clock.h"
#include "platform/Platform.h"

KAPI void ClockUpdate(Clock* clock) {
    if (clock->startTime != 0) 
        clock->elapsed = PlatformGetAbsoluteTime() - clock->startTime;
}

KAPI void ClockStart(Clock* clock) {
    clock->startTime = PlatformGetAbsoluteTime();
    clock->elapsed = 0;
}

KAPI void ClockStop(Clock* clock) {
    clock->startTime = 0;
}