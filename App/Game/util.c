#include "util.h"

static uint32_t s_rng = 0x12345678u;

void Rand_Seed(uint32_t seed) { s_rng = seed ? seed : 0x12345678u; }

uint32_t Rand_U32(void)           /* xorshift32 */
{
    uint32_t x = s_rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return s_rng = x;
}

int32_t Rand_Range(int32_t lo, int32_t hi)
{
    if (hi <= lo) return lo;
    return lo + (int32_t)(Rand_U32() % (uint32_t)(hi - lo + 1));
}

float Util_ClampF(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

void Util_Countdown(int16_t *t, uint16_t dt)
{
    if (*t > 0) {
        if (*t > (int16_t)dt) *t -= (int16_t)dt; else *t = 0;
    }
}

void Util_U32ToStr(uint32_t v, char *buf)
{
    char tmp[11]; int n = 0, i = 0;
    do { tmp[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (n) buf[i++] = tmp[--n];
    buf[i] = '\0';
}
