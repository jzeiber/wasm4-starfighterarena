#pragma once

#include "istate.h"
#include "game.h"
#include "starfighter.h"

class StateEnteringCode:public IState
{
public:
    StateEnteringCode();
    ~StateEnteringCode();

    uint8_t State() const;

    void StateChanged(const uint8_t prevstate, void *params);
    bool HandleInput(const Input *input, const uint8_t playerindex);
    void Update(const int ticks, const uint8_t playerindex, Game *game=nullptr);
    void Draw();

private:

    enum Location
    {
        LOC_CODE=1,
        LOC_PRESET=2,
        LOC_INVALID=3,
        LOC_VALID=4
    };

    uint64_t m_ticks;
    uint64_t m_blinkticks;
    uint8_t m_code[16];
    uint8_t m_codepos;
    uint8_t m_letterpos;
    uint8_t m_presetpos;
    uint8_t m_loc;

    static constexpr int32_t m_lettercols=13;
    static constexpr int32_t m_letterrows=5;

    Starfighter m_preset[4];
    Starfighter m_starfighter;
    uint8_t m_changestate;

};
