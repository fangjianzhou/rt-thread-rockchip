/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-20     ErikChan     the first version
 */

#include <rtthread.h>

#ifdef RT_USING_OFW
#include <drivers/ofw_io.h>
#include <drivers/ofw_irq.h>
#endif
#include <drivers/core/rtdm.h>

#ifdef RT_USING_SMP
static int rti_secondary_cpu_start(void)
{
    return 0;
}
INIT_EXPORT(rti_secondary_cpu_start, "6.end");

static int rti_secondary_cpu_end(void)
{
    return 0;
}
INIT_EXPORT(rti_secondary_cpu_end, "7.end");

void rt_dm_secondary_cpu_init(void)
{
#if RT_DEBUG_INIT
    int result;
    const struct rt_init_desc *desc;

    rt_kprintf("do secondary cpu initialization.\n");
    for (desc = &__rt_init_desc_rti_secondary_cpu_start; desc < &__rt_init_desc_rti_secondary_cpu_end; ++desc)
    {
        rt_kprintf("initialize %s", desc->fn_name);
        result = desc->fn();
        rt_kprintf(":%d done\n", result);
    }
#else
    volatile const init_fn_t *fn_ptr;

    for (fn_ptr = &__rt_init_rti_secondary_cpu_start; fn_ptr < &__rt_init_rti_secondary_cpu_end; ++fn_ptr)
    {
        (*fn_ptr)();
    }
#endif /* RT_DEBUG_INIT */
}
#endif /* RT_USING_SMP */

struct prefix_track
{
    rt_list_t list;

    int uid;
    const char *prefix;
};
static struct rt_spinlock _prefix_nodes_lock = { 0 };
static rt_list_t _prefix_nodes = RT_LIST_OBJECT_INIT(_prefix_nodes);

int rt_dm_dev_set_name_auto(rt_device_t dev, const char *prefix)
{
    int uid = -1;
    struct prefix_track *pt = RT_NULL;

    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(prefix != RT_NULL);

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_spin_lock(&_prefix_nodes_lock);

    rt_list_for_each_entry(pt, &_prefix_nodes, list)
    {
        /* caller always input constants string, check ptr is faster */
        if (pt->prefix == prefix || !rt_strcmp(pt->prefix, prefix))
        {
            uid = ++pt->uid;
            break;
        }
    }

    rt_spin_unlock(&_prefix_nodes_lock);

    if (uid < 0)
    {
        pt = rt_malloc(sizeof(*pt));

        if (!pt)
        {
            return -RT_ENOMEM;
        }

        rt_list_init(&pt->list);

        pt->uid = uid = 0;
        pt->prefix = prefix;

        rt_spin_lock(&_prefix_nodes_lock);

        rt_list_insert_before(&_prefix_nodes, &pt->list);

        rt_spin_unlock(&_prefix_nodes_lock);
    }

    return rt_dm_dev_set_name(dev, "%s%u", prefix, uid);
}

int rt_dm_dev_get_name_id(rt_device_t dev)
{
    int id = 0, len;
    const char *name;

    RT_ASSERT(dev != RT_NULL);

    name = rt_dm_dev_get_name(dev);
    len = rt_strlen(name) - 1;
    name += len;

    while (len --> 0)
    {
        if (*name < '0' || *name > '9')
        {
            while (*(++name))
            {
                id *= 10;
                id += *name - '0';
            }

            break;
        }

        --name;
    }

    return id;
}

int rt_dm_dev_set_name(rt_device_t dev, const char *format, ...)
{
    int n;
    va_list arg_ptr;

    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(format != RT_NULL);

    va_start(arg_ptr, format);
    n = rt_vsnprintf(dev->parent.name, RT_NAME_MAX, format, arg_ptr);
    va_end(arg_ptr);

    return n;
}

const char *rt_dm_dev_get_name(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    return dev->parent.name;
}

#ifdef RT_USING_OFW
#define ofw_api_call(name, ...)     rt_ofw_##name(__VA_ARGS__)
#define ofw_api_call_ptr(name, ...) ofw_api_call(name, __VA_ARGS__)
#else
#define ofw_api_call(name, ...)     (-RT_ENOSYS)
#define ofw_api_call_ptr(name, ...) RT_NULL
#endif

