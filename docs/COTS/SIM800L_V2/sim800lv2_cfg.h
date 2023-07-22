/**
 * @file sim800lv2_cfg.h
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
#ifndef __SIM800LV2_CFG_H__
#define __SIM800LV2_CFG_H__

/*
 * ----------------------------------------------------------------------------
 * Debugging Configuration Parameters
 * ----------------------------------------------------------------------------
**/

/**
 * @brief Channel selecting for the debugging output.
 * 
 * @defgroup Configuration parameters:
 *      @arg DEBUGGING_FLAG_ACTIVE   
 *      @arg DEBUGGING_FLAG_INACTIVE
 * 
 */
#define UART_DEBUGGING_FLAG     (DEBUGGING_FLAG_ACTIVE)
#define SWO_DEBUGGING_FLAG      (DEBUGGING_FLAG_INACTIVE)

/**
 * @brief Channel selecting for the debugging output.
 * 
 * @defgroup Configuration parameters:
 *      @arg STATIC_ASSERTION_ACTIVE   
 *      @arg STATIC_ASSERTION_INACTIVE
 * 
 */
#define STATIC_ASSERTION          (STATIC_ASSERTION_ACTIVE)


/*
 * ----------------------------------------------------------------------------
 * Module Configuration Parameters
 * ----------------------------------------------------------------------------
**/

/**
 * @brief Channel selecting for the module uart.
 * 
 * @defgroup Configuration parameters:
 *      @arg MODULE_UART1   
 *      @arg MODULE_UART2
 *      @arg MODULE_UART3
 * 
 * @note For the module uart channel it depends 
 *       on the connected microcontroller.
 *      
 */
#define SIM800_UART_CHANNEL    (MODULE_UART1)

#endif /* __SIM800LV2_CFG_H__ */
