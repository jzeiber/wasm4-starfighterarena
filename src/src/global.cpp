
#include "global.h"

#include "wasm4.h"

namespace global
{
    Game *game;
    Input *input;

    void SetupGlobals()
    {
        game=new Game();
        input=new Input();
    }
}
