#include <stdio.h>
#include "esp_log.h"

#include "iot_button.h"
#include "light_driver.h"
#include "include/board_esp32c3_devkitc.h"
#include "include/app_priv.h"

#define TAG "app_driver"

// Định nghĩa chân nối với WS2812 và số lượng bóng
#define WS2812_GPIO_PIN    GPIO_NUM_8  // Sửa lại thành chân GPIO thực tế bạn đang cắm
#define WS2812_NUM_LEDS    16          // Số bóng LED WS2812 (ví dụ: vòng Ring 16 bóng)

static bool g_output_state = true;
static uint8_t g_brightness = 100; 
static int8_t g_fade_dir = -5;    // Giảm 5% độ sáng mỗi nhịp hold

/* Callback: Bật / Tắt đèn khi Click 1 lần */
static void push_btn_cb(void *arg)
{
    g_output_state = !g_output_state;
    ESP_LOGI(TAG, "Light %s", g_output_state ? "ON" : "OFF");
    light_driver_set_state(g_output_state);
}

/* Callback: Tăng/giảm độ sáng liên tục khi NHẤN GIỮ */
static void hold_btn_cb(void *arg)
{
    if (!g_output_state) return; // Nếu đang tắt thì không cho chỉnh độ sáng

    int new_brightness = g_brightness + g_fade_dir;

    if (new_brightness <= 10) {
        new_brightness = 10;
        g_fade_dir = 5; // Tới ngưỡng min, tự động đảo chiều tăng lên
    } else if (new_brightness >= 100) {
        new_brightness = 100;
        g_fade_dir = -5; // Tới ngưỡng max, tự động đảo chiều giảm xuống
    }

    g_brightness = (uint8_t)new_brightness;
    light_driver_set_brightness(g_brightness);
}

/* Callback: Khi THẢ NÚT ra sau khi nhấn giữ -> Đảo chiều cho lần hold sau */
static void release_btn_cb(void *arg)
{
    g_fade_dir = -g_fade_dir; 
}

void app_driver_init()
{
    /* 1. Khởi tạo Đèn WS2812 */
    light_driver_config_t driver_config = {
        .gpio_pin = WS2812_GPIO_PIN,
        .max_leds = WS2812_NUM_LEDS,
    };
    ESP_ERROR_CHECK(light_driver_init(&driver_config));
    
    // Set màu mặc định là Vàng Ấm (Warm White: R=255, G=200, B=100)
    light_driver_set_color(255, 200, 100); 
    light_driver_set_brightness(g_brightness);
    light_driver_set_state(g_output_state); // Bật lên

    /* 2. Khởi tạo Nút nhấn */
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num     = LIGHT_BUTTON_GPIO,
            .active_level = LIGHT_BUTTON_ACTIVE_LEVEL,
        },
    };
    button_handle_t btn_handle = iot_button_create(&btn_cfg);
    if (btn_handle) {
        iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, push_btn_cb);
        iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_HOLD, hold_btn_cb);
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, release_btn_cb);
    }
}