#include "game.h"
#include "wasm4.h"
#include "global.h"
#include "palette.h"
#include "textprinter.h"
#include "outputstringstream.h"
#include "font5x7.h"
#include "randommt.h"
#include "cppfuncs.h"
#include "wasmmath.h"
#include "base64.h"
#include "netfuncs.h"
#include "statewaitfornet.h"
#include "stateenteringcode.h"
#include "stategameplayer.h"

Game::Game():m_ticks(0)
{
	for(int i=0; i<countof(m_playerstate); i++)
	{
		//m_playerstate[i]=&StateWaitForNet::Instance();
		m_playerstate[i]=new StateWaitForNet();
	}
	RandomMT r(0);
	for(int i=0; i<countof(m_stars); i++)
	{
		m_stars[i].m_x=r.NextDouble()*static_cast<float>(SCREEN_SIZE*2);
		m_stars[i].m_y=r.NextDouble()*static_cast<float>(SCREEN_SIZE*2);
	}
}

Game::~Game()
{

}

void Game::ChangeState(const uint8_t playerindex, const uint8_t newstate, void *params)
{
	if(newstate>=0 && newstate<STATE_MAX && newstate!=m_playerstate[playerindex]->State())
	{
		const uint8_t oldstate=m_playerstate[playerindex]->State();
		delete m_playerstate[playerindex];
		m_playerstate[playerindex]=nullptr;

		switch(newstate)
		{
		case STATE_WAITINGFORNET:
			//m_playerstate[playerindex]=&StateWaitForNet::Instance();
			m_playerstate[playerindex]=new StateWaitForNet();
			m_playerstate[playerindex]->StateChanged(oldstate,params);
			break;
		case STATE_ENTERINGCODE:
			m_playerstate[playerindex]=new StateEnteringCode();
			m_playerstate[playerindex]->StateChanged(oldstate,params);
			break;
		case STATE_PLAYING:
			m_playerstate[playerindex]=new StateGamePlayer();
			m_playerstate[playerindex]->StateChanged(oldstate,params);
			break;
		default:
			trace("Game::ChangeState State not impelemented!");
		}
	}
}

void Game::Update(const int ticks, const uint8_t nothing, Game *game)
{
	m_ticks+=ticks;

	// remove old projectiles
	for(int i=m_projectiles.size()-1; i>=0; i--)
	{
		if(m_projectiles[i].m_ticks<=ticks)
		{
			m_projectiles.erase(i);
		}
		else
		{
			m_projectiles[i].m_ticks-=ticks;
		}
	}

	// move projectiles
	for(int i=0; i<m_projectiles.size(); i++)
	{
		m_projectiles[i].m_lastpos=m_projectiles[i].m_pos;
		m_projectiles[i].m_pos.m_x+=(static_cast<float>(ticks)*m_projectiles[i].m_dx);
		m_projectiles[i].m_pos.m_y+=(static_cast<float>(ticks)*m_projectiles[i].m_dy);
	}

	// check projectile collision

	for(int p=0; p<countof(m_playerstate); p++)
	{	
		for(int i=m_projectiles.size()-1; i>=0; i--)
		{
			if(CheckCollision(p,i)==true)
			{
				bool killed=false;
				// TODO - register hit on ship and hit count on owner
				if(m_playerstate[p]->State()==STATE_PLAYING && ((StateGamePlayer *)m_playerstate[p])->Status()==StateGamePlayer::STATUS_ALIVE)
				{
					((StateGamePlayer *)m_playerstate[p])->RegisterHitDestination(m_projectiles[i],killed);
				}
				// we can still register hit for owner if they're dead
				if(m_projectiles[i].m_owner>=0u && m_projectiles[i].m_owner<countof(m_playerstate) && m_playerstate[m_projectiles[i].m_owner]->State()==STATE_PLAYING)
				{
					((StateGamePlayer *)m_playerstate[m_projectiles[i].m_owner])->RegisterHitSource(m_projectiles[i],killed);
				}

				m_projectiles.erase(i);
			}
		}
	}

	for(int i=0; i<countof(m_playerstate); i++)
	{
		m_playerstate[i]->Update(ticks,i,game);
	}

}

