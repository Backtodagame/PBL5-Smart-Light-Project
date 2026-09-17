set(srcs "app_main.c"
                    "app_driver.c")
set(include_dirs "include")
set(DEVELOPMENT_BOARD "board_esp32c3_devkitc.h")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}"
                    REQUIRES esp_wifi esp_netif esp_event nvs_flash freertos lwip esp_timer app_storage button light_driver)
                    
idf_build_set_property(COMPILE_DEFINITIONS "-DDEVELOPMENT_BOARD=\"${DEVELOPMENT_BOARD}\"" APPEND)
