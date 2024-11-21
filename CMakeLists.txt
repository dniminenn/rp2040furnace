cmake_minimum_required(VERSION 3.13)

include(/usr/share/pico-sdk/pico_sdk_init.cmake)

project(furnace_controller C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(furnace_controller
    src/main.c
    src/max6675.c
    src/relays.c
    src/wifi.c
)

target_include_directories(furnace_controller PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(furnace_controller
    pico_stdlib
    hardware_spi
    pico_cyw43_arch_lwip_threadsafe_background
    pico_multicore
)

pico_enable_stdio_usb(furnace_controller 1)
pico_enable_stdio_uart(furnace_controller 0)

pico_add_extra_outputs(furnace_controller)
