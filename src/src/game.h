#pragma once

#include <stdint.h>

#include "global.h"
#include "istate.h"
#include "iupdatable.h"
#include "idrawable.h"
#include "iinputhandler.h"
#include "statewaitfornet.h"
#include "fpoint2d.h"
#include "dynamicarray.h"

class Game:public IUpdatable,public IDrawable,public IInputHandler
{
public:
	Game();
	~Game();

	enum PlayerState
	{
		STATE_NONE=0,
		STATE_WAITINGFORNET=1,
		STATE_ENTERINGCODE=2,
		STATE_PLAYING=3,
		STATE_MAX
	};

	struct projectile
	{
		uint8_t m_owner;
		fpoint2d m_pos;
		fpoint2d m_lastpos;
		float m_dx;
		float m_dy;
		uint32_t m_ticks;
		float m_yield;
	};
	
	bool HandleInput(const Input *input, const uint8_t nothing);
	void Update(const int ticks, const uint8_t nothing, Game *game=nullptr);
	void Draw();

	void ChangeState(const uint8_t playerindex, const uint8_t newstate, void *params);

	uint64_t GetTicks() const;

	int8_t PlayerIndex() const;

	IState *GetPlayerState(const uint8_t playerindex);

	uint8_t PlayerCount() const;

	void AddProjectile(const uint8_t playerindex, const fpoint2d &point, const float vel, const float rotrad);
	void DrawProjectiles(const float cx, const float cy);		// draws projectiles offset by position cx,cy at center of screen
	void DrawLeaderboard(const int32_t x, const int32_t y, const int32_t width, const bool border);
	void DrawStarfield(const float x, const float y);

private:

	struct changestate
	{
		int8_t m_newstate;
		void *m_params;
	};

	uint64_t m_ticks;

	IState *m_playerstate[4];
	changestate m_changestate[4];
	fpoint2d m_stars[100];

	DynamicArray<projectile> m_projectiles;

	bool CheckCollision(const uint8_t playerindex, const uint32_t projectileindex) const;
	double WrapPositive(const double val, const double max) const;

	void HandleChangeState();

};
