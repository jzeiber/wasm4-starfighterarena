#pragma once

#include <stdint.h>

#include "dynamicarray.h"
#include "outputstringstream.h"
#include "fpoint2d.h"

class Starfighter
{
public:
    Starfighter();
    ~Starfighter();

    bool ToCode(uint8_t *code);
    bool FromCode(const char *codestr);
    bool FromCode(const uint8_t *code);
    bool ValidateCode(const char *codestr, OutputStringStream &err) const;
    bool ValidateCode(const uint8_t *code, OutputStringStream &err) const;
    bool ValidateUnpacked(uint8_t *data, OutputStringStream &err) const;

    void Draw(float x, float y, float scale, float rotrad);

    void WeaponCoord(const uint8_t weaponindex, const float scale, const float rotrad, fpoint2d &point);

private:

    struct gridpoint
    {
        bool m_used;
        uint8_t m_xidx;
        uint8_t m_yidx;
    };

    gridpoint m_gridpoints[10];
    uint8_t m_weaponpoint;

    DynamicArray<fpoint2d> m_modelvertex;

    bool ExtractPoint(const uint8_t val, bool &used, uint8_t &gridx, uint8_t &gridy) const;
    void Translate(const fpoint2d &inpoint, const float scale, const float rotrad, fpoint2d &outpoint) const;

};
