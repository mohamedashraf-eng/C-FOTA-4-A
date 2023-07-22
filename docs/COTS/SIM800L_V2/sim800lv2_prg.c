/**
 * @file sim800lv2_prg.c
 * @author Mohamed Ashraf (mohamedashrafwx@gmail.com)
 * @author Abdelrahman Ali (abdelrahmanali@gmail.com)
 * @brief The implementation file for the sim module.
 * @version 0.1
 * @date 2022-12-19
 *
 * @copyright Copyright (c) Wx Bo 2022
 *
 */

/*
 * ----------------------------------------------------------------------------
 * Included headers
 * ----------------------------------------------------------------------------
*/

/** @defgroup external headers */
// #include "std_types.h"
// #include "macros.h"
// #include "uart_int.h"

/** @defgroup internal headers */
#include "sim800lv2_prv.h"
#include "sim800lv2_cfg.h"
#include "sim800lv2_int.h"

/*
 * ----------------------------------------------------------------------------
 * Global macros
 * ----------------------------------------------------------------------------
**/


/*
 * ----------------------------------------------------------------------------
 * Global datatypes
 * ----------------------------------------------------------------------------
**/


/*
 * ----------------------------------------------------------------------------
 * Public functions implementation
 * ----------------------------------------------------------------------------
**/

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
enSendCmd(const u8 * const copy_u8cmd)
{
#if (STATIC_ASSERTION == ASSERTION_ACTIVE)
    STATIC_ASSERT( (copy_u8cmd != NULL) );
    STATIC_ASSERT( (string_length(copy_u8cmd) > 0x02U) );
#endif
    /* Function data types */
    en_sim800lv2_status_t l_enThisFunctionStatus = sim800lv2_nack;
    en_uart_status_t l_enUartFunctionStatus = uart_nack;

    /* Debug info */
#if (UART_DEBUGGING_FLAG == DEBUGGING_FLAG_ACTIVE)
    uart_debug_info("Passed command: %s", copy_u8cmd);
#endif

#if (SWO_DEBUGGING_FLAG == DEBUGGING_FLAG_ACTIVE)
    swo_debug_info("Passed command: %s", copy_u8cmd);
#endif

    /* Error checking */
    if( (NULL == copy_u8cmd) || 
        (string_length(copy_u8cmd) > 0x02U) )
    {
        l_enThisFunctionStatus = sim800lv2_invalidCmd;
    }
    else 
    {
        /* Send the command via uart */
        l_enUartFunctionStatus = uart_send_data(copy_u8cmd);

    #if (STATIC_ASSERTION == ASSERTION_ACTIVE)
        STATIC_ASSERT( (l_enUartFunctionStatus != uart_nack) );
    #endif

        /* Check if there is no errors */
        if( (uart_ack == l_enUartFunctionStatus) )
        {
            l_enThisFunctionStatus = sim800lv2_ack;
        }
        else {;}
    }

    return l_enThisFunctionStatus;
}