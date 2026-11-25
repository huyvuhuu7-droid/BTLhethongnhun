# STM32 FreeRTOS Space Shooter

Dự án game Space Shooter chạy trên vi điều khiển **STM32F103C8T6**, sử dụng hệ điều hành thời gian thực **FreeRTOS** và màn hình OLED SSD1306.

## 1. Giới thiệu

Space Shooter là trò chơi bắn máy bay không gian. Người chơi điều khiển phi thuyền bằng cách nghiêng cảm biến ADXL345, sử dụng nút nhấn để bắn đạn và kích hoạt kỹ năng.

## 2. Phần cứng

| Linh kiện | Chức năng |
|---|---|
| STM32F103C8T6 | Vi điều khiển chính |
| OLED SSD1306 128×64 | Hiển thị trò chơi |
| ADXL345 | Cảm biến nghiêng điều khiển |
| Các nút nhấn | Fire, Skill, Start, Pause |

## 3. Công nghệ sử dụng

- STM32 HAL
- FreeRTOS
- I2C giao tiếp với OLED và ADXL345
- UART USART1 để debug
- Ngôn ngữ lập trình C

## 4. Cấu trúc thư mục

```text
App/
├── Config/       # Cấu hình game
├── Drivers/      # Driver OLED, ADXL345, UART, buttons
├── Game/         # Logic trò chơi
├── Gfx/          # Đồ họa và render
└── RTOS/         # Các task FreeRTOS
Core/             # Mã nguồn STM32
Drivers/          # STM32 HAL và CMSIS
Middlewares/      # FreeRTOS
docs/             # Tài liệu dự án
```

## 5. Các task FreeRTOS

| Task | Priority | Chu kỳ |
|---|---:|---:|
| Input | 3 | 10 ms |
| Game | 2 | 30 ms |
| Render | 1 | 40 ms |
| Debug | 1 | Theo cấu hình |

Task Debug chỉ được tạo khi `DEBUG_UART = 1`.

## 6. Điều khiển trò chơi

| Nút | Chức năng |
|---|---|
| PA0 | Fire |
| PA1 | Skill |
| PA2 | Start |
| PA3 | Pause |

Các nút sử dụng active-low và pull-up.

## 7. Debug qua UART

- USART1 TX: PA9
- Baud rate: 115200
- Format: 8N1

Tham khảo hướng dẫn chi tiết tại `docs/UART_DEBUG.md`.

## 8. Cấu hình

Các thông số game được tập trung tại:

`App/Config/app_config.h`

Có thể thay đổi chu kỳ task, tốc độ phi thuyền, số lượng đạn, máu, mana và thông số boss.

## 9. Build

Mở file `BTLhethongnhun.ioc` bằng STM32CubeMX hoặc STM32CubeIDE, sau đó mở project bằng STM32CubeIDE và build.

