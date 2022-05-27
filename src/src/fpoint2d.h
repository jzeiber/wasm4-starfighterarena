#pragma once

struct fpoint2d
{
    fpoint2d():m_x(0),m_y(0)                                { }
    fpoint2d(const float x, const float y):m_x(x),m_y(y)    { }
    ~fpoint2d()                                             { }
    float m_x;
    float m_y;
};
