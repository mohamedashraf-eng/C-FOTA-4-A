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
#include "can.h"
#include "crc.h"
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
#			define PIPE_LISTEN(__BUFFER) HAL_UART_Receive(&__BTL_COMM_ST_UART_HANDLE, (uint8*)(&__BUFFER[0]), PIPE_BUFFER_MAX_SIZE, HAL_MAX_DELAY)
#			define PIPE_ECHO(__BUFFER) HAL_UART_Transmit(&__BTL_COMM_ST_UART_HANDLE, (uint8*)(&__BUFFER), sizeof(__BUFFER), HAL_MAX_DELAY)
#    else
#			define PIPE_LISTEN(__BUFFER) HAL_UART_Receive_IT(&__BTL_COMM_ST_UART_HANDLE, (uint8*)(&__BUFFER[0]), PIPE_BUFFER_MAX_SIZE)
#			define PIPE_ECHO(__BUFER) HAL_UART_Transmit_IT(&__BTL_COMM_ST_UART_HANDLE, (uint8*)(&__BUFFER), sizeof(__BUFFER))
#    endif /* (BL_COMM_TYPE == BL_COMM_SYNC) */
#	elif(BL_COMM_PIPE == BL_COMM_OVER_CAN)
__BTL_COMM_ST_CAN_HANDLE_DEF();
#		if(BL_COMM_TYPE == BL_COMM_SYNC)
#			define PIPE_LISTEN(__BUFFER)
#    else
#			define PIPE_LISTEN(__BUFFER)
#    endif /* (BL_COMM_TYPE == BL_COMM_SYNC) */
#	else
#	error (" No communication pipe interface defined !")
#	endif /* (BL_COMM_PIPE == BL_COMM_OVER_UART) */
#endif /* (BL_COMM_PIPE >= 0x00) */

#if (BL_DBG_PORT >= 0x00)
# define BL_DBG_SEND(DBG_MSG, ...)	__bl_vDbgWrt("\r\nVENDOR_ID: %d, MODULE_ID: %d 	    \
														  <FILE: %s - FUNC: %s - LINE: %s>| DBG_MSG:> "DBG_MSG,     \
												__FILE__,														    												\
												__LINE__,														    												\
												__FUNCTION__,													  												\
												BOOTLOADER_VENDOR_ID,											    									\
												BOOTLOADER_MODULE_ID,										 	    									\
												##__VA_ARGS__)													    										\
												
#	if (BL_DBG_PORT == DBG_PORT_UART)
__BTL_DBG_ST_UART_HANDLE_DEF();
#			define __DBG_SEND_OVER_X(__BUFFER) HAL_UART_Transmit(&__BTL_DBG_ST_UART_HANDLE, (uint8*)(&__BUFFER)[0], sizeof(__BUFFER), HAL_MAX_DELAY)
#	elif(BL_DBG_PORT == DBG_PORT_CAN)
__BTL_DBG_ST_CAN_HANDLE_DEF();
#		define __DBG_SEND_OVER_X 
#	else
#		error (" No dbg port interface defined !")
#	endif /* (BL_DBG_PORT == DBG_PORT_UART) */
#else
#		define BL_DBG_SEND(DBG_MSG, ...) ()
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

__STATIC flag_t volatile global_tIsValidApp    = FALSE;
__STATIC flag_t volatile global_tIsValidHash   = FALSE;
__STATIC flag_t volatile global_tIsFromAppToBl = FALSE;

__STATIC sha256_t volatile global_tApplicationHash;

/**
* ===============================================================================================
*   > Private Functions Implementation
* ===============================================================================================
*/

#if (BL_DBG_PORT >= 0x00)
__STATIC __NORETURN __bl_vDbgWrt(const uint8 * pArg_u8StrFormat, ...) {
	va_list args;
	va_start(args, pArg_u8StrFormat);
	uint8 local_u8DbgBuffer[256u];
	vsnprintf(local_u8DbgBuffer, sizeof(local_u8DbgBuffer), pArg_u8StrFormat, args);
	va_end(args);
	__DBG_SEND_OVER_X(local_u8DbgBuffer);
}
#endif

__LOCAL_INLINE __en_blStatus_t __enGetIsValidAppFlag(void) {
	__en_blStatus_t local_enThisFuncStatus = BL_E_NONE;

	if( (FALSE == global_tIsValidApp) ) {
		local_enThisFuncStatus = BL_APP_NOT_VALID;
	} else if( (TRUE == global_tIsValidApp) ) {
		local_enThisFuncStatus = BL_APP_VALID;
	} else {
		/* UNDEFINED */
	}

	return local_enThisFuncStatus;
}

__LOCAL_INLINE __en_blStatus_t __enGetIsValidHashFlag(void) {
	__en_blStatus_t local_enThisFuncStatus = BL_E_NONE;

	if( (FALSE == global_tIsValidHash) ) {
		local_enThisFuncStatus = BL_HASH_NOT_VALID;
	} else if( (TRUE == global_tIsValidHash) ) {
		local_enThisFuncStatus = BL_HASH_VALID;
	} else {
		/* UNDEFINED */
	}

	return local_enThisFuncStatus;
}

