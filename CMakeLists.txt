# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoGPU C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1


file(GLOB_RECURSE DECODER "${CMAKE_CURRENT_LIST_DIR}/mcufont/decoder/*.c")
file(GLOB_RECURSE ENCODER "${CMAKE_CURRENT_LIST_DIR}/mcufont/encoder/*.c")

add_executable(picoGPU picoGPU.cpp Drawing.cpp I2CHandling.cpp ${DECODER} ${ENCODER})

pico_set_program_name(picoGPU "picoGPU")
pico_set_program_version(picoGPU "0.1")

# Generate PIO header
pico_generate_pio_header(picoGPU ${CMAKE_CURRENT_LIST_DIR}/dac_out.pio)

pico_generate_pio_header(picoGPU ${CMAKE_CURRENT_LIST_DIR}/sync.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picoGPU 1)
pico_enable_stdio_usb(picoGPU 0)

# Add the standard library to the build
target_link_libraries(picoGPU
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(picoGPU PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/mcufont
        ${CMAKE_CURRENT_LIST_DIR}/mcufont/decoder
        ${CMAKE_CURRENT_LIST_DIR}/mcufont/encoder
        ${CMAKE_CURRENT_LIST_DIR}/mcufont/fonts
)

# Add any user requested libraries
target_link_libraries(picoGPU 
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        hardware_adc
        pico_multicore
        )

pico_add_extra_outputs(picoGPU)

