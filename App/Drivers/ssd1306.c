#include "ssd1306.h"
#include "bsp_i2c.h"
#include "app_config.h"

#define CTRL_CMD    0x00
#define CTRL_DATA   0x40

static bool cmds(const uint8_t *c, uint16_t n) { return BspI2c_Write(SSD1306_ADDR7, CTRL_CMD, c, n); }

bool SSD1306_Init(void)
{
    static const uint8_t init_seq[] = {
        0xAE,             /* display off */
        0x20, 0x02,       /* page addressing mode */
        0xB0, 0xC8,       /* page 0, COM scan remapped */
        0x00, 0x10,       /* column 0 */
        0x40,             /* start line 0 */
        0x81, 0xCF,       /* contrast */
        0xA1,             /* segment remap */
        0xA6,             /* normal (không đảo màu) */
        0xA8, 0x3F,       /* multiplex 64 */
        0xA4,             /* theo RAM */
        0xD3, 0x00,       /* offset 0 */
        0xD5, 0x80,       /* clock div */
        0xD9, 0xF1,       /* pre-charge */
        0xDA, 0x12,       /* COM pins */
        0xDB, 0x40,       /* VCOMH */
        0x8D, 0x14,       /* charge pump on */
        0xAF              /* display on */
    };
    return cmds(init_seq, sizeof(init_seq));
}

bool SSD1306_Flush(const uint8_t *fb)
{
    for (uint8_t page = 0; page < SCREEN_H / 8; page++) {
        const uint8_t set_pos[3] = { (uint8_t)(0xB0 | page), 0x00, 0x10 };
        if (!cmds(set_pos, 3)) return false;
        if (!BspI2c_Write(SSD1306_ADDR7, CTRL_DATA, &fb[page * SCREEN_W], SCREEN_W)) return false;
    }
    return true;   /* TODO người 1: chuyển sang DMA để CPU rảnh hơn nếu FPS thấp */
}
