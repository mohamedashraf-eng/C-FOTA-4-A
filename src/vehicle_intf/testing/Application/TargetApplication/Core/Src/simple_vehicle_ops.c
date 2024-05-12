/**
******************************************************************************
* @file           : simple_vehicle_ops.c
* @brief          : simple_vehicle_ops implementation file
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
#include "simple_vehicle_ops.h"
/** @defgroup ST libs */
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "rtc.h"

/** @defgroup STD Libs */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define __BTL_COMM_ST_UART_HANDLE huart1
#define __LOG_ST_UART_HANDLE huart2
#define __TIM3_HANDLE htim3

#define BOOTLOADER_START_ADDR 		( (uint32) (0x08000000UL) )

#define __WRITE_FLAG_APP_TO_BL_ADDR(_FV) HAL_RTCEx_BKUPWrite(&hrtc, __FLAG_APP_TO_BL_ADDR, _FV)
#define __READ_FLAG_APP_TO_BL_ADDR() 	 HAL_RTCEx_BKUPRead(&hrtc, __FLAG_APP_TO_BL_ADDR)
#define __FLAG_APP_TO_BL_ADDR 	RTC_BKP_DR3

/**
 * @defgroup Communication
 * 
 * @todo
 *      1. Prepare UART1 to handle the incoming msg [RESET SIGNAL] to jump to bootloader
 *      2. Characther '#' received => DeInit application, Jump to 0 Address [Bootloader]
 */
__STATIC __NORETURN __vJumpToBootloader(void) {
	__disable_irq();
	/* Pre Init Jump */
	memset((uint32*)NVIC->ICER, 0xFF, sizeof(NVIC->ICER));
	SysTick->CTRL = 0;
	SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
	memset((uint32*)NVIC->ICPR, 0xFF, sizeof(NVIC->ICPR));
	/* Load Vector Table */
	SCB->VTOR = (uint32)(BOOTLOADER_START_ADDR);
	/* Read the data stored in the first 4 bytes (Main Stack Pointer) */
	uint32 local_u32MspValue = *((uint32_t volatile *)(BOOTLOADER_START_ADDR));
	/* Read the next 4 bytes from the base address (Reset Handler Function) */
	uint32 local_u32ResetHandler = *((uint32_t volatile *) (BOOTLOADER_START_ADDR + 4u));
	/* Set the reset handler as function */
#ifdef DIRECT_BOOTLOADER_JUMP
	void (*local_vAppResetFunc)(void) = (void*)local_u32ResetHandler;
#endif /* DIRECT_BOOTLOADER_JUMP */
	/* Set the MSP for the application */
	__set_MSP(local_u32MspValue);
	/* Set stack pointer */
	__set_CONTROL(0);
	/* Activate APP_TO_BL flag */
	__WRITE_FLAG_APP_TO_BL_ADDR(TRUE);
	/* Reset clock before start */
	HAL_RCC_DeInit();

	/* Call the reset function to start the application */
#ifdef DIRECT_BOOTLOADER_JUMP
	local_vAppResetFunc();
#else
	HAL_NVIC_SystemReset();
#endif /* DIRECT_BOOTLOADER_JUMP */
}

static uint8 rx_data = 0;

void start_gp_procedure(void);
void stop_gp_procedure(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if( (&huart1 == huart) ) {
		switch(rx_data) {
			case '#': __vJumpToBootloader(); break;
			case 's': start_gp_procedure();  break;
			case 'S': stop_gp_procedure();   break;
			default: break;
		}
		HAL_UART_Receive_IT(&huart1, &rx_data, 1u);
	}
}

#ifdef DBG
# define __DBG_SEND_OVER_X(__BUFFER, __LEN) HAL_UART_Transmit(&huart2, (uint8*)(&__BUFFER[0]), (uint16)(__LEN), HAL_MAX_DELAY)
#	define DBG_BUFFER_MAX_SIZE ( (uint8) (128) )

#define SEND_LOG(_fmt, ...) send_log(_fmt "\r\n", ##__VA_ARGS__)

void static send_log(const uint8* restrict pArg_u8StrFormat, ...) {
	__STATIC uint8 local_u8DbgBuffer[DBG_BUFFER_MAX_SIZE];

	va_list args;
	va_start(args, pArg_u8StrFormat);
	vsnprintf((char *)local_u8DbgBuffer, DBG_BUFFER_MAX_SIZE, (char *)pArg_u8StrFormat, args);
	va_end(args);

	__DBG_SEND_OVER_X(local_u8DbgBuffer, strlen((const char *)local_u8DbgBuffer));
}
#else
#	define SEND_LOG(_fmt, ...)
#endif /* DBG */

