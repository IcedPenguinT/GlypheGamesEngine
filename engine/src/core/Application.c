#include "Application.h"
#include "GameTypes.h"

#include "Logger.h"
#include "platform/Platform.h"
#include "core/Memory.h"
#include "core/Event.h"
#include "core/Input.h"


typedef struct ApplicationState {
    Game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    PlatformState platfrom;
    i16 width;
    i16 height;
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

    if (!appState.gameInst->initialize(appState.gameInst)){
        KFATAL("Game failed to Initialize!");
        return FALSE;
    }

    appState.gameInst->onResize(appState.gameInst, appState.width, appState.height);

    initialized = TRUE;
    return TRUE;
}

b8 RunApplication() {
    KINFO(GetMemoryUsageStr())

    while (appState.isRunning) {
        if (!PlatformPumpMessages(&appState.platfrom))
            appState.isRunning = FALSE;

        if (!appState.isSuspended) {
            if (!appState.gameInst->update(appState.gameInst, (f32)0)) {
                KFATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            if (!appState.gameInst->render(appState.gameInst, (f32)0)) {
                KFATAL("Game render failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            // Input will always be checked at the end.
            InputUpdate(0);
        }
    }
    appState.isRunning = FALSE;

    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    EventUnregister(EVENT_CODE_BUTTON_RELEASED, 0, applicationOnKey);

    EventShutdown();
    InputShutdown();

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