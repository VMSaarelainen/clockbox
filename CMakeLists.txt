cmake_minimum_required(VERSION 3.5)
add_definitions( -D__ESP32_ESP32__ -D__DEBUG__)
#add_compile_options(-Wno-unused-function)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(clock)
