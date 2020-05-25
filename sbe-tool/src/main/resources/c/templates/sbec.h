#ifndef _SBE_POLYFILL_HEADER_H
#define _SBE_POLYFILL_HEADER_H

#include <errno.h>
#include <stddef.h>
#include <string.h>
#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS 1
#endif
#include <limits.h>
#include <math.h>

#ifdef NAN

#define SBE_FLOAT_NAN NAN
#define SBE_DOUBLE_NAN NAN
#else

#if defined(__DCC__)
#define SBE_FLOAT_NAN (sbe_float_nan())
#define SBE_DOUBLE_NAN (sbe_double_nan())
#define SBE_NAN_DEFINE_AS_FUNCTION
#else
#define SBE_HUGE_ENUF 1e+300 /* SBE_HUGE_ENUF*SBE_HUGE_ENUF must overflow */
#define SBE_INFINITY ((float)(SBE_HUGE_ENUF * SBE_HUGE_ENUF))
#define SBE_FLOAT_NAN ((float)(SBE_INFINITY * 0.0F))
#define SBE_DOUBLE_NAN ((double)(SBE_INFINITY * 0.0))
#endif

#endif /* NAN */

#if defined(SBE_NO_BOUNDS_CHECK)
#define SBE_BOUNDS_CHECK_EXPECT(exp, c) (false)
#elif defined(_MSC_VER) || (defined(__GNUC__) && __GNUC__ == 2)
#define SBE_BOUNDS_CHECK_EXPECT(exp, c) (exp)
#else
#define SBE_BOUNDS_CHECK_EXPECT(exp, c) (__builtin_expect(exp, c))
#endif /* !SBE_NO_BOUNDS_CHECK */

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if (defined(_MSC_VER) && (_MSC_VER < 1800)) || defined(__DCC__)
#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1

#ifndef __cplusplus

#if defined(_MSC_VER)
typedef unsigned char _Bool;
#endif

#define bool _Bool
#define false 0
#define true 1

#endif /* __cplusplus */

#endif /* __bool_true_false_are_defined*/

#elif !defined(__cplusplus)
#include <stdbool.h>
#endif

#if defined(__vxworks)
#include <vxWorks.h>
#if __GNUC__ > 2
#include <stdint.h>
#endif
#else
/* TODO: support vc8 and earlier */
#include <stdint.h>
#endif


#ifdef __vxworks

#ifndef INT8_C
#define INT8_C(x) x
#endif

#ifndef UINT8_C
#define UINT8_C(x) x
#endif

#ifndef INT16_C
#define INT16_C(x) x
#endif

#ifndef UINT16_C
#define UINT16_C(x) x
#endif

#define _SBE_VX_JOIN(X, Y)     _SBE_VX_DO_JOIN(X, Y)
#define _SBE_VX_DO_JOIN(X, Y)  _SBE_VX_DO_JOIN2(X, Y)
#define _SBE_VX_DO_JOIN2(X, Y) X##Y

/* Correctly setup the macros */
#undef  INT32_C
#undef  UINT32_C
#undef  INT64_C
#undef  UINT64_C
#define INT32_C(x)  _SBE_VX_JOIN(x, L)
#define UINT32_C(x) _SBE_VX_JOIN(x, UL)
#define INT64_C(x)  _SBE_VX_JOIN(x, LL)
#define UINT64_C(x) _SBE_VX_JOIN(x, ULL)

#if !defined(INT8_MIN)

#define INT8_MIN (-0x7f - 1)
#define INT16_MIN (-0x7fff - 1)
#define INT32_MIN INT_MIN
#define INT64_MIN LONG_LONG_MIN

#define INT8_MAX 0x7f
#define INT16_MAX 0x7fff
#define INT32_MAX INT_MAX
#define INT64_MAX __LONG_LONG_MAX__

#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX UINT_MAX
#define UINT64_MAX ULONG_LONG_MAX

#define SIZE_MAX ULONG_MAX

#endif /* !INT8_MIN */

#endif

#ifdef __vxworks
#define __ORDER_LITTLE_ENDIAN__ 0
#define __ORDER_BIG_ENDIAN__ 1
#if defined(__BIG_ENDIAN__) && __BIG_ENDIAN__
#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
#else
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif

/* Swap bytes in 16-bit value.  */
#ifndef __builtin_bswap16
#define __builtin_bswap16(x)                    \
  ((uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#endif

/* Swap bytes in 32-bit value.  */
#ifndef __builtin_bswap32
#define __builtin_bswap32(x)                    \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)	\
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))
#endif

/* Swap bytes in 64-bit value.  */
#ifndef __builtin_bswap64
#define __builtin_bswap64(x)                   \
  ((((x) & 0xff00000000000000ull) >> 56)       \
   | (((x) & 0x00ff000000000000ull) >> 40)     \
   | (((x) & 0x0000ff0000000000ull) >> 24)     \
   | (((x) & 0x000000ff00000000ull) >> 8)      \
   | (((x) & 0x00000000ff000000ull) << 8)      \
   | (((x) & 0x0000000000ff0000ull) << 24)     \
   | (((x) & 0x000000000000ff00ull) << 40)     \
   | (((x) & 0x00000000000000ffull) << 56))
#endif

#endif /* __vxworks */

/*
 * Define some byte ordering macros
 */
#if defined(WIN32) || defined(_WIN32)
#define SBE_BIG_ENDIAN_ENCODE_16(v) _byteswap_ushort(v)
#define SBE_BIG_ENDIAN_ENCODE_32(v) _byteswap_ulong(v)
#define SBE_BIG_ENDIAN_ENCODE_64(v) _byteswap_uint64(v)
#define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#define __ORDER_LITTLE_ENDIAN__ 0
#define __ORDER_BIG_ENDIAN__ 1
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define SBE_BIG_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#define SBE_BIG_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#define SBE_BIG_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SBE_BIG_ENDIAN_ENCODE_16(v) (v)
#define SBE_BIG_ENDIAN_ENCODE_32(v) (v)
#define SBE_BIG_ENDIAN_ENCODE_64(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#define SBE_LITTLE_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#define SBE_LITTLE_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#else
#error "Byte Ordering of platform not determined. Set __BYTE_ORDER__ manually before including this file."
#endif

#define _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data_copy_from, offset, len) \
    { \
        uint8_t *dst = ((uint8_t*)(view.data)) + offset; \
        memcpy((void*)dst, (void*)data_copy_from, (size_t)len); \
    }

#define _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data_copy_from, offset, len) \
    if (view.data == NULL || offset + len > view.length || data_copy_from == NULL) \
    { \
        return false; \
    } \
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data_copy_from, offset, len) \
    return true;

#define _SBE_BUFFER_VIEW_UNSAFE_SET_AT(view, index, value) \
    ((uint8_t*)(view.data))[index] = (uint8_t)value;

#define _SBE_BUFFER_VIEW_SAFE_SET_AT(view, index, value) \
    if (view.data == NULL || index >= view.length) \
    { \
        return false; \
    } \
    ((uint8_t*)(view.data))[index] = (uint8_t)value; \
    return true;