uint64_t Game::GetTicks() const
{
	return m_ticks;
}

void Game::Draw()
{
	m_playerstate[PlayerIndex()]->Draw();
}

bool Game::HandleInput(const Input *input, const uint8_t nothing)
{
	bool handled=false;

	for(int i=0; i<countof(m_playerstate); i++)
	{
		m_playerstate[i]->HandleInput(input,i);
	}

	return handled;
}

int8_t Game::PlayerIndex() const
{
	if(netplay_active()==true)
	{
		return netplay_playeridx();
	}
	return 0;
}

IState *Game::GetPlayerState(const uint8_t playerindex)
{
	if(playerindex>=0 && playerindex<countof(m_playerstate))
	{
		return m_playerstate[playerindex];
	}
	return 0;
}

uint8_t Game::PlayerCount() const
{
	return countof(m_playerstate);
}

void Game::AddProjectile(const uint8_t playerindex, const fpoint2d &point, const float vel, const float rotrad)
{
	float rot=rotrad;

	float dx=_cos(rot)*vel;
	float dy=_sin(rot)*vel;

	projectile p;
	p.m_pos=point;
	p.m_lastpos=point;
	p.m_owner=playerindex;
	p.m_ticks=120;
	p.m_dx=dx;
	p.m_dy=dy;
	p.m_yield=75.0;
	
	m_projectiles.push_back(p);
}

void Game::DrawProjectiles(const float cx, const float cy)
{
	*DRAW_COLORS=PALETTE_WHITE;
	for(int i=0; i<m_projectiles.size(); i++)
	{
		float dx=m_projectiles[i].m_pos.m_x-cx;
		float dy=m_projectiles[i].m_pos.m_y-cy;
		float x=(SCREEN_SIZE/2)+dx;
		float y=(SCREEN_SIZE/2)+dy;
		if(x>=0 && y>=0 && x<SCREEN_SIZE && y<SCREEN_SIZE)
		{
			line(x,y,x,y);
		}
	}
}

bool Game::CheckCollision(const uint8_t playerindex, const uint32_t projectileindex) const
{
	if(playerindex<0u || playerindex>=countof(m_playerstate) || m_playerstate[playerindex]->State()!=STATE_PLAYING)
	{
		return false;
	}

	if(projectileindex<0u || projectileindex>=m_projectiles.size() || m_projectiles[projectileindex].m_owner==playerindex)
	{
		return false;
	}

	StateGamePlayer *pl=(StateGamePlayer *)m_playerstate[playerindex];

	if(pl->Status()!=StateGamePlayer::STATUS_ALIVE)
	{
		return false;
	}

	float dx=m_projectiles[projectileindex].m_pos.m_x-pl->PlayerX();
	float dy=m_projectiles[projectileindex].m_pos.m_y-pl->PlayerY();

	if((dx*dx)+(dy*dy) <= (pl->ShieldRadius()*pl->ShieldRadius()))
	{
		return true;
	}

	// TODO - check line collision with previous point and this point against circle in case shot grazes by circle

	return false;

}

