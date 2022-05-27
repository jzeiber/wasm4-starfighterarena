#include "stateenteringcode.h"

#include "wasm4.h"
#include "palette.h"
#include "textprinter.h"
#include "font5x7.h"
#include "base64.h"
#include "outputstringstream.h"
#include "cppfuncs.h"
#include "stategameplayer.h"

StateEnteringCode::StateEnteringCode():m_ticks(0),m_blinkticks(0),m_codepos(0),m_letterpos(0),m_presetpos(0),m_loc(LOC_CODE),m_changestate(Game::STATE_NONE)
{
    for(int i=0; i<countof(m_code); i++)
    {
        m_code[i]='\0';
    }

    m_preset[0].FromCode("AQHwsqS3+fvLyc3L");
    m_preset[1].FromCode("AQPx49jJh83sAAAA");
    m_preset[2].FromCode("AQDwq/wAAAAAAAAA");
    m_preset[3].FromCode("AQX26OnHtaSVI7vs");

}

StateEnteringCode::~StateEnteringCode()
{

}

uint8_t StateEnteringCode::State() const
{
    return Game::STATE_ENTERINGCODE;
}

void StateEnteringCode::StateChanged(const uint8_t prevstate, void *params)
{
    m_loc=LOC_CODE;
    m_codepos=0;
    m_letterpos=0;
    m_presetpos=0;
    m_changestate=Game::STATE_NONE;
}

bool StateEnteringCode::HandleInput(const Input *input, const uint8_t playerindex)
{
    if(input->GamepadButtonPress(playerindex+1,BUTTON_LEFT))
    {
        if(m_loc==LOC_CODE)
        {
            if(m_letterpos%m_lettercols==0)
            {
                m_letterpos+=m_lettercols-1;
            }
            else
            {
                m_letterpos--;
            }
        }
        else if(m_loc==LOC_PRESET)
        {
            if(m_presetpos==0)
            {
                m_presetpos=countof(m_preset)-1;
            }
            else
            {
                m_presetpos--;
            }
        }
    }
    if(input->GamepadButtonPress(playerindex+1,BUTTON_UP))
    {
        if(m_loc==LOC_CODE)
        {
            if(m_letterpos<m_lettercols)
            {
                //m_letterpos+=(m_lettercols*(m_letterrows-1));
                m_loc=LOC_PRESET;
            }
            else
            {
                m_letterpos-=m_lettercols;
            }
        }
        else if(m_loc==LOC_PRESET)
        {
            m_loc=LOC_CODE;
            m_blinkticks=0;
        }
    }
    if(input->GamepadButtonPress(playerindex+1,BUTTON_RIGHT))
    {
        if(m_loc==LOC_CODE)
        {
            m_letterpos++;
            if(m_letterpos%m_lettercols==0)
            {
                m_letterpos-=m_lettercols;
            }
        }
        else if(m_loc==LOC_PRESET)
        {
            m_presetpos++;
            if(m_presetpos==countof(m_preset))
            {
                m_presetpos=0;
            }
        }
    }
    if(input->GamepadButtonPress(playerindex+1,BUTTON_DOWN))
    {
        if(m_loc==LOC_CODE)
        {
            m_letterpos+=m_lettercols;
            if(m_letterpos>=(m_lettercols*m_letterrows))
            {
                m_loc=LOC_PRESET;
                m_letterpos-=m_lettercols;
            }
            /*
            if(m_letterpos>=(m_letterrows*m_lettercols))
            {
                m_letterpos=m_letterpos-(m_letterrows*m_lettercols);
            }
            */
        }
        else if(m_loc==LOC_PRESET)
        {
            m_loc=LOC_CODE;
            m_blinkticks=0;
        }
    }

    if(input->GamepadButtonPress(playerindex+1,BUTTON_1))
    {
        if(m_loc==LOC_CODE)
        {
            if(m_codepos<countof(m_code))
            {
                if(m_letterpos>=0 && m_letterpos<64)
                {
                    m_code[m_codepos++]=base64_encoding_char(m_letterpos);
                    m_blinkticks=0;
                }
                if(m_codepos==countof(m_code))
                {
                    OutputStringStream err;
                    if(m_starfighter.ValidateCode(m_code,err)==true)
                    {
                        m_starfighter.FromCode(m_code);
                        m_loc=LOC_VALID;
                    }
                    else
                    {
                        m_loc=LOC_INVALID;
                    }
                }
            }
        }
        else if(m_loc==LOC_PRESET)
        {
            m_preset[m_presetpos].ToCode(m_code);
            m_starfighter.FromCode(m_code);
            m_codepos=countof(m_code);
            m_loc=LOC_VALID;
        }
        else if(m_loc==LOC_VALID)
        {
            m_changestate=Game::STATE_PLAYING;
        }
    }

    if(input->GamepadButtonPress(playerindex+1,BUTTON_2))
    {
        if(m_loc==LOC_CODE)
        {
            if(m_codepos>0)
            {
                m_codepos--;
                m_blinkticks=0;
            }
        }
        else if(m_loc==LOC_INVALID || m_loc==LOC_VALID)
        {
            m_codepos=sizeof(m_code)-1;
            m_loc=LOC_CODE;
            m_blinkticks=0;
        }
    }

    return false;
}

