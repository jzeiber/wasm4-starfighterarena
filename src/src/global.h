#pragma once

#define MAX_PLAYERS     4
#define MAX_PROJECTILES 20
#define MAX_OBSTACLES   10

#include "game.h"
#include "input.h"

class Game;
class Input;

enum Direction
{
DIR_UP=1,
DIR_RIGHT=2,
DIR_DOWN=3,
DIR_LEFT=4
};

namespace global
{
    extern Game *game;
    extern Input *input;

    void SetupGlobals();
}
