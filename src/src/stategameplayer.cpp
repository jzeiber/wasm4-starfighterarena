#include "stategameplayer.h"

#include "wasm4.h"
#include "palette.h"
#include "textprinter.h"
#include "font5x7.h"
#include "base64.h"
#include "outputstringstream.h"
#include "cppfuncs.h"
#include "wasmmath.h"
#include "randommt.h"

StateGamePlayer::StateGamePlayer():m_ticks(0),m_lastshot(0),m_lastdead(0),m_nextweapon(0),m_x(0),m_y(0),m_rot(0),m_vel(0),m_velrot(0),m_impulse(0),m_impulserot(0),m_status(STATUS_DEAD),m_energy(0),m_allowshoot(false),m_showhud(false),m_showstats(false)
{

}

StateGamePlayer::~StateGamePlayer()
{

}

uint8_t StateGamePlayer::State() const
{
    return Game::STATE_PLAYING;
}

void StateGamePlayer::StateChanged(const uint8_t prevstate, void *params)
{
    if(params)
    {
        m_game=((GameParams *)params)->m_game;
        m_starfighter.FromCode(((GameParams *)params)->m_code);
        delete params;
        m_showhud=false;
        m_showstats=false;
        m_ticks=m_game->GetTicks();

        ResetStats();
        Spawn();
    }
}

bool StateGamePlayer::HandleInput(const Input *input, const uint8_t playerindex)
{
    if(input->GamepadButtonDown(playerindex+1,BUTTON_LEFT)==true)
    {
        if(m_status==STATUS_ALIVE)
        {
            m_rot-=0.05;
            while(m_rot<0)
            {
                m_rot+=(2.0*M_PI);
            }
        }
    }
    if(input->GamepadButtonDown(playerindex+1,BUTTON_RIGHT)==true)
    {
        if(m_status==STATUS_ALIVE)
        {
            m_rot+=0.05;
            while(m_rot>(2.0*M_PI))
            {
                m_rot-=(2.0*M_PI);
            }
        }
    }
    if(input->GamepadButtonDown(playerindex+1,BUTTON_UP)==true)
    {
        if(m_status==STATUS_ALIVE)
        {
            m_impulse=0.01;
            m_impulserot=m_rot;
        }
    }
    if(input->GamepadButtonDown(playerindex+1,BUTTON_DOWN)==true)
    {
        if(m_status==STATUS_ALIVE)
        {
            m_showstats=true;
        }
    }
    else
    {
        if(m_status==STATUS_ALIVE)
        {
            m_showstats=false;
        }
    }
    if(input->GamepadButtonDown(playerindex+1,BUTTON_1)==true)
    {
        if(m_status==STATUS_ALIVE && m_allowshoot==true)
        {
            if(m_lastshot+10u<=m_ticks)
            {
                if(m_energy>20.0)
                {
                    fpoint2d point;
                    m_starfighter.WeaponCoord(m_nextweapon,1.0,m_rot,point);
                    point.m_x+=m_x;
                    point.m_y+=m_y;
                    m_game->AddProjectile(playerindex,point,3.0,m_rot);
                    m_nextweapon==1 ? m_nextweapon=0 : m_nextweapon++;
                    m_lastshot=m_ticks;
                    m_stats.m_projectilesfired++;
                    m_energy-=25.0;
                }
            }
        }
    }
    else    // button 1 not pressed - allow shooting
    {
        m_allowshoot=true;
    }

    if(input->GamepadButtonPress(playerindex+1,BUTTON_1)==true)
    {
        if(m_status==STATUS_DEAD)
        {
            Spawn();
        }
    }

    if(input->GamepadButtonPress(playerindex+1,BUTTON_2)==true)
    {
        if(m_status==STATUS_ALIVE)
        {
            m_showhud=!m_showhud;
        }
    }

    return false;
}

