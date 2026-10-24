#include <stdio.h>
#include "esp_log.h"

#include "iot_button.h"
#include "light_driver.h"
#include "include/board_esp32c3_devkitc.h"
#include "include/app_priv.h"

#define TAG "app_driver"

// Định nghĩa chân nối với WS2812 và số lượng bóng
#define WS2812_GPIO_PIN    GPIO_NUM_10  // Sửa lại thành chân GPIO thực tế bạn đang cắm
#define WS2812_NUM_LEDS    8         // Số bóng LED WS2812 (ví dụ: vòng Ring 16 bóng)

static bool g_output_state = true;
static uint8_t g_brightness = 100; 
static int8_t g_fade_dir = -1;    // Giảm 1% độ sáng mỗi nhịp hold
typedef enum
{   
    default_color,
    red,
    yellow,
    green,
    cyan,
    blue,
    magenta
} basic_color;

static uint8_t color_now = 0;

/* Callback: Bật / Tắt đèn khi Click 1 lần */
static void turn_on_off(void *arg)
{
    g_output_state = !g_output_state;
    ESP_LOGI(TAG, "Light %s", g_output_state ? "ON" : "OFF");
    light_driver_set_state(g_output_state);
}

/* Callback: Tăng/giảm độ sáng liên tục khi NHẤN GIỮ */
static void dimming(void *arg)
{
    if (!g_output_state) return; // Nếu đang tắt thì không cho chỉnh độ sáng

    int new_brightness = g_brightness + g_fade_dir;

    if (new_brightness <= 0) {
        new_brightness = 0;
        g_fade_dir = 1; // Tới ngưỡng min, tự động đảo chiều tăng lên
    } else if (new_brightness >= 100) {
        new_brightness = 100;
        g_fade_dir = -1; // Tới ngưỡng max, tự động đảo chiều giảm xuống
    }

    g_brightness = (uint8_t)new_brightness;
    light_driver_set_brightness(g_brightness);
}

/* Callback: Khi THẢ NÚT ra sau khi nhấn giữ -> Đảo chiều cho lần hold sau */
static void release_btn_cb(void *arg)
{
    g_fade_dir = -g_fade_dir; 
}

static void change_color(void *arg)
{   
    if (!g_output_state) return ; 
    
    color_now ++ ;
    if (color_now == 7) {
        color_now = default_color;
    } 
    switch (color_now)
    {
    case red:
        light_driver_set_color(255,0,0);
        break;

    case yellow:
        light_driver_set_color(255,255,0);
        break;

    case green:
        light_driver_set_color(0,255,0);
        break;

    case cyan:
        light_driver_set_color(0,255,255);
        break;

    case blue:
        light_driver_set_color(0,0,255);
        break;
    
    case magenta:
        light_driver_set_color(255,0,255);
        break;

    default:
        light_driver_set_color(255,200,100);
        break;
    }
    ESP_LOGI(TAG,"Color has changed\n");
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
        iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, turn_on_off);
        iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_HOLD, dimming);
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, release_btn_cb);
        iot_button_register_cb(btn_handle, BUTTON_DOUBLE_CLICK, change_color);
    }
}