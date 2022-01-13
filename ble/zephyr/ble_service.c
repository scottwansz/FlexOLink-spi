/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ble_service.h"
#include <zephyr/types.h>
#include <syscall_handler.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
// #include <bluetooth/gatt_dm.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(ble_service);

#define BT_UUID_MY_SERVICE      BT_UUID_DECLARE_128(MY_SERVICE_UUID)
#define BT_UUID_MY_SERVICE_RX   BT_UUID_DECLARE_128(RX_CHARACTERISTIC_UUID)
#define BT_UUID_MY_SERVICE_TX   BT_UUID_DECLARE_128(TX_CHARACTERISTIC_UUID)

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

#define MAX_TRANSMIT_SIZE 240

uint8_t data_rx[MAX_TRANSMIT_SIZE];
uint8_t data_tx[MAX_TRANSMIT_SIZE];

static K_SEM_DEFINE(ble_init_ok, 0, 1);
bool ble_ready = false;

static const struct bt_data ad[] = 
{
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = 
{
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, MY_SERVICE_UUID),
};

struct bt_conn *my_connection;

static void exchange_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params)
{
	if (!err) {
		printk("MTU exchange done");
	} else {
		printk("MTU exchange failed (err %" PRIu8 ")", err);
	}
};


static void connected(struct bt_conn *conn, uint8_t err)
{
	struct bt_conn_info info; 
	char addr[BT_ADDR_LE_STR_LEN];

	my_connection = conn;

	if (err) 
	{
		printk("Connection failed (err %u)\n", err);
		return;
	}
	else if(bt_conn_get_info(conn, &info))
	{
		printk("Could not parse connection info\n");
	}
	else
	{
		bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
		
		printk("Connection established!		\n\
		Connected to: %s					\n\
		Role: %u							\n\
		Connection interval: %u				\n\
		Slave latency: %u					\n\
		Connection supervisory timeout: %u	\n"
		, addr, info.role, info.le.interval, info.le.latency, info.le.timeout);
	};


	static struct bt_gatt_exchange_params exchange_params;

	exchange_params.func = exchange_func;
	int e = bt_gatt_exchange_mtu(my_connection, &exchange_params);
	if (e) {
		printk("MTU exchange failed (err %d)", e);
	}

}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	//If acceptable params, return true, otherwise return false.
	return true; 
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
	struct bt_conn_info info; 
	char addr[BT_ADDR_LE_STR_LEN];
	
	if(bt_conn_get_info(conn, &info))
	{
		printk("Could not parse connection info\n");
	}
	else
	{
		bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
		
		printk("Connection parameters updated!	\n\
		Connected to: %s						\n\
		New Connection Interval: %u				\n\
		New Slave Latency: %u					\n\
		New Connection Supervisory Timeout: %u	\n"
		, addr, info.le.interval, info.le.latency, info.le.timeout);
	}
}

static struct bt_conn_cb conn_callbacks = 
{
	.connected				= connected,
	.disconnected   		= disconnected,
	.le_param_req			= le_param_req,
	.le_param_updated		= le_param_updated
};

static void bt_ready(int err)
{
	if (err) 
	{
		printk("BLE init failed with error code %d\n", err);
		return;
	}

	//Configure connection callbacks
	bt_conn_cb_register(&conn_callbacks);

	//Start advertising
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) 
	{
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");

	k_sem_give(&ble_init_ok);
}


static void error(void)
{
	while (true) {
		printk("Error!\n");
		/* Spin for ever */
		k_sleep(K_MSEC(1000)); //1000ms
	}
}

static struct ble_dev_data {
	uint32_t foo;
} data;


