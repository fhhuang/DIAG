/***************************************************************************
***
***    Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without
***    the written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.
***    Co. Ltd. company private, proprietary, and trade secret property and
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************
***
***    FILE NAME :
***      intr_hal.c
***
***    DESCRIPTION :
***      for interrupt hal
***
***    HISTORY :
***       - 2009/05/20, 10:30:52, Eden Weng
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *                                                                          
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
/* System Library */
/* User-defined Library */
#ifndef FOX_KERNEL
#include "config.h"
#include "common.h"
#endif

#include "cmn_type.h"
#include "intr_hal.h"
#include "porting.h"
#include "port_utils.h"
#include "sys_utils.h"
#include "gpio_hal.h"
#include "err_type.h"
#include "log.h"
#include "i2c_fpga.h"
#include "mcu_hal.h"

/*==========================================================================
 *
 *      Constant Definition Segment
 *
 *==========================================================================
 */
#define DBG_PRINTF(...)
//#define DBG_PRINTF printf
/*==========================================================================
 *
 *      Structrue Definition segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Static Variable segment
 *
 *==========================================================================
 */

/*==========================================================================
 *                                                                          
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */

/*==========================================================================
 *
 *      Local Function segment
 *
 *==========================================================================
 */ 

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_halIntStatusGet
 *
 *  DESCRIPTION :
 *      special interrupt source has trig
 *
 *  INPUT :
 *      extIntr -interrupt source
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 intr_halIntStatusGet
(
    IN  E_EXT_INTR                  extIntr,
    OUT UINT32                      *status
)
{
    UINT32                          gpioNum = 0;
    UINT8                           data;

    switch (extIntr)
    {
        case E_EXT_INTR_SFP_PRESENT:
        case E_EXT_INTR_SFP_RX_LOSS:
        case E_EXT_INTR_SFP_TX_FAULT:
        case E_EXT_INTR_SB:
        case E_EXT_INTR_MCU:
            gpioNum = GPIO_CPU_GPIO_9;
            break;
        case E_EXT_INTR_PUSH_BUTTON:
            gpioNum = GPIO_CPU_GPIO_13;
            break;
        default:
            break;
    }
    
    if (gpio_halGetVal(gpioNum, &data) != E_TYPE_SUCCESS)
    {
        DBG_PRINTF("\nGet CPU GPIO fail!");
        return E_TYPE_INIT_FAIL;
    }
    
    *status = (UINT32)data;

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_halExtIntrClear
 *
 *  DESCRIPTION :
 *      clear external interrupt in an assigned time
 *
 *  INPUT :
 *      extIntr -interrupt source
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 intr_halIntrClear
(
    IN  E_EXT_INTR                  extIntr
)
{
    INT32                           ret = E_TYPE_SUCCESS;
    UINT16                          regVal;

    switch(extIntr)
    {
        case E_EXT_INTR_PUSH_BUTTON:
            break;
        case E_EXT_INTR_SB:
            break;
        case E_EXT_INTR_MCU:
            /* reset MCU interrupt */
            ret = mcu_halDataSet(0x80, 0, 0x1);
            if( ret != E_TYPE_SUCCESS )
            {
                goto __INTR_ERROR;
            }
            break;
        case E_EXT_INTR_SFP_PRESENT:
        case E_EXT_INTR_SFP_RX_LOSS:
        case E_EXT_INTR_SFP_TX_FAULT:   
            break;
        default:
            ret = E_TYPE_INT_NO_SUPPORT;
            goto __INTR_ERROR;
    }

__INTR_ERROR:
    fpgaRegRead(FPGA_INT_STA_REG, &regVal); /* Read and Clear the interrupt status value */ 
    return ret;

}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_halIntrGen
 *
 *  DESCRIPTION :
 *      trigger interrupt
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 intr_halIntrGen
(
    IN  E_EXT_INTR                  extIntr
)
{
    INT32                           ret = E_TYPE_SUCCESS;

    switch(extIntr)
    {
        case E_EXT_INTR_MCU:
            /* Trigger MCU interrupt */
            ret = mcu_halDataSet(0x80, 0, 0x0);

            if( ret != E_TYPE_SUCCESS )
            {
                goto __INTR_ERROR;
            }
            break;
        case E_EXT_INTR_SB:
            break;
        default:
            ret = E_TYPE_INT_NO_SUPPORT;
            goto __INTR_ERROR;
    }

__INTR_ERROR:
    return ret;
}


