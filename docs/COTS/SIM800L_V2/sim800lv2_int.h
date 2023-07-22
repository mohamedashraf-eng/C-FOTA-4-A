/**
 * @file sim800lv2_int.h
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
#ifndef __SIM800LV2_INT_H__
#define __SIM800LV2_INT_H__

/*
 * ----------------------------------------------------------------------------
 * Public datatypes
 * ----------------------------------------------------------------------------
**/

/**
 * @brief enum for the module error status.
 * 
 */
typedef enum sim800lv2_Status
{
    sim800lv2_timeout       = 0x00U,
    sim800lv2_ack,
    sim800lv2_nack,
    sim800lv2_invalidParam,
    sim800lv2_invalidData,
    sim800lv2_invalidCmd
}en_sim800lv2_status_t;

/*
 * ----------------------------------------------------------------------------
 * Public macros
 * ----------------------------------------------------------------------------
**/

/*
 * ----------------------------------------------------------------------------
 * Public functions implementation
 * ----------------------------------------------------------------------------
**/











#endif /* __SIM800LV2_INT_H__ */
