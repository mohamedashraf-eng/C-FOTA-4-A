/**
******************************************************************************
* @file           : bootloader_prg.c
* @brief          : Bootloader implementation file
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
/** @defgroup Bootloader libs */
#include "bootloader_intf.h"
#include "bootloader_prv.h"
#include "bootloader_cfg.h"
/** @defgroup ST libs */
#include "usart.h"
//#include "can.h"
#include "crc.h"
#include "rtc.h"
/** @defgroup STD Libs */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/**
* ===============================================================================================
*   > Private Macros
* ===============================================================================================
*/

#if (BL_COMM_PIPE >= 0x00)
#	if(BL_COMM_PIPE == BL_COMM_OVER_UART)
__BTL_COMM_ST_UART_HANDLE_DEF();
#		if(BL_COMM_TYPE == BL_COMM_SYNC)
#			define PIPE_LISTEN(__BUFFER, __LEN) HAL_UART_Receive(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN), HAL_MAX_DELAY)
#			define PIPE_ECHO(__BUFFER, __LEN) HAL_UART_Transmit(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN), HAL_MAX_DELAY)
#    else
#			define PIPE_LISTEN(__BUFFER, __LEN) HAL_UART_Receive_IT(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN))
#			define PIPE_ECHO(__BUFFER, __LEN) HAL_UART_Transmit_IT(&__BTL_COMM_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN))
#    endif /* (BL_COMM_TYPE == BL_COMM_SYNC) */
#	elif(BL_COMM_PIPE == BL_COMM_OVER_CAN)
__BTL_COMM_ST_CAN_HANDLE_DEF();
#		if(BL_COMM_TYPE == BL_COMM_SYNC)
#			define PIPE_LISTEN(__BUFFER)
#			define PIPE_ECHO(__BUFFER, __LEN) 
#    else
#			define PIPE_LISTEN(__BUFFER)
#			define PIPE_ECHO(__BUFFER, __LEN) 
#    endif /* (BL_COMM_TYPE == BL_COMM_SYNC) */
#	else
#	error (" No communication pipe interface defined !")
#	endif /* (BL_COMM_PIPE == BL_COMM_OVER_UART) */
#endif /* (BL_COMM_PIPE >= 0x00) */

#if defined(BL_LOG_PORT)
#		if defined(BL_DBG_PORT)
#			undef (DBG_PORT_UART)
#		endif /* DBG_PORT_UART */
__BTL_LOG_ST_UART_HANDLE_DEF();
__STATIC uint8 global_bl_log_buffer[BL_LOG_BUFFER_SIZE];

#		define BL_LOG_SEND(LOG_LEVEL, LOG_MSG) __bl_vLogWrt(LOG_LEVEL, LOG_MSG)
#		define __LOG_SEND_OVER_X(__BUFFER, __LEN) HAL_UART_Transmit_DMA(&__BTL_LOG_ST_UART_HANDLE, (__BUFFER), (uint16)(__LEN))

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
	if( (huart->Instance == __BTL_LOG_ST_UART_HANDLE.Instance) ) {
		;
	} else return;
}

#else
#		define BL_LOG_SEND(LOG_LEVEL, LOG_MSG)
#endif /* BL_LOG_PORT*/

#if defined(BL_DBG_PORT)
#		if defined(LWD)
# define BL_DBG_SEND(DBG_MSG, ...)	__bl_vDbgWrt("\r\nVENDOR_ID: %d, MODULE_ID: %d - DBG_MSG:> "DBG_MSG,   \
												BOOTLOADER_VENDOR_ID,											    									\
												BOOTLOADER_MODULE_ID,										 	    									\
												##__VA_ARGS__)													    										\

#		else
# define BL_DBG_SEND(DBG_MSG, ...)	__bl_vDbgWrt("\r\nVENDOR_ID: %d, MODULE_ID: %d <FILE: %s - FUNC: %s - LINE: %d> \nDBG_MSG:> "DBG_MSG,   \
												BOOTLOADER_VENDOR_ID,											    									\
												BOOTLOADER_MODULE_ID,										 	    									\
												__FILE__,														    												\
												__FUNCTION__,														    										\
												__LINE__,													  														\
												##__VA_ARGS__)													    										\

#		endif /* LWD */

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
#		define BL_DBG_SEND(DBG_MSG, ...)
#endif /* (BL_DBG_PORT > 0x00) */

/**
* ===============================================================================================
*   > Global Private Types
* ===============================================================================================
*/

/** @brief Struct container for the bootloader sw version */
__STATIC const __st_blVersion_t global_stMyBootLoaderVersion = {
	.vendorID	=	BOOTLOADER_VENDOR_ID,
	.moduleID = BOOTLOADER_MODULE_ID,
	.sw_major_version = BOOTLOADER_SW_MAJOR_VERSION,
	.sw_minor_version = BOOTLOADER_SW_MINOR_VERSION,
	.sw_mpatch_version = BOOTLOADER_SW_PATCH_VERSION
};

#define __FLAG_VALID_APP_ADDR 	RTC_BKP_DR1
#define __FLAG_VALID_HASH_ADDR  RTC_BKP_DR2
#define __FLAG_APP_TO_BL_ADDR 	RTC_BKP_DR3

