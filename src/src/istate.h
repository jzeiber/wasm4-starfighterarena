#pragma once

#include <stdint.h>

#include "idrawable.h"
#include "iinputhandler.h"
#include "iupdatable.h"

class IState:public IDrawable, public IInputHandler, public IUpdatable
{
public:
    IState()            { };
    virtual ~IState()   { };

    virtual void StateChanged(const uint8_t prevstate, void *params)=0;
    virtual uint8_t State() const=0;
};
