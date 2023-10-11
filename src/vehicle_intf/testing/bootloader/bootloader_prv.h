/**
******************************************************************************
* @file           : bootloader_prv.h
* @brief          : Bootloader private interface file
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
#ifndef __BOOTLOADER_PRV_H__
#define __BOOTLOADER_PRV_H__

/** @brief Vendor specific ID */
#define BOOTLOADER_VENDOR_ID                       (1332)
#define BOOTLOADER_MODULE_ID 					   				   (120)
/** @brief Program based version */
#define BOOTLOADER_SW_MAJOR_VERSION                (1)
#define BOOTLOADER_SW_MINOR_VERSION                (0)
#define BOOTLOADER_SW_PATCH_VERSION                (0)


/**
* ===============================================================================================
*   > Private Macros
* ===============================================================================================
*/

#define NOTIMPLEMENTED ({ do 		\
									\
				while(1) })			\

/** @defgroup ST Handles */
#define __BTL_COMM_ST_UART_HANDLE	huart1
#define __BTL_COMM_ST_CAN_HANDLE 	hcan
#define __BTL_DBG_ST_UART_HANDLE   huart2
#define __BTL_DBG_ST_CAN_HANDLE    hcan

#define __BTL_COMM_ST_UART_HANDLE_DEF()	 extern UART_HandleTypeDef __BTL_COMM_ST_UART_HANDLE
#define __BTL_COMM_ST_CAN_HANDLE_DEF()   extern CAN_HandleTypeDef __BTL_COMM_ST_CAN_HANDLE
#define __BTL_DBG_ST_UART_HANDLE_DEF()	 extern UART_HandleTypeDef __BTL_DBG_ST_UART_HANDLE
#define __BTL_DBG_ST_CAN_HANDLE_DEF()    extern CAN_HandleTypeDef __BTL_DBG_ST_CAN_HANDLE

#define PIPE_BUFFER_MAX_SIZE ( (uint8) (255U) )
#define DBG_BUFFER_MAX_SIZE  ( (uint8) (255U) )
/** @defgroup debugging configurations */
#define DBG_PORT_UART  ( (0x00U) )
#define DBG_PORT_CAN   ( (0x01U) )

/** @defgroup Bootloader Base Init */
#define BL_START_ADDRESS	
#define BL_MAX_SIZE	

/** @defgroup Bootloader configurations */
#define BL_COMM_OVER_CAN  ( (0x00U) )
#define BL_COMM_OVER_UART ( (0x01U) )
// #define BL_COMM_OVER_I2C  ( (0x02U) )

#define BL_COMM_SYNC  	  ( (0x00U) )
#define BL_COMM_ASYNC 	  ( (0x01U) )

/** @defgroup Bootloader response */
#define BL_CMD_RESPONSE_NACK				( (uint8) (0) )
#define BL_CMD_RESPONSE_ACK				  ( (uint8) (1) )

/** @defgroup Packet informations */
#define CRC_LENGTH_IN_BYTES 					(4u)

/** @defgroup Device Specific informations */
#define STM32F103C8Tx_SRAM_PAGE_SIZE 	(1024u)
#define STM32F103C8Tx_SRAM_PAGE_NUM 	(20u)
#define STM32F103C8Tx_SRAM1_SIZE 			(STM32F103C8Tx_SRAM_PAGE_SIZE * STM32F103C8Tx_SRAM_PAGE_NUM)
#define STM32F103C8Tx_SRAM1_END				(SRAM_BASE + STM32F103C8Tx_SRAM1_SIZE)
#define STM32F103C8Tx_FLASH_PAGE_NUM 	(64u)
#define STM32F103C8Tx_FLASH_PAGE_SIZE (1024u)
#define STM32F103C8Tx_FLASH_SIZE 			(STM32F103C8Tx_FLASH_PAGE_SIZE * STM32F103C8Tx_FLASH_PAGE_NUM)
#define STM32F103C8Tx_FLASH_END 			(FLASH_BASE + STM32F103C8Tx_FLASH_SIZE)
#define BTL_FLASH_MAX_PAGE_NUM 				(STM32F103C8Tx_FLASH_PAGE_NUM)
#define BTL_FLASH_MASS_ERASE 					(BTL_FLASH_MAX_PAGE_NUM + 1u)

