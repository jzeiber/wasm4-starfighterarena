#pragma once

#include "istate.h"
#include "game.h"
#include "starfighter.h"

class StateGamePlayer:public IState
{
public:
    StateGamePlayer();
    ~StateGamePlayer();

    struct GameParams
    {
        Game *m_game;
        uint8_t m_code[16];
    };

    struct Stats
    {
        uint64_t m_projectilesfired;
        uint64_t m_projectileshit;
        uint32_t m_kills;
        uint32_t m_deaths;
    };

    enum Status
    {
        STATUS_ALIVE=1,
        STATUS_DEAD=2
    };

    uint8_t State() const;

    void StateChanged(const uint8_t prevstate, void *params);
    bool HandleInput(const Input *input, const uint8_t playerindex);
    void Update(const int ticks, const uint8_t playerindex, Game *game=nullptr);
    void Draw();
    void DrawShip(const float x, const float y, const float scale);
    void DrawShield(const float x, const float y, const float scale);

    uint8_t Status() const;
    Stats GetStats() const;
    float PlayerX() const;
    float PlayerY() const;

    float ShieldRadius() const;
    float MaxEnergy() const;
    void ResetStats();

    void DrawEnergy(const int32_t x, const int32_t y, const int32_t width, const int32_t height) const;
    void DrawRadar(const int32_t x, const int32_t y, const int32_t radius) const;

    void Spawn();

    void RegisterHitDestination(const Game::projectile &p, bool &killed);
    void RegisterHitSource(const Game::projectile &p, const bool killed);

private:

    Game *m_game;
    uint64_t m_ticks;
    uint64_t m_lastshot;
    uint64_t m_lastdead;
    uint8_t m_nextweapon;
    Starfighter m_starfighter;
    uint8_t m_status;
    float m_energy;
    float m_x;
    float m_y;
    float m_rot;
    float m_vel;
    float m_velrot;
    float m_impulse;
    float m_impulserot;
    Stats m_stats;
    bool m_allowshoot;
    bool m_showhud;
    bool m_showstats;

};