#define _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data_copy_to, offset, len) \
    { \
        uint64_t len_capable_copy = view.length - offset; \
        uint64_t len_to_copy = len_capable_copy < len ? len_capable_copy : len; \
        uint8_t *src = ((uint8_t*)(view.data)) + offset; \
        memcpy((void*)data_copy_to, (void*)src, (size_t)len_to_copy); \
        return len_to_copy; \
    }

#define _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data_copy_to, offset, len) \
    if (view.data == NULL || data_copy_to == NULL || offset >= view.length ) \
    { \
        return 0; \
    } \
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data_copy_to, offset, len)

#define _SBE_BUFFER_VIEW_UNSAFE_GET_AT(view, index, data_type) \
    return ((data_type*)(view.data))[index]; \

#define _SBE_BUFFER_VIEW_SAFE_GET_AT(view, index, null_value, data_type) \
    if (view.data == NULL || index >= view.length) \
    { \
        return null_value; \
    } \
    _SBE_BUFFER_VIEW_UNSAFE_GET_AT(view, index, data_type)

/* For byte order buffers */

#define _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data_copy_from, offset, len, byte_order_encode, data_type_encode) \
    { \
        uint64_t i; \
        for (i = offset; i < offset + len; i += 1) \
        { \
            data_type_encode tmp = ((data_type_encode*)(data_copy_from))[i]; \
            tmp = byte_order_encode(tmp); \
            memcpy((void*)((data_type_encode*)(view.data) + i), (void*)&tmp, sizeof(tmp)); \
        } \
    }

#define _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data_copy_from, offset, len, byte_order_encode, data_type_encode) \
    if (view.data == NULL || offset + len > view.length || data_copy_from == NULL) \
    { \
        return false; \
    } \
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data_copy_from, offset, len, byte_order_encode, data_type_encode) \
    return true;

#define _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, value, byte_order_encode, data_type_encode) \
        { \
            data_type_encode tmp = (data_type_encode)(value); \
            tmp = byte_order_encode(tmp); \
            memcpy((void*)((data_type_encode*)(view.data) + index), (void*)&tmp, sizeof(tmp)); \
        } \

#define _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, value, byte_order_encode, data_type_encode) \
    if (view.data == NULL || index >= view.length) \
    { \
        return false; \
    } \
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, value, byte_order_encode, data_type_encode) \
    return true;

#define _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data_copy_to, offset, len, byte_order_encode, data_type_encode) \
    { \
        uint64_t len_capable_copy = view.length - offset; \
        uint64_t len_to_copy = len_capable_copy < len ? len_capable_copy : len; \
        uint64_t i; \
        for (i = offset; i < offset + len_to_copy; i += 1) \
        { \
            data_type_encode tmp; \
            memcpy(&tmp, ((data_type_encode*)(view.data)) + i, sizeof(tmp)); \
            *(data_type_encode*)(data_copy_to + i) = byte_order_encode(tmp); \
        } \
        return len_to_copy; \
    } \

#define _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data_copy_to, offset, len, byte_order_encode, data_type_encode) \
    if (view.data == NULL || data_copy_to == NULL || offset >= view.length) \
    { \
        return 0; \
    } \
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data_copy_to, offset, len, byte_order_encode, data_type_encode)

#define _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, data_type, byte_order_encode, data_type_encode) \
    { \
        data_type_encode tmp; \
        memcpy(&tmp, (data_type_encode*)view.data + index, sizeof(tmp)); \
        return (data_type)byte_order_encode(tmp); \
    } \

#define _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, null_value, data_type, byte_order_encode, data_type_encode) \
    if (view.data == NULL || index >= view.length) \
    { \
        return null_value; \
    } \
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, data_type, byte_order_encode, data_type_encode)


#if defined(__cplusplus)

#include <cctype>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#if defined(__vxworks)
#if defined(__RTP__)
#define _SBE_HAVE_CSTDINT
#elif !defined(__GNUC__) || __GNUC__ > 2
#define _SBE_NEED_POLYFILL_CSTDINT
#endif
#if !defined(__GNUC__) || __GNUC__ > 2
#define _SBE_HAVE_OSTREAM
#endif
#else
#define _SBE_HAVE_CSTDINT
#define _SBE_HAVE_OSTREAM
#endif

#if defined(__vxworks)
#include <iostream>
#if !defined(__GNUC__) || __GNUC__ > 2
#include <ostream>
#endif
#else
#include <stdexcept>
#include <tuple>
#include <vector>
#endif

#if defined(_SBE_HAVE_CSTDINT)
#include <cstdint>
#endif
#if defined(_SBE_HAVE_OSTREAM)
#include <ostream>
#endif

#if __cplusplus >= 201103L
#define SBE_CONSTEXPR constexpr
#define SBE_NOEXCEPT noexcept
#else
#define SBE_CONSTEXPR
#define SBE_NOEXCEPT
#define nullptr NULL
#endif

#if __cplusplus >= 201402L
#define SBE_CONSTEXPR_14 constexpr
#else
#define SBE_CONSTEXPR_14
#endif

#if __cplusplus >= 201703L
#include <string_view>
#define SBE_NODISCARD [[nodiscard]]
#else
#define SBE_NODISCARD
#endif

#if defined(_SBE_NEED_POLYFILL_CSTDINT)
namespace std
{
typedef int8_t int8_t;
typedef int16_t int16_t;
typedef int32_t int32_t;
typedef int64_t int64_t;
typedef uint8_t uint8_t;
typedef uint16_t uint16_t;
typedef uint32_t uint32_t;
typedef uint64_t uint64_t;
}
#endif

#endif

#ifdef __cplusplus
#define SBE_ONE_DEF static inline
#else

#if defined(__GNUC__)
#if __GNUC__==2
#define SBE_ONE_DEF static __inline__
#else
#define SBE_ONE_DEF static __attribute__((always_inline)) __inline__
#endif
#else

#if (defined(_MSC_VER) && _MSC_VER < 1800)
#define SBE_ONE_DEF static __inline
#endif

#endif

#ifndef SBE_ONE_DEF
#define SBE_ONE_DEF static inline
#endif

#endif /* !__cplusplus */

union sbe_float_as_uint
{
    float fp_value;
    uint32_t uint_value;
};

union sbe_double_as_uint
{
    double fp_value;
    uint64_t uint_value;
};

struct sbe_group
{
    char *buffer;
    uint64_t buffer_length;
    uint64_t *position_ptr;
    uint64_t block_length;
    uint64_t count;
    uint64_t index;
    uint64_t offset;
    uint64_t acting_version;
};

struct sbe_message
{
    char *buffer;
    uint64_t buffer_length;
    uint64_t offset;
    uint64_t position;
    uint64_t acting_version;
};

struct sbe_composite
{
    char *buffer;
    uint64_t buffer_length;
    uint64_t offset;
    uint64_t acting_version;
};

enum sbe_meta_attribute
{
    sbe_meta_attribute_EPOCH,
    sbe_meta_attribute_TIME_UNIT,
    sbe_meta_attribute_SEMANTIC_TYPE,
    sbe_meta_attribute_PRESENCE
};

typedef enum sbe_meta_attribute sbe_meta_attribute;