#define __WRITE_FLAG_VALID_APP_ADDR(_FV) HAL_RTCEx_BKUPWrite(&hrtc, __FLAG_VALID_APP_ADDR, _FV)
#define __READ_FLAG_VALID_APP_ADDR() 	 HAL_RTCEx_BKUPRead(&hrtc, __FLAG_VALID_APP_ADDR)

#define __WRITE_FLAG_VALID_HASH_ADDR(_FV) HAL_RTCEx_BKUPWrite(&hrtc, __FLAG_VALID_HASH_ADDR, _FV)
#define __READ_FLAG_VALID_HASH_ADDR() 	  HAL_RTCEx_BKUPRead(&hrtc, __FLAG_VALID_HASH_ADDR)

#define __WRITE_FLAG_APP_TO_BL_ADDR(_FV) HAL_RTCEx_BKUPWrite(&hrtc, __FLAG_APP_TO_BL_ADDR, _FV)
#define __READ_FLAG_APP_TO_BL_ADDR() 	 HAL_RTCEx_BKUPRead(&hrtc, __FLAG_APP_TO_BL_ADDR)

__STATIC sha256_t volatile global_tApplicationHash;

/**
* ===============================================================================================
   > Private Functions Implementation
* ===============================================================================================
*/

#if defined(BL_DBG_PORT)
__STATIC __NORETURN __bl_vDbgWrt(const uint8* restrict pArg_u8StrFormat, ...) {
	__STATIC uint8 local_u8DbgBuffer[DBG_BUFFER_MAX_SIZE];

	va_list args;
	va_start(args, pArg_u8StrFormat);
	vsnprintf((char *)local_u8DbgBuffer, DBG_BUFFER_MAX_SIZE, (char *)pArg_u8StrFormat, args);
	va_end(args);

	__DBG_SEND_OVER_X(local_u8DbgBuffer, strlen((const char *)local_u8DbgBuffer));
}
#endif

#if defined(BL_LOG_PORT)
__STATIC void __bl_vLogWrt(const uint8 Arg_u8LogLevel, const uint8 * pArg_u8LogMsg) {
	switch(Arg_u8LogLevel) {
		case LOGL_INFO:
		case LOGL_WARN:
		case LOGL_ERR:
		case LOGL_CRIT:
			sprintf(global_bl_log_buffer, "[TimeStamp] [LogLevel] - %s\r\n", pArg_u8LogMsg);
			
			uint16_t global_bl_log_buffer_len = strlen(global_bl_log_buffer);
			__LOG_SEND_OVER_X(global_bl_log_buffer, global_bl_log_buffer_len);

			break;
		default: break;
	}
}
#endif

__STATIC __NORETURN __vCheckForAppValidity(void) {
	uint32 local_u32ResetVectorAddrValue = *((uint32*)APP_START_ADDR + 4);

	if (local_u32ResetVectorAddrValue != 0xFFFFFFFFUL) {
		__WRITE_FLAG_VALID_APP_ADDR(TRUE);
	} else {
		__WRITE_FLAG_VALID_APP_ADDR(FALSE);
	}
}

__STATIC __NORETURN __vPipeEcho(const uint8* pArg_u8TxBuffer, uint8 Arg_u8Length) {
	__STATIC uint8 volatile local_u8TxBuffer[255u] = {0};
	if( ((NULL == pArg_u8TxBuffer) || (Arg_u8Length <= 0u)) ) {
		/* INVALID */
		BL_DBG_SEND("INVALID Argument Values");
	} else {
		memcpy(local_u8TxBuffer, pArg_u8TxBuffer, Arg_u8Length);
		if( (HAL_OK != PIPE_ECHO((uint8*)&local_u8TxBuffer[0], Arg_u8Length)) ) {
			BL_DBG_SEND("pipe echo is not ok.");
		} else {
			BL_DBG_SEND("pipe echo succeeded.");
		}
	}
}

__STATIC __NORETURN __vSendAck(uint8 Arg_u8DatatoHostLength) {
	uint8 local_u8AckValue[2u] = {BL_CMD_RESPONSE_ACK, Arg_u8DatatoHostLength};
	__vPipeEcho(&local_u8AckValue, 2u);
	BL_DBG_SEND("Sent Ack successfully");
}

__STATIC __NORETURN __vSendNack(uint8 Arg_u8ErrorCode) {
	uint8 local_u8NackValue[2u] = {BL_CMD_RESPONSE_NACK, Arg_u8ErrorCode};
	__vPipeEcho((uint8*)&local_u8NackValue[0], 2u);
	BL_DBG_SEND("Sent Nack successfully");
}