/** @defgroup Commands */
#define NUM_OF_CMD 										( (uint8) (12u) )
#define	CBL_GET_VER_CMD								( (uint8) (0) )
#define	CBL_GET_HELP_CMD							( (uint8) (1) )
#define	CBL_GET_CID_CMD								( (uint8) (2) )
#define	CBL_GET_RDP_STATUS_CMD				( (uint8) (3) )
#define	CBL_GO_TO_ADDR_CMD						( (uint8) (4) )
#define	CBL_FLASH_ERASE_CMD						( (uint8) (5) )
#define	CBL_MEM_WRITE_CMD							( (uint8) (6) )
#define	CBL_EN_R_W_PROTECT_CMD				( (uint8) (7) )
#define	CBL_MEM_READ_CMD							( (uint8) (8) )
#define	CBL_OTP_READ_CMD							( (uint8) (9) )
#define	CBL_DIS_R_W_PROTECT_CMD				( (uint8) (10) )
#define CBL_READ_SECTOR_STATUS 				( (uint8) (11) )

/** @defgroup Packet Type */
#define PACKET_TYPE_REQUEST_DATA				( (uint8) (0) )
#define PACKET_TYPE_DATA_FOR_FLASH			( (uint8) (1) )
#define PACKET_TYPE_CMD									( (uint8) (2) )

/**
* ===============================================================================================
*   > Private Datatypes
* ===============================================================================================
*/

/** @defgroup Bootloader defined types */
typedef struct __packetSerialization packet_t;
typedef boolean flag_t;

typedef uint8* hash_t;
typedef uint8 sha256_t[256u];

/**
* ===============================================================================================
*   > Private Structs
* ===============================================================================================
*/

/** @brief Struct container for the bootloader sw version */
struct __bootloaderVersion {
	uint16 vendorID;
	uint16 moduleID;
	uint8 sw_major_version;
	uint8 sw_minor_version;
	uint8 sw_mpatch_version;
};

/** @brief Struct for packet serialization */
struct __packetSerialization {
	uint8 PacketLength;
	uint8 PacketType;
	uint8 Command;
	uint32 Data;
	uint32 DataCRC;
	uint32 PacketCRC;
};

/**
* ===============================================================================================
*   > Private Enums
* ===============================================================================================
*/

/**
* ===============================================================================================
*   > Private Functions Declaration
* ===============================================================================================
*/
#ifdef __cplusplus 
extern "c" {
#endif /* __cplusplus */

#if (BL_DBG_PORT >= 0x00)
__STATIC void __bl_vDbgWrt(const uint8 * pArg_u8StrFormat, ...);
#endif

__STATIC __en_blErrStatus_t __bl_enExecuteCommand(const packet_t* pArg_tPacket);

__LOCAL_INLINE __en_blStatus_t __enGetIsValidAppFlag(void);
__LOCAL_INLINE __en_blStatus_t __enGetIsValidHashFlag(void);
__LOCAL_INLINE __en_blStatus_t __enGetIsAppToBlFlag(void);
__LOCAL_INLINE __NORETURN __vSetIsValidAppFlag(flag_t volatile Arg_tValue);
__LOCAL_INLINE __NORETURN __vSetIsValidHashFlag(flag_t volatile Arg_tValue);
__LOCAL_INLINE __NORETURN __vSetIsAppToBlFlag(flag_t volatile Arg_tValue);

__STATIC __en_blErrStatus_t __enPipeListen(void);
__STATIC __NORETURN __vPipeEcho(uint8* pArg_u8TxBuffer, uint8 Arg_u8Length);

__STATIC __NORETURN __vSendAck(void);
__STATIC __NORETURN __vSendNack(void);

__STATIC hash_t __tCalculateSHA256HashForApplication(void);

__STATIC __FORCE_NORETURN __vJumpToApplication(void);
__STATIC __en_blErrStatus_t __enVerifyAddress(uint32 Arg_McuAddressValue);

__STATIC __NORETURN __vSeralizeReceivedBuffer(packet_t* pArg_tPacket, uint8* pArg_tReceivedBuffer);

/** 
 * @defgroup commands handlers 
 **/ 

__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_VER_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_HELP_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_CID_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_RDP_STATUS_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GO_TO_ADDR_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_CBL_FLASH_ERASE_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_MEM_WRITE_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_EN_R_W_PROTECT_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_READ_SECTOR_STATUS(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_MEM_READ_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_OTP_READ_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_DIS_R_W_PROTECT_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_READ_SECTOR_STATUS(void);

/**
  * @}
  */

#ifdef __cplusplus 
}
#endif /* __cplusplus */
#endif /* __BOOTLOADER_PRV_H__ */
