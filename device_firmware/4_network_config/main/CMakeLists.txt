set(srcs "app_main.c"
                    "app_driver.c")
set(include_dirs "include")
set(DEVELOPMENT_BOARD "board_esp32c3_devkitc.h")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}"
                    REQUIRES freertos button light_driver esp_event esp_system esp_wifi qrcode nvs_flash app_storage network_provisioning)

idf_build_set_property(COMPILE_DEFINITIONS "-DDEVELOPMENT_BOARD=\"${DEVELOPMENT_BOARD}\"" APPEND)
