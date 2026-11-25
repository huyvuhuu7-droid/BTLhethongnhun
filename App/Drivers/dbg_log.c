#include "dbg_uart.h"

#if DEBUG_UART

#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_i2c.h"
#include "adxl345.h"
#include "ssd1306.h"

/* ------------------------------------------------------------------ hàng đợi sự kiện (không chặn) */
typedef struct {
    uint32_t    tick;
    const char *s1;
    int32_t     v1;
    const char *s2;
    int32_t     v2;
    uint8_t     nv;                                   /* số giá trị đi kèm: 0, 1 hoặc 2 */
} Evt_t;

#define Q_LEN  24u

static Evt_t            s_q[Q_LEN];
static volatile uint8_t s_head, s_tail;
static volatile uint16_t s_dropped;

void Dbg_Post(const char *s1, int32_t v1, const char *s2, int32_t v2, uint8_t nv)
{
    uint32_t tick = (uint32_t)xTaskGetTickCount();
    taskENTER_CRITICAL();
    uint8_t next = (uint8_t)((s_head + 1u) % Q_LEN);
    if (next == s_tail) {
        s_dropped++;                                  /* đầy: bỏ sự kiện mới, đếm lại để báo cáo */
    } else {
        s_q[s_head].tick = tick;
        s_q[s_head].s1 = s1;
        s_q[s_head].v1 = v1;
        s_q[s_head].s2 = s2;
        s_q[s_head].v2 = v2;
        s_q[s_head].nv = nv;
        s_head = next;
    }
    taskEXIT_CRITICAL();
}

static bool pop(Evt_t *e)
{
    bool ok = false;
    taskENTER_CRITICAL();
    if (s_tail != s_head) {
        *e = s_q[s_tail];
        s_tail = (uint8_t)((s_tail + 1u) % Q_LEN);
        ok = true;
    }
    taskEXIT_CRITICAL();
    return ok;
}

/* ------------------------------------------------------------------ in log (chỉ gọi trong Dbg_Task) */
static void prefix(uint32_t tick)                     /* "[ 12345 ms] " */
{
    Dbg_WriteStr("[");
    Dbg_WriteInt((int32_t)(tick * (uint32_t)portTICK_PERIOD_MS));
    Dbg_WriteStr(" ms] ");
}

static void print_evt(const Evt_t *e)
{
    prefix(e->tick);
    Dbg_WriteStr(e->s1);
    if (e->nv >= 1u) Dbg_WriteInt(e->v1);
    if (e->nv >= 2u) { Dbg_WriteStr(e->s2); Dbg_WriteInt(e->v2); }
    Dbg_NewLine();
}

/* Quét I2C: dùng để biết OLED / ADXL345 có cắm đúng dây và đúng địa chỉ không. */
static void scan_one(uint8_t addr, const char *name, uint8_t code_addr, bool is_adxl, bool *found_at_code_addr)
{
    bool    found = BspI2c_Probe(addr);
    uint8_t id = 0;
    bool    id_ok = false;
    if (found && is_adxl) id_ok = BspI2c_Read(addr, 0x00, &id, 1) && (id == 0xE5u);

    prefix((uint32_t)xTaskGetTickCount());
    Dbg_WriteStr("  ");
    Dbg_WriteHex8(addr);
    Dbg_WriteStr(" ");
    Dbg_WriteStr(name);
    Dbg_WriteStr(found ? " : FOUND" : " : no ACK");
    if (found && is_adxl) {
        Dbg_WriteStr(", DEVID=");
        Dbg_WriteHex8(id);
        Dbg_WriteStr(id_ok ? " (OK)" : " (SAI, can 0xE5)");
    }
    if (addr == code_addr) {
        Dbg_WriteStr("   <- dia chi code dang dung");
        *found_at_code_addr = found && (!is_adxl || id_ok);
    }
    Dbg_NewLine();
}

static void i2c_scan(void)
{
    bool oled_ok = false, adxl_ok = false;

    prefix((uint32_t)xTaskGetTickCount());
    Dbg_WriteStr("I2C scan (SCL=PB6, SDA=PB7):");
    Dbg_NewLine();
    scan_one(0x3C, "OLED SSD1306", SSD1306_ADDR7, false, &oled_ok);
    scan_one(0x3D, "OLED SSD1306", SSD1306_ADDR7, false, &oled_ok);
    scan_one(0x53, "ADXL345     ", ADXL345_ADDR7, true,  &adxl_ok);
    scan_one(0x1D, "ADXL345     ", ADXL345_ADDR7, true,  &adxl_ok);

    if (!oled_ok) {
        prefix((uint32_t)xTaskGetTickCount());
        Dbg_WriteStr("WARN: khong thay OLED o dia chi code dung -> man hinh se trong (kiem tra SCL/SDA/nguon/dia chi)");
        Dbg_NewLine();
    }
    if (!adxl_ok) {
        prefix((uint32_t)xTaskGetTickCount());
        Dbg_WriteStr("WARN: khong thay ADXL345 o dia chi code dung -> khong co nghieng, nut bam van chay");
        Dbg_NewLine();
    }
}

static void print_stat(void)
{
    prefix((uint32_t)xTaskGetTickCount());
    Dbg_WriteStr("STAT heap_free=");
    Dbg_WriteInt((int32_t)xPortGetFreeHeapSize());
    Dbg_WriteStr(" heap_min_free=");
    Dbg_WriteInt((int32_t)xPortGetMinimumEverFreeHeapSize());
    Dbg_WriteStr(" log_dropped=");
    Dbg_WriteInt((int32_t)s_dropped);
    Dbg_NewLine();
}

void Dbg_Task(void *arg)
{
    (void)arg;
    bool        scanned = false;
    uint16_t    dropped_reported = 0;
    TickType_t  next_stat = xTaskGetTickCount() + pdMS_TO_TICKS(DEBUG_UART_STATS_MS);

    for (;;) {
        Evt_t e;
        while (pop(&e)) print_evt(&e);

        if (s_dropped != dropped_reported) {
            dropped_reported = s_dropped;
            prefix((uint32_t)xTaskGetTickCount());
            Dbg_WriteStr("WARN: hang doi log day, da mat ");
            Dbg_WriteInt((int32_t)dropped_reported);
            Dbg_WriteStr(" su kien");
            Dbg_NewLine();
        }

        if (!scanned && xTaskGetTickCount() >= pdMS_TO_TICKS(500)) {   /* đợi các task khác khởi tạo xong */
            scanned = true;
            i2c_scan();
        }

#if DEBUG_UART_STATS_MS
        if ((int32_t)(xTaskGetTickCount() - next_stat) >= 0) {
            next_stat += pdMS_TO_TICKS(DEBUG_UART_STATS_MS);
            print_stat();
        }
#else
        (void)next_stat;
#endif
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif /* DEBUG_UART */