__LOCAL_INLINE __en_blStatus_t __enGetIsAppToBlFlag(void) {
	__en_blStatus_t local_enThisFuncStatus = BL_E_NONE;

	if( (FALSE == global_tIsFromAppToBl) ) {
		local_enThisFuncStatus = BL_FRESH;
	} else if( (TRUE == global_tIsFromAppToBl) ) {
		local_enThisFuncStatus = BL_APP_TO_BL;
	} else {
		/* UNDEFINED */
	}

	return local_enThisFuncStatus;
}

__LOCAL_INLINE __NORETURN __vSetIsValidAppFlag(flag_t volatile Arg_tValue) {
	if( (Arg_tValue <= 0 || Arg_tValue >= 1) ) {
		global_tIsValidApp = Arg_tValue;
	} else {
		/* INVALID */
		BL_DBG_SEND("INVALID Arg_tValue: %d", Arg_tValue);
	}
}

__LOCAL_INLINE __NORETURN __vSetIsValidHashFlag(flag_t volatile Arg_tValue) {
	if( (Arg_tValue <= 0 || Arg_tValue >= 1) ) {
		global_tIsValidHash = Arg_tValue;
	} else {
		/* INVALID */
		BL_DBG_SEND("INVALID Arg_tValue: %d", Arg_tValue);
	}
}

__LOCAL_INLINE __NORETURN __vSetIsAppToBlFlag(flag_t volatile Arg_tValue) {
	if( (Arg_tValue <= 0 || Arg_tValue >= 1) ) {
		global_tIsFromAppToBl = Arg_tValue;
	} else {
		/* INVALID */
		BL_DBG_SEND("INVALID Arg_tValue: %d", Arg_tValue);
	}
}

__STATIC __NORETURN __vPipeEcho(uint8* pArg_u8TxBuffer, uint16 Arg_u16BufferSize) {
	if( ((NULL == pArg_u8TxBuffer) || (Arg_u16BufferSize <= 0)) ) {
		/* INVALID */
		BL_DBG_SEND("INVALID Argument Values");
	} else {
		PIPE_ECHO(pArg_u8TxBuffer);
	}
}

__STATIC __NORETURN __vSendAck(void) {
	uint8 local_u8AckValue = BL_CMD_RESPONSE_ACK;
	__vPipeEcho(&local_u8AckValue, sizeof(local_u8AckValue));
}

__STATIC __NORETURN __vSendNack(void) {
	uint8 local_u8NackValue = BL_CMD_RESPONSE_NACK;
	__vPipeEcho(&local_u8NackValue, sizeof(local_u8NackValue));
}

__STATIC __en_blErrStatus_t __bl_enExecuteCommand(const cmd_t* pArg_tCommand) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	__en_blErrStatus_t local_enCmdHandlerErrStatus = BL_E_NONE;

	uint8 local_u8CommandToExecute = 0;
	
	switch(local_u8CommandToExecute) {
		case CBL_GET_VER_CMD					:	BL_DBG_SEND("Executing Command: CBL_GET_VER_CMD");
			local_enCmdHandlerErrStatus = __enCmdHandler_CBL_GET_VER_CMD();
			break;
		case CBL_GET_HELP_CMD					: BL_DBG_SEND("Executing Command: CBL_GET_HELP_CMD");
			break;
		case CBL_GET_CID_CMD					:	BL_DBG_SEND("Executing Command: CBL_GET_CID_CMD");	
			break;
		case CBL_GET_RDP_STATUS_CMD		:	BL_DBG_SEND("Executing Command: CBL_GET_RDP_STATUS_CMD");	
			break;
		case CBL_GO_TO_ADDR_CMD				:	BL_DBG_SEND("Executing Command: CBL_GO_TO_ADDR_CMD");	
			break;
		case CBL_FLASH_ERASE_CMD			:	BL_DBG_SEND("Executing Command: CBL_FLASH_ERASE_CMD");	
			break;
		case CBL_MEM_WRITE_CMD				:	BL_DBG_SEND("Executing Command: CBL_MEM_WRITE_CMD");	
			break;
		case CBL_EN_R_W_PROTECT_CMD		:	BL_DBG_SEND("Executing Command: CBL_EN_R_W_PROTECT_CMD");	
			break;
		case CBL_MEM_READ_CMD					:	BL_DBG_SEND("Executing Command: CBL_GET_CBL_MEM_READ_CMDVER_CMD");	
			break;
		case CBL_OTP_READ_CMD					:	BL_DBG_SEND("Executing Command: CBL_OTP_READ_CMD");	
			break;
		case CBL_DIS_R_W_PROTECT_CMD	:	BL_DBG_SEND("Executing Command: CBL_DIS_R_W_PROTECT_CMD");	
			break;
		case CBL_READ_SECTOR_STATUS 	:	BL_DBG_SEND("Executing Command: CBL_READ_SECTOR_STATUS");	
			break;

		default: BL_DBG_SEND("Unkown received command"); break;
	}

	return local_enThisFuncErrStatus;
}

