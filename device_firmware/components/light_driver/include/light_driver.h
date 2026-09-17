#ifndef __LIGHT_DRIVER_H__
#define __LIGHT_DRIVER_H__

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"

// Cấu hình khởi tạo cho WS2812
typedef struct {
    gpio_num_t gpio_pin; // Chân GPIO nối vào chân DI của WS2812
    uint32_t max_leds;   // Số lượng bóng LED WS2812 trên dây
} light_driver_config_t;

esp_err_t light_driver_init(const light_driver_config_t *config);
esp_err_t light_driver_set_state(bool state);
esp_err_t light_driver_set_color(uint8_t r, uint8_t g, uint8_t b);
esp_err_t light_driver_set_brightness(uint8_t brightness_pct); // brightness_pct: 0-100%

#endif