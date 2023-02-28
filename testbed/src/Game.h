#pragma once

#include <Defines.h>
#include <GameTypes.h>

typedef struct GameState {
    f32 deltaTime;
} GameState;

b8 GameInitialize(Game* gameInst);

b8 GameUpdate(Game* gameInst, f32 deltaTime);

b8 GameRender(Game* gameInst, f32 deltaTime);

void GameOnResize(Game* gameInst, u32 width, u32 height);