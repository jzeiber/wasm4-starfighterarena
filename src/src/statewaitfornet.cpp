#include "statewaitfornet.h"

#include "wasm4.h"
#include "palette.h"
#include "textprinter.h"
#include "font5x7.h"
#include "netfuncs.h"

StateWaitForNet::StateWaitForNet():m_ticks(0)
{

}

StateWaitForNet::~StateWaitForNet()
{

}

uint8_t StateWaitForNet::State() const
{
    return Game::STATE_WAITINGFORNET;
}

void StateWaitForNet::StateChanged(const uint8_t prevstate, void *params)
{

}

bool StateWaitForNet::HandleInput(const Input *input, const uint8_t playerindex)
{
    return false;
}

void StateWaitForNet::Update(const int ticks, const uint8_t playerindex, Game *game)
{
    m_ticks=game->GetTicks();
    if(netplay_active())
    {
        game->ChangeState(playerindex,Game::STATE_ENTERINGCODE,nullptr);
    }
}

void StateWaitForNet::Draw()
{
    TextPrinter tp;
    tp.SetCustomFont(&Font5x7::Instance());

    tp.PrintCentered("Starfighter Arena",SCREEN_SIZE/2,20,18,PALETTE_WHITE);

    if((m_ticks % 40ULL)<30)
    {
        tp.PrintCentered("Waiting For Netplay Session",SCREEN_SIZE/2,60,27,PALETTE_WHITE);
    }
    
    tp.PrintWrapped("Start netplay session or connect to existing session to continue",15,100,200,SCREEN_SIZE-30,PALETTE_WHITE,TextPrinter::JUSTIFY_LEFT);
}