/**
 * @defgroup Motors 
 * 
 * @todo
 *      1. Make functions to control each motor in car [FL, FR, BL, BR]
 *      2. Make function to stop all motors at once
 *
 */
#define COMMON_PWM_MOTOR_SPEED_O_PORT    (GPIOA) 
#define COMMON_PWM_MOTOR_SPEED_O_PIN     (TIM_CHANNEL_2) 

#define MOTOR_1_PORT_P      (0u)
#define MOTOR_1_PORT_N      (0u)
#define MOTOR_1_PIN_P       (0u)
#define MOTOR_1_PIN_N       (0u)
#define MOTOR_1_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

#define MOTOR_2_PORT_P      (0u)
#define MOTOR_2_PORT_N      (0u)
#define MOTOR_2_PIN_P       (0u)
#define MOTOR_2_PIN_N       (0u)
#define MOTOR_2_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

#define MOTOR_3_PORT_P      (0u)
#define MOTOR_3_PORT_N      (0u)
#define MOTOR_3_PIN_P       (0u)
#define MOTOR_3_PIN_N       (0u)
#define MOTOR_3_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

#define MOTOR_4_PORT_P      (0u)
#define MOTOR_4_PORT_N      (0u)
#define MOTOR_4_PIN_P       (0u)
#define MOTOR_4_PIN_N       (0u)
#define MOTOR_4_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

void ControlMotorFR(uint16 speed) {
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_1_PIN_E, speed);
}

void ControlMotorFL(uint16 speed) {
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_2_PIN_E, speed);
}

void ControlMotorBR(uint16 speed) {
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_3_PIN_E, speed);
}

void ControlMotorBL(uint16 speed) {
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_4_PIN_E, speed);
}

void SetRotateCW_FR(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_1_PIN_P, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_1_PIN_N, GPIO_PIN_RESET);

}

void SetRotateCW_FL(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_2_PIN_P, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_2_PIN_N, GPIO_PIN_RESET);
}

void SetRotateCW_BR(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_3_PIN_P, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_3_PIN_N, GPIO_PIN_RESET);
}

void SetRotateCW_BL(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_4_PIN_P, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_4_PIN_N, GPIO_PIN_RESET);
}

void SetRotateCCW_FR(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_1_PIN_N, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_1_PIN_P, GPIO_PIN_RESET);

}

void SetRotateCCW_FL(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_2_PIN_N, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_2_PIN_P, GPIO_PIN_RESET);
}

void SetRotateCCW_BR(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_3_PIN_N, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_3_PIN_P, GPIO_PIN_RESET);
}

void SetRotateCCW_BL(void) {
	HAL_GPIO_WritePin(MOTOR_4_PORT_N, MOTOR_4_PIN_N, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_4_PORT_P, MOTOR_4_PIN_P, GPIO_PIN_RESET);
}

void StopAllMotors(void) {
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_1_PIN_E, 0u);
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_2_PIN_E, 0u);
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_3_PIN_E, 0u);
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_4_PIN_E, 0u);
}

void ControlMotorSpeed(uint8 motor, uint8 speed) {
    uint16 cvtd_speed = (uint16)(speed * 553.85f);
    if (speed >= 0 && speed <= 100) {
        switch (motor) {
            case MOTOR_FR:
                    ControlMotorFR(cvtd_speed);
                break;
            case MOTOR_FL:
                    ControlMotorFL(cvtd_speed);
                break;
            case MOTOR_BR:
                    ControlMotorBR(cvtd_speed);
                break;
            case MOTOR_BL:
                    ControlMotorBL(cvtd_speed);
                break;
            case MOTOR_ALL:
            		if( (0u == speed) ) {
            			StopAllMotors();
            		} else {
                        ControlMotorFR(cvtd_speed);
                        ControlMotorFL(cvtd_speed);
                        ControlMotorBR(cvtd_speed);
                        ControlMotorBL(cvtd_speed);
            		}
                break;
            default:
                break;
        }
    } else {
        StopAllMotors();
    }
}

/**
 * @defgroup Buzzer 
 * 
 * @todo
 *
 */

#define BUZZER_PIN      (BUZZER_Pin)
#define BUZZER_PORT     (BUZZER_GPIO_Port)  

