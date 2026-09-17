
idf_component_register(SRCS "./light_driver.c" 
                    INCLUDE_DIRS "." "./include"
                    REQUIRES app_storage esp_driver_gptimer esp_driver_ledc esp_timer esp_driver_gpio driver led_strip
)
