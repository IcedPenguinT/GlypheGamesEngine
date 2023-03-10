#include "Application.h"
#include "GameTypes.h"

#include "Logger.h"
#include "platform/Platform.h"
#include "core/Memory.h"
#include "core/Event.h"
#include "core/Input.h"
#include "core/Clock.h"

#include "renderer/RendererFrontend.h"


typedef struct ApplicationState {
    Game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    PlatformState platfrom;
    i16 width;
    i16 height;
    Clock clock;
    f64 lastTime;
} ApplicationState;

static b8 initialized = FALSE;
static ApplicationState appState = {0};

b8 applicationOnEvent(u16 code, void* sender, void* listnerInst, EventContext context);
b8 applicationOnKey(u16 code, void* sender, void* listnerInst, EventContext context);

b8 CreateApplication(Game* gameInst){
    if (initialized){
        KERROR("CreateApplication called more than once.");
        return FALSE;
    }

    appState.gameInst = gameInst;

    InitializeLogging();
    InputInitialize();

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

    if (!EventInitialize()) {
        KERROR("Event system failed initialization. Application cannot continue!");
        return FALSE;
    }

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    EventRegister(EVENT_CODE_BUTTON_RELEASED, 0, applicationOnKey);

    if (!PlatformStartup(&appState.platfrom, gameInst->config.name, gameInst->config.startPosX, gameInst->config.startPosY, gameInst->config.startWidth, gameInst->config.startHeight)) {
        return FALSE;
    }

    if (!RendererInitialize(gameInst->config.name, &appState.platfrom)){
        KFATAL("Failed to initialize renderer. Aborting application.");
        return FALSE;
    }

    if (!appState.gameInst->initialize(appState.gameInst)){
        KFATAL("Game failed to Initialize!");
        return FALSE;
    }

    appState.gameInst->onResize(appState.gameInst, appState.width, appState.height);

    initialized = TRUE;
    return TRUE;
}

b8 RunApplication() {
    ClockStart(&appState.clock);
    ClockUpdate(&appState.clock);

    appState.lastTime = appState.clock.elapsed;
    f64 runningTime = 0;
    u8 frameCount = 0;
    f64 targetFrameSeconds = 1.0f / 60;

    KINFO(GetMemoryUsageStr())

    while (appState.isRunning) {
        if (!PlatformPumpMessages(&appState.platfrom))
            appState.isRunning = FALSE;

        if (!appState.isSuspended) {
            ClockUpdate(&appState.clock);
            f64 currentTime = appState.clock.elapsed;
            f64 delta = (currentTime - appState.lastTime);
            f64 frameStartTime = PlatformGetAbsoluteTime();

            if (!appState.gameInst->update(appState.gameInst, (f32)delta)) {
                KFATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            if (!appState.gameInst->render(appState.gameInst, (f32)delta)) {
                KFATAL("Game render failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            //TODO:
            RenderPacket packet;
            packet.deltaTime = delta;
            RendererDrawFrame(&packet);

            f64 frameEndTime = PlatformGetAbsoluteTime();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

            if (remainingSeconds > 0){
                u64 remainingMs = (remainingSeconds * 1000);
                b8 limitFrames = FALSE;
                if (remainingMs > 0 && limitFrames)
                    PlatformSleep(remainingMs - 1);
                frameCount++;
            }

            // Input will always be checked at the end.
            InputUpdate(delta);

            appState.lastTime = currentTime;
        }
    }
    appState.isRunning = FALSE;

    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    EventUnregister(EVENT_CODE_BUTTON_RELEASED, 0, applicationOnKey);

    EventShutdown();
    InputShutdown();
    RendererShutdown();

    PlatformShutdown(&appState.platfrom);
    return TRUE;
}

b8 applicationOnEvent(u16 code, void* sender, void* listnerInst, EventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.\n");
            appState.isRunning = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

b8 applicationOnKey(u16 code, void* sender, void* listnerInst, EventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 keyCode = context.Data.u16[0];
        if (keyCode == KEY_ESCAPE) {
            EventContext data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return TRUE;
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
    return FALSE;
}