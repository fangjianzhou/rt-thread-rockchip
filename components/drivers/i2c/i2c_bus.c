/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 
 */

#include "i2c_dm.h"

#define DBG_TAG "i2c.bus"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static struct rt_bus i2c_bus;

void i2c_bus_scan_clients(struct rt_i2c_bus_device *bus)
{
#ifdef RT_USING_OFW
    if (bus->parent.ofw_node)
    {
        struct rt_ofw_node *np = bus->parent.ofw_node, *i2c_client_np;

        rt_ofw_foreach_available_child_node(np, i2c_client_np)
        {
            rt_uint32_t client_addr;
            struct rt_i2c_client *client;

            if (!rt_ofw_prop_read_bool(i2c_client_np, "compatible"))
            {
                continue;
            }

            client = rt_calloc(1, sizeof(*client));

            if (!client)
            {
                LOG_E("Not memory to create i2c client: %s",
                        rt_ofw_node_full_name(i2c_client_np));

                return;
            }

            rt_ofw_prop_read_u32(i2c_client_np, "reg", &client_addr);

            client->parent.ofw_node = i2c_client_np;
            client->name = rt_ofw_node_name(i2c_client_np);
            client->bus = bus;
            client->client_addr = client_addr;

            rt_i2c_device_register(client);
        }
    }
#endif /* RT_USING_OFW */
}

rt_err_t rt_i2c_driver_register(struct rt_i2c_driver *driver)
{
    RT_ASSERT(driver != RT_NULL);

    driver->parent.bus = &i2c_bus;

    return rt_driver_register(&driver->parent);
}

rt_err_t rt_i2c_device_register(struct rt_i2c_client *client)
{
    RT_ASSERT(client != RT_NULL);

    return rt_bus_add_device(&i2c_bus, &client->parent);
}

static rt_bool_t i2c_match(rt_driver_t drv, rt_device_t dev)
{
    const struct rt_i2c_device_id *id;
    struct rt_i2c_driver *driver = rt_container_of(drv, struct rt_i2c_driver, parent);
    struct rt_i2c_client *client = rt_container_of(dev, struct rt_i2c_client, parent);

    if ((id = driver->ids))
    {
        for (; id->name[0]; ++id)
        {
            if (!rt_strcmp(id->name, client->name))
            {
                client->id = id;
                client->ofw_id = RT_NULL;

                return RT_TRUE;
            }
        }
    }

#ifdef RT_USING_OFW
    client->ofw_id = rt_ofw_node_match(client->parent.ofw_node, driver->ofw_ids);

    if (client->ofw_id)
    {
        client->id = RT_NULL;

        return RT_TRUE;
    }
#endif

    return RT_FALSE;
}

static rt_err_t i2c_probe(rt_device_t dev)
{
    rt_err_t err;
    struct rt_i2c_driver *driver = rt_container_of(dev->drv, struct rt_i2c_driver, parent);
    struct rt_i2c_client *client = rt_container_of(dev, struct rt_i2c_client, parent);

    if (!client->bus)
    {
        return -RT_EINVAL;
    }

    err = driver->probe(client);

    return err;
}

static struct rt_bus i2c_bus =
{
    .name = "i2c",
    .match = i2c_match,
    .probe = i2c_probe,
};

static int i2c_bus_init(void)
{
    rt_bus_register(&i2c_bus);

    return 0;
}
INIT_CORE_EXPORT(i2c_bus_init);
