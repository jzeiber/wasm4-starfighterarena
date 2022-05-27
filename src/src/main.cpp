#include "wasm4.h"
#include "global.h"

void start()
{
    PALETTE[0]=0x000000;
    PALETTE[1]=0x303030;
    PALETTE[2]=0x808080;
    PALETTE[3]=0xffffff;

    global::SetupGlobals();

}

void update()
{
    global::input->Update();
    global::game->HandleInput(global::input,0);
    global::game->Update(1,0,global::game);
    global::game->Draw();
}
