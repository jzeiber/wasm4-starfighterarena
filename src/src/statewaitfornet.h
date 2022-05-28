#pragma once

#include "istate.h"
#include "game.h"

class StateWaitForNet:public IState
{
public:
    StateWaitForNet();
    ~StateWaitForNet();

    uint8_t State() const;

    void StateChanged(const uint8_t prevstate, void *params);
    bool HandleInput(const Input *input, const uint8_t playerindex);
    void Update(const int ticks, const uint8_t playerindex, Game *game=nullptr);
    void Draw();

private:
    uint64_t m_ticks;

};