__STATIC __en_blErrStatus_t __bl_enExecuteCommand(const packet_t* pArg_tPacket) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	__en_blErrStatus_t local_enCmdHandlerErrStatus = BL_E_NONE;

	switch(pArg_tPacket->Command) {
		case CBL_GET_INFO_CMD			  : BL_DBG_SEND("Executing Command: CBLP_GET_INFO_CMD");
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GET_INFO_CMD();
			break;
		case CBL_GET_VER_CMD					:	BL_DBG_SEND("Executing Command: CBL_GET_VER_CMD");
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GET_VER_CMD();
			break;
		case CBL_GET_HELP_CMD					: BL_DBG_SEND("Executing Command: CBL_GET_HELP_CMD");
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GET_HELP_CMD();
			break;
		case CBL_GET_CID_CMD					:	BL_DBG_SEND("Executing Command: CBL_GET_CID_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GET_CID_CMD();
			break;
		case CBL_GET_RDP_STATUS_CMD		:	BL_DBG_SEND("Executing Command: CBL_GET_RDP_STATUS_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GET_RDP_STATUS_CMD();
			break;
		case CBL_GO_TO_ADDR_CMD				:	BL_DBG_SEND("Executing Command: CBL_GO_TO_ADDR_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GO_TO_ADDR_CMD(pArg_tPacket->Data);
			break;
		case CBL_FLASH_ERASE_CMD			:	BL_DBG_SEND("Executing Command: CBL_FLASH_ERASE_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_CBL_FLASH_ERASE_CMD(pArg_tPacket->Data);
			break;
		case CBL_MEM_WRITE_CMD				:	BL_DBG_SEND("Executing Command: CBL_MEM_WRITE_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_MEM_WRITE_CMD(pArg_tPacket->Data);
			break;
		case CBL_EN_R_W_PROTECT_CMD		:	BL_DBG_SEND("Executing Command: CBL_EN_R_W_PROTECT_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_EN_R_W_PROTECT_CMD();
			break;
		case CBL_MEM_READ_CMD					:	BL_DBG_SEND("Executing Command: CBL_GET_CBL_MEM_READ_CMDVER_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_MEM_READ_CMD(pArg_tPacket->Data);
			break;
		case CBL_OTP_READ_CMD					:	BL_DBG_SEND("Executing Command: CBL_OTP_READ_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_OTP_READ_CMD();
			break;
		case CBL_DIS_R_W_PROTECT_CMD	:	BL_DBG_SEND("Executing Command: CBL_DIS_R_W_PROTECT_CMD");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_DIS_R_W_PROTECT_CMD();
			break;
		case CBL_READ_SECTOR_STATUS 	:	BL_DBG_SEND("Executing Command: CBL_READ_SECTOR_STATUS");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_READ_SECTOR_STATUS();
			break;
		case CBL_SW_RESET: BL_DBG_SEND("Executing Command: CBL_SW_RESET");	
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_SW_RESET();
			break;
		default: BL_DBG_SEND("Unkown received command"); break;
	}
	/** @todo Make logic for return */
	local_enThisFuncErrStatus = local_enCmdHandlerErrStatus;
	return local_enThisFuncErrStatus;
}

__STATIC __en_blErrStatus_t __enVerifyAddress(uint32 Arg_u32McuAddressValue) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	/* FLASH validity ** SRAM1 Validity */
	if( ((Arg_u32McuAddressValue >= FLASH_BASE) && (Arg_u32McuAddressValue <= STM32F103C8Tx_FLASH_END)) ||
			((Arg_u32McuAddressValue >= SRAM_BASE) && (Arg_u32McuAddressValue <= STM32F103C8Tx_SRAM1_END))) {
		local_enThisFuncErrStatus = BL_E_OK;
	} else {
		local_enThisFuncErrStatus = BL_E_INVALID_ADDR;
	}
	return local_enThisFuncErrStatus;
}

__STATIC __NORETURN __vJumpToApplication(void) {
	__disable_irq();
	if( (BL_E_INVALID_ADDR != __enVerifyAddress(APP_START_ADDR)) ) {
		/* Set shared flags */
		__WRITE_FLAG_APP_TO_BL_ADDR(FALSE);
		__WRITE_FLAG_VALID_APP_ADDR(TRUE);
		__WRITE_FLAG_VALID_HASH_ADDR(TRUE);
		/* Pre Init Jump */
		memset((uint32*)NVIC->ICER, 0xFF, sizeof(NVIC->ICER));
		SysTick->CTRL = 0;
		SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
		memset((uint32*)NVIC->ICPR, 0xFF, sizeof(NVIC->ICPR));
		/* Load Vector Table */
		SCB->VTOR = (uint32)(APP_START_ADDR);
		/* Read the data stored in the first 4 bytes (Main Stack Pointer) */
		uint32 local_u32MspValue = *((uint32_t volatile *)(APP_START_ADDR));
		/* Read the next 4 bytes from the base address (Reset Handler Function) */
		uint32 local_u32ResetHandler = *((uint32_t volatile *) (APP_START_ADDR + 4u));
		/* Set the reset handler as function */
		void (*local_vAppResetFunc)(void) = (void*)local_u32ResetHandler;
		/* Set the MSP for the application */
		__set_MSP(local_u32MspValue);
		/* Set stack pointer */
		__set_CONTROL(0);
		/* Reset clock before start */
		HAL_RCC_DeInit();
		/* Call the reset function to start the application */
		BL_DBG_SEND("Succesfully jumped to application.");
		local_vAppResetFunc();
	} else {
		/* Error handle */
		BL_DBG_SEND("Invalid application address");
	}
}

__STATIC __NORETURN __vSeralizeReceivedBuffer(packet_t* pArg_tPacket, uint8* pArg_tReceivedBuffer) {
	if( (NULL == pArg_tPacket) || (NULL == pArg_tReceivedBuffer) ) {
		BL_DBG_SEND("Invalid arguments");
	} else {
		pArg_tPacket->PacketType = pArg_tReceivedBuffer[0];
		BL_DBG_SEND("pArg_tPacket->PacketType: %X", pArg_tPacket->PacketType);

		pArg_tPacket->Command = pArg_tReceivedBuffer[1];
		BL_DBG_SEND("pArg_tPacket->Command: %X", pArg_tPacket->Command);

		pArg_tPacket->DataLength = pArg_tReceivedBuffer[2];
		BL_DBG_SEND("pArg_tPacket->DataLength: %X", pArg_tPacket->DataLength);

		memcpy((uint8*)pArg_tPacket->Data, (uint8*)&pArg_tReceivedBuffer[3], pArg_tPacket->DataLength);

		memcpy(&pArg_tPacket->DataCRC32, (uint8*)&pArg_tReceivedBuffer[3u + pArg_tPacket->DataLength], sizeof(uint32));
		pArg_tPacket->DataCRC32 = APPLYDATACONVERSION(pArg_tPacket->DataCRC32);
		BL_DBG_SEND("pArg_tPacket->DataCRC32: %X", pArg_tPacket->DataCRC32);

		memcpy(&pArg_tPacket->PacketCRC32, (uint8*)&pArg_tReceivedBuffer[7u + pArg_tPacket->DataLength], sizeof(uint32));
		pArg_tPacket->PacketCRC32 = APPLYDATACONVERSION(pArg_tPacket->PacketCRC32);
		BL_DBG_SEND("pArg_tPacket->PacketCRC32: %X", pArg_tPacket->PacketCRC32);

	}
	return;
}

/**
 * @details
 * 	Packet structure format:
 * 					[Packet length] | [Packet Type] | [Command] | [Data Length] | [Data]		[Data CRC] | [Packet CRC]
 * 					[1 Byte]				| [1 Byte] 			| [1 Byte]  | [1 Byte] 			| [n Byte]	[4 Byte] 	 | [4 Byte]
 * 
 * Max Packet length (Excluding Length): 12 + n Bytes
 * Min Packet length (Excluding Length): 12 Bytes
 * 
 */
__en_blErrStatus_t __enPipeListen(void) {
	BL_LOG_SEND(LOGL_INFO, "Bootloader is listening");
	BL_DBG_SEND("Command Listening Start pipe: %d | type: %d\n", BL_COMM_PIPE, BL_COMM_TYPE);
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	packet_t local_tPacketSeralized = {0};
	
	__STATIC uint8 volatile local_u8PipeListenrBuffer[PIPE_BUFFER_MAX_SIZE];
	memset(local_u8PipeListenrBuffer, 0, PIPE_BUFFER_MAX_SIZE);
	
	/* Start listening for the packet */
	BL_DBG_SEND("Waitsing for the packet length.");
	if( (HAL_OK != PIPE_LISTEN((uint8*)&local_u8PipeListenrBuffer[0], 1u)) ) {
		BL_LOG_SEND(LOGL_INFO, "Bad message");
		BL_DBG_SEND("The pipe listner is not ok.");
		local_enThisFuncErrStatus = BL_E_NOK;
	} else {	
		/* Receive the data */
		local_tPacketSeralized.PacketLength = local_u8PipeListenrBuffer[0];
		BL_DBG_SEND("Waiting for the packet with length (%d).", local_tPacketSeralized.PacketLength);
		
//		__vStartSessiontimeOutCount();

		if( (HAL_OK != PIPE_LISTEN((uint8*)&local_u8PipeListenrBuffer[1], local_tPacketSeralized.PacketLength)) ) {
			BL_DBG_SEND("The pipe listner is not ok.");
			local_enThisFuncErrStatus = BL_E_NOK;
		} else {
//			__vResetSessionTimeOutCounter(&global_u32LastPacketTime);

			BL_LOG_SEND(LOGL_INFO, "Got the packet");
			BL_DBG_SEND("The pipe listner received a packet successfully.");
			/* Seralize the received packet */
			__vSeralizeReceivedBuffer(&local_tPacketSeralized, (uint8*)&local_u8PipeListenrBuffer[1]);
			/* Validate CRC32 for packet */
			/** @note Calculating CRC32 for packet excluding the packet length and packetCRC32 */
			if( (BL_E_INVALID_CRC != __enVerifyPacketCRC32(local_tPacketSeralized.PacketCRC32, (uint8*)&local_u8PipeListenrBuffer[1], (local_tPacketSeralized.PacketLength - 4u))) ) {
				/* Validate CRC32 for data */
				if( (BL_E_INVALID_CRC != __enVerifyPacketDataCRC32(&local_tPacketSeralized)) ) {
					/** 
					 * @todo Add more verifications
					 * /
					/* Execute the command */
					if( (BL_E_OK != __bl_enExecuteCommand(&local_tPacketSeralized)) ) {
						__vSendNack(BL_E_NOK);
						local_enThisFuncErrStatus = BL_E_NOK;
						BL_LOG_SEND(LOGL_INFO, "Executing the command");
						BL_DBG_SEND("Command execution error");
					} else {
						__vSendAck(0u);
						local_enThisFuncErrStatus = BL_E_OK;
						BL_LOG_SEND(LOGL_INFO, "Done");
						BL_DBG_SEND("Command execution done");
					}
				} else {
					local_enThisFuncErrStatus = BL_E_INVALID_CRC;
					BL_LOG_SEND(LOGL_INFO, "Probably a corrupted data");
					BL_DBG_SEND("Packet error, Invalid packet data CRC32");
				}
			} else {
				local_enThisFuncErrStatus = BL_E_INVALID_CRC;
				BL_LOG_SEND(LOGL_INFO, "Probably a corrupted packet");
				BL_DBG_SEND("Packet error, Invalid packet CRC32");
			}
		}
	}
	return local_enThisFuncErrStatus;
}

__LOCAL_INLINE __en_blErrStatus_t __enVerifyPacketDataCRC32(const packet_t* pArg_tReceivedPacket) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_INVALID_CRC;
	uint32 local_u32CalculatedCrc32 = 0xFFFFFFFFU;
	BL_DBG_SEND("Calculating CRC32 for data");
	
	/* CRC32 using standard poly: 0x04C11DB7 */
	uint8 local_u8Counter = 0;
	for(local_u8Counter = 0; (local_u8Counter < pArg_tReceivedPacket->DataLength); ++local_u8Counter) {
		uint32 temp = (uint32)pArg_tReceivedPacket->Data[local_u8Counter];
		local_u32CalculatedCrc32 = 
			HAL_CRC_Accumulate(&hcrc, &temp, 1u);
	}
  __HAL_CRC_DR_RESET(&hcrc);
	
	BL_DBG_SEND("Calculated CRC32 for data: %X", local_u32CalculatedCrc32);
	if( (local_u32CalculatedCrc32 == pArg_tReceivedPacket->DataCRC32 ) ) {
		local_enThisFuncErrStatus = BL_E_OK;
		BL_DBG_SEND("Valid CRC32 for packet data");
	} else {
		BL_DBG_SEND("Invalid CRC32 for packet data");
	}

	return local_enThisFuncErrStatus;
}

__LOCAL_INLINE __en_blErrStatus_t __enVerifyPacketCRC32(const uint32 Arg_u32ReceivedCrc32, const uint8* pArg_u8ReceivedBuffer, const uint8 Arg_u8ReceivedBufferSize) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_INVALID_CRC;
	uint32 local_u32CalculatedCrc32 = 0;
	BL_DBG_SEND("Calculating CRC32 for packet");
	
	/* CRC32 using standard poly: 0x04C11DB7 */
	uint8 local_u8Counter = 0;
	for(local_u8Counter = 0; (local_u8Counter < Arg_u8ReceivedBufferSize); ++local_u8Counter) {
		uint32 temp = (uint32)pArg_u8ReceivedBuffer[local_u8Counter];
		local_u32CalculatedCrc32 = 
			HAL_CRC_Accumulate(&hcrc, &temp, 1u);
	}
 	__HAL_CRC_DR_RESET(&hcrc);
	
	BL_DBG_SEND("Calculated CRC32 for packet: %X", local_u32CalculatedCrc32);
	if( (Arg_u32ReceivedCrc32 == local_u32CalculatedCrc32) ) {
		local_enThisFuncErrStatus = BL_E_OK;
		BL_DBG_SEND("Valid CRC32 for packet");
	} else {
		BL_DBG_SEND("Invalid CRC32 for packet");
	}

	return local_enThisFuncErrStatus;
}

/**
 * @defgroup Command handlers implementation 
 * 
 */

__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_INFO_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_OK;
	uint8 local_u8Info[] = "This bootloader is made by Mohamed Ashraf Wx Copyright (C) Wx Inc, 2023";

	__vSendAck(sizeof(local_u8Info));
	__vPipeEcho(local_u8Info, sizeof(local_u8Info));

	return local_enThisFuncErrStatus;
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_VER_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_OK;
	
	__STATIC uint8 local_u8BootloaderVersion[3u] = {
		BOOTLOADER_SW_MAJOR_VERSION,
		BOOTLOADER_SW_MINOR_VERSION,
		BOOTLOADER_SW_PATCH_VERSION
	};
	
	__vSendAck(sizeof(local_u8BootloaderVersion));
	__vPipeEcho(local_u8BootloaderVersion, sizeof(local_u8BootloaderVersion));
	
	BL_DBG_SEND("Sent the bootloader version successfully.");
	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_HELP_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_OK;
	
	__STATIC uint8 local_u8Commands[NUM_OF_CMD] = {
		[0] = 	CBL_GET_VER_CMD,
		[1] =	CBL_GET_HELP_CMD,			
		[2] =	CBL_GET_CID_CMD,				
		[3] =	CBL_GET_RDP_STATUS_CMD,
		[4] =	CBL_GO_TO_ADDR_CMD,		
		[5] =	CBL_FLASH_ERASE_CMD,		
		[6] =	CBL_MEM_WRITE_CMD,			
		[7] =	CBL_EN_R_W_PROTECT_CMD,
		[8] =	CBL_MEM_READ_CMD,			
		[9] =	CBL_OTP_READ_CMD,			
		[10] =	CBL_DIS_R_W_PROTECT_CMD,
		[11] =	CBL_READ_SECTOR_STATUS 
	};
	
	__vSendAck(sizeof(local_u8Commands));
	__vPipeEcho(local_u8Commands, sizeof(local_u8Commands));
	
	BL_DBG_SEND("Sent the bootloader commands helper successfully.");

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_CID_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	
	uint32 local_u32CID = HAL_GetDEVID();
	__vSendAck(4u);
	__vPipeEcho((uint8*)&local_u32CID, 4u);

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GET_RDP_STATUS_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	
	uint8 local_u8RdpLevel;
	if( (BL_E_OK == __enGetMcuRdpLevel(&local_u8RdpLevel)) ) {
		__vSendAck(1u);
		__vPipeEcho(&local_u8RdpLevel, 1u);
		local_enThisFuncErrStatus = BL_E_OK;
	} else {
		/* Error Handle */
		local_enThisFuncErrStatus = BL_E_NOK;
	}
	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_GO_TO_ADDR_CMD(const uint8* pArg_u8Data) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	
	uint32 local_u32AddressToJump = (uint32)(*((uint32*)pArg_u8Data));
	if( (BL_E_OK != __enJumpToAddress(local_u32AddressToJump)) ) {
		local_enThisFuncErrStatus = BL_E_NOK;
		BL_DBG_SEND("Jumping to address error");
	} else {
		local_enThisFuncErrStatus = BL_E_OK;
		BL_DBG_SEND("Jumped to address successfully");
	}
	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_CBL_FLASH_ERASE_CMD(const uint8* pArg_u8Data) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	
	uint8 local_u8PageIdx = pArg_u8Data[0];
	uint8 local_u8NumOfPages = pArg_u8Data[1];

	if( (BL_E_OK != __enEraseFlashPages(local_u8PageIdx, local_u8NumOfPages)) ) {
		local_enThisFuncErrStatus = BL_E_NOK;
		BL_DBG_SEND("Flash erasing error");
	} else {
		local_enThisFuncErrStatus = BL_E_OK;
		BL_DBG_SEND("Flash erasing succeeded");
	}

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_MEM_WRITE_CMD(const uint8* pArg_u8Data) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	
	uint32 local_u32BaseAdrress = *((uint32*)&pArg_u8Data[0]);
	uint8 local_u8DataLength = *((uint8*)&pArg_u8Data[4]);
	uint8 local_u8Data[256] = {0};

	memcpy((uint8*)&local_u8Data[0], (uint8*)&pArg_u8Data[5], local_u8DataLength);

	if( (BL_E_OK != __enWriteToAddr(local_u8Data, local_u32BaseAdrress, local_u8DataLength)) ) {
		local_enThisFuncErrStatus = BL_E_NOK;
		BL_DBG_SEND("Writing to memory error");
	} else {
		BL_DBG_SEND("Written to memory");
		local_enThisFuncErrStatus = BL_E_OK;
	}

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_EN_R_W_PROTECT_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_READ_SECTOR_STATUS(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_MEM_READ_CMD(const uint8* pArg_u8Data) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_OTP_READ_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	

	return local_enThisFuncErrStatus; 
}
__STATIC __en_blErrStatus_t __enCmdHandler_CBL_DIS_R_W_PROTECT_CMD(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	

	return local_enThisFuncErrStatus; 
}

__STATIC __en_blErrStatus_t __enCmdHandler_CBL_SW_RESET(void) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_OK;
	
	BL_DBG_SEND("Performing SW reset. -- No Return");
	__sw_reset_signal();

	return local_enThisFuncErrStatus; 
}

/**
  * @}
  */

/**
 * @defgroup Command handlers helpers 
 * 
 */

__LOCAL_INLINE uint32 __vPageIdx2PhysicalAddress(uint8 Arg_u8PageIdx) {
	uint32 local_u32PageAddress = (FLASH_BASE) | (Arg_u8PageIdx * STM32F103C8Tx_FLASH_PAGE_SIZE);
	if( (local_u32PageAddress < FLASH_BASE) || (local_u32PageAddress > STM32F103C8Tx_FLASH_END) ) {
		BL_DBG_SEND("Page indexig went to fatal error. SYNC STUCK");
		while(1) {
			;
		}
	} else {
		;
	}
	return local_u32PageAddress;
}

__LOCAL_INLINE __en_blErrStatus_t __enGetMcuRdpLevel(uint8* pArg_u8RdpLevel) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;

	FLASH_OBProgramInitTypeDef L_StMyFlashRDPSett;

	/* Get the RDP level */
	HAL_FLASHEx_OBGetConfig(&L_StMyFlashRDPSett);

	if( (L_StMyFlashRDPSett.RDPLevel < 0) ) {
		BL_DBG_SEND("Invalid RDP level read");
		local_enThisFuncErrStatus = BL_E_NOK;
	} else {
		*pArg_u8RdpLevel = (uint8)L_StMyFlashRDPSett.RDPLevel;

		local_enThisFuncErrStatus = BL_E_OK;
	}
	
	return local_enThisFuncErrStatus;
}

__STATIC __en_blErrStatus_t __enEraseFlashPages(const uint8 Arg_u8PageIdx, uint8 Arg_u8NumOfPages) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;

	if (Arg_u8PageIdx > BTL_FLASH_MASS_ERASE) {
		local_enThisFuncErrStatus = BL_E_NOK;
		BL_DBG_SEND("Invalid flash erase operation");
	} else {
		/* Init the Flash HAL Driver */
		FLASH_EraseInitTypeDef local_stMyErasingConfig;

		if (BTL_FLASH_MASS_ERASE == Arg_u8PageIdx) {
			BL_DBG_SEND("Selected mass erase");
			local_stMyErasingConfig.TypeErase = FLASH_TYPEERASE_MASSERASE;
		} else {
			/* Limit the page size */
			uint8_t local_u8RemainingPages = BTL_FLASH_MAX_PAGE_NUM - Arg_u8PageIdx;
			if (Arg_u8NumOfPages > local_u8RemainingPages) {
				Arg_u8NumOfPages = local_u8RemainingPages;
			} else {
				;
			}
				BL_DBG_SEND("Selected page erase (page: %d, nop: %d)", Arg_u8PageIdx, Arg_u8NumOfPages);
				local_stMyErasingConfig.TypeErase = FLASH_TYPEERASE_PAGES;
				local_stMyErasingConfig.NbPages = Arg_u8NumOfPages;
				local_stMyErasingConfig.PageAddress = __vPageIdx2PhysicalAddress(Arg_u8PageIdx);
				/* Erasing bootloader firmware guard */
				if( (local_stMyErasingConfig.PageAddress >= BTL_FIRMWARE_START_ADDRESS) &&
						(local_stMyErasingConfig.PageAddress <= BTL_FIRMWARE_LAST_ADDR) ) {
					BL_DBG_SEND("CANOT SELF TERMINATE THE BOOTLOADER FIRMWARE!");
					goto BTL_SELF_TERMINATE_GUARD;
				} else {
					;
				}
			}
			local_stMyErasingConfig.Banks = FLASH_BANK_1;
			uint32 local_u32PageEraseErr = 0;
			
			BL_DBG_SEND("Flash erasing started");
			if (HAL_OK == HAL_FLASH_Unlock()) {
				if(HAL_OK == HAL_FLASHEx_Erase(&local_stMyErasingConfig, &local_u32PageEraseErr)) {
						/* 0xFFFFFFFFU == ERASE_OK */
						if (0xFFFFFFFFU == local_u32PageEraseErr) { 
							local_enThisFuncErrStatus = BL_E_OK;
							BL_DBG_SEND("Done flash erasing");
						} else {
							local_enThisFuncErrStatus = BL_E_NOK;
							BL_DBG_SEND("Flash erasing failed on page addr: %p", local_u32PageEraseErr);
						}
						if (HAL_OK == HAL_FLASH_Lock()) {
							;
						} else {
							BL_DBG_SEND("Error with flash locking");
						}
				} else {
					BL_DBG_SEND("Error with flash erasing");
				}
		} else {
			BL_DBG_SEND("Error with flash unlocking");
		}
	}
	BTL_SELF_TERMINATE_GUARD:
	return local_enThisFuncErrStatus;
}

__LOCAL_INLINE __en_blErrStatus_t __enJumpToAddress(uint32 Arg_u32Address) {
	__vJumpToApplication();
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	if( (BL_E_OK != __enVerifyAddress(Arg_u32Address)) ) {
		local_enThisFuncErrStatus = BL_E_INVALID_ADDR;	
		BL_DBG_SEND("Invalid jumping address: %p", Arg_u32Address);
	} else {
		local_enThisFuncErrStatus = BL_E_OK;
		/* L_u32HostJmpAddr + 1 : For ARM-Thumb ISA indication */
		void (*localFuncPtr_vAddrToJmp)(void) = (void*)(Arg_u32Address + 1U);
		/* Jump to the specified address */
		BL_DBG_SEND("Jumping to address: %p", Arg_u32Address);
		localFuncPtr_vAddrToJmp();
	}
	return local_enThisFuncErrStatus;
}

/**
 * @brief Secure program flash memory
 * 
 * @note You can't program the same memory address twice,
 * 			 the memory address should be erased then re programmed.
 * @param pArg_u8Data 
 * @param Arg_u32BaseAddr 
 * @param Arg_u16Length 
 * @return __en_blErrStatus_t 
 */
__LOCAL_INLINE __en_blErrStatus_t __enWriteToAddr(const uint8* pArg_u8Data, const uint32 Arg_u32BaseAddr, uint16 Arg_u16Length) {
  __en_blErrStatus_t local_enThisFuncErrStatus = BL_E_OK;
  
  if (BL_E_OK == __enVerifyAddress(Arg_u32BaseAddr)) {
    if (HAL_OK == HAL_FLASH_Unlock()) {
      uint16 local_u16DataCounter = 0;
      uint16 local_u16HwordData = 0;
      
      for (local_u16DataCounter = 0; local_u16DataCounter < (Arg_u16Length-1u); local_u16DataCounter += 2u) {
        local_u16HwordData = /* Combining two QW as HW for HWORD RES FPEC compatbility */
					(pArg_u8Data[local_u16DataCounter] | (pArg_u8Data[local_u16DataCounter + 1] << 8u));
      	BL_DBG_SEND("Writing %X to address %p", local_u16HwordData, (Arg_u32BaseAddr + local_u16DataCounter));
        if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (Arg_u32BaseAddr + local_u16DataCounter), local_u16HwordData)) {
					local_enThisFuncErrStatus = BL_E_NOK;
          BL_DBG_SEND("Error writing 0x%04X to flash address %p", local_u16HwordData, (Arg_u32BaseAddr + local_u16DataCounter));
          break;
        } else {
          BL_DBG_SEND("Written 0x%04X to address %p successfully", local_u16HwordData, (Arg_u32BaseAddr + local_u16DataCounter));
        }
      }
      
      if (HAL_OK == HAL_FLASH_Lock()) {
				;
      } else {
        BL_DBG_SEND("Error with flash locking");
				local_enThisFuncErrStatus = BL_E_NOK;
      }
    } else {
      BL_DBG_SEND("Error with flash unlocking");
    }
  } else {
    BL_DBG_SEND("Invalid write memory address: %p ", Arg_u32BaseAddr);
    local_enThisFuncErrStatus = BL_E_INVALID_ADDR;
  }
  
  return local_enThisFuncErrStatus;
}