#define SBE_NULLVALUE_INT8 INT8_MIN
#define SBE_NULLVALUE_INT16 INT16_MIN
#define SBE_NULLVALUE_INT32 INT32_MIN
#define SBE_NULLVALUE_INT64 INT64_MIN
#define SBE_NULLVALUE_UINT8 UINT8_MAX
#define SBE_NULLVALUE_UINT16 UINT16_MAX
#define SBE_NULLVALUE_UINT32 UINT32_MAX
#define SBE_NULLVALUE_UINT64 UINT64_MAX

#define E100 -50100 /* E_BUF_SHORT */
#define E103 -50103 /* VAL_UNKNOWN_ENUM */
#define E104 -50104 /* I_OUT_RANGE_NUM */
#define E105 -50105 /* I_OUT_RANGE_NUM */
#define E106 -50106 /* I_OUT_RANGE_NUM */
#define E107 -50107 /* BUF_SHORT_FLYWEIGHT */
#define E108 -50108 /* BUF_SHORT_NXT_GRP_IND */
#define E109 -50109 /* STR_TOO_LONG_FOR_LEN_TYP */
#define E110 -50110 /* CNT_OUT_RANGE */

SBE_ONE_DEF const char *sbe_strerror(const int errnum)
{
    switch (errnum)
    {
    case E100:
        return "buffer too short";
    case E103:
        return "unknown value for enum";
    case E104:
        return "index out of range";
    case E105:
        return "index out of range";
    case E106:
        return "length too large";
    case E107:
        return "buffer too short for flyweight";
    case E108:
        return "buffer too short to support next group index";
    case E109:
        return "std::string too long for length type";
    case E110:
        return "count outside of allowed range";
    default:
        return "unknown error";
    }
}

SBE_ONE_DEF const char *sbe_throw_errnum(const int errnum, const char *message)
{
    errno = errnum;
#if !defined(__vxworks) && defined(__cplusplus)
    throw std::runtime_error(message);
#endif
    return message;
}

#if defined(SBE_NAN_DEFINE_AS_FUNCTION)
SBE_ONE_DEF float sbe_float_nan()
{
    union sbe_float_as_uint val;
    val.uint_value = 0x7ff80000;
    return val.fp_value;
}

SBE_ONE_DEF double sbe_double_nan()
{
    union sbe_double_as_uint val;
    val.uint_value = 0x7ff8000000000000ULL;
    return val.fp_value;
}
#endif

/* https://developer.squareup.com/blog/reversing-bits-in-c/ */

/* Function to reverse bits of uint8_t */
SBE_ONE_DEF uint8_t sbe_reverse_bits_uint8_t(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & ~0xF0) << 4; /* 0b11110000 */
    b = (b & 0xCC) >> 2 | (b & ~0xCC) << 2; /* 0b11001100 */
    b = (b & 0xAA) >> 1 | (b & ~0xAA) << 1; /* 0b10101010 */
    return b;
}

/* Function to reverse bits of uint16_t */
SBE_ONE_DEF uint16_t sbe_reverse_bits_uint16_t(uint16_t b)
{
    b = (b & 0xFF00) >> 8 | (b & ~0xFF00) << 8; /* 0b1111111100000000 */
    b = (b & 0xF0F0) >> 4 | (b & ~0xF0F0) << 4; /* 0b1111000011110000 */
    b = (b & 0xCCCC) >> 2 | (b & ~0xCCCC) << 2; /* 0b1100110011001100 */
    b = (b & 0xAAAA) >> 1 | (b & ~0xAAAA) << 1; /* 0b1010101010101010 */
    return b;
}

/* Function to reverse bits of uint32_t */
SBE_ONE_DEF uint32_t sbe_reverse_bits_uint32_t(uint32_t b)
{
    b = (b & 0xFFFF0000) >> 16 | (b & ~0xFFFF0000) << 16; /* 0b11111111111111110000000000000000 */
    b = (b & 0xFF00FF00) >> 8  | (b & ~0xFF00FF00) << 8;  /* 0b11111111000000001111111100000000 */
    b = (b & 0xF0F0F0F0) >> 4  | (b & ~0xF0F0F0F0) << 4;  /* 0b11110000111100001111000011110000 */
    b = (b & 0xCCCCCCCC) >> 2  | (b & ~0xCCCCCCCC) << 2;  /* 0b11001100110011001100110011001100 */
    b = (b & 0xAAAAAAAA) >> 1  | (b & ~0xAAAAAAAA) << 1;  /* 0b10101010101010101010101010101010 */
    return b;
}

/* Function to reverse bits of uint64_t */
SBE_ONE_DEF uint64_t sbe_reverse_bits_uint64_t(uint64_t b)
{
    b = (b & 0xFFFFFFFF00000000ULL) >> 32 | (b & ~0xFFFFFFFF00000000ULL) << 32; /* 0b1111111111111111111111111111111100000000000000000000000000000000 */
    b = (b & 0xFFFF0000FFFF0000ULL) >> 16 | (b & ~0xFFFF0000FFFF0000ULL) << 16; /* 0b1111111111111111000000000000000011111111111111110000000000000000 */
    b = (b & 0xFF00FF00FF00FF00ULL) >> 8  | (b & ~0xFF00FF00FF00FF00ULL) << 8;  /* 0b1111111100000000111111110000000011111111000000001111111100000000 */
    b = (b & 0xF0F0F0F0F0F0F0F0ULL) >> 4  | (b & ~0xF0F0F0F0F0F0F0F0ULL) << 4;  /* 0b1111000011110000111100001111000011110000111100001111000011110000 */
    b = (b & 0xCCCCCCCCCCCCCCCCULL) >> 2  | (b & ~0xCCCCCCCCCCCCCCCCULL) << 2;  /* 0b1100110011001100110011001100110011001100110011001100110011001100 */
    b = (b & 0xAAAAAAAAAAAAAAAAULL) >> 1  | (b & ~0xAAAAAAAAAAAAAAAAULL) << 1;  /* 0b1010101010101010101010101010101010101010101010101010101010101010 */
    return b;
}

#if defined(__cplusplus)

template <typename T>
std::string sbe_view_to_string(T view)
{
    return std::string(view.data, view.length);
}

template <typename T>
struct sbe_vector_view
{
    sbe_vector_view(T *in_data, size_t in_length) : data(in_data), length(in_length)
    {
    }

    template <size_t S>
    sbe_vector_view<T> &operator=(T (&in_data)[S])
    {
        data = in_data;
        length = S;
        return *this;
    }

    T *data;
    size_t length;

    size_t size() const
    {
        return length;
    }

    typedef T *iterator;
    typedef const T *const_iterator;

    iterator begin()
    {
        return data;
    }

    const_iterator begin() const
    {
        return data;
    }

    iterator end()
    {
        return data + length;
    }

    const_iterator end() const
    {
        return data + length;
    }
};

template <typename T, size_t S>
sbe_vector_view<T> sbe_vector_view_make(T (&data)[S])
{
    return sbe_vector_view<T>(data, S);
}

template <typename T, size_t S>
sbe_vector_view<const T> sbe_vector_view_make(const T (&data)[S])
{
    return sbe_vector_view<const T>(data, S);
}

