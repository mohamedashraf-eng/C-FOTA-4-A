/**
******************************************************************************
* @file           : Compiler_Cfg.h
* @brief          : Compiler Configuration File
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
/**
 * @file Compiler_Cfg.h
 * @author Mohamed Ashraf (wx@wx.com)
 * @brief AUTOSAR Base - Module specific memory and pointer clases
 * @version 1.0.0
 * @date 2023-01-04
 * @details The file Compiler_Cfg.h contain the module specific parameters (ptrclass and memclass) that are
 *                  passed to the macros defined in Compiler.h
 *
 * @copyright Copyright (c) 2023
 *
 */
/*
 * =============================================================================
 * Project              : AUTOSAR 4.4.0 MCAL
 * Platform             : ARM
 * Peripheral           : STM32F103
 * Autosar Version      : 4.4.0
 * Autosar Revision     : ASR_REL_4_0_REV_0000
 * SW Version           : 1.0.0
 * =============================================================================
*/
/** @def Header Guards */
#ifndef __COMPILER_CFG_H__
#define __COMPILER_CFG_H__

/*
 * =============================================================================
 * = SOURCE FILE VERSION INFORMATION
 * =============================================================================
*/

/** @brief Vendor specific ID */
#define COMPILER_CFG_VENDOR_ID                               (1332)
/** @brief Program based version */
#define COMPILER_CFG_SW_MAJOR_VERSION                        (1)
#define COMPILER_CFG_SW_MINOR_VERSION                        (0)
#define COMPILER_CFG_SW_PATCH_VERSION                        (0)


#endif /* __COMPILER_CFG_H__ */
