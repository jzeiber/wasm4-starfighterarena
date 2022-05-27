#pragma once

#include <stdint.h>

void base64_encode(const uint8_t *in, const uint32_t inlen, uint8_t *out);
void base64_decode(const uint8_t *in, const uint32_t inlen, uint8_t *out);
char base64_encoding_char(const uint8_t pos);