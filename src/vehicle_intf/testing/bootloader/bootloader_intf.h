/**
******************************************************************************
* @file           : bootloader_intf.h
* @brief          : Bootloader public interface file
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
#ifndef __BOOTLOADER_INTF_H__
#define __BOOTLOADER_INTF_H__

#include "Std_Types.h"

/** @brief Vendor specific ID */
#define BOOTLOADER_VENDOR_ID                       (1332)
#define BOOTLOADER_MODULE_ID 					   					 (120)
/** @brief Program based version */
#define BOOTLOADER_SW_MAJOR_VERSION                (1)
#define BOOTLOADER_SW_MINOR_VERSION                (0)
#define BOOTLOADER_SW_PATCH_VERSION                (0)

/**
* ===============================================================================================
*   > Public Macros
* ===============================================================================================
*/

/**
* ===============================================================================================
*   > Public Structs
* ===============================================================================================
*/

/**
* ===============================================================================================
*   > Public Enums
* ===============================================================================================
*/

/**
* ===============================================================================================
*   > Public Datatypes
* ===============================================================================================
*/


/**
* ===============================================================================================
*   > Public Functions Declaration
* ===============================================================================================
*/
#ifdef __cplusplus 
extern "c" {
#endif /* __cplusplus */

/**
* @brief	Function to listen for packet
* @details
* @param None
* @return Bootloader status
*/
__NORETURN BL_enBootManager(void);

#ifdef __cplusplus 
}
#endif /* __cplusplus */
#endif /* __BOOTLOADER_INTF_H__ */