void BuzzerUUUUUH(void) {
    // SEND_LOG("Buzzer ON");
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void BuzzerNO(void) {
    // SEND_LOG("Buzzer OFF");
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

/**
 * @defgroup Leds 
 * 
 * @todo
 *
 */

#define LED_FR_PIN      (TIM_CHANNEL_3)
#define LED_FL_PIN      (TIM_CHANNEL_3)

/**
 * @todo To be modifed for intensity (PWM)
 * 
 */

void ControlLedFR(uint16 intensity) {
	__HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, LED_FR_PIN, intensity);
}

void ControlLedFL(uint16 intensity) {
	__HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, LED_FL_PIN, intensity);
}

void ControlFrontLeds(uint8 led, uint8 intensity) {
	if( ((intensity >= 0) && (intensity <= 100)) ) {
		uint16 cvtd_intensity = (uint16)(intensity * 553.85f);
		switch(led) {
			case LED_FR : ControlLedFR(cvtd_intensity); break;
			case LED_FL : ControlLedFL(cvtd_intensity); break;
			case LED_ALL:
				ControlLedFR(cvtd_intensity);
				ControlLedFL(cvtd_intensity);
				break;
			default: break;
		}
	}
}

/**
 * @defgroup Ultrasonic 
 * 
 * @todo
 *
 */
static uint8 uss_work_flag = FALSE;

static void delay_us(uint16 delay) {
	if(0 != delay) {
	    __HAL_TIM_SET_COUNTER(&htim4, 0);
	    while(__HAL_TIM_GET_COUNTER(&htim4) < delay);
	}
}

#define DELAY_US(_D) delay_us(_D)
#define DELAY_MS(_D) do { 										 	 \
							if (0 != _D) {							 \
								for (uint16_t i = 0; i < 1000u; ++i) \
									delay_us(_D); 					 \
							} 										 \
                    	} while(0)

#define USS_TRIGGER_PORT    (USS_TRIGGER_GPIO_Port)
#define USS_TRIGGER_PIN     (USS_TRIGGER_Pin)

static void uss_trigger(void) {
    HAL_GPIO_WritePin(USS_TRIGGER_PORT, USS_TRIGGER_PIN, GPIO_PIN_RESET);
    DELAY_US(10);
    HAL_GPIO_WritePin(USS_TRIGGER_PORT, USS_TRIGGER_PIN, GPIO_PIN_SET);
    DELAY_US(10);
    HAL_GPIO_WritePin(USS_TRIGGER_PORT, USS_TRIGGER_PIN, GPIO_PIN_RESET);

    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC3);
}

static uint8 g_UssDistance = USS_THRESHOLD_DISTANCE_CM + 1u;
static boolean IsFirstCaptured = FALSE;
static uint32 IcuCapturedVal1 = 0;
static uint32 IcuCapturedVal2 = 0;
static uint32 Diff = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
        if (!IsFirstCaptured) {
            IcuCapturedVal1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
            IsFirstCaptured = TRUE;
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
        } else {
            IcuCapturedVal2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
            __HAL_TIM_SET_COUNTER(htim, 0);

            if (IcuCapturedVal2 > IcuCapturedVal1) {
                Diff = (IcuCapturedVal2 - IcuCapturedVal1);
            } else if (IcuCapturedVal2 < IcuCapturedVal1) {
                // Handle timer overflow case
                Diff = (0xFFFFU - IcuCapturedVal1) + IcuCapturedVal2 + 1; // Add 1 to account for timer overflow
            } else {
                // Equal values, should not occur in normal operation
                SEND_LOG("Error: Equal capture values received");
            }

            // Calculate distance based on the pulse duration
            g_UssDistance = (uint8_t)(0.017 * Diff) + 1U;
            SEND_LOG("g_UssDistance = %d cm", g_UssDistance);

            IsFirstCaptured = FALSE;
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
            __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC3);
        }
    }
}

uint8 GetUltraSonicDistance(void) {
    /**
     * @brief 
     * 1. Trigger USS
     *      [ <_|-|_>________... ] -> PIN_LOW -> PIN_HIGH (10us) -> PIN_LOW
     * 2. Capture the returned tao of echo pulse
     * 3. Calculate the distance from speed equation
     */

    /** Trigger */
    uss_trigger();

    return g_UssDistance;
}

