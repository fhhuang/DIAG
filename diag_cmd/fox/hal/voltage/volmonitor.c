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
***      volmonitor.c
***
***    DESCRIPTION :
***      for voltage monitor
***
***    HISTORY :
***       - 2009/05/11, 10:30:52, Eden Weng
***             File Creation
***
***       - 2010/01/13, Jungle Chen
***             Modified to monitor 11 voltage inputs
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include "cmn_type.h"
#include "porting.h"
#include "volmonitor.h"

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
 *      voltage_read
 *
 *  DESCRIPTION :
 *      Get the Voltage from MCU
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      decimal, dotpoint
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
INT32 voltage_read
(
  OUT UINT8 *vol_int,
  OUT UINT8 *vol_decimal,
  IN E_VOL_SOURCE_TYPE vol_source
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    UINT32 read_data;
    E_MCU_CMD mcu_cmd;

    mcu_cmd = vol_source + 5;

    if((ret = mcu_halDataSet(mcu_cmd, 0, 0)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Set MCU data fail\n", __FUNCTION__);
    }
	
    if((ret = mcu_halDataGet(mcu_cmd, 0, &read_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }
    else
    {
        *vol_int = (UINT8)((read_data >> 16) & 0xff);
        *vol_decimal = (UINT8)((read_data >> 24) & 0xff);
    }

    return ret;
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      voltage_write
 *
 *  DESCRIPTION :
 *      Set the Voltage to MCU
 *
 *  INPUT :
 *      voltage level, source type
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
INT32 voltage_write
(
  IN E_VOL_LEVEL vol_level,
  IN E_VOL_SOURCE_TYPE vol_source
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    INT32 write_data;

    switch(vol_source)
    {
        /* Byte2: voltage level, Byte 1: point, Byte0: integer */
        case VOL_3_3_SOURCE: write_data = (INT32)((vol_level << 16) | (3 << 8) | 3); break;
        case VOL_1_8_SOURCE: write_data = (INT32)((vol_level << 16) | (8 << 8) | 1); break;
        case VOL_1_5_SOURCE: write_data = (INT32)((vol_level << 16) | (5 << 8) | 1); break;
        case VOL_0_9_SOURCE: write_data = (INT32)((vol_level << 16) | (9 << 8) | 0); break;
		case VOL_ALL_SOURCE: write_data = (INT32)(vol_level << 16); break;
    }

    DBG_PRINTF("%s, write_data = 0x%x\n", __FUNCTION__, write_data);
    if((ret = mcu_halDataSet(MCU_SET_VOL_MAR, 0, write_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Set MCU data fail\n", __FUNCTION__);
    }

    return ret;
}