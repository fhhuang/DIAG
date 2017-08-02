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
***            volmonintor.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/11, 10:02:52, Eden Weng
***             File Creation
***
***       - 2010/01/13, Jungle Chen
***             Modified to monitor 11 voltage inputs
***             <0.9|3.3|all>
***
***************************************************************************/

#ifndef __VOLMONITOR_H_
#define __VOLMONITOR_H_

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
typedef enum {
    VOL_3_3_SOURCE = 0x1,
    VOL_2_5_SOURCE,
    VOL_1_8_SOURCE,
    VOL_1_5_SOURCE,
    VOL_0_99_SOURCE,
    VOL_0_9_SOURCE,
    VOL_ALL_SOURCE,
}E_VOL_SOURCE_TYPE;


typedef enum {
    VOL_NORMAL = 0,
    VOL_LOW = 1,
    VOL_HIGH = 2
}E_VOL_LEVEL;

/*==========================================================================
 *
 *      Function Definition Segment
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
 *      voltage source
 *
 *  OUTPUT :
 *      vol_int, vol_decimal
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
);
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
);

#ifdef __cplusplus
}
#endif

#endif /* __VOLMONITOR_H_ */