__STATIC __en_blErrStatus_t __enVerifyAddress(uint32 Arg_McuAddressValue) {
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	/* FLASH validity ** SRAM1 Validity */
	if( ((Arg_McuAddressValue >= FLASH_BASE) && (Arg_McuAddressValue <= STM32F103C8Tx_FLASH_END)) ||
			((Arg_McuAddressValue >= SRAM_BASE) && (Arg_McuAddressValue <= STM32F103C8Tx_SRAM1_END))) {
		local_enThisFuncErrStatus = BL_E_OK;
	} else {
		local_enThisFuncErrStatus = BL_E_INVALID_ADDR;
	}
	return local_enThisFuncErrStatus;
}

__STATIC __NORETURN __vJumpToApplication(void) {
	if( (BL_E_INVALID_ADDR != __enVerifyAddress(APP_START_ADDR)) ) {
		/* Read the data stored in the first 4 bytes (Main Stack Pointer) */
		uint32 local_u32MspValue = *((uint32_t volatile *)(APP_START_ADDR));
		/* Read the next 4 bytes from the base address (Reset Handler Function) */
		uint32 local_u32ResetHandler = *((uint32_t volatile *) (APP_START_ADDR + 4U));
		/* Set the reset handler as function */
		void (*local_vAppResetFunc)(void) = (void*)local_u32ResetHandler;
		/* Set the MSP for the application */
		__set_MSP(local_u32MspValue);
		/* Reset all the modules before start */
		HAL_RCC_DeInit();
		/* Call the reset function to start the application */
		BL_DBG_SEND("Succesfully jumped to application.");
		local_vAppResetFunc();
	} else {
		/* Error handle */
		BL_DBG_SEND("Invalid application address");
		return;
	}
}

/**
* ===============================================================================================
*   > Public Functions Implementation
* ===============================================================================================
*/

/**
 * @brief 
 * @details
 * 	Each reset cycle:
 * 		1. Enters boot manager state (BL_enBootManager)
 * 		2. If there is an application already
 * 		 		. Calculate the hash for the exisiting application
 * 					> If it matches the latest received application hash from OEM
 * 			  		then set the BL_APP_VALIDITY && HASH_VALIDITY to True, and jump to the application.
 * 		 	 		> Else Set the BL_APP_VALIDITY && HASH_VALIDITY to False, Start Pipe listner
 * 
 */
__NORETURN BL_enBootManager(void) {
	BL_DBG_SEND("Started the boot manager.");
	if( (BL_FRESH == __enGetIsAppToBlFlag()) || (BL_APP_VALID != __enGetIsValidAppFlag()) ) {
		/* Start listner */
		BL_DBG_SEND("Invalid application, waiting for a valid application");
		__enPipeListen();
	} else {
		/* Process */
		if( (BL_APP_VALID == __enGetIsValidAppFlag()) ) {
			__STATIC sha256_t local_tApplicationHash;  
			strncpy((uint8*)local_tApplicationHash, (uint8*)__tCalculateSHA256HashForApplication(), sizeof(local_tApplicationHash) - 1);
			local_tApplicationHash[sizeof(local_tApplicationHash) - 1] = '\0';
			if ( (0 == strcmp(local_tApplicationHash, global_tApplicationHash)) ) {
				__vSetIsValidHashFlag(TRUE);
				__vSetIsValidAppFlag(TRUE);
				BL_DBG_SEND("Valid hash");
				/* Jump to the application */
				__vJumpToApplication();
			} else {
				__vSetIsValidHashFlag(FALSE);
				__vSetIsValidAppFlag(FALSE);
				BL_DBG_SEND("Invalid hash");
				__enPipeListen();
			}
		} else {
			;
		}
	}
}

__en_blErrStatus_t __enPipeListen(void) {
	BL_DBG_SEND("Command Listening Start pipe: %d | type: %d", BL_COMM_PIPE, BL_COMM_TYPE);
	__en_blErrStatus_t local_enThisFuncErrStatus = BL_E_NONE;
	
	uint8 local_u8PipeListenrBuffer[PIPE_BUFFER_MAX_SIZE];
	memset(local_u8PipeListenrBuffer, 0, PIPE_BUFFER_MAX_SIZE);
	
	if( (HAL_OK != PIPE_LISTEN(local_u8PipeListenrBuffer)) ) {
		BL_DBG_SEND("The pipe listner is not ok.");
		local_enThisFuncErrStatus = BL_E_NOK;
	} else {
		/* Process the buffer */
		cmd_t local_tCommandSeralized;
		
		/* */
		
		if( (BL_E_OK != __bl_enExecuteCommand(&local_tCommandSeralized)) ) {
			__vSendNack();
			local_enThisFuncErrStatus = BL_E_NOK;
			BL_DBG_SEND("Command execution error");
		} else {
			__vSendAck();
			local_enThisFuncErrStatus = BL_E_OK;
		}
	}
	return local_enThisFuncErrStatus;
}

__st_blVersion_t BL_stGetSwVersion(void) {
	return global_stMyBootLoaderVersion;
}








