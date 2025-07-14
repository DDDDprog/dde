#ifndef TYPES_H
#define TYPES_H

/* Basic type definitions for freestanding environment */

/* Signed integer types */
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

/* Unsigned integer types */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/* Size and pointer types */
typedef unsigned int size_t;
typedef int ptrdiff_t;
typedef void* intptr_t;
typedef unsigned int uintptr_t;

/* Boolean type */
typedef enum {
    false = 0,
    true = 1
} bool;

/* NULL pointer */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* Limits */
#define INT8_MAX 127
#define INT8_MIN (-128)
#define UINT8_MAX 255

#define INT16_MAX 32767
#define INT16_MIN (-32768)
#define UINT16_MAX 65535

#define INT32_MAX 2147483647
#define INT32_MIN (-2147483648)
#define UINT32_MAX 4294967295U

#define SIZE_MAX UINT32_MAX

#endif /* TYPES_H */