#include <drivers/spi.h>
#include <logging/log.h>

#include "icm20948.h"

LOG_MODULE_REGISTER(icm20948, CONFIG_ICM20948_DRIVER_LOG_LEVEL);

#define MY_SPIM DT_NODELABEL(spi0)

const struct device * device_spi;

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
		device_spi = device_get_binding(DT_LABEL(MY_SPIM)); // recommended syntax

		if (device_spi != NULL){
		printk("\nICM20948 Device: %s\n", DT_PROP(MY_SPIM,label));
		printk("ICM20948 CSN PIN %d, MISO PIN %d, MOSI PIN %d, CLK PIN %d\n",
						DT_GPIO_PIN(MY_SPIM, cs_gpios),
						DT_PROP(MY_SPIM, miso_pin),
						DT_PROP(MY_SPIM, mosi_pin),
						DT_PROP(MY_SPIM, sck_pin)); 
		} //(device_spi != NULL)
		else 
		printk("\nCan NOT get SPIM Device: %s\n", DT_PROP(MY_SPIM,label));
	} //(spim_cs_one.gpio_dev != NULL)
	else
		printk("Could not get GPIO LABEL %s device\n", DT_GPIO_LABEL(MY_SPIM, cs_gpios));
}


static int spi_rw(	// spi read and write
        uint8_t *opcode, 
        size_t  opcode_size,
        uint8_t *data,
        size_t  data_size
    )
{
    bool isWREG = ( opcode[0] >= 0x80 ? false : true );
	printk("\nisWRG: %s", isWREG ? "true" : "false");

    printk("\nSPI sent: ");
	for (int i = 0; i < opcode_size; i++)
	{
		printk("%#x ", opcode[i]);
	}
    printk("\n");

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

    
    int err = spi_transceive(device_spi, &spi_cfg, &tx, &rx);

	if (err >= 0) {

        // printk("\nSPI success: %d", err);
		printk("\nSPI data: ");
		for (size_t i = 0; i < data_size; i++)
		{
			printk("%#x ", data[i]);
		}

    } else {

		printk("\nSPI error: %d", err);
	};

    return err;
};


void icm20948_setup(void)
{
	spi_config();

	uint8_t tx_buffer[4] = {0x06, 0x01, 0x00, 0x00};	// disable sleep mode, enable gyro and accl
	// uint8_t rx_buffer[0];

	spi_rw(tx_buffer, sizeof(tx_buffer), NULL, 0); // rx_buffer, sizeof(rx_buffer)
}


void icm20948_fetch_data(){

    uint8_t opcode[1] = {
		// 0x00 | 0x80,
		// 0x05 | 0x80,	// out: 0x40
        0x2d | 0x80,	// out: acc_x_h
        // 0x34 | 0x80
    };  // 0x80: read register

    uint8_t data[12];
    spi_rw(opcode, sizeof(opcode), data, sizeof(data));   

};