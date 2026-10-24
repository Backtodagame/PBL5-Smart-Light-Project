#include <stdio.h>
#include "esp_log.h"
#include "iot_button.h"
#include "light_driver.h"
#include "include/board_esp32c3_devkitc.h"
#include "include/app_priv.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>

#define TAG "app_driver"

// Định nghĩa chân nối với WS2812 và số lượng bóng
#define WS2812_GPIO_PIN    GPIO_NUM_10  // Sửa lại thành chân GPIO thực tế bạn đang cắm
#define WS2812_NUM_LEDS    8     // Số bóng LED WS2812 (ví dụ: vòng Ring 16 bóng)

static bool g_power = DEFAULT_POWER;
static uint8_t g_brightness = DEFAULT_BRIGHTNESS; 
static uint8_t g_satuation = DEFAULT_SATURATION;
static uint8_t g_hue = DEFAULT_HUE;
static int8_t g_fade_dir = -1;   

/* Callback: Bật / Tắt đèn khi Click 1 lần */
static void push_btn_cb(void *arg)
{
    g_power = !g_power;
    ESP_LOGI(TAG, "Light %s", g_power ? "ON" : "OFF");
    light_driver_set_state(g_power);
    if(light_device){
        esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_type(light_device, ESP_RMAKER_PARAM_POWER), /* Tham số 1: Trích xuất nút Nguồn từ thiết bị */
            esp_rmaker_bool(g_power));                                                 /* Tham số 2: Gói giá trị bật/tắt để gửi lên */
    }
}



/* Callback: Tăng/giảm độ sáng liên tục khi NHẤN GIỮ */
static void hold_btn_cb(void *arg)
{
    if (!g_power) return; // Nếu đang tắt thì không cho chỉnh độ sáng

    int new_brightness = g_brightness + g_fade_dir;

    if (new_brightness <= 10) {
        new_brightness = 10;
        g_fade_dir = 1; // Tới ngưỡng min, tự động đảo chiều tăng lên
    } else if (new_brightness >= 100) {
        new_brightness = 100;
        g_fade_dir = -1; // Tới ngưỡng max, tự động đảo chiều giảm xuống
    }

    g_brightness = (uint16_t)new_brightness;
    light_driver_set_brightness(g_brightness);

    if(light_device)
    {
        esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_type(light_device, ESP_RMAKER_PARAM_BRIGHTNESS),
        esp_rmaker_int(g_brightness));
    }
}

/* Callback: Khi THẢ NÚT ra sau khi nhấn giữ -> Đảo chiều cho lần hold sau */
static void release_btn_cb(void *arg)
{
    g_fade_dir = -g_fade_dir; 
}

static void double_click_cb(void *arg)
{
    ESP_LOGW(TAG,"Factory Reset: Clear old nvs flash");
    light_driver_set_color(255, 0, 0); 
    light_driver_set_brightness(50);

    nvs_flash_erase();
    esp_restart();
}





// Initialize SYSTEM 

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
    light_driver_set_state(g_power); // Bật lên

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
        iot_button_register_cb(btn_handle, BUTTON_DOUBLE_CLICK, double_click_cb);

    }
}

// RAINMAKER HERE 


esp_err_t app_light_set_power(bool power)
{
    g_power = power;    
    light_driver_set_state(g_power);
    
    return ESP_OK; 
}

esp_err_t app_light_set_brightness(uint16_t brightness)
{
    g_brightness = brightness;
    if (!g_power) {
        g_power = true;
        light_driver_set_state(g_power);
        
        esp_rmaker_param_update_and_report(
                esp_rmaker_device_get_param_by_type(light_device, ESP_RMAKER_PARAM_POWER),
                esp_rmaker_bool(g_power));
    }

    light_driver_set_brightness(g_brightness);
    return ESP_OK;
}

