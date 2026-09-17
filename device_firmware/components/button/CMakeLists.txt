idf_component_register(SRCS "button_adc.c" "button_gpio.c" "iot_button.c"
                        INCLUDE_DIRS include
                        PRIV_REQUIRES esp_adc	
                        REQUIRES esp_driver_gpio esp_driver_ledc esp_driver_gptimer esp_timer esp_adc esp_hal_ana_conv)
