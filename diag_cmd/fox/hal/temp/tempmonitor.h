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
***            tempmonitor.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/11, 10:02:52, Eden Weng
***             File Creation
***
***       - 2010/01/14, Jungle Chen
***             Modified to monitor 6 temperature inputs
***             <cpu|mac1|mac2|t1|t2|t3>
***
***************************************************************************/
#ifndef __TEMPMONITOR_H_
#define __TEMPMONITOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mcu_hal.h"
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
 *      Function Definition Segment
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
 *      sign_flag - positive , or negative 
 *
 *  OUTPUT :
 *      temp_int - integer portion of temperature
 *      temp_decimal - register address
 *      sign_flag - 0:positive , or 1:negative 
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
);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPMONITOR_H_ */