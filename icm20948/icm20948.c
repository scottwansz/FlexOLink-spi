#include <drivers/spi.h>

#define MY_SPIM DT_NODELABEL(spi1)

const struct device * device_spi;

struct spi_cs_control spim_cs_one = { 
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

void spi_config(void)
{
	spim_cs_one.gpio_dev = device_get_binding(DT_GPIO_LABEL(MY_SPIM, cs_gpios));
   
  	if (spim_cs_one.gpio_dev != NULL){
		device_spi = device_get_binding(DT_LABEL(MY_SPIM)); // recommended syntax

		if (device_spi != NULL){
		printk("\nSPIM Device: %s\n", DT_PROP(MY_SPIM,label));
		printk("SPIM CSN PIN %d, MISO PIN %d, MOSI PIN %d, CLK PIN %d\n",
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

void spi_test_send(void)
{
	int err;
	static uint8_t tx_buffer[1] = {0x80};
	static uint8_t rx_buffer[2];

	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = sizeof(tx_buffer)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};

	err = spi_transceive(device_spi, &spi_cfg, &tx, &rx);
	if (err) {
		printk("SPI error: %d\n", err);
	} else {
		/* Connect MISO to MOSI for loopback */
		printk("SPI sent/received: %x/%x\n", tx_buffer[0], rx_buffer[1]);
		tx_buffer[0]++;
	}	
}