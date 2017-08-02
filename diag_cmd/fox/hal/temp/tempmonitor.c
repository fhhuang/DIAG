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
***      tempmonitor.c
***
***    DESCRIPTION :
***      for temperature monitor
***
***    HISTORY :
***       - 2009/05/11, 10:30:52, Eden Weng
***             File Creation
***
***       - 2010/01/14, Jungle Chen
***             Modified to monitor 6 temperature inputs
***             <cpu|mac1|mac2|t1|t2|t3>
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
#include "tempmonitor.h"
#include "err_type.h"
#include "log.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
 
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
 *      temp_read
 *
 *  DESCRIPTION :
 *      read the temperature from MCU. 
 *
 *  INPUT :
 *      temp_int - integer portion of temperature
 *      temp_decimal - register address
 *      sign_flag - 0:positive , or 1:negative 
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
INT32 temp_read
(
  IN  UINT8 temp_source,
  OUT UINT8 *temp_int, 
  OUT UINT8 *temp_decimal, 
  OUT UINT8  *sign_flag
)
{
    UINT32 ret= E_TYPE_SUCCESS;
    UINT32 read_data; 


    if((ret = mcu_halDataSet(temp_source, 0, 0)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Set MCU data fail\n", __FUNCTION__);
    }
	
    if((ret = mcu_halDataGet(temp_source, 0, &read_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }
    else    
    {
        *temp_int = (UINT8)(read_data & 0xff);
        *temp_decimal = (UINT8)((read_data >> 8) & 0xff);
        *sign_flag = (UINT8)((read_data >> 16) & 0xff);
    }
    
    return ret;  
}