esp_err_t app_light_set_hue(uint16_t hue)
{
    g_hue = hue;
    if (!g_power) {
        g_power = true;
        light_driver_set_state(g_power);
        
        esp_rmaker_param_update_and_report(
                esp_rmaker_device_get_param_by_type(light_device, ESP_RMAKER_PARAM_POWER),
                esp_rmaker_bool(g_power));
    }

uint32_t h = g_hue % 360;           // Hue: 0 - 359 độ
    uint32_t s = g_satuation;           // Saturation: 0 - 100 %
    uint32_t v = g_brightness;          // Value/Brightness: 0 - 100 %

    // Tính toán biên độ sáng (Scale từ 0-100 thành 0-255)
    uint32_t rgb_max = (v * 255) / 100;
    uint32_t rgb_min = rgb_max * (100 - s) / 100;

    // Chia dải màu Hue thành 6 vùng (mỗi vùng 60 độ)
    uint32_t i = h / 60;
    uint32_t diff = h % 60;
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    uint8_t r = 0, g = 0, b = 0;

    // Phân bổ tỉ lệ Đỏ, Xanh lá, Xanh dương tùy theo vùng màu (i)
    switch (i) {
        case 0: r = rgb_max;           g = rgb_min + rgb_adj; b = rgb_min;           break; // Đỏ -> Vàng
        case 1: r = rgb_max - rgb_adj; g = rgb_max;           b = rgb_min;           break; // Vàng -> Xanh lá
        case 2: r = rgb_min;           g = rgb_max;           b = rgb_min + rgb_adj; break; // Xanh lá -> Xanh lơ (Cyan)
        case 3: r = rgb_min;           g = rgb_max - rgb_adj; b = rgb_max;           break; // Xanh lơ -> Xanh dương
        case 4: r = rgb_min + rgb_adj; g = rgb_min;           b = rgb_max;           break; // Xanh dương -> Tím
        case 5: r = rgb_max;           g = rgb_min;           b = rgb_max - rgb_adj; break; // Tím -> Đỏ
    }

    // 4. Ra lệnh cho phần cứng đổi màu theo chuẩn RGB vừa tính ra
    light_driver_set_color(r, g, b);

    // Bắt buộc phải có return để hàm kết thúc đúng chuẩn esp_err_t
    return ESP_OK;
}


esp_err_t app_light_set_saturation(uint16_t saturation)
{
    g_satuation = saturation;
    if (!g_power) {
        g_power = true;
        light_driver_set_state(g_power);
        
        esp_rmaker_param_update_and_report(
                esp_rmaker_device_get_param_by_type(light_device, ESP_RMAKER_PARAM_POWER),
                esp_rmaker_bool(g_power));
    }

    uint32_t h = g_hue % 360;           // Hue: 0 - 359 độ
    uint32_t s = g_satuation;           // Saturation: 0 - 100 %
    uint32_t v = g_brightness;          // Value/Brightness: 0 - 100 %

    // Tính toán biên độ sáng (Scale từ 0-100 thành 0-255)
    uint32_t rgb_max = (v * 255) / 100;
    uint32_t rgb_min = rgb_max * (100 - s) / 100;

    // Chia dải màu Hue thành 6 vùng (mỗi vùng 60 độ)
    uint32_t i = h / 60;
    uint32_t diff = h % 60;
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    uint8_t r = 0, g = 0, b = 0;

    // Phân bổ tỉ lệ Đỏ, Xanh lá, Xanh dương tùy theo vùng màu (i)
    switch (i) {
        case 0: r = rgb_max;           g = rgb_min + rgb_adj; b = rgb_min;           break; // Đỏ -> Vàng
        case 1: r = rgb_max - rgb_adj; g = rgb_max;           b = rgb_min;           break; // Vàng -> Xanh lá
        case 2: r = rgb_min;           g = rgb_max;           b = rgb_min + rgb_adj; break; // Xanh lá -> Xanh lơ (Cyan)
        case 3: r = rgb_min;           g = rgb_max - rgb_adj; b = rgb_max;           break; // Xanh lơ -> Xanh dương
        case 4: r = rgb_min + rgb_adj; g = rgb_min;           b = rgb_max;           break; // Xanh dương -> Tím
        case 5: r = rgb_max;           g = rgb_min;           b = rgb_max - rgb_adj; break; // Tím -> Đỏ
    }

    // 4. Ra lệnh cho phần cứng đổi màu theo chuẩn RGB vừa tính ra
    light_driver_set_color(r, g, b);

    // Bắt buộc phải có return để hàm kết thúc đúng chuẩn esp_err_t
    return ESP_OK;
}