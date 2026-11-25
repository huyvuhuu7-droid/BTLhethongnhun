/* app_config.h - TOÀN BỘ hằng số chỉnh được của game nằm ở đây.
 * Muốn cân bằng game (nhanh/chậm, khó/dễ) -> sửa file này, không sửa trong code. */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* ===== Màn hình ===== */
#define SCREEN_W            128
#define SCREEN_H            64
#define HUD_H               8                 /* dải HUD trên cùng: máu, mana, điểm */
#define PLAYFIELD_TOP       HUD_H

/* ===== Chu kỳ các task (ms) =====
 * Chip F103C8T6 clone chạy HCLK thấp (khuyên dùng 24 MHz, xem docs/CUBEMX_SETUP.md mục 2.3).
 * Các chu kỳ dưới đây tính theo FreeRTOS tick (1 ms) nên KHÔNG đổi khi đổi clock.
 * Chỉ cần nới ra nếu chạy 8 MHz (GAME_TICK_MS 40) hoặc I2C 100 kHz (RENDER_TICK_MS 100). */
#define INPUT_TICK_MS       10                /* đọc ADXL345 + nút */
#define GAME_TICK_MS        30                /* cập nhật logic (dt cố định) */
#define RENDER_TICK_MS      40                /* ~25 FPS, giới hạn bởi tốc độ I2C (400 kHz ≈ 25 ms/khung) */

/* ===== Nút bấm ===== */
#define BTN_DEBOUNCE_SAMPLES  3                   /* số mẫu LIÊN TIẾP (mỗi mẫu INPUT_TICK_MS) để chấp nhận đổi trạng thái -> chống dội ~30 ms */

/* ===== DEBUG qua UART (USART1, chỉ TX = PA9, 115200 8N1) =====
 * 1 = bật: in log ra cổng nối tiếp (cách nối dây + mẫu log: docs/UART_DEBUG.md).
 * 0 = tắt hoàn toàn: không tốn Flash/RAM/CPU, không tạo task debug. */
#define DEBUG_UART            1
#define DEBUG_UART_BAUD       115200u             /* chỉ dùng cho driver thanh ghi dự phòng; khi dùng UART của CubeMX thì baud do CubeMX đặt */
#define DEBUG_UART_STATS_MS   10000u              /* chu kỳ in dòng STAT (heap...). 0 = không in */

/* ===== Cảm biến nghiêng (ADXL345) ===== */
#define TILT_FULL_SCALE_G   0.50f             /* nghiêng ~30 độ = full tốc độ */
#define TILT_DEADZONE       0.08f             /* bỏ qua rung nhẹ quanh vị trí cân bằng */
#define TILT_FILTER_ALPHA   0.30f             /* lọc thông thấp, 0..1 (nhỏ = mượt hơn) */
#define TILT_X_SIGN         (+1.0f)           /* đảo thành -1.0f nếu trái/phải bị ngược */
#define TILT_Y_SIGN         (+1.0f)           /* +: nghiêng về phía trước -> đi lên */

/* ===== Phi thuyền ===== */
#define PLAYER_W            8
#define PLAYER_H            8
#define PLAYER_MAX_HP       5
#define PLAYER_ACCEL        220.0f            /* px/s^2 ứng với tilt = 1 */
#define PLAYER_DRAG         3.0f              /* 1/s - quán tính: càng nhỏ càng "trôi" */
#define PLAYER_MAX_SPEED    60.0f             /* px/s */
#define PLAYER_Y_MIN        (PLAYFIELD_TOP + 20)
#define INVINCIBLE_MS       1000              /* bất tử sau khi bị trúng */

/* ===== Đạn thường & Skill ===== */
#define FIRE_COOLDOWN_MS    180
#define BULLET_SPEED        110.0f            /* px/s */
#define MANA_MAX            100
#define MANA_PER_KILL       20                /* mỗi kẻ địch bị hạ */
#define SKILL_DURATION_MS   1200
#define SKILL_WIDTH         6                 /* bề rộng tia năng lượng (px) */
#define SKILL_HIT_INTERVAL_MS 100
#define SKILL_DAMAGE        3                 /* sát thương mỗi nhịp */

/* ===== Số lượng tối đa mỗi loại vật thể (pool tĩnh, không malloc) ===== */
#define MAX_BULLETS         8
#define MAX_ENEMIES         6
#define MAX_ASTEROIDS       4
#define MAX_BOSS_BULLETS    10

/* ===== Điểm ===== */
#define SCORE_ENEMY         10
#define SCORE_ASTEROID      5
#define SCORE_BOSS          100               /* nhân (level + 1) */

/* ===== Boss ===== */
#define BOSS_W              24
#define BOSS_H              12
#define BOSS_Y              (PLAYFIELD_TOP + 4)
#define BOSS_SPEED          22.0f
#define BOSS_ENTER_SPEED    20.0f
#define BOSS_BULLET_SPEED   45.0f
#define BOSS_FIRST_SHOT_MS  800
#define BOSS_FIRE_MS_BASE   1400              /* giảm dần theo level */
#define BOSS_FIRE_MS_STEP   250

/* ===== Màn chơi ===== */
#define LEVELS_TOTAL        3

#endif /* APP_CONFIG_H */