void StateGamePlayer::Update(const int ticks, const uint8_t playerindex, Game *game)
{
    m_ticks=game->GetTicks();
    if(m_status==STATUS_ALIVE)
    {
        m_energy+=2.0;
        if(m_energy>MaxEnergy())
        {
            m_energy=MaxEnergy();
        }

        if(m_impulse!=0.0)
        {
            m_energy-=1.5;
            float dx=_cos(m_impulserot)*m_impulse;
            float dy=_sin(m_impulserot)*m_impulse;
            float cx=_cos(m_velrot)*m_vel;
            float cy=_sin(m_velrot)*m_vel;
            cx+=dx;
            cy+=dy;
            m_velrot=_atan2(cy,cx);
            m_vel=_sqrt((cx*cx)+(cy*cy));
            if(m_vel>1.5)
            {
                m_vel=1.5;
            }
            m_impulse=0.0;
        }
        else
        {
            m_vel-=0.005;
            if(m_vel<0)
            {
                m_vel=0;
            }
        }
        if(m_vel!=0.0)
        {
            m_x+=_cos(m_velrot)*m_vel*ticks;
            m_y+=_sin(m_velrot)*m_vel*ticks;
        }
    }
    if(m_status==STATUS_DEAD)
    {
        // if waiting too long - send back to code entry
        if(m_lastdead+3600 < m_ticks)
        {
            m_game->ChangeState(playerindex,Game::STATE_ENTERINGCODE,nullptr);
        }
    }
}

void StateGamePlayer::Draw()
{
    if(m_status==STATUS_ALIVE)
    {
        m_game->DrawStarfield(m_x,m_y);

        DrawShip(SCREEN_SIZE/2,SCREEN_SIZE/2,1.0);
        DrawShield(SCREEN_SIZE/2,SCREEN_SIZE/2,1.0);

        for(int i=0; i<m_game->PlayerCount(); i++)
        {
            IState *ps=m_game->GetPlayerState(i);
            if(ps!=(IState *)this && ps->State()==Game::STATE_PLAYING)
            {
                StateGamePlayer *gs=static_cast<StateGamePlayer *>(ps);
                if(gs->Status()==STATUS_ALIVE)
                {
                    float dx=gs->PlayerX()-m_x;
                    float dy=gs->PlayerY()-m_y;
                    gs->DrawShip((SCREEN_SIZE/2)+dx,(SCREEN_SIZE/2)+dy,1.0);
                    gs->DrawShield((SCREEN_SIZE/2)+dx,(SCREEN_SIZE/2)+dy,1.0);
                }
            }
        }

        m_game->DrawProjectiles(m_x,m_y);

        DrawEnergy((SCREEN_SIZE/2)-15,SCREEN_SIZE-5,30,2);
        if(m_showhud==true)
        {
            DrawRadar(SCREEN_SIZE-48,1,23);
        }

        if(m_showstats==true)
        {
            m_game->DrawLeaderboard(10,10,SCREEN_SIZE-20,true);
        }

    }
    else
    {
        m_game->DrawLeaderboard(10,10,SCREEN_SIZE-20,false);
        TextPrinter tp;
        tp.SetCustomFont(&Font5x7::Instance());
        tp.PrintCentered("Waiting To Enter Arena",SCREEN_SIZE/2,(SCREEN_SIZE/2)-10,22,PALETTE_WHITE);
        tp.PrintCentered("Press Button 1 To Continue",SCREEN_SIZE/2,(SCREEN_SIZE/2)+30,26,PALETTE_WHITE);
    }

}

void StateGamePlayer::DrawShip(const float x, const float y, const float scale)
{
    m_starfighter.Draw(x,y,scale,m_rot);
}

void StateGamePlayer::DrawShield(const float x, const float y, const float scale)
{
    // TODO - damaged shields
    *DRAW_COLORS=PALETTE_LIGHTGREY << 4;
    float size=scale*2.0*ShieldRadius();
    oval(x-(size/2.0),y-(size/2.0),size,size);
}

uint8_t StateGamePlayer::Status() const
{
    return m_status;
}

StateGamePlayer::Stats StateGamePlayer::GetStats() const
{
    return m_stats;
}

float StateGamePlayer::PlayerX() const
{
    return m_x;
}

float StateGamePlayer::PlayerY() const
{
    return m_y;
}

float StateGamePlayer::MaxEnergy() const
{
    return 1000.0;
}

float StateGamePlayer::ShieldRadius() const
{
    return 9.5;
}

void StateGamePlayer::ResetStats()
{
    m_stats.m_deaths=0;
    m_stats.m_kills=0;
    m_stats.m_projectilesfired=0;
    m_stats.m_projectileshit=0;
}

