#include "gfx.h"
#include <string.h>

static uint8_t s_fb[SCREEN_W * SCREEN_H / 8];

uint8_t *Gfx_Buffer(void) { return s_fb; }
void Gfx_Clear(void)      { memset(s_fb, 0, sizeof(s_fb)); }

void Gfx_Pixel(int x, int y, uint8_t color)
{
    if ((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return;
    uint8_t *p = &s_fb[x + (y >> 3) * SCREEN_W];
    if (color) *p |=  (uint8_t)(1u << (y & 7));
    else       *p &= (uint8_t)~(1u << (y & 7));
}

void Gfx_FillRect(int x, int y, int w, int h, uint8_t color)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            Gfx_Pixel(x + i, y + j, color);
}

void Gfx_HLine(int x, int y, int w, uint8_t color) { Gfx_FillRect(x, y, w, 1, color); }

void Gfx_DrawRect(int x, int y, int w, int h, uint8_t color)
{
    if (w <= 0 || h <= 0) return;
    Gfx_FillRect(x, y, w, 1, color);
    Gfx_FillRect(x, y + h - 1, w, 1, color);
    Gfx_FillRect(x, y, 1, h, color);
    Gfx_FillRect(x + w - 1, y, 1, h, color);
}

void Gfx_Bitmap(int x, int y, const uint8_t *bmp, int w, int h, uint8_t color)
{
    int bpr = (w + 7) / 8;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            if (bmp[j * bpr + (i >> 3)] & (0x80u >> (i & 7)))
                Gfx_Pixel(x + i, y + j, color);
}

/* ---------------------------------------------------------------- font 3x5
 * Mỗi glyph 5 byte (5 hàng), 3 bit thấp = 3 cột (bit2 = cột trái).
 * Thứ tự: '0'-'9', 'A'-'Z', ' ', ':', '-'   (TODO người 3: thêm ký tự nếu cần) */
static const uint8_t k_font[39][5] = {
    {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
    {7,4,7,1,7},{7,4,7,5,7},{7,1,1,2,2},{7,5,7,5,7},{7,5,7,1,7},   /* 0-9 */
    {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},{7,4,7,4,7},   /* A-E */
    {7,4,7,4,4},{3,4,5,5,3},{5,5,7,5,5},{7,2,2,2,7},{1,1,1,5,2},   /* F-J */
    {5,5,6,5,5},{4,4,4,4,7},{5,7,7,5,5},{6,5,5,5,5},{2,5,5,5,2},   /* K-O */
    {6,5,6,4,4},{2,5,5,6,3},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2},   /* P-T */
    {5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},{5,5,2,2,2},   /* U-Y */
    {7,1,2,4,7},                                                   /* Z */
    {0,0,0,0,0},{0,2,0,2,0},{0,0,7,0,0}                            /* ' ' ':' '-' */
};

static const uint8_t *glyph(char c)
{
    if (c >= '0' && c <= '9') return k_font[c - '0'];
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return k_font[10 + (c - 'A')];
    if (c == ':') return k_font[37];
    if (c == '-') return k_font[38];
    return k_font[36];
}

int Gfx_TextWidth(const char *s, int scale)
{
    int n = (int)strlen(s);
    return n ? n * 4 * scale - scale : 0;
}

void Gfx_Text(int x, int y, const char *s, int scale, uint8_t color)
{
    for (; *s; s++, x += 4 * scale) {
        const uint8_t *gl = glyph(*s);
        for (int row = 0; row < 5; row++)
            for (int col = 0; col < 3; col++)
                if (gl[row] & (4u >> col))
                    Gfx_FillRect(x + col * scale, y + row * scale, scale, scale, color);
    }
}

void Gfx_TextCentered(int y, const char *s, int scale, uint8_t color)
{
    Gfx_Text((SCREEN_W - Gfx_TextWidth(s, scale)) / 2, y, s, scale, color);
}
