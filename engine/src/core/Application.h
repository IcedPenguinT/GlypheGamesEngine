#pragma once

#include "Defines.h"
struct Game;

typedef struct ApplicationConfig {
    i16 startPosX;
    i16 startPosY;
    i16 startWidth;
    i16 startHeight;
    char* name;
} ApplicationConfig;


KAPI b8 CreateApplication(struct Game* gameInst);
KAPI b8 RunApplication();