void Game::DrawLeaderboard(const int32_t x, const int32_t y, const int32_t width, const bool border)
{
	if(border)
	{
		*DRAW_COLORS=PALETTE_WHITE << 4 | PALETTE_BLACK;
	}
	else
	{
		*DRAW_COLORS=PALETTE_BLACK;
	}
	rect(x,y,width,54);

	int draworder[countof(m_playerstate)];
	for(int i=0; i<countof(m_playerstate); i++)
	{
		if(m_playerstate[i]->State()==STATE_PLAYING)
		{
			draworder[i]=i;
		}
		else
		{
			draworder[i]=-1;
		}
	}

	for(int i=1; i<countof(m_playerstate); i++)
	{
		for(int j=0; j<countof(m_playerstate)-i; j++)
		{
			if(draworder[j]<0 && draworder[j+1]>=0)
			{
				draworder[j]=draworder[j+1];
				draworder[j+1]=-1;
			}
			else if(draworder[j]>=0 && draworder[j+1]>=0 && static_cast<int64_t>(((StateGamePlayer *)m_playerstate[draworder[j]])->GetStats().m_kills) < static_cast<int64_t>(((StateGamePlayer *)m_playerstate[draworder[j+1]])->GetStats().m_kills))
			{
				int temp=draworder[j+1];
				draworder[j+1]=draworder[j];
				draworder[j]=temp;
			}
		}
	}

	TextPrinter tp;
	tp.SetCustomFont(&Font5x7::Instance());
	tp.Print("Player",x+2,y+2,6,PALETTE_WHITE);
	tp.Print("Kills",x+width-80,y+2,5,PALETTE_WHITE);
	tp.Print("Deaths",x+width-40,y+2,6,PALETTE_WHITE);
	for(int i=0; i<countof(m_playerstate); i++)
	{
		if(draworder[i]!=-1)
		{
			OutputStringStream ostr;
			if(draworder[i]!=PlayerIndex())
			{
				ostr << "Player " << (draworder[i]+1);
			}
			else
			{
				ostr << "You";
			}
			tp.Print(ostr.Buffer(),x+2,y+12+(10*i),ostr.TextLength(),PALETTE_WHITE);
			ostr.Clear();
			ostr << ((StateGamePlayer *)m_playerstate[draworder[i]])->GetStats().m_kills;
			tp.PrintWrapped(ostr.Buffer(),x+width-110,y+12+(i*10),ostr.TextLength(),45,PALETTE_WHITE,TextPrinter::JUSTIFY_RIGHT);
			//tp.Print(ostr.Buffer(),x+width-80,y+12+(i*10),ostr.TextLength(),PALETTE_WHITE);
			ostr.Clear();
			ostr << ((StateGamePlayer *)m_playerstate[draworder[i]])->GetStats().m_deaths;
			tp.PrintWrapped(ostr.Buffer(),x+width-70,y+12+(i*10),ostr.TextLength(),60,PALETTE_WHITE,TextPrinter::JUSTIFY_RIGHT);
			//tp.Print(ostr.Buffer(),x+width-40,y+12+(i*10),ostr.TextLength(),PALETTE_WHITE);
		}
	}

}

void Game::DrawStarfield(const float x, const float y)
{
	const double maxval=SCREEN_SIZE*2;
	*DRAW_COLORS=PALETTE_DARKGREY;

	float dx1=WrapPositive(_fmod(x*.8,maxval),maxval);
	float dy1=WrapPositive(_fmod(y*.8,maxval),maxval);
	float dx2=WrapPositive(_fmod(x*.5,maxval),maxval);
	float dy2=WrapPositive(_fmod(y*.5,maxval),maxval);

	for(int i=0; i<50; i++)
	{
		fpoint2d p1(WrapPositive(m_stars[i].m_x-dx1,maxval),WrapPositive(m_stars[i].m_y-dy1,maxval));

		if(p1.m_x>=0 && p1.m_x<SCREEN_SIZE && p1.m_y>=0 && p1.m_y<SCREEN_SIZE)
		{
			line(p1.m_x,p1.m_y,p1.m_x,p1.m_y);
		}
	}

	for(int i=50; i<countof(m_stars); i++)
	{
		fpoint2d p1(WrapPositive(m_stars[i].m_x-dx2,maxval),WrapPositive(m_stars[i].m_y-dy2,maxval));

		if(p1.m_x>=0 && p1.m_x<SCREEN_SIZE && p1.m_y>=0 && p1.m_y<SCREEN_SIZE)
		{
			line(p1.m_x,p1.m_y,p1.m_x,p1.m_y);
		}
	}

}

double Game::WrapPositive(const double val, const double max) const
{
	double n=val;
	while(n<0 && max>0)
	{
		n+=max;
	}
	while(n>max && max>0)
	{
		n-=max;
	}
	return n;
}