/**
  * @}
  */

/**
* ===============================================================================================
*   > Public Functions Implementation
* ===============================================================================================
*/

#define __SHA256_BUFFER_SIZE ( (uint8) (256u) )
#define __COMPARE_HASHES strcmp

/**
 * @brief Function to use WOLFCRYPT (WC) library to calculate SHA256 hash on existant application
 * @param[1] LDCB256 (Local Digest Buffer Context for SHA256)
 *
 */
#if (BL_SECURE_BOOT == BL_SECURE_BOOT_ON)
#	include "wolfssl/wolfcrypt/sha256.h"
#	define __SHA_CONTUPDATE_BUFFER() do {                                              \
			uint32_t local_u32FlashAddrIterator = APP_START_ADDR;                      \
			uint8_t local_u8Sha256Buffer[__SHA256_BUFFER_SIZE];	                       \
			while (*(uint32_t*)local_u32FlashAddrIterator != 0xFFFFFFFFUL) {           \
				for (uint8_t local_u8ChunkIterator = 0; local_u8ChunkIterator < __SHA256_BUFFER_SIZE; ++local_u8ChunkIterator) { \
					local_u8Sha256Buffer[local_u8ChunkIterator] = *(uint8_t*)local_u32FlashAddrIterator++; \
				}                                                                      					   \
				wc_Sha256Update(&local_tSha256Ctx, local_u8Sha256Buffer, sizeof(local_u8Sha256Buffer));    \
			}                                                                          					   \
		} while (0)