void StateEnteringCode::Update(const int ticks, const uint8_t playerindex, Game *game)
{
    m_ticks=game->GetTicks();
    m_blinkticks+=ticks;
    if(m_changestate==Game::STATE_PLAYING)
    {
        struct StateGamePlayer::GameParams *gp=new StateGamePlayer::GameParams();
        for(int i=0; i<countof(m_code); i++)
        {
            gp->m_game=game;
            gp->m_code[i]=m_code[i];
        }
        game->ChangeState(playerindex,m_changestate,gp);
    }
}

void StateEnteringCode::Draw()
{
    TextPrinter tp;
    tp.SetCustomFont(&Font5x7::Instance());

	tp.PrintCentered("Enter Ship Security Code",SCREEN_SIZE/2,5,24,PALETTE_WHITE);

    if(m_codepos>0)
    {
        *DRAW_COLORS=PALETTE_WHITE;
        OutputStringStream ostr;
        for(int i=0; i<m_codepos; i++)
        {
            ostr << (char)m_code[i];
        }
        text(ostr.Buffer(),16,20);
    }
    if(m_loc==LOC_CODE && m_codepos<countof(m_code))
    {
        if((m_blinkticks%120ULL) < 60)
        {
            *DRAW_COLORS=(PALETTE_WHITE) << 4 | PALETTE_WHITE;
            rect(16+(m_codepos*8),20,8,8);
        }
    }
    
    if(m_loc!=LOC_VALID)
    {
        char str[2]={0,0};
        for(int y=0; y<m_letterrows; y++)
        {
            for(int x=0; x<m_lettercols; x++)
            {
                if(((y*m_lettercols)+x) < 64)
                {
                    if(m_loc==LOC_CODE && (y*m_lettercols)+x == m_letterpos)
                    {
                        *DRAW_COLORS=PALETTE_WHITE << 4;
                        rect(15+(x*10),39+(y*10),10,10);
                    }
                    else
                    {
                        *DRAW_COLORS=PALETTE_WHITE;
                    }
                    str[0]=base64_encoding_char((y*m_lettercols)+x);
                    text(str,16+(x*10),(40)+(y*10));
                }
            }
        }
    }

    if(m_loc==LOC_CODE || m_loc==LOC_PRESET)
    {
        tp.PrintCentered("Preset Starfighter",SCREEN_SIZE/2,100,20,PALETTE_WHITE);

        for(int i=0; i<countof(m_preset); i++)
        {
            m_preset[i].Draw((SCREEN_SIZE/8)+(i*(SCREEN_SIZE/4)),SCREEN_SIZE-30,3.0,static_cast<float>(m_ticks)/40.0f);
            if(m_loc==LOC_PRESET && m_presetpos==i)
            {
                *DRAW_COLORS=PALETTE_WHITE << 4;
                rect(i*(SCREEN_SIZE/4),SCREEN_SIZE-50,SCREEN_SIZE/4,SCREEN_SIZE/4);
            }
        }
    }
    else if(m_loc==LOC_VALID)
    {
        tp.PrintCentered("Starfighter Ready!",SCREEN_SIZE/2,40,20,PALETTE_WHITE);
        tp.PrintCentered("Enter Starfighter Arena",SCREEN_SIZE/2,130,23,PALETTE_WHITE);
        m_starfighter.Draw(SCREEN_SIZE/2,90,5.0,static_cast<float>(m_ticks)/40.0f);
    }
    else if(m_loc==LOC_INVALID)
    {
        tp.PrintCentered("Invalid Starfighter",SCREEN_SIZE/2,100,20,PALETTE_WHITE);
    }

}
