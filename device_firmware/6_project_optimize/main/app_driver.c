/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

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
#include "app_pm.h"

#define TAG "app_driver"
#define WS2812_GPIO_PIN    GPIO_NUM_8  // Sửa lại thành chân GPIO thực tế bạn đang cắm
#define WS2812_NUM_LEDS    8     // Số bóng LED WS2812 (ví dụ: vòng Ring 16 bóng)

static bool g_power = DEFAULT_POWER;
static uint16_t g_brightness = DEFAULT_BRIGHTNESS; 
static uint16_t g_hue = DEFAULT_HUE;
static uint16_t g_saturation = DEFAULT_SATURATION ;
static int8_t g_fade_dir = -1;   


static void push_btn_cb(void *arg)
{
    g_power = !g_power;
    ESP_LOGI(TAG, "Light %s", g_power ? "ON" : "OFF");
    app_light_set_power(g_power);
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


void app_driver_init()
{
    /* 1. Khởi tạo Đèn WS2812 */
    light_driver_config_t driver_config = {
        .gpio_pin = WS2812_GPIO_PIN,
        .max_leds = WS2812_NUM_LEDS,
    };
    ESP_ERROR_CHECK(light_driver_init(&driver_config));

    app_light_set(g_hue, g_saturation, g_brightness);
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


esp_err_t app_light_set_power(bool power)
{
    if (power) {
        // PM Lock
        app_pm_lock_acquire();
        // light on
        light_driver_set_state(true);
    } else {
        // light off
        light_driver_set_state(false);
        // PM UnLock
        app_pm_lock_release();
    }
    return ESP_OK;
}

esp_err_t app_light_set(uint16_t hue, uint16_t saturation, uint16_t brightness)
{
    return light_driver_set_hsv(hue,saturation, brightness);
}

esp_err_t app_light_set_brightness(uint16_t brightness)
{
    return light_driver_set_brightness(brightness);
}

esp_err_t app_light_set_hue(uint16_t hue)
{
    return light_driver_set_hue(hue);
}

esp_err_t app_light_set_saturation(uint16_t saturation)
{
    return light_driver_set_saturation(saturation);
}
