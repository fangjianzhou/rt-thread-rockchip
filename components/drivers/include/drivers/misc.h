/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-25     GuEe-GUI     the first version
 */

#ifndef __MISC_H__
#define __MISC_H__

#include <rtdef.h>
#include <cpuport.h>

#ifdef ARCH_CPU_64BIT
#define RT_BITS_PER_LONG 64
#else
#define RT_BITS_PER_LONG 32
#endif
#define RT_BITS_PER_LONG_LONG 64

#define RT_DIV_ROUND_UP(n, d)   (((n) + (d) - 1) / (d))

#define RT_DIV_ROUND_DOWN_ULL(ll, d)    ({ rt_uint64_t _tmp = (ll); rt_do_div(_tmp, d); _tmp; })

#define RT_DIV_ROUND_UP_ULL(ll, d)      RT_DIV_ROUND_DOWN_ULL((rt_uint64_t)(ll) + (d) - 1, (d))

#define RT_DIV_ROUND_CLOSEST(x, divisor)        \
({                                              \
    typeof(x) __x = x;                          \
    typeof(divisor) __d = divisor;              \
    (((typeof(x))-1) > 0 ||                     \
     ((typeof(divisor))-1) > 0 ||               \
     (((__x) > 0) == ((__d) > 0))) ?            \
            (((__x) + ((__d) / 2)) / (__d)) :   \
            (((__x) - ((__d) / 2)) / (__d));    \
})

#define RT_DIV_ROUND_CLOSEST_ULL(x, divisor)    \
({                                              \
    typeof(divisor) __d = divisor;              \
    rt_uint64_t _tmp = (x) + (__d) / 2;         \
    rt_do_div(_tmp, __d);                       \
    _tmp;                                       \
})

#define __KEY_PLACEHOLDER_1                     0,
#define ____KEY_ENABLED(__ignored, val, ...)    val
#define ___KEY_ENABLED(arg1_or_junk)            ____KEY_ENABLED(arg1_or_junk 1, 0)
#define __KEY_ENABLED(value)                    ___KEY_ENABLED(__KEY_PLACEHOLDER_##value)
#define RT_KEY_ENABLED(key)                     __KEY_ENABLED(key)

#define RT_FIELD_PREP(mask, val)    (((rt_uint64_t)(val) << (__rt_ffsl((mask)) - 1)) & (mask))
#define RT_FIELD_GET(mask, val)     (((val) & (mask)) >> (__rt_ffsl((mask)) - 1))

#define RT_BIT(n)               (1UL << (n))
#define RT_BIT_ULL(n)           (1ULL << (n))
#define RT_BIT_MASK(nr)         (1UL << ((nr) % RT_BITS_PER_LONG))
#define RT_BIT_WORD(nr)         ((nr) / RT_BITS_PER_LONG)

#define RT_BITS_PER_BYTE        8
#define RT_BITS_PER_TYPE(type)  (sizeof(type) * RT_BITS_PER_BYTE)
#define RT_BITS_TO_BYTES(nr)    RT_DIV_ROUND_UP(nr, RT_BITS_PER_TYPE(char))
#define RT_BITS_TO_LONGS(nr)    RT_DIV_ROUND_UP(nr, RT_BITS_PER_TYPE(long))

#define RT_GENMASK(h, l)        (((~0UL) << (l)) & (~0UL >> (RT_BITS_PER_LONG - 1 - (h))))
#define RT_GENMASK_ULL(h, l)    (((~0ULL) << (l)) & (~0ULL >> (RT_BITS_PER_LONG_LONG - 1 - (h))))

#define RT_ARRAY_SIZE(arr)      (sizeof(arr) / sizeof(arr[0]))

#define rt_offsetof(s, field)   ((rt_size_t)&((s *)0)->field)

#define rt_likely(x)            __builtin_expect(!!(x), 1)
#define rt_unlikely(x)          __builtin_expect(!!(x), 0)

