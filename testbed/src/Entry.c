#include "Game.h"
#include <Entry.h>
#include <core/Memory.h>

//Define the function to create a game
b8 CreateGame(Game* outGame){
    outGame->config.startPosX = 100;
    outGame->config.startPosY = 100;
    outGame->config.startWidth = 1280;
    outGame->config.startHeight = 720;
    outGame->config.name = "GlypheGames Engine Testbed";
    outGame->update = GameUpdate;
    outGame->render = GameRender;
    outGame->initialize = GameInitialize;
    outGame->onResize = GameOnResize;

    outGame->state = Allocate(sizeof(GameState), MEMORY_TAG_GAME);

    return TRUE;
}