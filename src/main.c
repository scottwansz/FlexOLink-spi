/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>


#include "../icm20948/icm20948.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

void main(void)
{
	
	printk("SPI config begin ...\n");
	spi_config();

	printk("Blinky with SPI started\n");

	while (1) {
		spi_test_send();
		printk("print in while loop\n");
		k_msleep(SLEEP_TIME_MS);
	}
}
