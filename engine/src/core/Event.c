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

static b8 isInitialized = FALSE;
static EventSystemState state;

b8 EventInitialize() {
    if (isInitialized == TRUE)
        return FALSE;

    isInitialized = FALSE;
    ZeroMemory(&state, sizeof(state));

    isInitialized = TRUE;
    return TRUE;
}

void EventShutdown() {
    for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
        if (state.registerd[i].events != 0) {
            DarrayDestroy(state.registerd[i].events);
            state.registerd[i].events = 0;
        }
    }
}

b8 EventRegister(u16 code, void* listener, PFN_OnEvent onEvent) {
    if (isInitialized == FALSE)
        return FALSE;
    
    if (state.registerd[code].events == 0)
        state.registerd[code].events = DarrayCreate(RegisterEvent);
    
    u64 registeredCount = DarrayLength(state.registerd[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        if (state.registerd[code].events[i].listner == listener)
            return FALSE;
    }

    RegisterEvent event;
    event.listner = listener;
    event.callback = onEvent;
    DarrayPush(state.registerd[code].events, event);
    return TRUE;
}

b8 EventUnregister(u16 code, void* listener, PFN_OnEvent onEvent){
    if (isInitialized == FALSE) 
        return FALSE;
    
    if (state.registerd[code].events == 0) 
        return FALSE;

    u64 registeredCount = DarrayLength(state.registerd[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisterEvent e = state.registerd[code].events[i];
        if (e.listner == listener && e.callback == onEvent) {
            RegisterEvent poppedEvent;
            DarrayPopAt(state.registerd[code].events, i, &poppedEvent);
            return TRUE;
        }
    }

    return FALSE;
}

b8 EventFire(u16 code, void* sender, EventContext context) {
    if (isInitialized == FALSE)
        return FALSE;
    
    if (state.registerd[code].events == 0)
        return FALSE;
    
    u64 registeredCount = DarrayLength(state.registerd[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisterEvent e = state.registerd[code].events[i];
        if (e.callback(code, sender, e.listner, context)) 
            return TRUE;
    }

    return FALSE;
}