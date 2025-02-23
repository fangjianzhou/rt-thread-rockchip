/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-11     GuEe-GUI     the first version
 */

#include <rtthread.h>

#if defined(RT_USING_POSIX_DEVIO) && defined(RT_USING_SMART)
#include <console.h>
#endif

#ifdef FINSH_USING_MSH

static int console(int argc, char **argv)
{
    rt_err_t result = RT_EOK;

    if (argc > 1)
    {
        if (!rt_strcmp(argv[1], "set"))
        {
            rt_kprintf("console change to %s\n", argv[2]);
            rt_console_set_device(argv[2]);

        #ifdef RT_USING_POSIX_DEVIO
            {
                rt_device_t dev = rt_device_find(argv[2]);

                if (dev != RT_NULL)
                {
                #ifdef RT_USING_SMART
                    console_set_iodev(dev);
                #else
                    rt_kprintf("TODO not supported\n");
                #endif
                }
            }
        #else
            finsh_set_device(argv[2]);
        #endif /* RT_USING_POSIX_DEVIO */
        }
        else
        {
            rt_kprintf("Unknown command. Please enter 'console' for help\n");
            result = -RT_ERROR;
        }
    }
    else
    {
        rt_kprintf("Usage: \n");
        rt_kprintf("console set <name>   - change console by name\n");
        result = -RT_ERROR;
    }
    return result;
}
MSH_CMD_EXPORT(console, set console name);

#endif /* FINSH_USING_MSH */
