/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "iot_button.h"
#include "light_driver.h"

#include "include/board_esp32c3_devkitc.h"
#include "include/app_priv.h"

#define TAG "app_driver"
#define WS2812_GPIO_PIN    GPIO_NUM_8  // Sửa lại thành chân GPIO thực tế bạn đang cắm
#define WS2812_NUM_LEDS    16          // Số bóng LED WS2812 (ví dụ: vòng Ring 16 bóng)

static bool g_output_state = true;
static uint8_t g_brightness = 100; 
static int8_t g_fade_dir = -1;


static void push_btn_cb(void *arg)
{
    app_driver_set_state(!g_output_state);
}


static void hold_btn_cb(void *arg)
{
    if (!g_output_state) return;

    static uint8_t hold_tick = 0 ;
    hold_tick++;

    if (hold_tick > 5 ){
        return ;
    }

    hold_tick = 0;

    int new_brightness = g_brightness + g_fade_dir;

    if (new_brightness <= 10) {
        new_brightness = 10;
        g_fade_dir = 1; // Tới ngưỡng min, tự động đảo chiều tăng lên
    } else if (new_brightness >= 100) {
        new_brightness = 100;
        g_fade_dir = -1; // Tới ngưỡng max, tự động đảo chiều giảm xuống
    }

    g_brightness = (uint8_t)new_brightness;
    light_driver_set_brightness(g_brightness);

}

static void release_btn_cb(void *arg)
{
    g_fade_dir = -g_fade_dir; 
}


void app_driver_init()
{
    /* Configure push button */
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num     = LIGHT_BUTTON_GPIO,
            .active_level = LIGHT_BUTTON_ACTIVE_LEVEL,
        },
    };
    button_handle_t btn_handle = iot_button_create(&btn_cfg);
    if (btn_handle) {
        /* Register a callback for a button short press event */
        iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, push_btn_cb);
        iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_HOLD, hold_btn_cb);
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, release_btn_cb);

    }

    /**
     * @brief Light driver initialization
     */
    light_driver_config_t driver_config = {
        .gpio_pin = WS2812_GPIO_PIN,
        .max_leds = WS2812_NUM_LEDS,
    };
    ESP_ERROR_CHECK(light_driver_init(&driver_config));
    light_driver_set_color(255, 200, 100); 
    light_driver_set_brightness(g_brightness);
    light_driver_set_state(g_output_state);
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if (g_output_state != state) {
        g_output_state = state;
        if (g_output_state) {
            // light on
            ESP_LOGI(TAG, "Light ON");
            light_driver_set_state(true);
        } else {
            // light off
            ESP_LOGI(TAG, "Light OFF");
            light_driver_set_state(false);
        }
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
