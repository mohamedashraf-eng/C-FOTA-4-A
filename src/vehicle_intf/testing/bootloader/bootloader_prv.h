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

#define LOGL_INFO		(0)		
#define LOGL_WARN		(1)
#define LOGL_ERR		(2)
#define LOGL_CRIT		(3)
#define BL_LOG_BUFFER_SIZE ( (uint8) (255U) )

// #define DATACONVERSION
#define BIG2LITTLE

#ifdef DATACONVERSION
#	ifdef BIG2LITTLE
#		define B2LENDIAN32(x) ((uint32)( 	 \
				(((x) & 0x000000FF) << 24) | 		\
				(((x) & 0x0000FF00) << 8)  |		\
				(((x) & 0x00FF0000) >> 8)  | 		\
				(((x) & 0xFF000000) >> 24) 			\
			))
#		define APPLYDATACONVERSION(x) B2LENDIAN32(x)
#	elif  LITTLE2BIG
#		define APPLYDATACONVERSION(x) 
# endif /* BIG2LITTLE*/
#else
#define APPLYDATACONVERSION(x) x
#endif /* DATACONVERSION */

#define NOTIMPLEMENTED ({ do 		\
									\
				while(1) })			\

/** @defgroup ST Handles */
#define __BTL_COMM_ST_UART_HANDLE	huart1
#define __BTL_COMM_ST_CAN_HANDLE 	hcan
#define __BTL_DBG_ST_UART_HANDLE   huart2
#define __BTL_LOG_ST_UART_HANDLE	 huart2
#define __BTL_DBG_ST_CAN_HANDLE    hcan

#define __BTL_COMM_ST_UART_HANDLE_DEF()	 extern UART_HandleTypeDef __BTL_COMM_ST_UART_HANDLE
#define __BTL_COMM_ST_CAN_HANDLE_DEF()   extern CAN_HandleTypeDef __BTL_COMM_ST_CAN_HANDLE
#define __BTL_DBG_ST_UART_HANDLE_DEF()	 extern UART_HandleTypeDef __BTL_DBG_ST_UART_HANDLE
#define __BTL_DBG_ST_CAN_HANDLE_DEF()    extern CAN_HandleTypeDef __BTL_DBG_ST_CAN_HANDLE
#define __BTL_LOG_ST_UART_HANDLE_DEF() 	 extern UART_HandleTypeDef __BTL_LOG_ST_UART_HANDLE

#define PIPE_BUFFER_MAX_SIZE ( (uint16) (512) )
#define DBG_BUFFER_MAX_SIZE  ( (uint8) 	(128) )
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

/* DON'T CHANGE THIS LINE */
#define BTL_FIRMWARE_SIZE							(25u * STM32F103C8Tx_FLASH_PAGE_SIZE)
#define BTL_FIRMWARE_START_ADDRESS		(FLASH_BASE)
#define BTL_FIRMWARE_LAST_ADDR				(FLASH_BASE | BTL_FIRMWARE_SIZE)

/** @defgroup Commands */
#define NUM_OF_CMD 										( (uint8) (13u) )
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
#define	CBL_GET_INFO_CMD							( (uint8) (12) )
#define	CBL_FLASH_APP_CMD							( (uint8) (13) )
#define	CBL_SW_RESET									( (uint8) (14) )

/** @defgroup Packet Type */
#define PACKET_TYPE_REQUEST_DATA				( (uint8) (0) )
#define PACKET_TYPE_DATA_FOR_FLASH			( (uint8) (1) )
#define PACKET_TYPE_CMD									( (uint8) (2) )

#define __STM32F103C8T6_SW_RESET_EXCECPTION() NVIC_SystemReset() 

#define __sw_reset_signal() __STM32F103C8T6_SW_RESET_EXCECPTION()

/**
* ===============================================================================================
*   > Private Datatypes
* ===============================================================================================
*/

/** @defgroup Bootloader defined types */
typedef struct __packetSerialization packet_t;
typedef boolean flag_t;

typedef uint8* hash_t;
typedef uint8 sha256_t[32u];

/** @brief Struct container for the bootloader sw version */
typedef struct __bootloaderVersion __st_blVersion_t;
/** @brief Enum for bootloader status */
typedef enum __bootloaderErrorStatus __en_blErrStatus_t;
typedef enum __bootloaderStatus __en_blStatus_t;

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
	uint8 PacketExtendedLength;
	uint8 PacketType;
	uint8 Command;
	uint8 DataLength;
	/* MTU */
	uint8 Data[512u];  
	uint32 DataCRC32;
	uint32 PacketCRC32;
};

/**
* ===============================================================================================
*   > Private Enums
* ===============================================================================================
*/