template <typename T>
sbe_vector_view<T> sbe_vector_view_make(T *data, size_t S)
{
    return sbe_vector_view<T>(data, S);
}

template <typename T>
sbe_vector_view<const T> sbe_vector_view_make(const T *data, size_t S)
{
    return sbe_vector_view<const T>(data, S);
}

template <typename T, size_t S>
size_t sbe_array_size(T (&)[S])
{
    return S;
}

template <typename T, size_t S>
size_t sbe_array_size(const T (&)[S])
{
    return S;
}

// the class to wrap a lambda expression
template <typename TSelf, typename TLambda>
class __sbe_LambdaWrapper
{
public:
    static const TLambda *pFuncPtr;

    static void Exec(TSelf self, void *ctx)
    {
        (*pFuncPtr)(self, ctx);
    }
};

// instantiate the static member data
template <typename TSelf, typename TLambda>
const TLambda *__sbe_LambdaWrapper<TSelf, TLambda>::pFuncPtr = NULL;

template <typename TSelf, typename TLambda>
void (*sbeLambda(const TLambda &rFunc))(TSelf, void *)
{
    __sbe_LambdaWrapper<TSelf, TLambda>::pFuncPtr = &rFunc;
    return &__sbe_LambdaWrapper<TSelf, TLambda>::Exec;
}

#endif /* __cplusplus */

/*
sbe_string_view
sbe_int8_view
sbe_uint8_view
sbe_int16_view_be
sbe_int16_view_le
sbe_uint16_view_be
sbe_uint16_view_le
sbe_int32_view_be
sbe_int32_view_le
sbe_uint32_view_be
sbe_uint32_view_le
sbe_int64_view_be
sbe_int64_view_le
sbe_uint64_view_be
sbe_uint64_view_le
sbe_float_view_be
sbe_float_view_le
sbe_double_view_be
sbe_double_view_le
*/

struct sbe_const_string_view
{
    const char *data;
    size_t length;
#if defined(__cplusplus)
    std::string str()
    {
        return std::string(data, length);
    }
    uint64_t get(char *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, 0, len);
    }
    uint64_t get_unsafe(char *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, 0, len);
    }
    char get(uint64_t index)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_AT((*this), index, '\0', char);
    }
    char get_unsafe(uint64_t index)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_AT((*this), index, char);
    }
    uint64_t get(char *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, offset, len);
    }
    uint64_t get_unsafe(char *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, offset, len);
    }
#endif
};
typedef struct sbe_const_string_view sbe_const_string_view;

SBE_ONE_DEF uint64_t sbe_const_string_view_get(sbe_const_string_view view, char *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, 0, len);
}

SBE_ONE_DEF uint64_t sbe_const_string_view_get_unsafe(sbe_const_string_view view, char *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, 0, len);
}

SBE_ONE_DEF char sbe_const_string_view_get_at(sbe_const_string_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_SAFE_GET_AT(view, index, '\0', char);
}

SBE_ONE_DEF char sbe_const_string_view_get_at_unsafe(sbe_const_string_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_AT(view, index, char);
}

SBE_ONE_DEF uint64_t sbe_const_string_view_get_range(sbe_const_string_view view, char *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, offset, len);
}

SBE_ONE_DEF uint64_t sbe_const_string_view_get_range_unsafe(sbe_const_string_view view, char *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, offset, len);
}

struct sbe_string_view
{
    char *data;
    size_t length;
#if defined(__cplusplus)
    std::string str()
    {
        return std::string(data, length);
    }
    bool set_str(const char *ptr)
    {
        uint64_t len = strlen(ptr);
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, 0, len);
    }
    bool set_str(const std::string &str)
    {
        const char* ptr = str.data();
        const uint64_t len = str.size();
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, 0, len);
    }
    sbe_string_view& set_buffer(const char *data)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), data, 0, length);
        return *this;
    }
    bool set(const char *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), data, 0, len);
    }
    sbe_string_view& set_unsafe(const char *ptr)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), ptr, 0, length);
        return *this;
    }
    bool set(uint64_t index, char val)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_AT((*this), index, val);
    }
    sbe_string_view& set_unsafe(uint64_t index, char val)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_AT((*this), index, val);
        return *this;
    }
    bool set(const char *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, offset, len);
    }
    sbe_string_view& set_unsafe(const char *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), ptr, offset, len);
        return *this;
    }
    uint64_t get(char *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, 0, len);
    }
    uint64_t get_unsafe(char *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, 0, len);
    }
    char get(uint64_t index)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_AT((*this), index, '\0', char);
    }
    char get_unsafe(uint64_t index)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_AT((*this), index, char);
    }
    uint64_t get(char *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, offset, len);
    }
    uint64_t get_unsafe(char *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, offset, len);
    }
#endif
};
typedef struct sbe_string_view sbe_string_view;

SBE_ONE_DEF bool sbe_string_view_set_str(sbe_string_view view, const char *str)
{
    uint64_t len = strlen(str);
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, str, 0, len);
}

SBE_ONE_DEF bool sbe_string_view_set(sbe_string_view view, const char *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data, 0, len);
}

SBE_ONE_DEF void sbe_string_view_set_buffer(sbe_string_view view, const char *data)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data, 0, view.length);
}

SBE_ONE_DEF bool sbe_string_view_set_at(sbe_string_view view, uint64_t index, char val)
{
    _SBE_BUFFER_VIEW_SAFE_SET_AT(view, index, val);
}

SBE_ONE_DEF void sbe_string_view_set_at_unsafe(sbe_string_view view, uint64_t index, char val)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_AT(view, index, val);
}

SBE_ONE_DEF bool sbe_string_view_set_range(sbe_string_view view, const char *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data, offset, len);
}

SBE_ONE_DEF void sbe_string_view_set_range_unsafe(sbe_string_view view, const char *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data, offset, len);
}

SBE_ONE_DEF uint64_t sbe_string_view_get(sbe_string_view view, char *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, 0, len);
}

SBE_ONE_DEF uint64_t sbe_string_view_get_unsafe(sbe_string_view view, char *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, 0, len);
}

SBE_ONE_DEF char sbe_string_view_get_at(sbe_string_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_SAFE_GET_AT(view, index, '\0', char);
}

SBE_ONE_DEF char sbe_string_view_get_at_unsafe(sbe_string_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_AT(view, index, char);
}

SBE_ONE_DEF uint64_t sbe_string_view_get_range(sbe_string_view view, char *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, offset, len);
}

SBE_ONE_DEF uint64_t sbe_string_view_get_range_unsafe(sbe_string_view view, char *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, offset, len);
}

/*@EXPANDING-BY-GENERATOR@*/

struct sbe_int8_view
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int8_t *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, 0, len);
    }
    sbe_int8_view& set_unsafe(const int8_t *ptr)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), ptr, 0, length);
        return *this;
    }
    bool set(uint64_t index, int8_t val)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_AT((*this), index, val);
    }
    sbe_int8_view& set_unsafe(uint64_t index, int8_t val)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_AT((*this), index, val);
        return *this;
    }
    bool set(const int8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, offset, len);
    }
    sbe_int8_view& set_unsafe(const int8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), ptr, offset, len);
        return *this;
    }
    uint64_t get(int8_t *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, 0, len);
    }
    uint64_t get_unsafe(int8_t *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, 0, len);
    }
    int8_t get(uint64_t index)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT8, int8_t);
    }
    int8_t get_unsafe(uint64_t index)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_AT((*this), index, int8_t);
    }
    uint64_t get(int8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, offset, len);
    }
    uint64_t get_unsafe(int8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, offset, len);
    }
