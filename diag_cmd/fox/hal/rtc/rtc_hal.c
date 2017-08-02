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
***      mcu_hal.c
***
***    DESCRIPTION :
***      for mcu hal
***
***    HISTORY :
***       - 2015/11/27, Eric Hsu
***             File Creation
***
***************************************************************************/
/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
/* User-defined Library */
#include "cmn_type.h"
#include "porting.h"

#include "err_type.h"
#include "log.h"
#include "mcu_hal.h"


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halGetDate
 *
 *  DESCRIPTION :
 *      Get the date from MCU
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      day, month, year
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
INT32 rtc_halGetDate
(
  OUT UINT8 *day,
  OUT UINT8 *month,
  OUT UINT8 *year,
  OUT UINT8 *week
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    INT32 read_data;

    if((ret = mcu_halDataSet(MCU_GET_DATE, 0, 0)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
        return ret;
    }
	
    if((ret = mcu_halDataGet(MCU_GET_DATE, 0, &read_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }
    else
    {
        *day = (UINT8)(read_data & 0xff);
        *month = (UINT8)((read_data >> 8) & 0xff);
        *year = (UINT8)((read_data >> 16) & 0xff);
        *week = (UINT8)((read_data >> 24) & 0xff);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halGetTime
 *
 *  DESCRIPTION :
 *      Get the Time from MCU
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      second, min, hour
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
INT32 rtc_halGetTime
(
  OUT UINT8 *second,
  OUT UINT8 *min,
  OUT UINT8 *hour
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    INT32 read_data;

    if((ret = mcu_halDataSet(MCU_GET_TIME, 0, 0)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
        return ret;
    }

    if((ret = mcu_halDataGet(MCU_GET_TIME, 0, &read_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }
    else
    {
        *second = (UINT8)(read_data & 0xff);
        *min = (UINT8)((read_data >> 8) & 0xff);
        *hour = (UINT8)((read_data >> 16) & 0xff);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halSetDate
 *
 *  DESCRIPTION :
 *      Set the Date to MCU
 *
 *  INPUT :
 *      day, month, year
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
INT32 rtc_halSetDate
(
  IN UINT8 day,
  IN UINT8 month,
  IN UINT8 year,
  IN UINT8 week
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    INT32 write_data;

    write_data = (INT32)((week << 24) | (year << 16) | (month << 8) | day);

    if((ret = mcu_halDataSet(MCU_SET_DATE, 0, write_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halSetTime
 *
 *  DESCRIPTION :
 *      Set the Time to MCU
 *
 *  INPUT :
 *      second, min, hour
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
INT32 rtc_halSetTime
(
  IN UINT8 second,
  IN UINT8 min,
  IN UINT8 hour
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    INT32 write_data;

    write_data = (INT32)((hour << 16) | (min << 8) | second);

    if((ret = mcu_halDataSet(MCU_SET_TIME, 0, write_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }

    return ret;
}