/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <syscall_handler.h>
#include <drivers/spi.h>
#include <logging/log.h>
#include <drivers/sensor.h>

#include "ad4696.h"

LOG_MODULE_REGISTER(ad4696, CONFIG_AD4696_DRIVER_LOG_LEVEL);

#define MY_SPIM DT_NODELABEL(spi1)

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


// spi read and write
static int spi_rw(
        uint8_t *opcode, 
        size_t  opcode_size,
        uint8_t *data,
        size_t  data_size
    )
{
    bool isWREG = (opcode_size > 0) && (opcode[0] < 0x80);
	// LOG_INF("op_size: %d", opcode_size);
	LOG_INF("isWRG: %s", isWREG ? "true" : "false");

	if( opcode_size > 0 ){
	    LOG_INF("SPI opcode: %#02x %02x", opcode[0], opcode[1]);

		if(isWREG){
			LOG_INF("write data: %#x", data[0]);
		}
	}

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


static int init(const struct device *dev)
{
	data.foo = 5;
	spi_config();

	return 0;
};


static void setup_impl(const struct device *dev){
	// enable temprature
	uint8_t opcode[] = {0x0, 0x29}, data[] = {0x01};
	spi_rw(opcode, sizeof(opcode), data, sizeof(data));

	// config standard sequence
	uint8_t opcode_cfg_seq[] = {0x0, 0x24}, data_cf_seq[] = {0xff, 0xff};
	spi_rw(opcode_cfg_seq, sizeof(opcode_cfg_seq), data_cf_seq, sizeof(data_cf_seq));

	// enable conversion mode
	uint8_t opcode2[] = {0x0, 0x20}, data2[] = {0x14};
	spi_rw(opcode2, sizeof(opcode2), data2, sizeof(data2));
};


static void fetch_data_impl(const struct device *dev, struct sensor_value *values){

	uint8_t data[34];
	spi_rw(NULL, 0, data, sizeof(data));
	// LOG_INF("Sample data: %#x %#x %#x %#x", data[0], data[1], data[2], data[3]);

	// int16_t code = 0;
	// double	volt = 0;

	LOG_INF("sample data: ");
	for (size_t i = 0; i < sizeof(data) / 2; i++)
	{
		printk("[%d] %#x ", i, data[i*2] << 8 | data[i*2 + 1]);
		// code = (sample_data[i*2] << 8) | sample_data[i*2 + 1];
		// volt = code / 0x8000 * 2.5;	// volt(V)
		// sensor_value_from_double(&values[i], volt);

		// printk("[%d] %#x %d ", i, (sample_data[i*2] << 8) | sample_data[i*2 + 1], (int16_t)(volt * 1000000));
		// printk("[%d] %d ", i, (uint16_t)(volt * 1000000));
	}
	printk("\n");	
};


static void print_impl(const struct device *dev)
{
	// LOG_INF("Hello World from the AD4696: %d", data.foo);

	uint8_t opcode[2] = {0x80, 0x03}, data[4];
	spi_rw(opcode, sizeof(opcode), data, sizeof(data));
	LOG_INF("SPI data: %#x %#x %#x %#x", data[0], data[1], data[2], data[3]);

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
		    &((struct ad4696_driver_api){ 
				.print = print_impl,
				.setup = setup_impl,
				.fetch_data = fetch_data_impl
			}));