#endif
};
typedef struct sbe_int8_view sbe_int8_view;

SBE_ONE_DEF bool sbe_int8_view_set(sbe_int8_view view, const int8_t *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data, 0, len);
}
SBE_ONE_DEF void sbe_int8_view_set_buffer(sbe_int8_view view, const int8_t *data)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data, 0, view.length);
}
SBE_ONE_DEF bool sbe_int8_view_set_at(sbe_int8_view view, uint64_t index, int8_t val)
{
    _SBE_BUFFER_VIEW_SAFE_SET_AT(view, index, val);
}
SBE_ONE_DEF void sbe_int8_view_set_at_unsafe(sbe_int8_view view, uint64_t index, int8_t val)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_AT(view, index, val);
}
SBE_ONE_DEF bool sbe_int8_view_set_range(sbe_int8_view view, const int8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data, offset, len);
}
SBE_ONE_DEF void sbe_int8_view_set_range_unsafe(sbe_int8_view view, const int8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data, offset, len);
}
SBE_ONE_DEF uint64_t sbe_int8_view_get(sbe_int8_view view, int8_t *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, 0, len);
}
SBE_ONE_DEF uint64_t sbe_int8_view_get_unsafe(sbe_int8_view view, int8_t *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, 0, len);
}
SBE_ONE_DEF int8_t sbe_int8_view_get_at(sbe_int8_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT8, int8_t);
}
SBE_ONE_DEF int8_t sbe_int8_view_get_at_unsafe(sbe_int8_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_AT(view, index, int8_t);
}
SBE_ONE_DEF uint64_t sbe_int8_view_get_range(sbe_int8_view view, int8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, offset, len);
}
SBE_ONE_DEF uint64_t sbe_int8_view_get_range_unsafe(sbe_int8_view view, int8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, offset, len);
}

