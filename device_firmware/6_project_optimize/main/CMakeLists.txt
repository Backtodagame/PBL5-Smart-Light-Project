set(srcs "app_main.c"
                    "app_pm.c"
                    "app_driver.c")
set(include_dirs "include")
set(DEVELOPMENT_BOARD "board_esp32c3_devkitc.h")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}"
                    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc esp_driver_gptimer esp_adc app_storage button 
                    esp_wifi esp_event light_driver nvs_flash esp_rainmaker app_wifi
                    )

target_add_binary_data(${COMPONENT_TARGET} "server.crt" TEXT)

idf_build_set_property(COMPILE_DEFINITIONS "-DDEVELOPMENT_BOARD=\"${DEVELOPMENT_BOARD}\"" APPEND)
