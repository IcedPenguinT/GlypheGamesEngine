#include "Clock.h"
#include "platform/Platform.h"

void ClockUpdate(Clock* clock) {
    if (clock->startTime != 0) 
        clock->elapsed = PlatformGetAbsoluteTime() - clock->startTime;
}

void ClockStart(Clock* clock) {
    clock->startTime = PlatformGetAbsoluteTime();
    clock->elapsed = 0;
}

void ClockStop(Clock* clock) {
    clock->startTime = 0;
}