#	define __CALCULATE_APP_HASH(_LDCB256) ({                                            \
			wc_Sha256 local_tSha256Ctx;                                                 \
			wc_InitSha256(&local_tSha256Ctx);                                           \
			__SHA_CONTUPDATE_BUFFER();                                                  \
			wc_Sha256Final(&local_tSha256Ctx, _LDCB256);                                \
		})
#endif /* BL_SECURE_BOOT */

/**
 * @brief 
 * @details
 * 	Each reset cycle:
 * 		1. Enters boot manager state (BL_enBootManager)
 * 		2. If there is an application already
 * 		 		. Calculate the hash for the exisiting application
 * 					> If it matches the latest received application hash from OEM
 * 			  		then set the BL_APP_VALIDITY && HASH_VALIDITY to True, and jump to the application.
 * 		 	 		> Else Set the BL_APP_VALIDITY && HASH_VALIDITY to False, Start Pipe listener
 * 
 */
__NORETURN BL_enBootManager(void) {
	BL_LOG_SEND(LOGL_INFO, "Bootloader manager started, looking for a valid application");
	BL_DBG_SEND("Started the boot manager");

	if(FALSE == __READ_FLAG_APP_TO_BL_ADDR()) {
		__vCheckForAppValidity();
	} else;

	if( (TRUE == __READ_FLAG_APP_TO_BL_ADDR()) || (FALSE == __READ_FLAG_VALID_APP_ADDR()) ) {
		BL_DBG_SEND("Invalid application, waiting for a valid application");
		BL_LOG_SEND(LOGL_INFO, "No valid application found");
		/* Start listener */
		__enPipeListen();
	} else {
#if (BL_SECURE_BOOT == BL_SECURE_BOOT_OFF)
		BL_LOG_SEND(LOGL_INFO, "Found appllication, started to jump");
		__vJumpToApplication();
#elif (BL_SECURE_BOOT == BL_SECURE_BOOT_ON)
		BL_LOG_SEND(LOGL_INFO, "Found appllication, started to validate");
		BL_DBG_SEND("Validating the existant application");
		/* Process */
		if( (TRUE == __READ_FLAG_VALID_APP_ADDR()) ) {
			BL_LOG_SEND(LOGL_INFO, "Calculating hash digest");
			BL_DBG_SEND("Calculating application hash.");
			__STATIC sha256_t local_tApplicationHash;
			__CALCULATE_APP_HASH(local_tApplicationHash);
			/* Calculate hash */
			if ( (TRUE == __COMPARE_HASHES(local_tApplicationHash, global_tApplicationHash)) ) {
				BL_LOG_SEND(LOGL_INFO, "Applicaton is valid");
				BL_DBG_SEND("Valid hash");
				/* Jump to the application */
				__vJumpToApplication();
			} else {
				__WRITE_FLAG_APP_TO_BL_ADDR(FALSE);
				__WRITE_FLAG_VALID_HASH_ADDR(FALSE);
				__WRITE_FLAG_VALID_APP_ADDR(FALSE);
				BL_LOG_SEND(LOGL_INFO, "Application not valid, waiting for a valid application");
				BL_DBG_SEND("Invalid hash");
				/* Start listner */
				__enPipeListen();
			}
		} else {
			;
		}
#endif /* BL_SECURE_BOOT */
	}
}

__st_blVersion_t BL_stGetSwVersion(void) {
	return global_stMyBootLoaderVersion;
}
