/**
******************************************************************************
* @file           : Std_Types.h
* @brief          : Macros defined for the project (Vertical Lib)
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
#ifndef __STDTYPES_H__
#define __STDTYPES_H__

/** @brief Included headers */
#include "Platform_Types.h"
#include "Compiler.h"

/** @brief Vendor specific ID */
#define STD_VENDOR_ID                                (1332)
/** @brief Program based version */
#define STD_SW_MAJOR_VERSION                         (1)
#define STD_SW_MINOR_VERSION                         (0)
#define STD_SW_PATCH_VERSION                         (0)

/*
 * =============================================================================
 * = CONSTANTS
 * =============================================================================
*/

/**
 * @brief Physical state 5V or 3.3V
 * @implements SymbolDefinitions_enumeration
 */
#define STD_HIGH 0x01
/**
 * @brief Physical state 0V
 * @implements SymbolDefinitions_enumeration
 */
#define STD_LOW 0x00
/**
 * @brief Logical state active.
 * @implements SymbolDefinitions_enumeration
 */
#define STD_ACTIVE 0x01
/**
 * @brief Physical state idle.
 * @implements SymbolDefinitions_enumeration
 */
#define STD_IDLE 0x00
/**
 * @brief ON State.
 * @implements SymbolDefinitions_enumeration
 */
#define STD_ON 0x01
/**
 * @brief OFF State.
 * @implements SymbolDefinitions_enumeration
 */
#define STD_OFF 0x00
/**
 * @brief Return code for failure/error.
 * @implements SymbolDefinitions_enumeration
 */
    #define E_NOT_OK 0x01
/**
 * @brief Because E_OK is already defined within OSEK, the symbol E_OK has to be shared.
 *              To avoid name clashed and redfinitions problems. The symbols have to be defined in the following way
 */
#ifndef STATUSTYPEDEFINED
    #define STATUSTYPEDEFINED
    /**
     * @brief Success return code
     */
    #define E_OK 0x00
    /**
     * @brief This type is defined for OSEK compilance.
     */
    typedef unsigned char StatusType;
#endif /* STATUSTYPEDEFINED */
/**
 * @brief This type can be used as standard API return type which is shared between the RTE and BSW modules.
 * @implements Std_ReturnType_type
 */
typedef uint8 Std_ReturnType;
/**
 * @brief This type shall be sued to request the version of a BSW module using the `"ModuleName"_GetVersionInfro()` function.
 * @implements Std_VersionInfoType_structure
 */
typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8 sw_major_version;
    uint8 sw_minor_version;
    uint8 sw_mpatch_version;
}Std_VersionInfoType;


#endif /* __STDTYPES_H__ */
