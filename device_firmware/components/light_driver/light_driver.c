#include "light_driver.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "light_ws2812";

static led_strip_handle_t led_strip;
static uint32_t g_num_leds = 0;

static bool g_is_on = false;
static uint8_t g_red = 255, g_green = 255, g_blue = 255; // Màu mặc định (Trắng)
static uint8_t g_brightness = 100; // 0 - 100%

// Hàm nội bộ để tính toán và cập nhật dải LED
static void update_led_strip(void)
{
    if (!led_strip) return;

    if (!g_is_on || g_brightness == 0) {
        led_strip_clear(led_strip);
        return;
    }

    // Nội suy độ sáng (Scale từ 0-100% sang dải 0-255 của PWM số)
    uint8_t r_out = (g_red * g_brightness) / 100;
    uint8_t g_out = (g_green * g_brightness) / 100;
    uint8_t b_out = (g_blue * g_brightness) / 100;

    for (int i = 0; i < g_num_leds; i++) {
        led_strip_set_pixel(led_strip, i, r_out, g_out, b_out);
    }
    led_strip_refresh(led_strip);
}

esp_err_t light_driver_init(const light_driver_config_t *config)
{
    g_num_leds = config->max_leds;

    // Cấu hình ngoại vi RMT cho WS2812 (Chuẩn IDF v6)
    led_strip_config_t strip_config = {
        .strip_gpio_num = config->gpio_pin,
        .max_leds = config->max_leds,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // Chuẩn màu của WS2812 là GRB
        .led_model = LED_MODEL_WS2812, 
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz resolution
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "WS2812 initialized on GPIO %d, %lu LEDs", config->gpio_pin, config->max_leds);

    led_strip_clear(led_strip);
    return ESP_OK;
}

esp_err_t light_driver_set_state(bool state)
{
    g_is_on = state;
    update_led_strip();
    return ESP_OK;
}

esp_err_t light_driver_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    g_red = r;
    g_green = g;
    g_blue = b;
    update_led_strip();
    return ESP_OK;
}

esp_err_t light_driver_set_brightness(uint8_t brightness_pct)
{
    if (brightness_pct > 100) brightness_pct = 100;
    g_brightness = brightness_pct;
    update_led_strip();
    return ESP_OK;
}