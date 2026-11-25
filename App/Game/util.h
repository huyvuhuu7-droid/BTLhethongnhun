#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

void     Rand_Seed(uint32_t seed);
uint32_t Rand_U32(void);
int32_t  Rand_Range(int32_t lo, int32_t hi);          /* gồm cả lo và hi */
float    Util_ClampF(float v, float lo, float hi);
void     Util_Countdown(int16_t *t_ms, uint16_t dt_ms); /* đếm lùi, dừng ở 0 */
void     Util_U32ToStr(uint32_t v, char *buf);          /* buf >= 11 byte */

#endif