void StateGamePlayer::DrawEnergy(const int32_t x, const int32_t y, const int32_t width, const int32_t height) const
{
    *DRAW_COLORS=PALETTE_DARKGREY;
    line(x-1,y,x-1,y+height-1);
    line(x,y-1,x+width-1,y-1);
    line(x,y+height,x+width-1,y+height);
    line(x+width,y,x+width,y+height-1);

    float percent=m_energy/MaxEnergy();
    *DRAW_COLORS=PALETTE_LIGHTGREY << 4 | PALETTE_LIGHTGREY;
    const int32_t rw=static_cast<float>(width)*percent;
    if(rw>0)
    {
        rect(x,y,rw,2);
    }
}

void StateGamePlayer::DrawRadar(const int32_t x, const int32_t y, const int32_t radius) const
{
    *DRAW_COLORS=PALETTE_LIGHTGREY << 4 | PALETTE_DARKGREY;
    oval(x,y,1+radius*2.0,radius*2.0);

    float maxrange;
    for(int i=0; i<m_game->PlayerCount(); i++)
    {
        IState *ps=m_game->GetPlayerState(i);
        if(ps!=this && ps->State()==Game::STATE_PLAYING)
        {
            StateGamePlayer *gp=(StateGamePlayer *)ps;
            if(gp->Status()==STATUS_ALIVE)
            {
                float dx=(gp->PlayerX()-m_x)/60.0;
                float dy=(gp->PlayerY()-m_y)/60.0;
                float distance=_sqrt((dx*dx)+(dy*dy));

                if(distance>=(radius-1.0))
                {
                    dx*=(static_cast<float>(radius-1.0)/distance);
                    dy*=(static_cast<float>(radius-1.0)/distance);
                }

                if(m_ticks%2==0)
                {
                    *DRAW_COLORS=PALETTE_WHITE;
                    line(x+radius+dx,y+radius+dy,x+radius+dx,y+radius+dy);
                }
            }
        }
    }

    *DRAW_COLORS=PALETTE_WHITE;
    line(x+radius,y+radius,x+radius,y+radius);

}

void StateGamePlayer::Spawn()
{
    m_allowshoot=false;     // force wait until button is released once
    m_status=STATUS_ALIVE;
    m_lastshot=0;
    m_nextweapon=0;
    m_energy=MaxEnergy();
    m_lastshot=0;
    m_vel=0;
    m_impulse=0;

    RandomMT rand(m_ticks);
    bool goodpos=false;
    float posrad=200.0;
    while(goodpos==false)
    {
        goodpos=true;
        m_rot=rand.NextDouble()*2.0*M_PI;
        m_x=(rand.NextDouble()*2.0*posrad)-posrad;
        m_y=(rand.NextDouble()*2.0*posrad)-posrad;
        for(int i=0; i<m_game->PlayerCount() && goodpos==true; i++)
        {
            if(m_game->GetPlayerState(i)!=this && m_game->GetPlayerState(i)->State()==Game::STATE_PLAYING)
            {
                StateGamePlayer *gp=(StateGamePlayer *)m_game->GetPlayerState(i);
                if(gp->Status()==STATUS_ALIVE)
                {
                    float dx=m_x-gp->PlayerX();
                    float dy=m_y-gp->PlayerY();
                    float d2=(dx*dx)+(dy*dy);
                    if(d2<=(100.0*100.0))
                    {
                        goodpos=false;
                    }
                }
            }
        }
        if(goodpos==false)
        {
            posrad+=100.0;
        }
    }

}

void StateGamePlayer::RegisterHitDestination(const Game::projectile &p, bool &killed)
{
    m_energy-=p.m_yield;
    if(m_energy<=0.0)
    {
        // TODO - animation? sound?
        killed=true;
        m_stats.m_deaths++;
        m_status=STATUS_DEAD;
        m_lastdead=m_ticks;
    }
    else
    {
        killed=false;
    }
}

void StateGamePlayer::RegisterHitSource(const Game::projectile &p, const bool killed)
{
    m_stats.m_projectileshit++;
    if(killed==true)
    {
        m_stats.m_kills++;
    }
}
