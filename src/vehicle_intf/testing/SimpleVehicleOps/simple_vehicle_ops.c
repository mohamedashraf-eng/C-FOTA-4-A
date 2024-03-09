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

/** @defgroup STD Libs */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define __BTL_COMM_ST_UART_HANDLE huart1
#define __LOG_ST_UART_HANDLE huart2
#define __TIM3_HANDLE htim3

/**
 * @defgroup Communication
 * 
 * @todo
 *      1. Prepare UART1 to handle the incoming msg [RESET SIGNAL] to jump to bootloader
 *      2. 
 */


void send_log(const uint8_t* format, ...) {
    // Buffer to store the formatted log message
    char buffer[256];  // Adjust the buffer size as needed

    // Use variable argument list for the formatted string
    va_list args;
    va_start(args, format);

    // Format the log message
    vsnprintf(buffer, sizeof(buffer), (const char*)format, args);

    // Transmit the formatted message using UART2
    HAL_UART_Transmit(&__LOG_ST_UART_HANDLE, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

    va_end(args);
}

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

#define MOTOR_1_PIN_P       (0u)
#define MOTOR_1_PIN_N       (0u)
#define MOTOR_1_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

#define MOTOR_2_PIN_P       (0u)
#define MOTOR_2_PIN_N       (0u)
#define MOTOR_2_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

#define MOTOR_3_PIN_P       (0u)
#define MOTOR_3_PIN_N       (0u)
#define MOTOR_3_PIN_E       (COMMON_PWM_MOTOR_SPEED_O_PIN)

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

void StopAllMotors(void) {
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_1_PIN_E, 0u);
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_2_PIN_E, 0u);
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_3_PIN_E, 0u);
    __HAL_TIM_SET_COMPARE(&__TIM3_HANDLE, MOTOR_4_PIN_E, 0u);
}

void ControlMotorSpeed(uint8 motor, uint8 speed) {
    uint16 cvtd_speed = 0;
    cvtd_speed = (uint16)(speed * 553.85f);
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
                    (0xFFU == (speed)) ? StopAllMotors() : NULL;
                    ControlMotorFR(cvtd_speed);
                    ControlMotorFL(cvtd_speed);
                    ControlMotorBR(cvtd_speed);
                    ControlMotorBL(cvtd_speed);
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
    // send_log("Buzzer ON");
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void BuzzerNO(void) {
    // send_log("Buzzer OFF");
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

/**
 * @defgroup Leds 
 * 
 * @todo
 *
 */

#define LED_FR_PIN      (VEHICLE_FRONT_LEDS_Pin)
#define LED_FL_PIN      (VEHICLE_FRONT_LEDS_Pin)

/**
 * @todo To be modifed for intensity (PWM)
 * 
 */
void ControlLedFR(uint16 intensity) {
    switch(intensity) {
        case 0u: 
        case 1u:
            HAL_GPIO_WritePin(VEHICLE_FRONT_LEDS_GPIO_Port, LED_FR_PIN, intensity);
            // send_log("LED[FR]: %d", intensity);
        default: break;
    }
}

void ControlLedFL(uint16 intensity) {
    switch(intensity) {
        case 0u: 
        case 1u:
            HAL_GPIO_WritePin(VEHICLE_FRONT_LEDS_GPIO_Port, LED_FL_PIN, intensity);
            // send_log("LED[FL]: %d", intensity);
        default: break;
    }
}

/**
 * @defgroup Ultrasonic 
 * 
 * @todo
 *
 */

#define USS_TRIGGER_PORT    (USS_TRIGGER_GPIO_Port)
#define USS_TRIGGER_PIN     (USS_TRIGGER_Pin)
#define USS_ECHO_PORT       (VEHICLE_FRONT_LEDS_GPIO_Port)
#define USS_ECHO_PIN        (USS_ECHO_Pin)

static void us_delay(uint16 delay_in_us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    while(__HAL_TIM_GET_COUNTER(&htim2) < delay_in_us);
}

static void uss_trigger(void) {
    HAL_GPIO_WritePin(USS_TRIGGER_PORT, USS_TRIGGER_PIN, GPIO_PIN_RESET);
    us_delay(15);
    HAL_GPIO_WritePin(USS_TRIGGER_PORT, USS_TRIGGER_PIN, GPIO_PIN_SET);
    us_delay(10);
    HAL_GPIO_WritePin(USS_TRIGGER_PORT, USS_TRIGGER_PIN, GPIO_PIN_RESET);
}

static float32 uss_echo(void) {
    return 0.0f;
}

static float32 g_UssDistance;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    static boolean IsFirstCaptured = FALSE;
    static uint16 IcuCapturedVal1 = 0;
    static uint16 IcuCapturedVal2 = 0;
    static uint16 Diff = 0;
    static float32 g_UssDistance = 0;

    if( (HAL_TIM_ACTIVE_CHANNEL_2 == htim->Channel) ) {
        /* */
        if( (FALSE == IsFirstCaptured) ) {
        IcuCapturedVal1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        IsFirstCaptured = TRUE;
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
        } else if( (TRUE == IsFirstCaptured) ) {
        IcuCapturedVal2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        __HAL_TIM_SET_COUNTER(htim, 0);

        if( (IcuCapturedVal2 > IcuCapturedVal1) ) {
            Diff = IcuCapturedVal2 - IcuCapturedVal1;
        } else if( (IcuCapturedVal1 > IcuCapturedVal2) ) {
            Diff = (0xFFFFU- IcuCapturedVal1) + IcuCapturedVal2;
        } else {
            ;
        }
        g_UssDistance = (float32)((Diff * 0.034f) / 2.0f);

        IsFirstCaptured = FALSE;
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
        // __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC1);
        } else {
        ;
        }
    }
}

float32 GetUltraSonicDistance(void) {
    float32 distance = 0.0f;
    float32 tao = 0.0f; 

    // send_log("Starting calculating ultrasonic distance");
    
    /**
     * @brief 
     * 1. Trigger USS
     *      [ <_|-|_>________... ] -> PIN_LOW -> PIN_HIGH (10us) -> PIN_LOW
     * 2. Capture the returned tao of echo pulse
     * 3. Calculate the distance from speed equation
     */

    /** Trigger */
    uss_trigger();
    /** Capture the ECHO PIN */
    tao = uss_echo();

    /** Calculate distance */
    distance = (float32)((float32)((float32)(tao * 1e-6) * 340.29f * 100u) / 2.0f);
    // distance = (float32)(tao * 1e-6 * 340.29f * 0.5f);

    // send_log("Calculated distance: %f", distance);

    return distance;
}

#define USS_THRESHOLD_DISTANCE_MIN_IN_CM     ( (float32) (2) )
#define USS_THRESHOLD_DISTANCE_MAX_IN_CM     ( (float32) (15) )

/**
 * @todo To be tested and accepted 
 */
boolean CheckIfDistanceInValidRange(float32 distance) {
    if( (distance >= USS_THRESHOLD_DISTANCE_MIN_IN_CM) && 
        (distance <= USS_THRESHOLD_DISTANCE_MAX_IN_CM) ) {
        BuzzerUUUUUH();
        return FALSE;
    } else {
        BuzzerNO();
        return TRUE;
    }
}

/**
 * @defgroup Movements 
 * 
 */

void VehicleMoveFwd(void) {

}

void VehicleMoveBwd(void) {

}


