/**
******************************************************************************
* @file           : fota.c
* @brief          : fota implementation file
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

#include "fota.h"
#include "esp8266/esp8266.h"
#include "mqtt/mqtt.h"

/** @defgroup ST libs */
#include "usart.h"
#include "can.h"
#include "crc.h"
#include "spi.h"
/** @defgroup STD Libs */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
* ===============================================================================================
*   > Private Macros
* ===============================================================================================
*/

/** @defgroup ST Handles */
#define __BTL_COMM_ST_UART_HANDLE	huart1
#define __BTL_COMM_ST_CAN_HANDLE 	hcan
#define __BTL_DBG_ST_UART_HANDLE   huart2
#define __BTL_DBG_ST_CAN_HANDLE    hcan

#define __BTL_COMM_ST_UART_HANDLE_DEF()	 extern UART_HandleTypeDef __BTL_COMM_ST_UART_HANDLE
#define __BTL_COMM_ST_CAN_HANDLE_DEF()   extern CAN_HandleTypeDef __BTL_COMM_ST_CAN_HANDLE
#define __BTL_DBG_ST_UART_HANDLE_DEF()	 extern UART_HandleTypeDef __BTL_DBG_ST_UART_HANDLE
#define __BTL_DBG_ST_CAN_HANDLE_DEF()    extern CAN_HandleTypeDef __BTL_DBG_ST_CAN_HANDLE

#if (BL_COMM_PIPE >= 0x00)
#	if(BL_COMM_PIPE == BL_COMM_OVER_UART)
__BTL_COMM_ST_UART_HANDLE_DEF();
#		if(BL_COMM_TYPE == BL_COMM_SYNC)
#			define PIPE_LISTEN(__BUFFER, __LEN) HAL_UART_Receive(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN), HAL_MAX_DELAY)
#			define PIPE_ECHO(__BUFFER, __LEN) HAL_UART_Transmit(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN), HAL_MAX_DELAY)
#    else
#			define PIPE_LISTEN(__BUFFER, __LEN) HAL_UART_Receive_IT(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN))
#			define PIPE_ECHO(__BUFER, __LEN) HAL_UART_Transmit_IT(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN))
#    endif /* (BL_COMM_TYPE == BL_COMM_SYNC) */
#	elif(BL_COMM_PIPE == BL_COMM_OVER_CAN)
__BTL_COMM_ST_CAN_HANDLE_DEF();
#		if(BL_COMM_TYPE == BL_COMM_SYNC)
#			define PIPE_LISTEN(__BUFFER)
#			define PIPE_ECHO(__BUFER, __LEN) 
#    else
#			define PIPE_LISTEN(__BUFFER)
#			define PIPE_ECHO(__BUFER, __LEN) 
#    endif /* (BL_COMM_TYPE == BL_COMM_SYNC) */
#	else
#	error (" No communication pipe interface defined !")
#	endif /* (BL_COMM_PIPE == BL_COMM_OVER_UART) */
#endif /* (BL_COMM_PIPE >= 0x00) */

#if defined(BL_DBG_PORT)
# define BL_DBG_SEND(DBG_MSG, ...)	__bl_vDbgWrt("\r\nVENDOR_ID: %d, MODULE_ID: %d <FILE: %s - FUNC: %s - LINE: %d> \nDBG_MSG:> "DBG_MSG,   \
												BOOTLOADER_VENDOR_ID,											    									\
												BOOTLOADER_MODULE_ID,										 	    									\
												__FILE__,														    												\
												__FUNCTION__,														    										\
												__LINE__,													  														\
												##__VA_ARGS__)													    										\

#	if (BL_DBG_PORT == DBG_PORT_UART)
__BTL_DBG_ST_UART_HANDLE_DEF();
#			define __DBG_SEND_OVER_X(__BUFFER, __LEN) HAL_UART_Transmit(&__BTL_DBG_ST_UART_HANDLE, (uint8*)(&__BUFFER[0]), (uint16)(__LEN), HAL_MAX_DELAY)
#	elif(BL_DBG_PORT == DBG_PORT_CAN)
__BTL_DBG_ST_CAN_HANDLE_DEF();
#		define __DBG_SEND_OVER_X 
#	else
#		error (" No dbg port interface defined !")
#	endif /* (BL_DBG_PORT == DBG_PORT_UART) */
#else
#		define BL_DBG_SEND(DBG_MSG, ...) (0)
#endif /* (BL_DBG_PORT > 0x00) */

/**
* ===============================================================================================
*   > Global Private Types
* ===============================================================================================
*/




/**
* ===============================================================================================
*   > Private Functions Declaration
* ===============================================================================================
*/

__STATIC void __bl_vDbgWrt(const uint8 * pArg_u8StrFormat, ...);

/**
* ===============================================================================================
*   > Private Functions Implementation
* ===============================================================================================
*/

#if defined(BL_DBG_PORT)
__STATIC void __bl_vDbgWrt(const uint8 * pArg_u8StrFormat, ...) {
	uint8 local_u8DbgBuffer[DBG_BUFFER_MAX_SIZE] = {0};
	/* Create variadic argument */
	va_list args;
	/* Enable access to the variadic argument */
	va_start(args, pArg_u8StrFormat);
	/* Write formatted data from variable argument list to string */
	vsprintf((char *)local_u8DbgBuffer, pArg_u8StrFormat, args);
	/* Print the message via the channel speicfied */
	__DBG_SEND_OVER_X(local_u8DbgBuffer, strlen((const char *)local_u8DbgBuffer));
	/* Clean up the instant */
	va_end(args);
}
#endif



/**
* ===============================================================================================
*   > Public Functions Implementation
* ===============================================================================================
*/
