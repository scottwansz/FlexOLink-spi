/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BLE_SERVICE_H__
#define __BLE_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

/*Note that UUIDs have Least Significant Byte ordering */
#define MY_SERVICE_UUID 0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x75

#define RX_CHARACTERISTIC_UUID 0xA6, 0xE8, 0xC4, 0x60, 0x7E, 0xAA, 0x41, 0x6B, \
0x95, 0xD4, 0x9D, 0xCC, 0x08, 0x4F, 0xCF, 0x6A

#define TX_CHARACTERISTIC_UUID 0xED, 0xAA, 0x20, 0x11, 0x92, 0xE7, 0x43, 0x5A, \
0xAA, 0xE9, 0x94, 0x43, 0x35, 0x6A, 0xD4, 0xD3

extern bool ble_ready;

__subsystem struct ble_service_api {
	void (*print)(const struct device *dev);
	void (*send)(const struct device *dev, const uint8_t *data, uint16_t len);
};

__syscall     void        ble_service_print(const struct device *dev);
static inline void z_impl_ble_service_print(const struct device *dev)
{
	const struct ble_service_api *api = dev->api;

	__ASSERT(api->print, "Callback pointer should not be NULL");

	api->print(dev);
}

__syscall     void        ble_service_send(const struct device *dev, const uint8_t *data, uint16_t len);
static inline void z_impl_ble_service_send(const struct device *dev, const uint8_t *data, uint16_t len)
{
	const struct ble_service_api *api = dev->api;

	__ASSERT(api->init, "Callback pointer should not be NULL");

	api->send(dev, data, len);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/ble_service.h>

#endif /* __BLE_SERVICE_H__ */
