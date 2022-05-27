#include "starfighter.h"
#include "base64.h"
#include "cppfuncs.h"
#include "palette.h"
#include "wasmmath.h"
#include "wasm4.h"

Starfighter::Starfighter():m_weaponpoint(0)
{

}

Starfighter::~Starfighter()
{

}

bool Starfighter::ToCode(uint8_t *code)
{
    if(!code)
    {
        return false;
    }

    uint8_t buff[countof(m_gridpoints)+2];

    buff[0]=0x1;
    buff[1]=m_weaponpoint & 0xf;
    for(int i=0; i<countof(m_gridpoints); i++)
    {
        if(m_gridpoints[i].m_used==true)
        {
            buff[i+2]=0b10000000;
            buff[i+2]|=(m_gridpoints[i].m_xidx << 4) & 0x70;
            buff[i+2]|=(m_gridpoints[i].m_yidx) & 0xf;
        }
        else
        {
            buff[i+2]=0;
        }
    }

    base64_encode(buff,countof(m_gridpoints)+2,code);

    return true;
}

bool Starfighter::FromCode(const char *codestr)
{
    return FromCode((const uint8_t *)codestr);
}

bool Starfighter::FromCode(const uint8_t *code)
{
    OutputStringStream err;
    uint8_t buff[12];
    base64_decode(code,16,buff);
    if(ValidateUnpacked(buff,err)==true)
    {
        m_weaponpoint=(buff[1] & 0xf);
        for(int i=0; i<countof(m_gridpoints); i++)
        {
            ExtractPoint(buff[i+2],m_gridpoints[i].m_used,m_gridpoints[i].m_xidx,m_gridpoints[i].m_yidx);
        }
        m_modelvertex.clear();
        for(int i=0; i<countof(m_gridpoints); i++)
        {
            if(m_gridpoints[i].m_used==true)
            {
                m_modelvertex.push_back(fpoint2d(static_cast<float>(m_gridpoints[i].m_xidx)-7.0,static_cast<float>(m_gridpoints[i].m_yidx)-7.0));
            }
        }
        for(int i=countof(m_gridpoints)-1; i>=0; i--)
        {
            if(m_gridpoints[i].m_used==true)
            {
                m_modelvertex.push_back(fpoint2d(7.0-static_cast<float>(m_gridpoints[i].m_xidx),static_cast<float>(m_gridpoints[i].m_yidx)-7.0));
            }            
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool Starfighter::ValidateCode(const char *codestr, OutputStringStream &err) const
{
    return ValidateCode((const uint8_t *)codestr,err);
}

bool Starfighter::ValidateCode(const uint8_t *code, OutputStringStream &err) const
{
    uint8_t buff[12];
    base64_decode(code,16,buff);
    return ValidateUnpacked(buff,err);
}

bool Starfighter::ValidateUnpacked(uint8_t *data, OutputStringStream &err) const
{
    static constexpr uint8_t xpoints[]={1,4,5,6,7,7,7,8,7,7,7,6,5,4,1};
    gridpoint gp[countof(m_gridpoints)];
    gridpoint uniquegp[3];
    for(int i=0; i<countof(uniquegp); i++)
    {
        uniquegp[i].m_used=false;
    }
    uint8_t uniquepos=0;
    if((data[0] & 0xf)==0x1)
    {
        for(int i=0; i<countof(m_gridpoints); i++)
        {
            bool used=false;
            uint8_t gridx=0;
            uint8_t gridy=0;
            ExtractPoint(data[i+2u],used,gridx,gridy);

            if(used==true && (gridy<0u || gridy>15u || gridx<0u || ((8u-xpoints[gridy]) > gridx)))
            {
                err << "Invalid vertex position";
                return false;
            }
            
            gp[i].m_used=used;
            gp[i].m_xidx=gridx;
            gp[i].m_yidx=gridy;
            if(gp[i].m_used==true)
            {
                bool foundunique=true;
                for(int j=0; j<countof(uniquegp) && foundunique==true; j++)
                {
                    if(uniquegp[j].m_used==true && uniquegp[j].m_xidx==gridx && uniquegp[j].m_yidx==gridy)
                    {
                        foundunique=false;
                    }
                }
                if(foundunique==true && uniquepos<countof(uniquegp))
                {
                    uniquegp[uniquepos++]=gp[i];
                }
            }
        }
        if(uniquepos<countof(uniquegp))
        {
            err << "Not enough unique vertices";
            return false;
        }
        uint8_t weaponpoint=(data[1] & 0xf);
        if(weaponpoint<0u || weaponpoint>=static_cast<uint8_t>(countof(m_gridpoints)) || gp[weaponpoint].m_used==false)
        {
            err << "No weapon vertex";
            return false;
        }
        return true;
    }
    else
    {
        err << "Invalid security code";
        return false;
    }
}

void Starfighter::Draw(float x, float y, float scale, float rotrad)
{
    fpoint2d p1;
    fpoint2d p2;

    float rad=rotrad+M_PI_2;    // add 1/2 pi to rotate 90 degrees

    *DRAW_COLORS=PALETTE_WHITE;

    if(m_modelvertex.size()>0)
    {
        Translate(m_modelvertex[0],scale,rad,p1);
    }

    for(int i=1; i<m_modelvertex.size(); i++)
    {
        Translate(m_modelvertex[i],scale,rad,p2);
        if(((x+p1.m_x)>=0 && (y+p1.m_y)>=0 && (x+p1.m_x)<SCREEN_SIZE && (y+p1.m_y)<SCREEN_SIZE) || ((x+p2.m_x)>=0 && (y+p2.m_y)>=0 && (x+p2.m_x)<SCREEN_SIZE && (y+p2.m_y)<SCREEN_SIZE))
        {
            line(x+p1.m_x,y+p1.m_y,x+p2.m_x,y+p2.m_y);
        }
        p1=p2;
    }

    if(m_modelvertex.size()>0)
    {
        Translate(m_modelvertex[0],scale,rad,p2);
        if(((x+p1.m_x)>=0 && (y+p1.m_y)>=0 && (x+p1.m_x)<SCREEN_SIZE && (y+p1.m_y)<SCREEN_SIZE) || ((x+p2.m_x)>=0 && (y+p2.m_y)>=0 && (x+p2.m_x)<SCREEN_SIZE && (y+p2.m_y)<SCREEN_SIZE))
        {
            line(x+p1.m_x,y+p1.m_y,x+p2.m_x,y+p2.m_y);
        }
    }
}

bool Starfighter::ExtractPoint(const uint8_t val, bool &used, uint8_t &gridx, uint8_t &gridy) const
{
    if(((val >> 7) & 0x1) == 0x1)
    {
        used=true;
        gridx=((val >> 4) & 0x7);
        gridy=(val & 0xf);
        return true;
    }
    else
    {
        used=false;
        return false;
    }
}

void Starfighter::Translate(const fpoint2d &inpoint, const float scale, const float rotrad, fpoint2d &outpoint) const
{
    float rad=rotrad;
    while(rad<0)
    {
        rad+=(2.0*M_PI);
    }
    while(rad>(2.0*M_PI))
    {
        rad-=(2.0*M_PI);
    }

    float sr=_sin(rad);
    float cr=_cos(rad);
    float xcos=static_cast<float>(inpoint.m_x)*cr;
    float xsin=static_cast<float>(inpoint.m_x)*sr;
    float ycos=static_cast<float>(inpoint.m_y)*cr;
    float ysin=static_cast<float>(inpoint.m_y)*sr;

    outpoint.m_x=scale*(xcos-ysin);
    outpoint.m_y=scale*(xsin+ycos);
}

void Starfighter::WeaponCoord(const uint8_t weaponindex, const float scale, const float rotrad, fpoint2d &point)
{
    if(weaponindex>=0u && weaponindex<2u && m_weaponpoint<countof(m_gridpoints))
    {
        fpoint2d inpoint;
        inpoint.m_x=static_cast<float>(m_gridpoints[m_weaponpoint].m_xidx);
        inpoint.m_y=static_cast<float>(m_gridpoints[m_weaponpoint].m_yidx)-7.0;
        if(weaponindex==1u)
        {
            inpoint.m_x=7.0-inpoint.m_x;
        }
        else
        {
            inpoint.m_x=inpoint.m_x-7.0;
        }

        Translate(inpoint,1.0,rotrad+M_PI_2,point);

    }
}
