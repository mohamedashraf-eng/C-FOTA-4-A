/**
 * @file sim800lv2_prv.h
 * @author Mohamed Ashraf (mohamedashrafwx@gmail.com)
 * @author Abdelrahman Ali (abdelrahmanali@gmail.com)
 * @brief The interfacing header for the sim module.
 * @version 0.1
 * @date 2022-12-19
 *
 * @copyright Copyright (c) Wx Bo 2022
 *
 */
/** @def Header guards */
#ifndef __SIM800LV2_PRV_H__
#define __SIM800LV2_PRV_H__

/*
 * ----------------------------------------------------------------------------
 * Private datatypes
 * ----------------------------------------------------------------------------
**/

/*
 * ----------------------------------------------------------------------------
 * Private macros
 * ----------------------------------------------------------------------------
**/

/** @defgroup SIM800L V2 - Module AT Commands */


/*
 * ----------------------------------------------------------------------------
 * Private functions implementation
 * ----------------------------------------------------------------------------
**/

/**
 * @brief Function to send a command to sim800 module.
 * 
 * @param copy_u8cmd 
 * @return en_sim800lv2_status_t 
 */
static inline en_sim800lv2_status_t
enSendCmd(const u8 * const copy_u8cmd);











#endif /* #define __SIM800LV2_PRV_H__ */
