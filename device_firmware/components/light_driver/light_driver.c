#include "light_driver.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "light_ws2812";

static led_strip_handle_t led_strip;
static uint32_t g_num_leds = 0;

static bool g_is_on = false;
static uint16_t g_red = 255, g_green = 255, g_blue = 255; // Màu mặc định (Trắng)
static uint16_t g_brightness = 100; // 0 - 100%
static uint16_t g_hue;
static uint16_t g_saturation;

// Hàm nội bộ để tính toán và cập nhật dải LED
static void update_led_strip(void)
{
    if (!led_strip) return;

    if (!g_is_on || g_brightness == 0) {
        led_strip_clear(led_strip);
        return;
    }

    // Nội suy độ sáng (Scale từ 0-100% sang dải 0-255 của PWM số)
    uint16_t r_out = (g_red * g_brightness) / 100;
    uint16_t g_out = (g_green * g_brightness) / 100;
    uint16_t b_out = (g_blue * g_brightness) / 100;

    for (int i = 0; i < g_num_leds; i++) {
        led_strip_set_pixel(led_strip, i, r_out, g_out, b_out);
    }
    led_strip_refresh(led_strip);
}

static void hsv2rgb(uint16_t hue_deg, uint16_t saturation_pct, uint16_t brightness_pct, uint16_t *red_out, uint16_t *green_out, uint16_t *blue_out)
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
        uint16_t grayscale_val = (uint16_t)(value_ratio * 255.0f);
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
    *red_out   = (uint16_t)(red_float * 255.0f);
    *green_out = (uint16_t)(green_float * 255.0f);
    *blue_out  = (uint16_t)(blue_float * 255.0f);
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

esp_err_t light_driver_set_color(uint16_t r, uint16_t g, uint16_t b)
{
    g_red = r;
    g_green = g;
    g_blue = b;
    update_led_strip();
    return ESP_OK;
}

esp_err_t light_driver_set_brightness(uint16_t brightness_pct)
{
    if (brightness_pct > 100) brightness_pct = 100;
    g_brightness = brightness_pct;
    update_led_strip();
    return ESP_OK;
}

esp_err_t light_driver_set_hue(uint16_t hue)
{   
    g_hue = hue ;
    hsv2rgb(g_hue, g_saturation, g_brightness, &g_red, &g_green, &g_blue);
    update_led_strip();
    return ESP_OK;
}

esp_err_t light_driver_set_saturation(uint16_t saturation)
{
    g_saturation = saturation ;
    hsv2rgb(g_hue, g_saturation, g_brightness, &g_red,&g_green, &g_blue);
    update_led_strip();
    return ESP_OK;   
}

esp_err_t light_driver_set_hsv(uint16_t hue, uint16_t saturation, uint16_t brightess)
{
    g_saturation = saturation ;
    g_hue  = hue;
    g_brightness = brightess;
    hsv2rgb(g_hue, g_saturation, g_brightness, &g_red,&g_green, &g_blue);
    update_led_strip();
    return ESP_OK;
}