/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <syscall_handler.h>
#include <drivers/spi.h>
#include <logging/log.h>

#include "ad4696.h"

LOG_MODULE_REGISTER(ad4696, CONFIG_AD4696_DRIVER_LOG_LEVEL);

#define MY_SPIM DT_NODELABEL(spi0)

/**
 * This is a minimal example of an out-of-tree driver
 * implementation. See the header file of the same name for details.
 */

static struct ad4696_dev_data {
	uint32_t foo;
} data;


const struct device * dev_ad4696;

static struct spi_cs_control spim_cs_one = { 
                        .gpio_pin = DT_GPIO_PIN(MY_SPIM, cs_gpios),
                        .gpio_dt_flags = GPIO_ACTIVE_LOW,
                        .delay = 0,
};

static const struct spi_config spi_cfg = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
		     SPI_MODE_CPOL | SPI_MODE_CPHA,
	.frequency = 4000000,
	.slave = 0,
	.cs = &spim_cs_one,	
};


static void spi_config(void)
{
	spim_cs_one.gpio_dev = device_get_binding(DT_GPIO_LABEL(MY_SPIM, cs_gpios));
   
  	if (spim_cs_one.gpio_dev != NULL){
		dev_ad4696 = device_get_binding(DT_LABEL(MY_SPIM)); // recommended syntax

		if (dev_ad4696 != NULL){
		LOG_INF("AD4696 Device: %s", DT_PROP(MY_SPIM,label));
		LOG_INF("AD4696 CSN PIN %d, MISO PIN %d, MOSI PIN %d, CLK PIN %d",
						DT_GPIO_PIN(MY_SPIM, cs_gpios),
						DT_PROP(MY_SPIM, miso_pin),
						DT_PROP(MY_SPIM, mosi_pin),
						DT_PROP(MY_SPIM, sck_pin)); 
		} //(dev_ad4696 != NULL)
		else 
		LOG_ERR("Can NOT get SPIM Device: %s", DT_PROP(MY_SPIM,label));
	} //(spim_cs_one.gpio_dev != NULL)
	else
		LOG_ERR("Could not get GPIO LABEL %s device", DT_GPIO_LABEL(MY_SPIM, cs_gpios));
}


static int init(const struct device *dev)
{
	data.foo = 5;
	spi_config();
	return 0;
}


// spi read and write
static int spi_rw(
        uint8_t *opcode, 
        size_t  opcode_size,
        uint8_t *data,
        size_t  data_size
    )
{
    bool isWREG = ( opcode[0] >= 0x80 ? false : true );
	LOG_INF("isWRG: %s", isWREG ? "true" : "false");

    LOG_INF("SPI opcode: %#02x %02x", opcode[0], opcode[1]);
	// for (int i = 0; i < opcode_size; i++)
	// {
	// 	printk("%#x ", opcode[i]);
	// }

	const struct spi_buf buf[2] = {
		{
			.buf = opcode,
			.len = opcode_size
		},
		{
			.buf = data,
			.len = data_size
		}
	};

	const struct spi_buf_set tx = {
		.buffers = buf,
		.count = isWREG ? 2 : 1
	};

	const struct spi_buf_set rx = {
		.buffers = buf,
		.count = isWREG ? 0 : 2
	};

    
    int err = spi_transceive(dev_ad4696, &spi_cfg, &tx, &rx);

	if (err >= 0) {

        // LOG_INF("SPI success: %d", err);

    } else {

		LOG_ERR("SPI error: %d", err);
	};

    return err;
};



static void print_impl(const struct device *dev)
{
	LOG_INF("Hello World from the AD4696: %d", data.foo);

	uint8_t opcode[] = {0x80, 0x0c}, data[2];
	LOG_INF("size of opcode: %d", sizeof(opcode));

	spi_rw(opcode, sizeof(opcode), data, sizeof(data));
	LOG_INF("spi data: %#02x %02x", data[0], data[1]);


	__ASSERT(data.foo == 5, "Device was not initialized!");
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_ad4696_print(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_AD4696(dev, print));

	z_impl_ad4696_print(dev);
}
#include <syscalls/ad4696_print_mrsh.c>
#endif /* CONFIG_USERSPACE */


DEVICE_DEFINE(ad4696, "ADI_AD4696",
		    init, NULL, &data, NULL,
		    APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &((struct ad4696_driver_api){ .print = print_impl }));
