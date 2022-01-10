/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __AD4696_DRIVER_H__
#define __AD4696_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>
#include <drivers/sensor.h>

/*
 * This driver has a 'print' syscall that prints the
 * famous 'Hello World!' string.
 *
 * The string is formatted with some internal driver data to
 * demonstrate that drivers are initialized during the boot process.
 *
 * The driver exists to demonstrate (and test) custom drivers that are
 * maintained outside of Zephyr.
 */


/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* AD469x registers */
#define AD469x_REG_IF_CONFIG_A		0x000
#define AD469x_REG_IF_CONFIG_B		0x001
#define AD469x_REG_DEVICE_TYPE		0x003
#define AD469x_REG_DEVICE_ID_L		0x004
#define AD469x_REG_DEVICE_ID_H		0x005
#define AD469x_REG_SCRATCH_PAD		0x00A
#define AD469x_REG_VENDOR_L		0x00C
#define AD469x_REG_VENDOR_H		0x00D
#define AD469x_REG_LOOP_MODE		0x00E
#define AD469x_REG_IF_CONFIG_C		0x010
#define AD469x_REG_IF_STATUS		0x011
#define AD469x_REG_STATUS		0x014
#define AD469x_REG_ALERT_STATUS1	0x015
#define AD469x_REG_ALERT_STATUS2	0x016
#define AD469x_REG_ALERT_STATUS3	0x017
#define AD469x_REG_ALERT_STATUS4	0x018
#define AD469x_REG_CLAMP_STATUS1	0x01A
#define AD469x_REG_CLAMP_STATUS2	0x01B
#define AD469x_REG_SETUP		0x020
#define AD469x_REG_REF_CTRL		0x021
#define AD469x_REG_SEQ_CTRL		0x022
#define AD469x_REG_AC_CTRL		0x023
#define AD469x_REG_STD_SEQ_CONFIG	0x024
#define AD469x_REG_GPIO_CTRL		0x026
#define AD469x_REG_GP_MODE		0x027
#define AD469x_REG_GPIO_STATE		0x028
#define AD469x_REG_TEMP_CTRL		0x029
#define AD469x_REG_CONFIG_IN(x)		((x & 0x0F) | 0x30)
#define AD469x_REG_AS_SLOT(x)		((x & 0x7F) | 0x100)

/* 5-bit SDI Conversion Mode Commands */
#define AD469x_CMD_REG_CONFIG_MODE		(0x0A << 3)
#define AD469x_CMD_SEL_TEMP_SNSOR_CH		(0x0F << 3)
#define AD469x_CMD_CONFIG_CH_SEL(x)		((0x10 | (0x0F & x)) << 3)

/* AD469x_REG_SETUP */
#define AD469x_SETUP_IF_MODE_MASK		(0x01 << 2)
#define AD469x_SETUP_IF_MODE_CONV		(0x01 << 2)
#define AD469x_SETUP_CYC_CTRL_MASK		(0x01 << 1)
#define AD469x_SETUP_CYC_CTRL_SINGLE(x)		((x & 0x01) << 1)
//Changed 
#define AD469x_SETUP_STATUSBIT_MODE_MASK		(0x01 << 5)
#define AD469x_SETUP_STATUSBIT_MODE_CONV		(0x01 << 5)

/* AD469x_REG_GP_MODE */
#define AD469x_GP_MODE_BUSY_GP_EN_MASK		(0x01 << 1)
#define AD469x_GP_MODE_BUSY_GP_EN(x)		((x & 0x01) << 1)
#define AD469x_GP_MODE_BUSY_GP_SEL_MASK		(0x01 << 4)
#define AD469x_GP_MODE_BUSY_GP_SEL(x)		((x & 0x01) << 4)

/* AD469x_REG_SEQ_CTRL */
#define AD469x_SEQ_CTRL_STD_SEQ_EN_MASK		(0x01 << 7)
#define AD469x_SEQ_CTRL_STD_SEQ_EN(x)		((x & 0x01) << 7)
#define AD469x_SEQ_CTRL_NUM_SLOTS_AS_MASK	(0x7f << 0)
#define AD469x_SEQ_CTRL_NUM_SLOTS_AS(x)		((x & 0x7f) << 0)

/* AD469x_REG_TEMP_CTRL */
#define AD469x_REG_TEMP_CTRL_TEMP_EN_MASK	(0x01 << 0)
#define AD469x_REG_TEMP_CTRL_TEMP_EN(x)		((x & 0x01) << 0)

