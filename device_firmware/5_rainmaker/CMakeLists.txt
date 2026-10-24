# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

add_compile_options(-fdiagnostics-color=always)

set(RAIMAKER_PATH "E:/HK9/PBL5/PBL5-Smart-Light-Project/device_firmware/components/esp-rainmaker/components/esp_rainmaker")


# Add RainMaker components and other common application components
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../components/light_driver
                        ${CMAKE_CURRENT_LIST_DIR}/../components/button
                        ${CMAKE_CURRENT_LIST_DIR}/../components/app_storage
                        ${CMAKE_CURRENT_LIST_DIR}/../components/app_wifi
                        ${CMAKE_CURRENT_LIST_DIR}/../components/managed_components/espressif__led_strip
                        ${CMAKE_CURRENT_LIST_DIR}/../components/managed_components/espressif__network_provisioning
                        ${RAIMAKER_PATH}
                        ${CMAKE_CURRENT_LIST_DIR}/../components/managed_components/espressif__qrcode
                        ${CMAKE_CURRENT_LIST_DIR}/../components/managed_components/espressif__cjson
                        )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(5_rainmaker)
