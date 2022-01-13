list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/icm20948
  ${CMAKE_CURRENT_SOURCE_DIR}/ad4696
  ${CMAKE_CURRENT_SOURCE_DIR}/ble
)

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE 
    src/main.c
)
