/**
******************************************************************************
* @file           : platform.h
* @brief          : platform dependent types
******************************************************************************
* @attention
*
* All rights reserved.
* CF4A
* Copyright (c) 2023 Neoprimalists
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************
*/
/*
* =============================================================================
* Project              : CF4A
* Platform             : ARM
* Peripheral           : STM32F103
* SW Version           : 1.0.0
* =============================================================================
*/
/** @def Header Guards */
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/** @brief Vendor specific ID */
#define PLATFORM_VENDOR_ID                       (1332)
/** @brief Program based version */
#define PLATFORM_SW_MAJOR_VERSION                (1)
#define PLATFORM_SW_MINOR_VERSION                (0)
#define PLATFORM_SW_PATCH_VERSION                (0)


/*
 * =============================================================================
 * = DEFINES AND MACROS
 * =============================================================================
*/
/** @defgroup CPU Type */
/**
 * @brief 8 bit Type Processor
 * @implements CPU_TYPE_enumration
*/
#define CPU_TYPE_8      8
/**
 * @brief 16 bit Type Processor
 * @implements CPU_TYPE_enumration
*/
#define CPU_TYPE_16     16
/**
 * @brief 32 bit Type Processor
 * @implements CPU_TYPE_enumration
*/
#define CPU_TYPE_32     32
/**
 * @brief 64 bit Type Processor
 * @implements CPU_TYPE_enumration
*/
#define CPU_TYPE_64     64

/** @defgroup MSB/LSB Type */
/**
 * @brief MSB First Processor
 * @implements CPU_BIT_ORDER_enumration
*/
#define MSB_FIRST       0
/**
 * @brief LSB First Processor
 * @implements CPU_BIT_ORDER_enumration
*/
#define LSB_FIRST       1

/** @defgroup LOW/HIGH Byte Type */
/**
 * @brief LOW_BYTE_FIRST Processor
 * @implements CPU_BYTE_ORDER_enumration
*/
#define LOW_BYTE_FIRST      0
/**
 * @brief HIGH_BYTE_FIRST Processor
 * @implements CPU_BYTE_ORDER_enumration
*/
#define HIGH_BYTE_FIRST     1

/**
 * @brief Processor Type
 * @implements CPU_TYPE_enumration
*/
#define CPU_TYPE                 (CPU_TYPE_32)
/**
 * @brief Bit order on register level
 * @implements CPU_BIT_ORDER_enumration
*/
#define CPU_BIT_ORDER            (LSB_FIRST)
/**
 * @brief The byte order on memory level shall be indicated in the platform types header file using
 *              the symbol CPU_BYTE_ORDER.
 * @implements CPU_BYTE_ORDER_enumration
*/
#define CPU_BYTE_ORDER          (LOW_BYTE_FIRST)

#ifndef TRUE
/**
 * @brief Boolean true value
 * @implements TRUE_FALSE_enumration
 */
    #define TRUE 1
#endif
#ifndef FALSE
/**
 * @brief Boolean false value
 * @implements TRUE_FALSE_enumration
 */
    #define FALSE 0
#endif

/*
 * =============================================================================
 * > STRUCTERS AND OTHER TYPEDEFS
 * =============================================================================
*/
#if (CPU_TYPE == CPU_TYPE_64)
#endif /* CPU_TYPE_64 */


#if (CPU_TYPE == CPU_TYPE_32)
/**
 * @brief The standard AUTOSAR type boolean shall be implemented on basis of and eight bits long
 *              unsigned integer.
 * @implements boolean_type
 */
typedef unsigned char boolean;
/** @defgroup Unsigned integers */
/**
 * @brief Unsigned 8 bit integer with range of 0 ..+255 (0x00..0xFF) - 8 bit
 * @implements uint8_type
 */
typedef unsigned char uint8;
/**
 * @brief Unsigned 16 bit integer with range of 0 ..+65535 (0x0000..0xFFFF) - 16 bit
 * @implements uint16_type
 */
typedef unsigned short uint16;
/**
 * @brief Unsigned 16 bit integer with range of 0 ..+4294967295 (0x00000000..0xFFFFFFFF) - 32 bit
 * @implements uint32_type
 */
typedef unsigned long uint32;
/**
 * @brief Unsigned 16 bit integer with range of 0 ..+18446744073709551615 (0x0000000000000000..0xFFFFFFFFFFFFFFFF) - 64 bit
 * @implements uint64_type
 */
typedef unsigned long long uint64;
/** @defgroup Signed integers */
/**
 * @brief Signed 8 bit integer with range of -128 ..+127 (0x80..0x7F) - 7 bit + 1 sign bit
 * @implements sint8_type
 */
typedef signed char sint8;
/**
 * @brief Signed 16 bit integer with range of -32768 ..+32767 (0x8000..0x7FFF) - 15 bit + 1 sign bit
 * @implements sint16_type
 */
typedef signed short sint16;
/**
 * @brief Signed 16 bit integer with range of -2147483648 ..+2147483647 (0x80000000..0x7FFFFFFF) - 31 bit + 1 sign bit
 * @implements sint32_type
 */
typedef signed long sint32;
/**
 * @brief Signed 16 bit integer with range of -9223372036854775808 ..+9223372036854775807 (0x8000000000000000..0x7FFFFFFFFFFFFFFF) - 63 bit + 1 sign bit
 * @implements sint64_type
 */
typedef signed long long sint64;
/** @defgroup unsigned least integers */
/**
 * @brief Unsigned 8 bit integer with range of 0 ..+255 (0x00..0xFF) - 8 bit
 * @implements uint8_least_type
 */
typedef unsigned long uint8_least;
/**
 * @brief Unsigned 16 bit integer with range of 0 ..+65535 (0x0000..0xFFFF) - 16 bit
 * @implements uint16_least_type
 */
typedef unsigned long uint16_least;
/**
 * @brief Unsigned 16 bit integer with range of 0 ..+4294967295 (0x00000000..0xFFFFFFFF) - 32 bit
 * @implements uint32_least_type
 */
typedef unsigned long uint32_least;
/** @defgroup Signed least integers */
/**
 * @brief Signed 8 bit integer with range of -128 ..+127 (0x80..0x7F) - 7 bit + 1 sign bit
 * @implements sint8_fast_type
 */
typedef signed char sint8_fast;
/**
 * @brief Signed 16 bit integer with range of -32768 ..+32767 (0x8000..0x7FFF) - 15 bit + 1 sign bit
 * @implements sint16_fast_type
 */
typedef signed short sint16_fast;
/**
 * @brief Signed 16 bit integer with range of -2147483648 ..+2147483647 (0x80000000..0x7FFFFFFF) - 31 bit + 1 sign bit
 * @implements sint32_fast_type
 */
typedef signed long sint32_fast;
/** @defgroup Floating point types */
/**
 * @brief 32 bit long floating point data types
 * @implements float32_type
 * @details
 *
 */
typedef float float32;
/**
 * @brief 64 bit long floating point data types
 * @implements float64_type
 */
typedef double float64;
#endif /* CPU_TYPE_32 */

#if (CPU_TYPE == CPU_TYPE_16)
#endif /* CPU_TYPE_16 */

#if (CPU_TYPE == CPU_TYPE_8)
#endif /* CPU_TYPE_8 */


#endif /* __PLATFORM_H__ */
