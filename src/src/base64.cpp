#include "base64.h"

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                            'w', 'x', 'y', 'z', '0', '1', '2', '3',
                            '4', '5', '6', '7', '8', '9', '+', '/'};

void base64_encode(const uint8_t *in, const uint32_t inlen, uint8_t *out)
{
    for(int i=0, j=0; i<inlen;)
    {
		out[j++]=encoding_table[(in[i] >> 2) & 0x3f];
		out[j++]=encoding_table[((in[i++] << 4) & 0x30) | ((in[i] >> 4) & 0x0f)];
		out[j++]=encoding_table[((in[i++] << 2) & 0x3c) | ((in[i] >> 6) & 0x03)];
		out[j++]=encoding_table[(in[i++] & 0x3f)];
    }
}

uint8_t encoding_pos(char c)
{
    for(int i=0; i<64; i++)
    {
        if(encoding_table[i]==c)
        {
            return i;
        }
    }
    return 0;
}

void base64_decode(const uint8_t *in, const uint32_t inlen, uint8_t *out)
{
    uint8_t pos[4];
    for(int i=0, j=0; i<inlen;)
    {
        pos[0]=encoding_pos(in[i++]);
        pos[1]=encoding_pos(in[i++]);
        pos[2]=encoding_pos(in[i++]);
        pos[3]=encoding_pos(in[i++]);

        out[j++]=((pos[0] << 2) & 0xfc) | ((pos[1] >> 4) & 0x03);
        out[j++]=((pos[1] << 4) & 0xf0) | ((pos[2] >> 2) & 0x0f);
        out[j++]=((pos[2] << 6) & 0xc0) | (pos[3] & 0x3f);
    }
}

char base64_encoding_char(const uint8_t pos)
{
    if(pos>=0 && pos<64)
    {
        return encoding_table[pos];
    }
    return '\0';
}
