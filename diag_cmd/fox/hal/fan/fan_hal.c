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
***      fan_hal.c
***
***    DESCRIPTION :
***      for fan hal
***
***    HISTORY :
***       - 2010/01/13, Jungle Chen
***             i2c_halI2CMuxSwitch() modified
***
***       - 2009/03/04, 13:30:52, Eden Weng
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
#include <pthread.h>
#include <sys/time.h>

/* User-defined Library */
#include "cmn_type.h"
#include "porting.h"

/* HAL-dep */
#include "fan_hal.h"
#include "err_type.h"
#include "log.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#undef DBG
#ifdef DBG
#define DBG_PRINTF log_dbgPrintf
#else
#define DBG_PRINTF
#endif
/*==========================================================================
 *
 *      Structrue segment
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
 *      Local Function Body segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fan_halSetSpeed
 *
 *  DESCRIPTION :
 *      write fan speed
 *
 *  INPUT :
 *      fan_speed    -   the percentage of duty cycle of fan
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
INT32 fan_halSetSpeed
(
  IN UINT32 fan_speed
)
{
    UINT32 ret= E_TYPE_SUCCESS;

    DBG_PRINTF("%s, write_data = 0x%x\n", __FUNCTION__, fan_speed);
    if((ret = mcu_halDataSet(MCU_SET_SPEED, 0, fan_speed)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Set MCU data fail.\n", __FUNCTION__);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fan_halGetSpeed
 *
 *  DESCRIPTION :
 *      read fan speed
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      fan_speed    -   the percentage of duty cycle of fan
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
INT32 fan_halGetSpeed
(
  OUT UINT32 *fan_speed
)
{
    UINT32 ret= E_TYPE_SUCCESS;

    if((ret = mcu_halDataSet(MCU_GET_SPEED, 0, 0)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Set MCU data fail.\n", __FUNCTION__);
        return ret;
    }

    if((ret = mcu_halDataGet(MCU_GET_SPEED, 0, &(*fan_speed))) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail.\n", __FUNCTION__);
    }

    return ret;
}