/* AD469x_REG_AS_SLOT */
#define AD469x_REG_AS_SLOT_INX(x)		((x & 0x0f) << 0)

/* AD469x_REG_IF_CONFIG_C */
#define AD469x_REG_IF_CONFIG_C_MB_STRICT_MASK	(0x01 << 5)
#define AD469x_REG_IF_CONFIG_C_MB_STRICT(x)	((x & 0x01) << 5)

/* AD469x_REG_CONFIG_INn */
#define AD469x_REG_CONFIG_IN_OSR_MASK		(0x03 << 0)
#define AD469x_REG_CONFIG_IN_OSR(x)		((x & 0x03) << 0)
#define AD469x_REG_CONFIG_IN_HIZ_EN_MASK	(0x01 << 3)
#define AD469x_REG_CONFIG_IN_HIZ_EN(x)		((x & 0x01) << 3)
#define AD469x_REG_CONFIG_IN_PAIR_MASK		(0x03 << 4)
#define AD469x_REG_CONFIG_IN_PAIR(x)		((x & 0x03) << 4)
#define AD469x_REG_CONFIG_IN_MODE_MASK		(0x01 << 6)
#define AD469x_REG_CONFIG_IN_MODE(x)		((x & 0x01) << 6)
#define AD469x_REG_CONFIG_IN_TD_EN_MASK		(0x01 << 7)
#define AD469x_REG_CONFIG_IN_TD_EN(x)		((x & 0x01) << 7)

#define AD469x_CHANNEL(x)			(BIT(x) & 0xFFFF)
#define AD469x_CHANNEL_NO			16
#define AD469x_SLOTS_NO				0x80
#define AD469x_CHANNEL_TEMP			16

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/**
 * @enum ad469x_channel_sequencing
 * @brief Channel sequencing modes
 */
enum ad469x_channel_sequencing {
	/** Single cycle read */
	AD469x_single_cycle,
	/** Two cycle read */
	AD469x_two_cycle,
	/** Sequence trough channels, standard mode */
	AD469x_standard_seq,
	/** Sequence trough channels, advanced mode */
	AD469x_advanced_seq,
};

/**
  * @enum ad469x_busy_gp_sel
  * @brief Busy state, possible general purpose pin selections
  */
enum ad469x_busy_gp_sel {
	/** Busy on gp0 */
	AD469x_busy_gp0 = 0,
	/** Busy on gp3 */
	AD469x_busy_gp3 = 1,
};

/**
  * @enum ad469x_reg_access
  * @brief Register access modes
  */
enum ad469x_reg_access {
	AD469x_BYTE_ACCESS,
	AD469x_WORD_ACCESS,
};

/**
  * @enum ad469x_supported_dev_ids
  * @brief Supported devices
  */
enum ad469x_supported_dev_ids {
	ID_AD4695,
	ID_AD4696,
	ID_AD4697,
};

/**
  * @enum ad469x_osr_ratios
  * @brief Supported oversampling ratios
  */
enum ad469x_osr_ratios {
	AD469x_OSR_1,
	AD469x_OSR_4,
	AD469x_OSR_16,
	AD469x_OSR_64
};


__subsystem struct ad4696_driver_api {
	/* This struct has a member called 'print'. 'print' is function
	 * pointer to a function that takes 'struct device *dev' as an
	 * argument and returns 'void'.
	 */
	void (*print)(const struct device *dev);
	void (*setup)(const struct device *dev);
	void (*fetch_data)(const struct device *dev, struct sensor_value *values);
};

__syscall     void        ad4696_setup(const struct device *dev);
static inline void z_impl_ad4696_setup(const struct device *dev)
{
	const struct ad4696_driver_api *api = dev->api;

	__ASSERT(api->setup, "Callback pointer should not be NULL");

	api->setup(dev);
};


__syscall     void        ad4696_fetch_data(const struct device *dev, struct sensor_value *values);
static inline void z_impl_ad4696_fetch_data(const struct device *dev, struct sensor_value *values)
{
	const struct ad4696_driver_api *api = dev->api;

	__ASSERT(api->fetch_data, "Callback pointer should not be NULL");

	api->fetch_data(dev, values);
};

__syscall     void        ad4696_print(const struct device *dev);
static inline void z_impl_ad4696_print(const struct device *dev)
{
	const struct ad4696_driver_api *api = dev->api;

	__ASSERT(api->print, "Callback pointer should not be NULL");

	api->print(dev);
};

#ifdef __cplusplus
}
#endif

#include <syscalls/ad4696.h>

#endif /* __AD4696_DRIVER_H__ */
