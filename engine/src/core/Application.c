#include "Application.h"
#include "GameTypes.h"

#include "Logger.h"
#include "platform/Platform.h"
#include "core/Memory.h"
#include "core/Event.h"
#include "core/Input.h"
#include "core/Clock.h"

#include "memory/LinearAllocator.h"

#include "renderer/RendererFrontend.h"


typedef struct ApplicationState {
    Game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    i16 width;
    i16 height;
    Clock clock;
    f64 lastTime;
    LinearAllocator systemsAllocator;
    
    u64 eventSystemMemoryRequirement;
    void* eventSystemState;

    u64 memorySystemMemoryRequirement;
    void* memorySystemState;

    u64 loggingSystemMemoryRequirement;
    void* loggingSystemState;

    u64 inputSystemMemoryRequirement;
    void* inputSystemState;

    u64 platformSystemMemoryRequirement;
    void* platformSystemState;

    u64 rendererSystemMemoryRequirement;
    void* rendererSystemState;
} ApplicationState;

static ApplicationState* appState;

b8 ApplicationOnEvent(u16 code, void* sender, void* listnerInst, EventContext context);
b8 ApplicationOnKey(u16 code, void* sender, void* listnerInst, EventContext context);
b8 ApplicationOnResized(u16 code, void* sender, void* listnerInst, EventContext context);


b8 CreateApplication(Game* gameInst){
    if (gameInst->applicationState){
        KERROR("CreateApplication called more than once.");
        return false;
    }

    gameInst->applicationState = Allocate(sizeof(ApplicationState), MEMORY_TAG_APPLICATION);
    appState = gameInst->applicationState;
    appState->gameInst = gameInst;
    appState->isRunning = false;
    appState->isSuspended = false;

    u64 systemAllocatorTotalSize = 64 * 1024 * 1024; // 64mb
    LinearAllocatorCreate(systemAllocatorTotalSize, 0, &appState->systemsAllocator);

    //Events
    EventSystemInitialize(&appState->eventSystemMemoryRequirement, 0);
    appState->eventSystemState = LinearAllocatorAllocate(&appState->systemsAllocator, appState->eventSystemMemoryRequirement);
    EventSystemInitialize(&appState->eventSystemMemoryRequirement, appState->eventSystemState);

    // Memory
    MemorySystemInitialize(&appState->memorySystemMemoryRequirement, 0);
    appState->memorySystemState = LinearAllocatorAllocate(&appState->systemsAllocator, appState->memorySystemMemoryRequirement);
    MemorySystemInitialize(&appState->memorySystemMemoryRequirement, appState->memorySystemState);

    // Logging
    InitializeLogging(&appState->loggingSystemMemoryRequirement, 0);
    appState->loggingSystemState = LinearAllocatorAllocate(&appState->systemsAllocator, appState->loggingSystemMemoryRequirement);
    if (!InitializeLogging(&appState->loggingSystemMemoryRequirement, appState->loggingSystemState)) {
        KERROR("Failed to initialize the logging system. Shutting down...");
        return false;
    }

    // Input
    InputSystemInitialize(&appState->inputSystemMemoryRequirement, 0);
    appState->inputSystemState = LinearAllocatorAllocate(&appState->systemsAllocator, appState->inputSystemMemoryRequirement);
    InputSystemInitialize(&appState->inputSystemMemoryRequirement, appState->inputSystemState);

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_BUTTON_RELEASED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_RESIZED, 0, ApplicationOnResized);

    // Platform
    PlatformSystemStartup(&appState->platformSystemMemoryRequirement, 0, 0, 0, 0, 0, 0);
    appState->platformSystemState = LinearAllocatorAllocate(&appState->systemsAllocator, appState->platformSystemMemoryRequirement);
    if (!PlatformSystemStartup(
            &appState->platformSystemMemoryRequirement,
            appState->platformSystemState,
            gameInst->config.name,
            gameInst->config.startPosX,
            gameInst->config.startPosY,
            gameInst->config.startWidth,
            gameInst->config.startHeight)) {
        return false;
    }

    // Render System
    RendererSystemInitialize(&appState->rendererSystemMemoryRequirement, 0, 0);
    appState->rendererSystemState = LinearAllocatorAllocate(&appState->systemsAllocator, appState->rendererSystemMemoryRequirement);
    if (!RendererSystemInitialize(&appState->rendererSystemMemoryRequirement, appState->rendererSystemState, gameInst->config.name)) {
        KFATAL("Failed to initialize renderer. Aborting application.");
        return false;
    }

    if (!appState->gameInst->initialize(appState->gameInst)){
        KFATAL("Game failed to Initialize!");
        return false;
    }

    // Call resize once to ensure the proper size has been set.
    appState->gameInst->onResize(appState->gameInst, appState->width, appState->height);

    return true;
}

