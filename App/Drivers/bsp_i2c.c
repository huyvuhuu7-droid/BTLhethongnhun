#include "bsp_i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define I2C_TIMEOUT_MS   50
#define I2C_LOCK_MS      100

static I2C_HandleTypeDef *s_hi2c;
static SemaphoreHandle_t  s_mutex;

void BspI2c_Init(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;
    if (!s_mutex) s_mutex = xSemaphoreCreateMutex();
}

bool BspI2c_Write(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len)
{
    bool ok = false;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(I2C_LOCK_MS)) == pdTRUE) {
        ok = HAL_I2C_Mem_Write(s_hi2c, (uint16_t)(addr7 << 1), reg, I2C_MEMADD_SIZE_8BIT,
                               (uint8_t *)data, len, I2C_TIMEOUT_MS) == HAL_OK;
        xSemaphoreGive(s_mutex);
    }
    return ok;   /* TODO người 1: nếu lỗi nhiều lần liên tiếp -> reset bus I2C */
}

bool BspI2c_Probe(uint8_t addr7)
{
    bool ok = false;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(I2C_LOCK_MS)) == pdTRUE) {
        ok = HAL_I2C_IsDeviceReady(s_hi2c, (uint16_t)(addr7 << 1), 1, I2C_TIMEOUT_MS) == HAL_OK;
        xSemaphoreGive(s_mutex);
    }
    return ok;
}

bool BspI2c_Read(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len)
{
    bool ok = false;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(I2C_LOCK_MS)) == pdTRUE) {
        ok = HAL_I2C_Mem_Read(s_hi2c, (uint16_t)(addr7 << 1), reg, I2C_MEMADD_SIZE_8BIT,
                              data, len, I2C_TIMEOUT_MS) == HAL_OK;
        xSemaphoreGive(s_mutex);
    }
    return ok;
}