void buzzerngy(uint8_t distance) {
    static uint8 delay = 15;

    if(distance <= (USS_THRESHOLD_DISTANCE_CM + 10u)) delay = 10;
    else if(distance <= (USS_THRESHOLD_DISTANCE_CM + 8u))  delay = 8;
    else if(distance <= (USS_THRESHOLD_DISTANCE_CM + 6u))  delay = 6;
    else if(distance <= (USS_THRESHOLD_DISTANCE_CM + 4u))  delay = 4;
    else if(distance <= (USS_THRESHOLD_DISTANCE_CM + 2u))  delay = 2;
    else if(distance <= (USS_THRESHOLD_DISTANCE_CM - 1u))  delay = 0;

	if (distance <= (USS_THRESHOLD_DISTANCE_CM + 10u)) {
        BuzzerUUUUUH();
        DELAY_MS(delay);
        (0 != delay) ? BuzzerNO() : 0;
    } else {
        BuzzerNO();
    }
}

boolean CheckIfDistanceInValidRange(uint8 distance) {
    boolean motors_are_stopped = FALSE;
    if( (TRUE == uss_work_flag) ) {
		if (distance >= 0 && distance <= USS_THRESHOLD_DISTANCE_CM) {
			buzzerngy(distance);
			if (distance <= (uint8)((USS_THRESHOLD_DISTANCE_CM * 0.50f) + 1u) && !motors_are_stopped) {
				ControlMotorSpeed(MOTOR_ALL, 0u);
				motors_are_stopped = TRUE;
			}
			return FALSE;
		} else {
	//        ControlMotorSpeed(MOTOR_ALL, MOTORS_SPEED);
			BuzzerNO();
			motors_are_stopped = FALSE;
			return TRUE;
		}
    }
}


/**
 * @defgroup Movements 
 * 
 */

void VehicleMoveFwd(void) {
	SetRotateCW_FL();
	SetRotateCW_FR();
	SetRotateCW_BR();
	SetRotateCW_BL();
}

void VehicleMoveBwd(void) {
	SetRotateCCW_FL();
	SetRotateCCW_FR();
	SetRotateCCW_BR();
	SetRotateCCW_BL();
}

static void front_leds_init(void) {
	uint8 i = 0;
	for(; (i <= 100); ++i) {
		ControlLedFL(i);
		ControlLedFR(i);
		DELAY_MS(2);
	}
}

typedef struct {
    uint16 tone;
    uint16 duration;
} RingtoneStep;

// Nokia ringtone sequence
static RingtoneStep nokiaRingtone[] = {
    {2637, 200},
    {0, 100},
    {2637, 200},
    {0, 100},
    {2637, 200},
    {0, 100},
    {3136, 200},
    {0, 100},
    {2349, 200},
    {0, 100},
    {2217, 200},
    {0, 100},
    {2349, 200},
    {0, 100},
    {0, 500}
};

static void buzzer_init(RingtoneStep *ringtone, uint16 steps) {
	uint16 i = 0;
    for (i = 0; i < steps; i++) {
        if (ringtone[i].tone == 0) {
        	DELAY_MS(25);
        } else {
            BuzzerUUUUUH();
            DELAY_MS(50);
            BuzzerNO();
        }
    }
}

static void uss_init(void) {
	GetUltraSonicDistance();
	CheckIfDistanceInValidRange(g_UssDistance);
}

static void motors_init(void) {
	ControlMotorSpeed(MOTOR_ALL, 10);
	DELAY_MS(30);
	ControlMotorSpeed(MOTOR_ALL, 0);
}

void vehicle_init(void) {
	SEND_LOG("\n Starting Application with VTOR: 0x%x", SCB->VTOR);
	__WRITE_FLAG_APP_TO_BL_ADDR(FALSE);
	/* Init front leds */
//	front_leds_init();
	/* Init buzzer */
	buzzer_init(nokiaRingtone, sizeof(nokiaRingtone) / sizeof(nokiaRingtone[0]));
	/* Init ultrasonic */
//	uss_init();
	/* Init motors */
//	motors_init();

//	ControlFrontLeds(LED_ALL, FRONT_LEDS_INTENSITY);
//	ControlMotorSpeed(MOTOR_ALL, MOTORS_SPEED);

	HAL_UART_Receive_IT(&huart1, &rx_data, 1u);
}

void start_gp_procedure(void) {
	uss_work_flag = TRUE;
	ControlMotorSpeed(MOTOR_ALL, MOTORS_SPEED);
	ControlFrontLeds(LED_ALL, FRONT_LEDS_INTENSITY);
}

void stop_gp_procedure(void) {
	uss_work_flag = FALSE;
	ControlMotorSpeed(MOTOR_ALL, 0);
	ControlFrontLeds(LED_ALL, 0);
}