int rt_dm_dev_get_address_count(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_address_count, dev->ofw_node);
    }

    return -RT_ENOSYS;
}

rt_err_t rt_dm_dev_get_address(rt_device_t dev, int index,
        rt_uint64_t *out_address, rt_uint64_t *out_size)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_address, dev->ofw_node, index,
                out_address, out_size);
    }

    return -RT_ENOSYS;
}

rt_err_t rt_dm_dev_get_address_by_name(rt_device_t dev, const char *name,
        rt_uint64_t *out_address, rt_uint64_t *out_size)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_address_by_name, dev->ofw_node, name,
                out_address, out_size);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_get_address_array(rt_device_t dev, int nr, rt_uint64_t *out_regs)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_address_array, dev->ofw_node, nr, out_regs);
    }

    return -RT_ENOSYS;
}

void *rt_dm_dev_iomap(rt_device_t dev, int index)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call_ptr(iomap, dev->ofw_node, index);
    }

    return RT_NULL;
}

void *rt_dm_dev_iomap_by_name(rt_device_t dev, const char *name)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call_ptr(iomap_by_name, dev->ofw_node, name);
    }

    return RT_NULL;
}

int rt_dm_dev_get_irq_count(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_irq_count, dev->ofw_node);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_get_irq(rt_device_t dev, int index)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_irq, dev->ofw_node, index);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_get_irq_by_name(rt_device_t dev, const char *name)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(get_irq_by_name, dev->ofw_node, name);
    }

    return -RT_ENOSYS;
}

void rt_dm_dev_bind_fwdata(rt_device_t dev, void *fw_np, void *data)
{
    RT_ASSERT(dev != RT_NULL);

    if (!dev->ofw_node && fw_np)
    {
    #ifdef RT_USING_OFW
        dev->ofw_node = fw_np;
        rt_ofw_data(fw_np) = data;
    #endif
    }

    RT_ASSERT(dev->ofw_node != RT_NULL);

#ifdef RT_USING_OFW
    rt_ofw_data(dev->ofw_node) = data;
#endif
}

void rt_dm_dev_unbind_fwdata(rt_device_t dev, void *fw_np)
{
    void *dev_fw_np = RT_NULL;

    RT_ASSERT(dev != RT_NULL);

    if (!dev->ofw_node && fw_np)
    {
    #ifdef RT_USING_OFW
        dev_fw_np = fw_np;
        rt_ofw_data(fw_np) = RT_NULL;
    #endif
    }

    RT_ASSERT(dev_fw_np != RT_NULL);

#ifdef RT_USING_OFW
    rt_ofw_data(dev_fw_np) = RT_NULL;
#endif
}

int rt_dm_dev_prop_read_u8_array_index(rt_device_t dev, const char *propname,
        int index, int nr, rt_uint8_t *out_values)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_read_u8_array_index, dev->ofw_node, propname,
                index, nr, out_values);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_prop_read_u16_array_index(rt_device_t dev, const char *propname,
        int index, int nr, rt_uint16_t *out_values)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_read_u16_array_index, dev->ofw_node, propname,
                index, nr, out_values);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_prop_read_u32_array_index(rt_device_t dev, const char *propname,
        int index, int nr, rt_uint32_t *out_values)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_read_u32_array_index, dev->ofw_node, propname,
                index, nr, out_values);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_prop_read_u64_array_index(rt_device_t dev, const char *propname,
        int index, int nr, rt_uint64_t *out_values)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_read_u64_array_index, dev->ofw_node, propname,
                index, nr, out_values);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_prop_read_string_array_index(rt_device_t dev, const char *propname,
        int index, int nr, const char **out_strings)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_read_string_array_index, dev->ofw_node, propname,
                index, nr, out_strings);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_prop_count_of_size(rt_device_t dev, const char *propname, int size)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_count_of_size, dev->ofw_node, propname, size);
    }

    return -RT_ENOSYS;
}

int rt_dm_dev_prop_index_of_string(rt_device_t dev, const char *propname, const char *string)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_index_of_string, dev->ofw_node, propname, string);
    }

    return -RT_ENOSYS;
}

rt_bool_t rt_dm_dev_prop_read_bool(rt_device_t dev, const char *propname)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ofw_node)
    {
        return ofw_api_call(prop_read_bool, dev->ofw_node, propname);
    }

    return RT_FALSE;
}
