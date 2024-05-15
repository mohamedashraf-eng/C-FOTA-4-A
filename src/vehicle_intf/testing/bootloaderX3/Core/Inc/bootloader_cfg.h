/**
******************************************************************************
* @file           : bootloader_cfg.h
* @brief          : Bootloader Configurations
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
* Autosar Version      : 4.4.0
* Autosar Revision     : ASR_REL_4_0_REV_0000
* SW Version           : 1.0.0
* =============================================================================
*/
#ifndef __BOOTLOADER_CFG_H__
#define __BOOTLOADER_CFG_H__

/** @brief Vendor specific ID */
#define BOOTLOADER_VENDOR_ID                       (1332)
#define BOOTLOADER_MODULE_ID                       (120)
/** @brief Program based version */
#define BOOTLOADER_SW_MAJOR_VERSION                (1)
#define BOOTLOADER_SW_MINOR_VERSION                (0)
#define BOOTLOADER_SW_PATCH_VERSION                (0)

/**
* ===============================================================================================
*   > Bootloader Communication Pipe Interface CFG
* ===============================================================================================
*/

/** 
 * @brief Communication pipe for the bootloader
 * @arguments
 *	@arg BL_COMM_OVER_CAN
 *	@arg BL_COMM_OVER_UART
*/
#define BL_COMM_PIPE	BL_COMM_OVER_UART
/** 
 * @brief Communication type for the bootloader
 * @arguments
 *	@arg BL_COMM_SYNC
 *	@arg BL_COMM_ASYNC
*/
#define BL_COMM_TYPE	BL_COMM_SYNC

/**
* ===============================================================================================
*   > Bootloader Internal CFG
* ===============================================================================================
*/

/** 
 * @brief MCU application start address
 * @attention DON'T CHANGE THIS LINE !!!!!
 * 
*/
#define APP_START_ADDR	( (uint32) (0x08008000UL) )

/** 
 * @brief Communication type for the bootloader
 * @arguments
 *	@arg BL_SECURE_BOOT_ON
 *	@arg BL_SECURE_BOOT_OFF
*/
#define BL_SECURE_BOOT_OFF 			(0u)
#define BL_SECURE_BOOT_ON 			(1u)
#define BL_SECURE_BOOT  BL_SECURE_BOOT_OFF

/** 
 * @brief Pipe listner timeout in milliseconds
*/
#define PIPE_TIMEOUT_MS       (20000u)

/**
* ===============================================================================================
*   > Bootloader Debugging & Logging CFG
* ===============================================================================================
*/

/** 
 * @brief ECU application start address
 *  @arguments
 *  	@arg DBG_PORT_UART
 *  	@arg DBG_PORT_CAN
*/
// #define BL_DBG_PORT DBG_PORT_UART
/** 
 * @brief Light Weight Debugging 
 * @note defining the LWD enables the functionality automatically
*/
#define LWD

// #define BL_LOG_PORT DBG_PORT_UART

#endif /* __BOOTLOADER_CFG_H__ */









