/**
******************************************************************************
* @file           : simple_vehicle_ops.h
* @brief          : simple_vehicle_ops public interface file
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
#ifndef __SIMPLE_VEHICLE_OPS_H__
#define __SIMPLE_VEHICLE_OPS_H__

#ifdef __cplusplus
extern "c" {
#endif /* __cplusplus */

#include "Std_Types.h"

//#define DBG
//#define DIRECT_BOOTLOADER_JUMP


#define MOTOR_FR  ( (uint8) (1u) )
#define MOTOR_FL  ( (uint8) (2u) )
#define MOTOR_BR  ( (uint8) (3u) )
#define MOTOR_BL  ( (uint8) (4u) )
#define MOTOR_ALL ( (uint8) (5u) )

#define LED_FR    ( (uint8) (0u) )
#define LED_FL    ( (uint8) (1u) )
#define LED_ALL   ( (uint8) (2u) )

/**
 * @defgroup App Cfgs
 *
*/
/* Motors */
#define MOTORS_SPEED								( (uint8) (60) )
/* Leds */
#define FRONT_LEDS_INTENSITY						( (uint8) (80) )
/* Ultrasonic */
#define USS_THRESHOLD_DISTANCE_CM					( (uint8) (15) )

/**
 * @defgroup Movements
 *
 */

void ControlLedFL(uint16 intensity);
void ControlLedFR(uint16 intensity);

uint8 GetUltraSonicDistance(void);

boolean CheckIfDistanceInValidRange(uint8 distance);

void ControlMotorSpeed(uint8 motor, uint8 speed);

void vehicle_init(void);

void ControlFrontLeds(uint8 led, uint8 intensity);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SIMPLE_VEHICLE_OPS_H__ */
