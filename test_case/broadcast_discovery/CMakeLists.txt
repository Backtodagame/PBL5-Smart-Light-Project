# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

add_compile_options(-fdiagnostics-color=always)

# Add common application components
set(EXTRA_COMPONENT_DIRS ${COMPONENT_DIR}E:/HK9/PBL5/PBL5-Smart-Light-Project/device_firmware/components/light_driver
                        ${COMPONENT_DIR}E:/HK9/PBL5/PBL5-Smart-Light-Project/device_firmware/components/button
                        ${COMPONENT_DIR}E:/HK9/PBL5/PBL5-Smart-Light-Project/device_firmware/components/app_storage
                        ${COMPONENT_DIR}E:/HK9/PBL5/PBL5-Smart-Light-Project/device_firmware/components/managed_components/espressif__led_strip
                        )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(broadcast_discovery)
