#include "Application.h"
#include "GameTypes.h"

#include "Logger.h"
#include "platform/Platform.h"
#include "core/Memory.h"


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

b8 CreateApplication(Game* gameInst){
    if (initialized){
        KERROR("CreateApplication called more than once.");
        return FALSE;
    }

    appState.gameInst = gameInst;

    InitializeLogging();

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

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
    }
    appState.isRunning = FALSE;
    PlatformShutdown(&appState.platfrom);
    return TRUE;
}