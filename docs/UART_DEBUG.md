# Debug qua UART — Space Shooter (STM32F103C8T6 + FreeRTOS)

Thay cho debug bằng LED PC13 (đã xoá toàn bộ). Firmware in log ra cổng nối tiếp để quan sát được
chính xác **thời điểm**, **nút nào**, **giữ bao lâu** và **debounce đã loại bao nhiêu xung nhiễu**.

## 1. Đấu dây

| STM32 (Blue Pill) | USB-TTL (CP2102 / CH340 / FT232) |
|---|---|
| **PA9** (USART1_TX) | **RX** |
| **GND** | **GND** |

- Chỉ cần dây TX. PA10 (RX) không dùng.
- Mức logic **3.3 V**. Không nối chân 5 V của USB-TTL vào board.
- Nếu board đã được cấp nguồn từ ST-Link/USB thì không nối thêm chân VCC của USB-TTL.

## 2. Bật USART1 trong CubeMX (khuyến nghị)

1. Mở `BTLhethongnhun.ioc` (nhấp đúp trong CubeIDE).
2. **Pinout & Configuration → Connectivity → USART1** → *Mode* = **Asynchronous**.
   CubeMX tự gán **PA9 = USART1_TX**, **PA10 = USART1_RX** (kiểm tra trên hình chip).
3. **Parameter Settings**: Baud Rate **115200**, Word Length **8 Bits**, Parity **None**, Stop Bits **1**.
   Không cần bật ngắt (NVIC) và DMA — firmware gửi bằng `HAL_UART_Transmit`.
4. **Ctrl+S → Generate Code**. CubeMX sẽ tạo `Core/Src/usart.c`, `Core/Inc/usart.h`, thêm `MX_USART1_UART_Init()`
   vào `main.c`, bật `HAL_UART_MODULE_ENABLED` và chép `stm32f1xx_hal_uart.c/.h` vào `Drivers`.
   Code trong các khối `USER CODE` và thư mục `App/` không bị mất.
5. Refresh (F5) → **Project → Clean** → Build.

`dbg_uart.c` tự nhận ra `HAL_UART_MODULE_ENABLED` và dùng `huart1` của CubeMX. Dòng banner khi khởi động sẽ ghi
`UART    : HAL (CubeMX USART1), TX=PA9, 115200 baud`. Nếu chưa bật USART1 trong CubeMX, firmware dùng driver thanh ghi
dự phòng (dòng banner ghi `register (khong dung CubeMX)`) nên vẫn build và chạy được.

> Baud khi dùng CubeMX do CubeMX quyết định (cột *Baud Rate*), `DEBUG_UART_BAUD` trong `app_config.h` chỉ áp dụng cho driver dự phòng.

## 3. Cấu hình terminal

PuTTY / Tera Term / CubeIDE (Window → Show View → Terminal) / `screen`:

- **115200 baud, 8 data bit, không parity, 1 stop bit (8N1), không flow control**
- Mỗi dòng kết thúc bằng `\r\n`.

## 4. Nội dung log

### 4.1 Lúc khởi động (in trực tiếp, trước khi scheduler chạy)

```
==== BTL He thong nhung: Space Shooter | STM32F103C8T6 + FreeRTOS ====
Build   : <ngay build>
SYSCLK  : <tan so> Hz
UART    : HAL (CubeMX USART1), TX=PA9, 115200 baud
Reset   : <nguyen nhan reset>
Buttons : PA0=FIRE PA1=SKILL PA2=START PA3=PAUSE (active-low, pull-up)
Debounce: 3 mau x 10 ms = 30 ms
  create mutex game : OK
  create task input  (prio 3) : OK
  create task game   (prio 2) : OK
  create task render (prio 1) : OK
  create task dbg    (prio 1) : OK
Heap    : free=<n> / 8192 bytes
Scheduler starting...
```

- `Reset` cho biết lý do board vừa reset: `POWER-ON`, `NRST-PIN`, `SOFTWARE`, `IWDG`, `WWDG`, `LOW-POWER`.
  Nếu bấm nút mà board tự khởi động lại, dòng này giúp phân biệt sụt nguồn (`POWER-ON`) với lỗi phần mềm.
- Nếu tạo task/mutex thất bại: in `FATAL: tao task/mutex that bai (het heap?)` lặp lại mỗi giây.

### 4.2 Quét I2C (khoảng 0,5 s sau khi khởi động)

```
[    5xx ms] I2C scan (SCL=PB6, SDA=PB7):
[    5xx ms]   0x3C OLED SSD1306 : FOUND   <- dia chi code dang dung
[    5xx ms]   0x3D OLED SSD1306 : no ACK
[    5xx ms]   0x53 ADXL345      : FOUND, DEVID=0xE5 (OK)   <- dia chi code dang dung
[    5xx ms]   0x1D ADXL345      : no ACK
```

