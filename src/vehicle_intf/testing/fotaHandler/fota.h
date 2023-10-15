/**
******************************************************************************
* @file           : fota.h
* @brief          : fota public interface file
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
#ifndef __FOTA_H__
#define __FOTA_H__

#include "Std_Types.h"

/** @brief Vendor specific ID */
#define FOTA_VENDOR_ID                       (1332)
#define FOTA_MODULE_ID 					   					 (120)
/** @brief Program based version */
#define FOTA_SW_MAJOR_VERSION                (1)
#define FOTA_SW_MINOR_VERSION                (0)
#define FOTA_SW_PATCH_VERSION                (0)


#ifdef __cplusplus 
extern "c" {
#endif /* __cplusplus */

/**
* ===============================================================================================
*   > Fota Communication Pipe Interface CFG
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
*   > Fota Debugging CFG
* ===============================================================================================
*/
/** 
 * @brief ECU application start address
 *  @arguments
 *  	@arg DBG_PORT_UART
 *  	@arg DBG_PORT_CAN
*/
#define BL_DBG_PORT DBG_PORT_UART


#ifdef __cplusplus 
}
#endif /* __cplusplus */
#endif /* __FOTA_H__ */
