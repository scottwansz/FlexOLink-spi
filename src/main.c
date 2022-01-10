/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include <drivers/sensor.h>

#include "ad4696.h"
#include "icm20948.h"

LOG_MODULE_REGISTER(main);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

const struct device *dev;
struct sensor_value ad4696_data[17];

static void user_entry(void *p1, void *p2, void *p3)
{

	ad4696_setup(dev);
	ad4696_print(dev);
	
	while (true)
	{
		k_msleep(2000);
		// ad4696_fetch_data(dev, ad4696_data);

		// LOG_INF("AD4696 data: ");
		// for (size_t i = 0; i < 17; i++)
		// {
		// 	printk("[%d]%d ", i, (int16_t)(sensor_value_to_double( &ad4696_data[i] ) * 1000000) ); // volt(µV)
		// }
		// printk("\n");
	}
}


void main(void)
{
	// LOG_INF("SPI config begin ...");
	// icm20948_setup();
	// LOG_INF("Blinky with SPI started");

	// while (1) {
	// 	// printk("print in while loop\n");
	// 	icm20948_fetch_data();
	// 	k_msleep(1000);
	// }



	printk("Hello World from the app!\n");

	dev = device_get_binding("ADI_AD4696");

	__ASSERT(dev, "Failed to get device binding");

	LOG_INF("device is %p, name is %s", dev, dev->name);

	k_object_access_grant(dev, k_current_get());
	k_thread_user_mode_enter(user_entry, NULL, NULL, NULL);
}
