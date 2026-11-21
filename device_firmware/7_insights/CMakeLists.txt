# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(IDF_COMPONENT_MANAGER 0)

add_compile_options(-fdiagnostics-color=always)

# Determine whether to define RAIMAKER_PATH
# set(RAIMAKER_PATH "E:/HK9/PBL5/PBL5-Smart-Light-Project/device_firmware/components/esp-rainmaker/components/esp_rainmaker")


# Add RainMaker components and other common application components
set(EXTRA_COMPONENT_DIRS 
                        "${CMAKE_CURRENT_LIST_DIR}/../components/managed_components"
                        "${CMAKE_CURRENT_LIST_DIR}/../components/esp-rainmaker/components"
                        "${CMAKE_CURRENT_LIST_DIR}/../components"
                        )

set(EXCLUDE_COMPONENTS "coap" "asio")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(7_insights)