b8 RunApplication() {
    appState->isRunning = true;
    ClockStart(&appState->clock);
    ClockUpdate(&appState->clock);
    appState->lastTime = appState->clock.elapsed;
    
    f64 runningTime = 0;
    u8 frameCount = 0;
    f64 targetFrameSeconds = 1.0f / 60;

    KINFO(GetMemoryUsageStr())

    while (appState->isRunning) {
        if (!PlatformPumpMessages())
            appState->isRunning = false;

        if (!appState->isSuspended) {
            ClockUpdate(&appState->clock);
            f64 currentTime = appState->clock.elapsed;
            f64 delta = (currentTime - appState->lastTime);
            f64 frameStartTime = PlatformGetAbsoluteTime();

            if (!appState->gameInst->update(appState->gameInst, (f32)delta)) {
                KFATAL("Game update failed, shutting down.");
                appState->isRunning = false;
                break;
            }

            if (!appState->gameInst->render(appState->gameInst, (f32)delta)) {
                KFATAL("Game render failed, shutting down.");
                appState->isRunning = false;
                break;
            }

            //TODO: refactor packet creation
            RenderPacket packet;
            packet.deltaTime = delta;
            RendererDrawFrame(&packet);

            f64 frameEndTime = PlatformGetAbsoluteTime();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

            if (remainingSeconds > 0){
                u64 remainingMs = (remainingSeconds * 1000);
                b8 limitFrames = false;
                if (remainingMs > 0 && limitFrames)
                    PlatformSleep(remainingMs - 1);
                frameCount++;
            }

            // Input will always be checked at the end.
            InputUpdate(delta);

            appState->lastTime = currentTime;
        }
    }
    appState->isRunning = false;

    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_BUTTON_RELEASED, 0, ApplicationOnKey);

    InputSystemShutdown(appState->inputSystemState);
    RendererSystemShutdown(appState->rendererSystemState);
    PlatformSystemShutdown(appState->platformSystemState);
    MemorySystemShutdown(appState->memorySystemState);
    EventSystemShutdown(appState->eventSystemState);
    
    return true;
}

void ApplicationGetFramebufferSize(u32* width, u32* height) {
    *width = appState->width;
    *height = appState->height;
}

b8 ApplicationOnEvent(u16 code, void* sender, void* listnerInst, EventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.\n");
            appState->isRunning = false;
            return true;
        }
    }
    return false;
}

b8 ApplicationOnKey(u16 code, void* sender, void* listnerInst, EventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 keyCode = context.Data.u16[0];
        if (keyCode == KEY_ESCAPE) {
            EventContext data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return true;
        } else if (keyCode == KEY_A) {
            KDEBUG("Explicit - A key Pressed!");
        } else {
            KDEBUG ("'%c' key pressed in window.", keyCode);
        }
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        u16 keyCode = context.Data.u16[0];
        if (keyCode == KEY_B){
            KDEBUG("Explicit - B key released!");
        } else {
            KDEBUG("'%c' key released in window.", keyCode);
        }
    }
    return false;
}

b8 ApplicationOnResized(u16 code, void* sender, void* listnerInst, EventContext context) {
    if (code == EVENT_CODE_RESIZED) {
        u16 width = context.Data.u16[0];
        u16 height = context.Data.u16[1];

        // Check if different. If so, trigger a resize event.
        if (width != appState->width || height != appState->height) {
            appState->width = width;
            appState->height = height;

            KDEBUG("Window resize: %i, %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                KINFO("Window minimized, suspending application.");
                appState->isSuspended = true;
                return true;
            } else {
                if (appState->isSuspended) {
                    KINFO("Window restored, resuming application.");
                    appState->isSuspended = false;
                }
                appState->gameInst->onResize(appState->gameInst, width, height);
                RendererOnResized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}