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

#include "esp8266.h"
/** @defgroup ST libs */
#include "usart.h"
/** @defgroup STD Libs */
#include <string.h>

/**
* ===============================================================================================
*   > Private Macros
* ===============================================================================================
*/

#if defined(ESP8266_PIPE_TYPE)
# if (ESP8266_PIPE_TYPE == ESP8266_PIPE_UART)
#   if defined(ESP8266_PIPE_MODE)
#     if (ESP8266_PIPE_MODE == ESP8266_PIPE_SYNC)
#       define ESP8266_PIPE_LISTEN(__BUFFER, __LEN) HAL_UART_Receive(&huart2, (__BUFFER), (uint16)(__LEN), HAL_MAX_DELAY)
#       define ESP8266_PIPE_ECHO(__BUFFER, __LEN) HAL_UART_Transmit(&huart2, (__BUFFER), (uint16)(__LEN), HAL_MAX_DELAY)
#     else
#       error "Please define 'ESP8266_PIPE_MODE'"
#     endif /* (ESP8266_PIPE_MODE == ESP8266_PIPE_SYNC)*/
#   endif /* ESP8266_PIPE_MODE */
#  endif /* (ESP8266_PIPE_TYPE == ESP8266_PIPE_UART) */
#else
# error "Please define 'ESP8266_PIPE_TYPE'"
#endif /* ESP8266_PIPE_TYPE */


#define ESP_RESPONSE_RX_BUFFER_LENGTH (256u)

/**
 * @defgroup Available commands
 * @brief 
 * @{
 */

#define ESP8266_CMD_AT        "AT\r\n"
#define ESP8266_CMD_AT_LEN    2u

/**
 * @}
 */

/**
* ===============================================================================================
*   > Private Enums
* ===============================================================================================
*/

/**
 * @brief Enum for the esp8266 status
 * 
 */
enum {
  ESP_S_NONE = -1,
  ESP_S_OK,
  ESP_S_NOK
  ESP_S_INVALID_CMD,
  ESP_S_INVALID_ARG
} __esp8266Ret;

/**
 * @brief Enum for the available esp8266 commands
 * 
 */
enum {
  CMD_AT = 0
} __esp8266Cmd;

/**
* ===============================================================================================
*   > Global Private Types
* ===============================================================================================
*/

typedef enum __esp8266Ret espStatus;

typedef enum __esp8266Cmd espCmd;

/**
* ===============================================================================================
*   > Private Functions Declaration
* ===============================================================================================
*/

__FORCE_INLINE
__STATIC espStatus __getCmdResponse(uint8* pArg_u8RxBuffer);
__STATIC espStatus __parseCmdResponse(uint8* pArg_u8Response);

__STATIC espStatus __wrtCmd(espCmd Arg_enCmd);
__LOCAL_INLINE espStatus __wrtCmd_AT(void);

/**
* ===============================================================================================
*   > Private Functions Implementation
* ===============================================================================================
*/

__STATIC espStatus __getCmdResponse(uint8* pArg_u8RxBuffer) {
  espStatus local_enThisFuncStatus = ESP_S_OK;
  __STATIC uint8 local_u8RxBuffer[ESP_RESPONSE_RX_BUFFER_LENGTH];
  
  // memset(local_u8RxBuffer, 0u, ESP_RESPONSE_RX_BUFFER_LENGTH);

  if( (HAL_OK != ESP8266_PIPE_LISTEN(local_u8RxBuffer, ESP_RESPONSE_RX_BUFFER_LENGTH)) ) {
    local_enThisFuncStatus =  ESP_S_NOK;
  } else {
    memccpy(pArg_u8RxBuffer, local_u8RxBuffer, strlen(local_u8RxBuffer));
  }
  return local_enThisFuncStatus;
}

__STATIC espStatus __parseCmdResponse(uint8* pArg_u8Response) {
  espStatus local_enThisFuncStatus = ESP_S_NOK;
  if( (NULL == pArg_u8Response) ) {
    local_enThisFuncStatus = ESP_S_INVALID_ARG;
  } else {
    // if( (0 == strcmp(pArg_u8Response, "OK")) ) {
      
    // } else if( (0 == strcmp(pArg_u8Response, "NOT OK")) ) {

    // } else {

    // }
  }
  return local_enThisFuncStatus;
}

__STATIC espStatus __wrtCmd(espCmd Arg_enCmd) {
  espStatus local_enThisFuncStatus = ESP_S_NONE;
  switch(Arg_enCmd) {
    case CMD_AT: 
      local_enThisFuncStatus = __wrtCmd_AT();
      break;
    default: break;
  }
  return local_enThisFuncStatus;
}

/**
 * @brief 
 * @defgroup Available commands
 * @{
 */

__LOCAL_INLINE espStatus __wrtCmd_AT(void) {
  espStatus local_enThisFuncStatus = ESP_S_OK;
  if( (HAL_OK != ESP8266_PIPE_ECHO(ESP8266_CMD_AT, ESP8266_CMD_AT_LEN)) ) {
    local_enThisFuncStatus = ESP_S_NOK;
  } else {
    ;
  }
  return local_enThisFuncStatus;
}

/**
 * @}
 */

/**
* ===============================================================================================
*   > Public Functions Implementation
* ===============================================================================================
*/

