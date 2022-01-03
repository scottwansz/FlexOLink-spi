# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ICM20948_DRIVER)
  # Add ad4696_driver.h to the set of global include paths.
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    icm20948.c
  )
endif()