enum __bootloaderStatus {
	BL_NONE = -1,
	BL_APP_VALID = 0,
	BL_APP_NOT_VALID,
	BL_HASH_VALID,
	BL_HASH_NOT_VALID,
	BL_APP_TO_BL,
	BL_FRESH,
	BL_NO_APP
};

/** @brief Enum for bootloader status */
enum __bootloaderErrorStatus {
	BL_E_NONE = -1,
	BL_E_OK 	= 0,
	BL_E_NOK,
	BL_E_INVALID_LENGTH,
	BL_E_INVALID_CMD,
	BL_E_INVALID_CRC,
	BL_E_TIMEOUT,
	BL_E_INVALID_ADDR
};


/**
* ===============================================================================================
*   > Private Functions Declaration
* ===============================================================================================
*/
#ifdef __cplusplus 
extern "c" {
#endif /* __cplusplus */

#if defined(BL_DBG_PORT)
__STATIC __NORETURN __bl_vDbgWrt(const uint8 * pArg_u8StrFormat, ...);
#endif

#if defined(BL_LOG_PORT)
__STATIC void __bl_vLogWrt(const uint8 Arg_u8LogLevel, const uint8 * pArg_u8LogMsg);
#endif

__STATIC __en_blErrStatus_t __bl_enExecuteCommand(const packet_t* pArg_tPacket);

__LOCAL_INLINE __en_blStatus_t __enGetIsValidAppFlag(void);
__LOCAL_INLINE __en_blStatus_t __enGetIsValidHashFlag(void);
__LOCAL_INLINE __en_blStatus_t __enGetIsAppToBlFlag(void);
__LOCAL_INLINE __NORETURN __vSetIsValidAppFlag(flag_t volatile Arg_tValue);
__LOCAL_INLINE __NORETURN __vSetIsValidHashFlag(flag_t volatile Arg_tValue);
__LOCAL_INLINE __NORETURN __vSetIsAppToBlFlag(flag_t volatile Arg_tValue);

__STATIC __en_blErrStatus_t __enPipeListen(void);
__STATIC __NORETURN __vPipeEcho(const uint8* pArg_u8TxBuffer, uint8 Arg_u8Length);

__STATIC __NORETURN __vSendAck(uint8 Arg_u8DatatoHostLength);
__STATIC __NORETURN __vSendNack(uint8 Arg_u8ErrorCode);

__STATIC hash_t __tCalculateSHA256HashForApplication(void);

__STATIC __en_blErrStatus_t __enVerifyAddress(uint32 Arg_u32McuAddressValue);
__STATIC __FORCE_NORETURN __vJumpToApplication(void);

__STATIC __NORETURN __vSeralizeReceivedBuffer(packet_t* pArg_tPacket, uint8* pArg_tReceivedBuffer);

__LOCAL_INLINE __en_blErrStatus_t __enVerifyPacketCRC32(const uint32 Arg_u32ReceivedCrc32, const uint8* pArg_u8ReceivedBuffer, const uint8 Arg_u8ReceivedBufferSize);
__LOCAL_INLINE __en_blErrStatus_t __enVerifyPacketDataCRC32(const packet_t* pArg_tReceivedPacket);


__LOCAL_INLINE __en_blErrStatus_t __enGetMcuRdpLevel(uint8* pArg_u8RdpLevel);
__STATIC __en_blErrStatus_t __enEraseFlashPages(const uint8 Arg_u8PageIdx, const uint8 Arg_u8NumOfPages);
__LOCAL_INLINE __en_blErrStatus_t __enJumpToAddress(uint32 Arg_u32Address);

__FORCE_INLINE
__LOCAL_INLINE uint32 __vPageIdx2PhysicalAddress(uint8 Arg_u8PageIdx);

__LOCAL_INLINE __en_blErrStatus_t __enWriteToAddr(const uint8* pArg_u8Data, const uint32 Arg_u32BaseAddr, uint16 Arg_u16Length);

/** 
 * @defgroup commands handlers 
 **/ 
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_INFO_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_VER_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_HELP_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_CID_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_RDP_STATUS_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GO_TO_ADDR_CMD(const uint8* pArg_u8Data);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_CBL_FLASH_ERASE_CMD(const uint8* pArg_u8Data);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_MEM_WRITE_CMD(const uint8* pArg_u8Data);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_EN_R_W_PROTECT_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_READ_SECTOR_STATUS(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_MEM_READ_CMD(const uint8* pArg_u8Data);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_OTP_READ_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_DIS_R_W_PROTECT_CMD(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_READ_SECTOR_STATUS(void);
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_SW_RESET(void);

/**
  * @}
  */

#ifdef __cplusplus 
}
#endif /* __cplusplus */
#endif /* __BOOTLOADER_PRV_H__ */