Nếu không thấy thiết bị ở địa chỉ mà code dùng, sẽ có thêm dòng `WARN: ...` nói rõ hậu quả
(OLED trống / không có nghiêng nhưng nút vẫn chạy).

### 4.3 Sự kiện nút bấm (sau debounce)

```
[ 12345 ms] BTN_START DOWN  glitch_rejected=2
[ 12480 ms] BTN_START UP    held_ms=135  glitch_rejected=0
```

- `DOWN` / `UP`: thời điểm nút được **chấp nhận** là đã nhấn / nhả (đã qua bộ lọc 3 mẫu × 10 ms).
- `held_ms`: thời gian giữ nút (từ DOWN đến UP).
- `glitch_rejected`: số xung đổi mức **đã bị debounce loại bỏ** kể từ lần DOWN/UP trước của nút đó.
  Đây là bằng chứng bộ lọc chống dội đang hoạt động: có nhiễu/dội nhưng game không nhận nhầm.

### 4.4 Phản ứng của game

```
[ 12345 ms] STATE -> PLAYING
```

In khi game đổi trạng thái (`MENU`, `PLAYING`, `PAUSED`, `GAME_OVER`, `WIN`).
Dòng này cho thấy lệnh nút đã đi hết đường: GPIO → debounce → Input_Task → Game_Task.

Ở màn `MENU`/`GAME_OVER`/`WIN`, **chỉ nút START (PA2)** mới có tác dụng; nút PAUSE chỉ có tác dụng khi đang
`PLAYING`/`PAUSED`; nút FIRE chỉ bắn khi đang `PLAYING`.

### 4.5 Thiết bị ngoại vi và thống kê

```
[   xxx ms] ADXL345 init OK        (hoặc: ADXL345 init FAIL ..., ADXL345 ket noi lai OK)
[   xxx ms] ADXL345 calib OK, offset_mg x=<n> y=<n>
[   xxx ms] OLED init OK           (hoặc: OLED init FAIL ...)
[ 10000 ms] STAT heap_free=<n> heap_min_free=<n> log_dropped=0
```

- `STAT` in mỗi 10 s: heap còn trống, mức heap thấp nhất từng đạt, số log bị mất.
- `WARN: hang doi log day, ...` xuất hiện nếu log sinh ra nhanh hơn tốc độ UART (hàng đợi 24 sự kiện).

## 5. Quy trình kiểm thử nút bấm (để lấy số liệu báo cáo)

1. Nạp firmware, mở terminal, bấm Reset → chụp phần banner + I2C scan.
2. Với **mỗi nút** (PA0..PA3): bấm nhả 1 lần → ghi lại cặp `DOWN`/`UP` và `held_ms`.
3. Bấm **rất nhanh** (chạm < 30 ms) → *không* có dòng DOWN (bị debounce loại), đúng thiết kế.
4. Bấm giữ ~1 s → `held_ms` xấp xỉ 1000.
5. Ở MENU bấm START → có `STATE -> PLAYING`; bấm PAUSE → `STATE -> PAUSED`; bấm PAUSE lần nữa → `PLAYING`.
6. Quan sát `glitch_rejected` trên nút thật: số > 0 chứng tỏ dội cơ học đã bị lọc.
7. Để chạy vài phút, chụp dòng `STAT` (heap ổn định, `log_dropped=0`).

## 6. Bật / tắt

`App/Config/app_config.h`:

```c
#define DEBUG_UART            1       // 0 = tắt hoàn toàn (không tốn Flash/RAM/CPU, không tạo task dbg)
#define DEBUG_UART_BAUD       115200u
#define DEBUG_UART_STATS_MS   10000u  // 0 = không in dòng STAT
#define BTN_DEBOUNCE_SAMPLES  3       // số mẫu liên tiếp để chấp nhận đổi trạng thái nút
```

## 7. Thiết kế (để trả lời thầy)

- **Không chặn task thời gian thực:** `Input_Task` (chu kỳ 10 ms) chỉ đẩy sự kiện vào hàng đợi vòng (24 phần tử)
  trong critical section rất ngắn. Việc in UART (chậm, ~87 µs/byte) do `Dbg_Task` (ưu tiên thấp, bằng Render_Task) làm.
- **Không dùng printf:** tự viết hàm in số/chuỗi, tránh tốn RAM/stack của newlib.
- **UART do CubeMX sinh (HAL)**, gửi bằng `HAL_UART_Transmit`; có driver thanh ghi dự phòng nếu chưa bật USART1.
- **Chỉ một nơi in tại một thời điểm:** trước scheduler là `App_Init`, sau đó chỉ `Dbg_Task` — nên không cần khoá UART.
- **Quét I2C dùng chung mutex** với OLED/ADXL345 (`BspI2c_Probe`), không gây xung đột bus.
