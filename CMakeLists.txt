# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(SUPPORTED_TARGETS esp32)

#set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/protocols/modbus/serial/mb_example_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(power_meter)

target_add_binary_data(power_meter.elf "main/mqtt_server.pem" TEXT)
