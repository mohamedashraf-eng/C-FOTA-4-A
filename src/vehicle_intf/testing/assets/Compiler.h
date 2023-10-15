/**
******************************************************************************
* @file           : Compiler.h
* @brief          : Compiler dependent types
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
* Project              : AUTOSAR 4.4.0 MCAL
* Platform             : ARM
* Peripheral           : STM32F103
* Autosar Version      : 4.4.0
* Autosar Revision     : ASR_REL_4_0_REV_0000
* SW Version           : 1.0.0
* =============================================================================
*/
/** @def Header Guards */
#ifndef __COMPILER_H__
#define __COMPILER_H__

/** @brief Included headers */
#include "Compiler_Cfg.h"

/*
 * =============================================================================
 * = SOURCE FILE VERSION INFORMATION
 * =============================================================================
*/

/** @brief Vendor specific ID */
#define COMPILER_VENDOR_ID                               (1332)
/** @brief Program based version */
#define COMPILER_SW_MAJOR_VERSION                        (1)
#define COMPILER_SW_MINOR_VERSION                        (0)
#define COMPILER_SW_PATCH_VERSION                        (0)

/*
 * =============================================================================
 * = FILE VERSION CHECKS
 * =============================================================================
*/

#ifndef DISABLE_MCAL_INDERMODULE_ASR_CHECK
    /**
     * @brief Check version compatibility for `Compiler_Cfg.h`
    */
    #if ((COMPILER_AR_RELEASE_MAJOR_VERSION != COMPILER_CFG_AR_RELEASE_MAJOR_VERSION) || \
            (COMPILER_AR_RELEASE_MINOR_VERSION != COMPILER_CFG_AR_RELEASE_MINOR_VERSION))
        /** @brief Flag an error. */
        #error ("AUTOSAR versions numbers of `Compiler.h` and `Compiler_Cfg.h` are different.")
    #endif
#endif /* #ifndef DISABLE_MCAL_INDERMODULE_ASR_CHECK */

/*
 * =============================================================================
 * = COMPILER GENERIC DECLARATION
 * =============================================================================
*/

/**
 * @brief Defining the compiler being used.
 *
 */
#define _GNU_ARM_C_STM32F103_
#define __COMPILER__ _GNU_ARM_C_STM32F103_
/**
 * @brief The memory class AUTOMATIC shall be provided as empty definition, used for the declaration
 *              of local pointers.
 */
#define AUTOMATIC
/**
 * @brief The memory class TYPEDEF shall be provided as empty definition. This memory class shall be
 *              used within type definitions, where no memory qualifier can be specified. This can be
 *              necessary for defining pointer types, with e.g. P2VAR, where the macros require two
 *              parameters. first parameter can be specified in the type definition (distance to memory location
 *              referenced by the pointer), but the second one (memory allocation of the pointer itself),
 *              cannot be defined at this time. Hence memory class TYPEDEF shall be applied
 *
 */
#define TYPEDEF
/**
 * @brief The compiler abstraction shall provide NULL_PTR define with a void pointer to zero definition.
 */
#define NULL_PTR ((void *) 0)

/**
 * @brief The memory class AUTOMATIC shall be provided as empty definition, user for the declaration
 *              of local pointers.
 */
#define AUTOMATIC

/*
 * =============================================================================
 * = COMPILER SPECIFIC DECLARATION
 * =============================================================================
*/

#ifndef COMPILER_BEING_USED
    #define COMPILER_BEING_USED __COMPILER__
#endif /* COMPILER_BEING_USED */

/**
 * @defgroup GREENHILLS Compiler specifications
*/
#ifdef _GREENHILLS_C_STM32F103_
    /**
     * @brief The compiler abstraction shall provide the __INLINE define for abstraction of the keyword
     *              inline.
     */
    #define __INLINE __inline
    /**
     * @brief The compiler abstraction shall provide the __LOCAL_INLINE define for abstraction of the keyword
     *              inline in function with static scope.
     */
    #define __STATIC
    #define __LOCAL_INLINE __STATIC __INLINE
    /**
     * @brief The compiler abstraction of specifiying an interrupt handler.
     */
    #define __INTERRUPT_FUNC __interrupt