#define rt_err_ptr(err)         ((void *)(rt_base_t)(err))
#define rt_ptr_err(ptr)         ((rt_err_t)(rt_base_t)(ptr))
#define rt_is_err_value(ptr)    rt_unlikely((rt_ubase_t)(void *)(ptr) >= (rt_ubase_t)-4095)
#define rt_is_err(ptr)          rt_is_err_value(ptr)
#define rt_is_err_or_null(ptr)  (rt_unlikely(!(ptr)) || rt_is_err_value((rt_ubase_t)(ptr)))

#define rt_upper_32_bits(n)     ((rt_uint32_t)(((n) >> 16) >> 16))
#define rt_lower_32_bits(n)     ((rt_uint32_t)((n) & 0xffffffff))
#define rt_upper_16_bits(n)     ((rt_uint16_t)((n) >> 16))
#define rt_lower_16_bits(n)     ((rt_uint16_t)((n) & 0xffff))

#define rt_min(x, y)            \
({                              \
    typeof(x) _x = (x);         \
    typeof(y) _y = (y);         \
    (void) (&_x == &_y);        \
    _x < _y ? _x : _y;          \
})

#define rt_max(x, y)            \
({                              \
    typeof(x) _x = (x);         \
    typeof(y) _y = (y);         \
    (void) (&_x == &_y);        \
    _x > _y ? _x : _y;          \
})

#define rt_min_t(type, x, y)    \
({                              \
    type _x = (x);              \
    type _y = (y);              \
    _x < _y ? _x: _y;           \
})

#define rt_max_t(type, x, y)    \
({                              \
    type _x = (x);              \
    type _y = (y);              \
    _x > _y ? _x: _y;           \
})

#define rt_clamp(val, lo, hi)   rt_min((typeof(val))rt_max(val, lo), hi)

#define rt_do_div(n, base)              \
({                                      \
    rt_uint32_t _base = (base), _rem;   \
    _rem = ((rt_uint64_t)(n)) % _base;  \
    (n) = ((rt_uint64_t)(n)) / _base;   \
    if (_rem > _base / 2)               \
        ++(n);                          \
    _rem;                               \
})

#define rt_abs(x)                       \
({                                      \
    long ret;                           \
    if (sizeof(x) == sizeof(long))      \
    {                                   \
        long __x = (x);                 \
        ret = (__x < 0) ? -__x : __x;   \
    }                                   \
    else                                \
    {                                   \
        int __x = (x);                  \
        ret = (__x < 0) ? -__x : __x;   \
    }                                   \
    ret;                                \
})

#define rt_roundup(x, y)                \
({                                      \
    typeof(y) __y = y;                  \
    (((x) + (__y - 1)) / __y) * __y;    \
})

#define rt_rounddown(x, y)              \
({                                      \
    typeof(x) __x = (x);                \
    __x - (__x % (y));                  \
})

#ifndef rt_ilog2
rt_inline int rt_ilog2(rt_ubase_t v)
{
    int l = 0;

    while ((1UL << l) < v)
    {
        l++;
    }

    return l;
}
#endif /* !rt_ilog2 */

#ifndef rt_bcd2bin
rt_inline rt_ubase_t rt_bcd2bin(rt_uint8_t val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}
#endif /* !rt_bcd2bin */

#ifndef rt_bin2bcd
rt_inline rt_uint8_t rt_bin2bcd(rt_ubase_t val)
{
    return ((val / 10) << 4) + val % 10;
}
#endif /* !rt_bin2bcd */

#ifndef rt_div_u64_rem
rt_inline rt_uint64_t rt_div_u64_rem(rt_uint64_t dividend, rt_uint32_t divisor,
        rt_uint32_t *remainder)
{
    *remainder = dividend % divisor;

    return dividend / divisor;
}
#endif /* !rt_div_u64_rem */

#ifndef rt_div_u64
rt_inline rt_uint64_t rt_div_u64(rt_uint64_t dividend, rt_uint32_t divisor)
{
    rt_uint32_t remainder;

    return rt_div_u64_rem(dividend, divisor, &remainder);
}
#endif /* !rt_div_u64 */

#endif /* __MISC_H__ */
