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
static uint16_t g_hue = DEFAULT_HUE;
static int8_t g_fade_dir = -1;   
static uint16_t mode ;

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
    button_config_t LIGHT_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num     = LIGHT_BUTTON_GPIO,
            .active_level = LIGHT_BUTTON_ACTIVE_LEVEL,
        },
    };
    button_handle_t LIGHT_btn_handle = iot_button_create(&LIGHT_btn_cfg);
    if (LIGHT_btn_handle) {
        iot_button_register_cb(LIGHT_btn_handle, BUTTON_SINGLE_CLICK, push_btn_cb);
        iot_button_register_cb(LIGHT_btn_handle, BUTTON_LONG_PRESS_HOLD, hold_btn_cb);
        iot_button_register_cb(LIGHT_btn_handle, BUTTON_PRESS_UP, release_btn_cb);
        iot_button_register_cb(LIGHT_btn_handle, BUTTON_DOUBLE_CLICK, double_click_cb);

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

static void hsv2rgb(uint16_t hue_deg, uint8_t saturation_pct, uint8_t brightness_pct, uint8_t *red_out, uint8_t *green_out, uint8_t *blue_out)
{
    float hue_float, p_val, q_val, t_val, fractional_part;
    long sector_index;
    
    // 1. Chuẩn hóa góc Hue tuyệt đối trong khoảng từ 0 đến 359 độ (Tránh tràn giá trị 360 hoặc số âm)
    hue_deg = hue_deg % 360;

    // Chuyển đổi phần trăm (0-100) sang dạng số thực (0.0 - 1.0)
    float saturation_ratio = saturation_pct / 100.0f;
    float value_ratio = brightness_pct / 100.0f;

    // Nếu độ bão hòa bằng 0, đèn hiển thị màu trắng/xám
    if (saturation_ratio <= 0.0f) {
        uint8_t grayscale_val = (uint8_t)(value_ratio * 255.0f);
        *red_out = grayscale_val;
        *green_out = grayscale_val;
        *blue_out = grayscale_val;
        return;
    }

    // Chia dải màu thành 6 vùng (mỗi vùng 60 độ)
    hue_float = (float)hue_deg / 60.0f;
    sector_index = (long)hue_float;
    
    // Đảm bảo sector_index luôn nằm an toàn từ 0 đến 5
    if (sector_index > 5) {
        sector_index = 5;
    }    
    fractional_part = hue_float - (float)sector_index;
    // Tính toán các hệ số pha màu trung gian
    p_val = value_ratio * (1.0f - saturation_ratio);
    q_val = value_ratio * (1.0f - (saturation_ratio * fractional_part));
    t_val = value_ratio * (1.0f - (saturation_ratio * (1.0f - fractional_part)));

    float red_float = 0.0f, green_float = 0.0f, blue_float = 0.0f;

    // Phân bổ tỷ lệ màu theo từng vùng không gian màu
    switch (sector_index) {
        case 0:
            red_float = value_ratio; green_float = t_val;     blue_float = p_val;
            break;
        case 1:
            red_float = q_val;     green_float = value_ratio; blue_float = p_val;
            break;
        case 2:
            red_float = p_val;     green_float = value_ratio; blue_float = t_val;
            break;
        case 3:
            red_float = p_val;     green_float = q_val;     blue_float = value_ratio;
            break;
        case 4:
            red_float = t_val;     green_float = p_val;     blue_float = value_ratio;
            break;
        case 5:
        default:
            red_float = value_ratio; green_float = p_val;     blue_float = q_val;
            break;
    }

    // Quy đổi hệ số thực ra dải giá trị 8-bit (0 - 255)
    *red_out   = (uint8_t)(red_float * 255.0f);
    *green_out = (uint8_t)(green_float * 255.0f);
    *blue_out  = (uint8_t)(blue_float * 255.0f);
}

esp_err_t app_light_set_hue(uint16_t hue)
{
    uint8_t r,g,b;
    g_hue =  hue;
    hsv2rgb(g_hue, g_satuation, g_brightness, &r, &g, &b);
    // 4. Ra lệnh cho phần cứng đổi màu theo chuẩn RGB vừa tính ra
    light_driver_set_color(r, g, b);
    // Bắt buộc phải có return để hàm kết thúc đúng chuẩn esp_err_t
    return ESP_OK;
}


esp_err_t app_light_set_saturation(uint16_t saturation)
{
    uint8_t r,g,b;
    g_satuation = saturation;
    hsv2rgb(g_hue, g_satuation, g_brightness, &r, &g, &b);
    // 4. Ra lệnh cho phần cứng đổi màu theo chuẩn RGB vừa tính ra
    light_driver_set_color(r, g, b);
    // Bắt buộc phải có return để hàm kết thúc đúng chuẩn esp_err_t
    return ESP_OK;
}

