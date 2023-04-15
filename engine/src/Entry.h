#pragma once

#include "core/Application.h"
#include "core/Logger.h"
#include "GameTypes.h"

extern b8 CreateGame(Game* outGame);

/**
 * The Main entry point of the application.
*/
int main(void) {

    Game gameInst;
    if (!CreateGame(&gameInst)) {
        KFATAL("Could not create game!");
        return -1;
    }

    if (!gameInst.render || !gameInst.update || !gameInst.initialize || !gameInst.onResize) {
        KFATAL("The game's function pointers are null!");
        return -2;
    }

    if(!CreateApplication(&gameInst)){
        KINFO("Application failed to create!");
        return 1;
    }
    
    if (!RunApplication()) {
        KINFO("Application did not shutdown gracefully.");
        return 2;
    }

    return 0;
}