static int init(const struct device *dev)
{
	data.foo = 5;

	int err = 0;

	printk("Starting Nordic BLE peripheral tutorial\n");

	
	err = bt_enable(bt_ready);

	if (err) 
	{
		printk("BLE initialization failed\n");
		error(); //Catch error
		return err;
	}
	
	/* 	Bluetooth stack should be ready in less than 100 msec. 								\
																							\
		We use this semaphore to wait for bt_enable to call bt_ready before we proceed 		\
		to the main loop. By using the semaphore to block execution we allow the RTOS to 	\
		execute other tasks while we wait. */	
	err = k_sem_take(&ble_init_ok, K_MSEC(500));

	if (!err) 
	{
		printk("Bluetooth initialized\n");
	} else 
	{
		printk("BLE initialization did not complete in time\n");
		error(); //Catch error
		return err;
	}

	
	// int err = 0;
    memset(&data_rx, 0, MAX_TRANSMIT_SIZE);
    memset(&data_tx, 0, MAX_TRANSMIT_SIZE);

	// if (err) 
	// {
	// 	printk("Failed to init LBS (err:%d)\n", err);
	// 	return err;
	// } else {
	// 	printk("ble service inited in bt_init().\n");
	// }

	return err;
}

static void print_impl(const struct device *dev)
{
	printk("Hello World from BLE Service: %d\n", data.foo);

	__ASSERT(data.foo == 5, "Device was not initialized!");
}


#ifdef CONFIG_USERSPACE
static inline void z_vrfy_hello_world_print(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/hello_world_print_mrsh.c>
#endif /* CONFIG_USERSPACE */


/* This function is called whenever the RX Characteristic has been written to by a Client */
static ssize_t on_receive(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr,
			  const void *buf,
			  uint16_t len,
			  uint16_t offset,
			  uint8_t flags)
{
    const uint8_t * buffer = buf;
    
	printk("Received data, handle %d, conn %p, data: 0x", attr->handle, conn);
    for(uint8_t i = 0; i < len; i++){
        printk("%02X", buffer[i]);
    }
    printk("\n");

	return len;
}

/* This function is called whenever a Notification has been sent by the TX Characteristic */
static void on_sent(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(user_data);

    // const bt_addr_le_t * addr = bt_conn_get_dst(conn);
    
    // LOG_INF("data sent");
	// printk("Data sent to Address 0x %02X %02X %02X %02X %02X %02X \n", addr->a.val[0]
    //                                                                 , addr->a.val[1]
    //                                                                 , addr->a.val[2]
    //                                                                 , addr->a.val[3]
    //                                                                 , addr->a.val[4]
    //                                                                 , addr->a.val[5]);
}


/* This function is called whenever the CCCD register has been changed by the client*/
void on_cccd_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    switch(value)
    {
        case BT_GATT_CCC_NOTIFY: 
            // Start sending stuff!
            ble_ready = true;

            LOG_INF("Start sending stuff! BLE ready: %s", ble_ready ? "True" : "False");
            break;

        case BT_GATT_CCC_INDICATE: 
            // Start sending stuff via indications
            LOG_INF("Start sending stuff via indications");
            break;

        case 0: 
            // Stop sending stuff
            break;
        
        default: 
            printk("Error, CCCD has been set to an invalid value");     
    }
}


/* LED Button Service Declaration and Registration */
BT_GATT_SERVICE_DEFINE(my_service,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_MY_SERVICE),
    BT_GATT_CHARACTERISTIC(BT_UUID_MY_SERVICE_RX,
                    BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, 
                    NULL, on_receive, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_MY_SERVICE_TX,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed,
            BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);



static void send_impl(const struct device *dev, const uint8_t *data, uint16_t len)
{
	printk("Data send from BLE Service\n");

	/* 
    The attribute for the TX characteristic is used with bt_gatt_is_subscribed 
    to check whether notification has been enabled by the peer or not.
    Attribute table: 0 = Service, 1 = Primary service, 2 = RX, 3 = TX, 4 = CCC.
    */
    const struct bt_gatt_attr *attr = &my_service.attrs[3]; 

    struct bt_gatt_notify_params params = 
    {
        .uuid   = BT_UUID_MY_SERVICE_TX,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = on_sent
    };

    // Check whether notifications are enabled or not
    if(bt_gatt_is_subscribed(my_connection, attr, BT_GATT_CCC_NOTIFY)) 
    {
        // Send the notification
	    if(bt_gatt_notify_cb(my_connection, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled on the selected attribute\n");
    }

	// __ASSERT(data.foo == 5, "Device was not initialized!");
};


DEVICE_DEFINE(ble_service, "BLE_SERVICE",
		    init, NULL, &data, NULL,
		    APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &((struct ble_service_api){ 
				.print = print_impl,
				.send = send_impl
			}));