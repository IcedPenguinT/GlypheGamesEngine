#include "core/Event.h"
#include "core/Memory.h"
#include "containers/Darray.h"

typedef struct RegisterEvent {
    void* listner;
    PFN_OnEvent callback;
} RegisterEvent;

typedef struct EventCodeEntry {
    RegisterEvent* events;
} EventCodeEntry;

#define MAX_MESSAGE_CODES 16384

typedef struct EventSystemState {
    EventCodeEntry registerd[MAX_MESSAGE_CODES];
} EventSystemState;

static EventSystemState* statePtr;

void EventSystemInitialize(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(EventSystemState);
    if (state == 0) 
        return;
    ZeroMemory(state, sizeof(state));
    statePtr = state;
}

void EventSystemShutdown(void* state) {
    if (statePtr) {
        // Free the events arrays. And objects pointed to should be destroyed on their own.
        for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
            if (statePtr->registerd[i].events != 0) {
                DarrayDestroy(statePtr->registerd[i].events);
                statePtr->registerd[i].events = 0;
            }
        }
    }
    statePtr = 0;
}

b8 EventRegister(u16 code, void* listener, PFN_OnEvent onEvent) {
    if (!statePtr)
        return false;
    
    if (statePtr->registerd[code].events == 0)
        statePtr->registerd[code].events = DarrayCreate(RegisterEvent);
    
    u64 registeredCount = DarrayLength(statePtr->registerd[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        if (statePtr->registerd[code].events[i].listner == listener)
            return false;
    }

    RegisterEvent event;
    event.listner = listener;
    event.callback = onEvent;
    DarrayPush(statePtr->registerd[code].events, event);
    return true;
}

b8 EventUnregister(u16 code, void* listener, PFN_OnEvent onEvent){
    if (!statePtr) 
        return false;
    
    if (statePtr->registerd[code].events == 0) 
        return false;

    u64 registeredCount = DarrayLength(statePtr->registerd[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisterEvent e = statePtr->registerd[code].events[i];
        if (e.listner == listener && e.callback == onEvent) {
            RegisterEvent poppedEvent;
            DarrayPopAt(statePtr->registerd[code].events, i, &poppedEvent);
            return true;
        }
    }

    return false;
}

b8 EventFire(u16 code, void* sender, EventContext context) {
    if (!statePtr)
        return false;
    
    if (statePtr->registerd[code].events == 0)
        return false;
    
    u64 registeredCount = DarrayLength(statePtr->registerd[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisterEvent e = statePtr->registerd[code].events[i];
        if (e.callback(code, sender, e.listner, context)) 
            return true;
    }

    return false;
}