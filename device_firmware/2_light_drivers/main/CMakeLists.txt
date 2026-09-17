set(srcs "app_main.c"
                    "app_driver.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc esp_driver_gptimer esp_adc app_storage button light_driver
)