struct sbe_int16_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_int16_view_be& set_unsafe(const int16_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(uint64_t index, int16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_int16_view_be& set_unsafe(uint64_t index, int16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(const int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_int16_view_be& set_unsafe(const int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    uint64_t get(int16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(int16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    int16_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT16, int16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    int16_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, int16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get(int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
#endif
};
typedef struct sbe_int16_view_be sbe_int16_view_be;

SBE_ONE_DEF bool sbe_int16_view_be_set(sbe_int16_view_be view, const int16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_int16_view_be_set_buffer(sbe_int16_view_be view, const int16_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_int16_view_be_set_at(sbe_int16_view_be view, uint64_t index, int16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_int16_view_be_set_at_unsafe(sbe_int16_view_be view, uint64_t index, int16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_int16_view_be_set_range(sbe_int16_view_be view, const int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_int16_view_be_set_range_unsafe(sbe_int16_view_be view, const int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_be_get(sbe_int16_view_be view, int16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_be_get_unsafe(sbe_int16_view_be view, int16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF int16_t sbe_int16_view_be_get_at(sbe_int16_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT16, int16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF int16_t sbe_int16_view_be_get_at_unsafe(sbe_int16_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, int16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_be_get_range(sbe_int16_view_be view, int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_be_get_range_unsafe(sbe_int16_view_be view, int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}

struct sbe_int16_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_int16_view_le& set_unsafe(const int16_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(uint64_t index, int16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_int16_view_le& set_unsafe(uint64_t index, int16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(const int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_int16_view_le& set_unsafe(const int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    uint64_t get(int16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(int16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    int16_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT16, int16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    int16_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, int16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get(int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(int16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
#endif
};
typedef struct sbe_int16_view_le sbe_int16_view_le;

SBE_ONE_DEF bool sbe_int16_view_le_set(sbe_int16_view_le view, const int16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_int16_view_le_set_buffer(sbe_int16_view_le view, const int16_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_int16_view_le_set_at(sbe_int16_view_le view, uint64_t index, int16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_int16_view_le_set_at_unsafe(sbe_int16_view_le view, uint64_t index, int16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_int16_view_le_set_range(sbe_int16_view_le view, const int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_int16_view_le_set_range_unsafe(sbe_int16_view_le view, const int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_le_get(sbe_int16_view_le view, int16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_le_get_unsafe(sbe_int16_view_le view, int16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF int16_t sbe_int16_view_le_get_at(sbe_int16_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT16, int16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF int16_t sbe_int16_view_le_get_at_unsafe(sbe_int16_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, int16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_le_get_range(sbe_int16_view_le view, int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_int16_view_le_get_range_unsafe(sbe_int16_view_le view, int16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}

struct sbe_int32_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_int32_view_be& set_unsafe(const int32_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(uint64_t index, int32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_int32_view_be& set_unsafe(uint64_t index, int32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(const int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_int32_view_be& set_unsafe(const int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    uint64_t get(int32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(int32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    int32_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT32, int32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    int32_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, int32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get(int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
#endif
};
typedef struct sbe_int32_view_be sbe_int32_view_be;

SBE_ONE_DEF bool sbe_int32_view_be_set(sbe_int32_view_be view, const int32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_int32_view_be_set_buffer(sbe_int32_view_be view, const int32_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_int32_view_be_set_at(sbe_int32_view_be view, uint64_t index, int32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_int32_view_be_set_at_unsafe(sbe_int32_view_be view, uint64_t index, int32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_int32_view_be_set_range(sbe_int32_view_be view, const int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_int32_view_be_set_range_unsafe(sbe_int32_view_be view, const int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_be_get(sbe_int32_view_be view, int32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_be_get_unsafe(sbe_int32_view_be view, int32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF int32_t sbe_int32_view_be_get_at(sbe_int32_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT32, int32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF int32_t sbe_int32_view_be_get_at_unsafe(sbe_int32_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, int32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_be_get_range(sbe_int32_view_be view, int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_be_get_range_unsafe(sbe_int32_view_be view, int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}

struct sbe_int32_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_int32_view_le& set_unsafe(const int32_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(uint64_t index, int32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_int32_view_le& set_unsafe(uint64_t index, int32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(const int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_int32_view_le& set_unsafe(const int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    uint64_t get(int32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(int32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    int32_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT32, int32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    int32_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, int32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get(int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(int32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
#endif
};
typedef struct sbe_int32_view_le sbe_int32_view_le;

SBE_ONE_DEF bool sbe_int32_view_le_set(sbe_int32_view_le view, const int32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_int32_view_le_set_buffer(sbe_int32_view_le view, const int32_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_int32_view_le_set_at(sbe_int32_view_le view, uint64_t index, int32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_int32_view_le_set_at_unsafe(sbe_int32_view_le view, uint64_t index, int32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_int32_view_le_set_range(sbe_int32_view_le view, const int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_int32_view_le_set_range_unsafe(sbe_int32_view_le view, const int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_le_get(sbe_int32_view_le view, int32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_le_get_unsafe(sbe_int32_view_le view, int32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF int32_t sbe_int32_view_le_get_at(sbe_int32_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT32, int32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF int32_t sbe_int32_view_le_get_at_unsafe(sbe_int32_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, int32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_le_get_range(sbe_int32_view_le view, int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_int32_view_le_get_range_unsafe(sbe_int32_view_le view, int32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}

struct sbe_int64_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_int64_view_be& set_unsafe(const int64_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(uint64_t index, int64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_int64_view_be& set_unsafe(uint64_t index, int64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(const int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_int64_view_be& set_unsafe(const int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    uint64_t get(int64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(int64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    int64_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT64, int64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    int64_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, int64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
#endif
};
typedef struct sbe_int64_view_be sbe_int64_view_be;

SBE_ONE_DEF bool sbe_int64_view_be_set(sbe_int64_view_be view, const int64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_int64_view_be_set_buffer(sbe_int64_view_be view, const int64_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_int64_view_be_set_at(sbe_int64_view_be view, uint64_t index, int64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_int64_view_be_set_at_unsafe(sbe_int64_view_be view, uint64_t index, int64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_int64_view_be_set_range(sbe_int64_view_be view, const int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_int64_view_be_set_range_unsafe(sbe_int64_view_be view, const int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_be_get(sbe_int64_view_be view, int64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_be_get_unsafe(sbe_int64_view_be view, int64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF int64_t sbe_int64_view_be_get_at(sbe_int64_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT64, int64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF int64_t sbe_int64_view_be_get_at_unsafe(sbe_int64_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, int64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_be_get_range(sbe_int64_view_be view, int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_be_get_range_unsafe(sbe_int64_view_be view, int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}

struct sbe_int64_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const int64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_int64_view_le& set_unsafe(const int64_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(uint64_t index, int64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_int64_view_le& set_unsafe(uint64_t index, int64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(const int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_int64_view_le& set_unsafe(const int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    uint64_t get(int64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(int64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    int64_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_INT64, int64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    int64_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, int64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(int64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
#endif
};
typedef struct sbe_int64_view_le sbe_int64_view_le;

SBE_ONE_DEF bool sbe_int64_view_le_set(sbe_int64_view_le view, const int64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_int64_view_le_set_buffer(sbe_int64_view_le view, const int64_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_int64_view_le_set_at(sbe_int64_view_le view, uint64_t index, int64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_int64_view_le_set_at_unsafe(sbe_int64_view_le view, uint64_t index, int64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_int64_view_le_set_range(sbe_int64_view_le view, const int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_int64_view_le_set_range_unsafe(sbe_int64_view_le view, const int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_le_get(sbe_int64_view_le view, int64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_le_get_unsafe(sbe_int64_view_le view, int64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF int64_t sbe_int64_view_le_get_at(sbe_int64_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_INT64, int64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF int64_t sbe_int64_view_le_get_at_unsafe(sbe_int64_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, int64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_le_get_range(sbe_int64_view_le view, int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_int64_view_le_get_range_unsafe(sbe_int64_view_le view, int64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}

struct sbe_uint8_view
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint8_t *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, 0, len);
    }
    sbe_uint8_view& set_unsafe(const uint8_t *ptr)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), ptr, 0, length);
        return *this;
    }
    bool set(uint64_t index, uint8_t val)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_AT((*this), index, val);
    }
    sbe_uint8_view& set_unsafe(uint64_t index, uint8_t val)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_AT((*this), index, val);
        return *this;
    }
    bool set(const uint8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_SET_RANGE((*this), ptr, offset, len);
    }
    sbe_uint8_view& set_unsafe(const uint8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE((*this), ptr, offset, len);
        return *this;
    }
    uint64_t get(uint8_t *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, 0, len);
    }
    uint64_t get_unsafe(uint8_t *ptr, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, 0, len);
    }
    uint8_t get(uint64_t index)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT8, uint8_t);
    }
    uint8_t get_unsafe(uint64_t index)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_AT((*this), index, uint8_t);
    }
    uint64_t get(uint8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_SAFE_GET_RANGE((*this), ptr, offset, len);
    }
    uint64_t get_unsafe(uint8_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE((*this), ptr, offset, len);
    }
#endif
};
typedef struct sbe_uint8_view sbe_uint8_view;

SBE_ONE_DEF bool sbe_uint8_view_set(sbe_uint8_view view, const uint8_t *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data, 0, len);
}
SBE_ONE_DEF void sbe_uint8_view_set_buffer(sbe_uint8_view view, const uint8_t *data)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data, 0, view.length);
}
SBE_ONE_DEF bool sbe_uint8_view_set_at(sbe_uint8_view view, uint64_t index, uint8_t val)
{
    _SBE_BUFFER_VIEW_SAFE_SET_AT(view, index, val);
}
SBE_ONE_DEF void sbe_uint8_view_set_at_unsafe(sbe_uint8_view view, uint64_t index, uint8_t val)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_AT(view, index, val);
}
SBE_ONE_DEF bool sbe_uint8_view_set_range(sbe_uint8_view view, const uint8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_SET_RANGE(view, data, offset, len);
}
SBE_ONE_DEF void sbe_uint8_view_set_range_unsafe(sbe_uint8_view view, const uint8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_SET_RANGE(view, data, offset, len);
}
SBE_ONE_DEF uint64_t sbe_uint8_view_get(sbe_uint8_view view, uint8_t *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, 0, len);
}
SBE_ONE_DEF uint64_t sbe_uint8_view_get_unsafe(sbe_uint8_view view, uint8_t *data, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, 0, len);
}
SBE_ONE_DEF uint8_t sbe_uint8_view_get_at(sbe_uint8_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT8, uint8_t);
}
SBE_ONE_DEF uint8_t sbe_uint8_view_get_at_unsafe(sbe_uint8_view view, uint64_t index)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_AT(view, index, uint8_t);
}
SBE_ONE_DEF uint64_t sbe_uint8_view_get_range(sbe_uint8_view view, uint8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_SAFE_GET_RANGE(view, data, offset, len);
}
SBE_ONE_DEF uint64_t sbe_uint8_view_get_range_unsafe(sbe_uint8_view view, uint8_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BUFFER_VIEW_UNSAFE_GET_RANGE(view, data, offset, len);
}

struct sbe_uint16_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_uint16_view_be& set_unsafe(const uint16_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(uint64_t index, uint16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_uint16_view_be& set_unsafe(uint64_t index, uint16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(const uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_uint16_view_be& set_unsafe(const uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    uint64_t get(uint16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(uint16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint16_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT16, uint16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint16_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, uint16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get(uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
    }
#endif
};
typedef struct sbe_uint16_view_be sbe_uint16_view_be;

SBE_ONE_DEF bool sbe_uint16_view_be_set(sbe_uint16_view_be view, const uint16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_uint16_view_be_set_buffer(sbe_uint16_view_be view, const uint16_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_uint16_view_be_set_at(sbe_uint16_view_be view, uint64_t index, uint16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_uint16_view_be_set_at_unsafe(sbe_uint16_view_be view, uint64_t index, uint16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_uint16_view_be_set_range(sbe_uint16_view_be view, const uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_uint16_view_be_set_range_unsafe(sbe_uint16_view_be view, const uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_be_get(sbe_uint16_view_be view, uint16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_be_get_unsafe(sbe_uint16_view_be view, uint16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint16_t sbe_uint16_view_be_get_at(sbe_uint16_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT16, uint16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint16_t sbe_uint16_view_be_get_at_unsafe(sbe_uint16_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, uint16_t ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_be_get_range(sbe_uint16_view_be view, uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_be_get_range_unsafe(sbe_uint16_view_be view, uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_16, uint16_t);
}

struct sbe_uint16_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_uint16_view_le& set_unsafe(const uint16_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(uint64_t index, uint16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_uint16_view_le& set_unsafe(uint64_t index, uint16_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    bool set(const uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    sbe_uint16_view_le& set_unsafe(const uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
        return *this;
    }
    uint64_t get(uint16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(uint16_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint16_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT16, uint16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint16_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, uint16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get(uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
    uint64_t get_unsafe(uint16_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
    }
#endif
};
typedef struct sbe_uint16_view_le sbe_uint16_view_le;

SBE_ONE_DEF bool sbe_uint16_view_le_set(sbe_uint16_view_le view, const uint16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_uint16_view_le_set_buffer(sbe_uint16_view_le view, const uint16_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_uint16_view_le_set_at(sbe_uint16_view_le view, uint64_t index, uint16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_uint16_view_le_set_at_unsafe(sbe_uint16_view_le view, uint64_t index, uint16_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF bool sbe_uint16_view_le_set_range(sbe_uint16_view_le view, const uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF void sbe_uint16_view_le_set_range_unsafe(sbe_uint16_view_le view, const uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_le_get(sbe_uint16_view_le view, uint16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_le_get_unsafe(sbe_uint16_view_le view, uint16_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint16_t sbe_uint16_view_le_get_at(sbe_uint16_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT16, uint16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint16_t sbe_uint16_view_le_get_at_unsafe(sbe_uint16_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, uint16_t ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_le_get_range(sbe_uint16_view_le view, uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}
SBE_ONE_DEF uint64_t sbe_uint16_view_le_get_range_unsafe(sbe_uint16_view_le view, uint16_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_16, uint16_t);
}

struct sbe_uint32_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_uint32_view_be& set_unsafe(const uint32_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(uint64_t index, uint32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_uint32_view_be& set_unsafe(uint64_t index, uint32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(const uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_uint32_view_be& set_unsafe(const uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    uint64_t get(uint32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(uint32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint32_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT32, uint32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint32_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, uint32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get(uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
#endif
};
typedef struct sbe_uint32_view_be sbe_uint32_view_be;

SBE_ONE_DEF bool sbe_uint32_view_be_set(sbe_uint32_view_be view, const uint32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_uint32_view_be_set_buffer(sbe_uint32_view_be view, const uint32_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_uint32_view_be_set_at(sbe_uint32_view_be view, uint64_t index, uint32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_uint32_view_be_set_at_unsafe(sbe_uint32_view_be view, uint64_t index, uint32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_uint32_view_be_set_range(sbe_uint32_view_be view, const uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_uint32_view_be_set_range_unsafe(sbe_uint32_view_be view, const uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_be_get(sbe_uint32_view_be view, uint32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_be_get_unsafe(sbe_uint32_view_be view, uint32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint32_t sbe_uint32_view_be_get_at(sbe_uint32_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT32, uint32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint32_t sbe_uint32_view_be_get_at_unsafe(sbe_uint32_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, uint32_t ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_be_get_range(sbe_uint32_view_be view, uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_be_get_range_unsafe(sbe_uint32_view_be view, uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}

struct sbe_uint32_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_uint32_view_le& set_unsafe(const uint32_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(uint64_t index, uint32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_uint32_view_le& set_unsafe(uint64_t index, uint32_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(const uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_uint32_view_le& set_unsafe(const uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    uint64_t get(uint32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(uint32_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint32_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT32, uint32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint32_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, uint32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get(uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(uint32_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
#endif
};
typedef struct sbe_uint32_view_le sbe_uint32_view_le;

SBE_ONE_DEF bool sbe_uint32_view_le_set(sbe_uint32_view_le view, const uint32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_uint32_view_le_set_buffer(sbe_uint32_view_le view, const uint32_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_uint32_view_le_set_at(sbe_uint32_view_le view, uint64_t index, uint32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_uint32_view_le_set_at_unsafe(sbe_uint32_view_le view, uint64_t index, uint32_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_uint32_view_le_set_range(sbe_uint32_view_le view, const uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_uint32_view_le_set_range_unsafe(sbe_uint32_view_le view, const uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_le_get(sbe_uint32_view_le view, uint32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_le_get_unsafe(sbe_uint32_view_le view, uint32_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint32_t sbe_uint32_view_le_get_at(sbe_uint32_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT32, uint32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint32_t sbe_uint32_view_le_get_at_unsafe(sbe_uint32_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, uint32_t ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_le_get_range(sbe_uint32_view_le view, uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_uint32_view_le_get_range_unsafe(sbe_uint32_view_le view, uint32_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}

struct sbe_uint64_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_uint64_view_be& set_unsafe(const uint64_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(uint64_t index, uint64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_uint64_view_be& set_unsafe(uint64_t index, uint64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(const uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_uint64_view_be& set_unsafe(const uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    uint64_t get(uint64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(uint64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT64, uint64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, uint64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
#endif
};
typedef struct sbe_uint64_view_be sbe_uint64_view_be;

SBE_ONE_DEF bool sbe_uint64_view_be_set(sbe_uint64_view_be view, const uint64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_uint64_view_be_set_buffer(sbe_uint64_view_be view, const uint64_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_uint64_view_be_set_at(sbe_uint64_view_be view, uint64_t index, uint64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_uint64_view_be_set_at_unsafe(sbe_uint64_view_be view, uint64_t index, uint64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_uint64_view_be_set_range(sbe_uint64_view_be view, const uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_uint64_view_be_set_range_unsafe(sbe_uint64_view_be view, const uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_be_get(sbe_uint64_view_be view, uint64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_be_get_unsafe(sbe_uint64_view_be view, uint64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_be_get_at(sbe_uint64_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT64, uint64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_be_get_at_unsafe(sbe_uint64_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, uint64_t ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_be_get_range(sbe_uint64_view_be view, uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_be_get_range_unsafe(sbe_uint64_view_be view, uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}

struct sbe_uint64_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const uint64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_uint64_view_le& set_unsafe(const uint64_t *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(uint64_t index, uint64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_uint64_view_le& set_unsafe(uint64_t index, uint64_t val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(const uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_uint64_view_le& set_unsafe(const uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    uint64_t get(uint64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(uint64_t *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_NULLVALUE_UINT64, uint64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, uint64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(uint64_t *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
#endif
};
typedef struct sbe_uint64_view_le sbe_uint64_view_le;

SBE_ONE_DEF bool sbe_uint64_view_le_set(sbe_uint64_view_le view, const uint64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_uint64_view_le_set_buffer(sbe_uint64_view_le view, const uint64_t *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_uint64_view_le_set_at(sbe_uint64_view_le view, uint64_t index, uint64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_uint64_view_le_set_at_unsafe(sbe_uint64_view_le view, uint64_t index, uint64_t val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_uint64_view_le_set_range(sbe_uint64_view_le view, const uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_uint64_view_le_set_range_unsafe(sbe_uint64_view_le view, const uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_le_get(sbe_uint64_view_le view, uint64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_le_get_unsafe(sbe_uint64_view_le view, uint64_t *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_le_get_at(sbe_uint64_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_NULLVALUE_UINT64, uint64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_le_get_at_unsafe(sbe_uint64_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, uint64_t ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_le_get_range(sbe_uint64_view_le view, uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_uint64_view_le_get_range_unsafe(sbe_uint64_view_le view, uint64_t *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}

struct sbe_float_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const float *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_float_view_be& set_unsafe(const float *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(uint64_t index, float val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_float_view_be& set_unsafe(uint64_t index, float val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(const float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_float_view_be& set_unsafe(const float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    uint64_t get(float *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(float *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    float get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_FLOAT_NAN, float ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    float get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, float ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get(float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
    }
#endif
};
typedef struct sbe_float_view_be sbe_float_view_be;

SBE_ONE_DEF bool sbe_float_view_be_set(sbe_float_view_be view, const float *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_float_view_be_set_buffer(sbe_float_view_be view, const float *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_float_view_be_set_at(sbe_float_view_be view, uint64_t index, float val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_float_view_be_set_at_unsafe(sbe_float_view_be view, uint64_t index, float val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_float_view_be_set_range(sbe_float_view_be view, const float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_float_view_be_set_range_unsafe(sbe_float_view_be view, const float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_be_get(sbe_float_view_be view, float *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_be_get_unsafe(sbe_float_view_be view, float *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF float sbe_float_view_be_get_at(sbe_float_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_FLOAT_NAN, float ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF float sbe_float_view_be_get_at_unsafe(sbe_float_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, float ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_be_get_range(sbe_float_view_be view, float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_be_get_range_unsafe(sbe_float_view_be view, float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_32, uint32_t);
}

struct sbe_float_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const float *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_float_view_le& set_unsafe(const float *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(uint64_t index, float val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_float_view_le& set_unsafe(uint64_t index, float val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    bool set(const float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    sbe_float_view_le& set_unsafe(const float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
        return *this;
    }
    uint64_t get(float *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(float *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    float get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_FLOAT_NAN, float ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    float get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, float ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get(float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
    uint64_t get_unsafe(float *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
    }
#endif
};
typedef struct sbe_float_view_le sbe_float_view_le;

SBE_ONE_DEF bool sbe_float_view_le_set(sbe_float_view_le view, const float *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_float_view_le_set_buffer(sbe_float_view_le view, const float *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_float_view_le_set_at(sbe_float_view_le view, uint64_t index, float val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_float_view_le_set_at_unsafe(sbe_float_view_le view, uint64_t index, float val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF bool sbe_float_view_le_set_range(sbe_float_view_le view, const float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF void sbe_float_view_le_set_range_unsafe(sbe_float_view_le view, const float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_le_get(sbe_float_view_le view, float *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_le_get_unsafe(sbe_float_view_le view, float *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF float sbe_float_view_le_get_at(sbe_float_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_FLOAT_NAN, float ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF float sbe_float_view_le_get_at_unsafe(sbe_float_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, float ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_le_get_range(sbe_float_view_le view, float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}
SBE_ONE_DEF uint64_t sbe_float_view_le_get_range_unsafe(sbe_float_view_le view, float *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_32, uint32_t);
}

struct sbe_double_view_be
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const double *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_double_view_be& set_unsafe(const double *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(uint64_t index, double val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_double_view_be& set_unsafe(uint64_t index, double val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(const double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_double_view_be& set_unsafe(const double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    uint64_t get(double *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(double *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    double get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_DOUBLE_NAN, double ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    double get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, double ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
    }
#endif
};
typedef struct sbe_double_view_be sbe_double_view_be;

SBE_ONE_DEF bool sbe_double_view_be_set(sbe_double_view_be view, const double *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_double_view_be_set_buffer(sbe_double_view_be view, const double *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_double_view_be_set_at(sbe_double_view_be view, uint64_t index, double val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_double_view_be_set_at_unsafe(sbe_double_view_be view, uint64_t index, double val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_double_view_be_set_range(sbe_double_view_be view, const double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_double_view_be_set_range_unsafe(sbe_double_view_be view, const double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_be_get(sbe_double_view_be view, double *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_be_get_unsafe(sbe_double_view_be view, double *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF double sbe_double_view_be_get_at(sbe_double_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_DOUBLE_NAN, double ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF double sbe_double_view_be_get_at_unsafe(sbe_double_view_be view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, double ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_be_get_range(sbe_double_view_be view, double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_be_get_range_unsafe(sbe_double_view_be view, double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_BIG_ENDIAN_ENCODE_64, uint64_t);
}

struct sbe_double_view_le
{
    void* data;
    size_t length;
#if defined(__cplusplus)
    bool set(const double *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_double_view_le& set_unsafe(const double *ptr)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, 0, length ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(uint64_t index, double val)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_double_view_le& set_unsafe(uint64_t index, double val)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT((*this), index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    bool set(const double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    sbe_double_view_le& set_unsafe(const double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
        return *this;
    }
    uint64_t get(double *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(double *ptr, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    double get(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT((*this), index, SBE_DOUBLE_NAN, double ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    double get_unsafe(uint64_t index)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT((*this), index, double ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get(double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
    uint64_t get_unsafe(double *ptr, uint64_t offset, uint64_t len)
    {
        _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE((*this), ptr, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
    }
#endif
};
typedef struct sbe_double_view_le sbe_double_view_le;

SBE_ONE_DEF bool sbe_double_view_le_set(sbe_double_view_le view, const double *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_double_view_le_set_buffer(sbe_double_view_le view, const double *data)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, 0, view.length ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_double_view_le_set_at(sbe_double_view_le view, uint64_t index, double val)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_double_view_le_set_at_unsafe(sbe_double_view_le view, uint64_t index, double val)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_AT(view, index, val ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF bool sbe_double_view_le_set_range(sbe_double_view_le view, const double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF void sbe_double_view_le_set_range_unsafe(sbe_double_view_le view, const double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_SET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_le_get(sbe_double_view_le view, double *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_le_get_unsafe(sbe_double_view_le view, double *data, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, 0, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF double sbe_double_view_le_get_at(sbe_double_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_AT(view, index, SBE_DOUBLE_NAN, double ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF double sbe_double_view_le_get_at_unsafe(sbe_double_view_le view, uint64_t index)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_AT(view, index, double ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_le_get_range(sbe_double_view_le view, double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_SAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}
SBE_ONE_DEF uint64_t sbe_double_view_le_get_range_unsafe(sbe_double_view_le view, double *data, uint64_t offset, uint64_t len)
{
    _SBE_BYTE_ORDER_BUFFER_UNSAFE_GET_RANGE(view, data, offset, len ,SBE_LITTLE_ENDIAN_ENCODE_64, uint64_t);
}

#endif /* _SBE_POLYFILL_HEADER_H */
