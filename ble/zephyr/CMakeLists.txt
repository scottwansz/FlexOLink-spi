# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BLE_SERVICE)
  # Add hello_world_driver.h to the set of global include paths.
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    ble_service.c
    )
endif()
