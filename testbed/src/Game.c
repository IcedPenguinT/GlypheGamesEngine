#include "Game.h"
#include <core/Logger.h>
#include <core/Memory.h>
#include <core/Input.h>

b8 GameInitialize(Game* gameInst) {
    KDEBUG("GameInitialize() called!");
    return true;
}

b8 GameUpdate(Game* gameInst, f32 deltaTime) {
    static u64 allocCount = 0;
    u64 prevAllocCount = allocCount;
    allocCount = GetMemoryAllocCount();
    if (InputIsKeyUp('M') && InputWasKeyDown('M'))
        KDEBUG("Allocations: %llu (%llu this frame)", allocCount, allocCount - prevAllocCount)
    return true;
}

b8 GameRender(Game* gameInst, f32 deltaTime) {
    return true;
}

void GameOnResize(Game* gameInst, u32 width, u32 height) {
    
}