#endif /* _GREENHILLS_C_STM32F103_ */

/**
 * @defgroup IAR Compiler specifications
*/
#ifdef _IAR_C_STM32F103_
    /**
     * @brief The compiler abstraction shall provide the __INLINE define for abstraction of the keyword
     *              inline.
     */
    #define __INLINE inline
    /**
     * @brief The compiler abstraction shall provide the __LOCAL_INLINE define for abstraction of the keyword
     *              inline in function with static scope.
     */
    #define __STATIC static
    #define __LOCAL_INLINE __STATIC __INLINE
    /**
     * @brief The compiler abstraction of specifiying an interrupt handler.
     */
    #define __INTERRUPT_FUNC
#endif /* _IAR_C_STM32F103_ */

/**
 * @defgroup GNU ARM Compiler specifications
*/
#ifdef _GNU_ARM_C_STM32F103_
    /**
     * @brief The compiler abstraction shall provide the __INLINE define for abstraction of the keyword
     *              inline.
     */
    #define __INLINE inline
    #define __FORCE_INLINE __attribute__((always_inline)) __INLINE
    /**
     * @brief The compiler abstraction shall provide the __LOCAL_INLINE define for abstraction of the keyword
     *              inline in function with static scope.
     */
    #define __STATIC static
    #define __LOCAL_INLINE __STATIC __INLINE
    /**
     * @brief The compiler abstraction of specifiying an interrupt handler.
     */
    #define __INTERRUPT_FUNC
    /**
     * @brief The compiler abstraction of const type
     */
    #define __FORCE_CONST __attribute__((const))
    /**
     * @brief The compiler abstraction of no return
     */
		#define __NORETURN void
    #define __FORCE_NORETURN __attribute__((noreturn)) void
    /**
     * @brief The compiler abstraction of assembly function
     */
		#define __ASM	__asm
    /**
     * @brief The compiler abstraction of struct packing
     */
    #define __PACKED	__attribute__((packed))
    /**
     * @brief The compiler abstraction of memory abstraction allignment
     */
    #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif /* _GNU_ARM_C_STM32F103_ */

/**
 * @brief The compiler abstraction shall define the FUNC macro for the declaration and definitions of
 *              functions, that ensures correct syntax of function declarations as required by a specific compiler.
 */
#define FUNC(rettype, memclass) rettype
/**
 * @brief The compiler abstraction shall define the P2VAR macro for the delcaration and definition of
 *              pointers in RAM, pointing to variables.
 */
#define P2VAR(ptrtype, memclass, ptrclass) ptrtype *
/**
 * @brief The compiler abstraction shall define the P2CONST macro for the delcaration and definition of
 *              pointers pointing to constants.
 */
#define P2CONST(ptrtype, memclass, ptrclass) const ptrtype *
/**
 * @brief The compiler abstraction shall define the CONSTP2CONST macro for the delcaration and definition of
 *              constant pointers pointing to constants.
 */
#define CONSTP2CONST(ptrtype, memclass, ptrclass) const ptrtype * const
/**
 * @brief The compiler abstraction shall define the P2FUNC macro for the delcaration and definition of
 *              pointer to function.
 */
#define P2FUNC(rettype, ptrclass, fctname) rettype (*fctname)
/**
 * @brief The compiler abstraction shall define the CONST macro for the delcaration and definition of
 *              constants
 */
#define CONST(consttype, memclass) const consttype
/**
 * @brief The compiler abstraction shall define the VAR macro for the delcaration and definition of
 *              variables.
 */
#define VAR(vartype, memclass) vartype

#endif /* __COMPILER_H__ */
