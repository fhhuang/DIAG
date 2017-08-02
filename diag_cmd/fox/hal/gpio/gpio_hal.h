/***************************************************************************
***
***     Copyright 2009  Foxconn
***     All Rights Reserved
***     No portions of this material may be reproduced in any form
***     without the written permission of:
***
***                 Foxconn CNSBG
***
***     All information contained in this document is Foxconn CNSBG
***     company private, proprietary, and trade secret.
***
****************************************************************************
***
***    FILE NAME :
***            gpio_hal.h
***
***    DESCRIPTION :
***      APIs for GPIO info
***
***    HISTORY :
***       - 2010/10/29, Chungmin Lai
***             File Creation
***
***************************************************************************/
#ifndef __GPIO_HAL_H
#define __GPIO_HAL_H

#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30

#define GPIO_CPU_GPIO_9      9
#define GPIO_CPU_GPIO_13    13
#define GPIO_HIGH		1
#define GPIO_LOW		0

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halInit
 *
 *  DESCRIPTION :
 *      Init GPIO
 *
 *  INPUT :
 *      whichFan    -   fan index
 *
 *  OUTPUT :
 *      speed       -   fan speed of target fan
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
INT32 gpio_halInit
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halGetVal
 *
 *  DESCRIPTION :
 *      Get GPIO data in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
 *
 *  OUTPUT :
 *      *val       -   GPIO value
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
INT32 gpio_halGetVal
(
    UINT32 pin,
    UINT8  *val
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halSetVal
 *
 *  DESCRIPTION :
 *      Set GPIO data in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
 *      val        -   GPIO value
 *
 *  OUTPUT :
 *      ret
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
INT32 gpio_halSetVal
(
    UINT32 pin,
    UINT8  val
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpiotest
 *
 *  DESCRIPTION :
 *      Execute gpio teset in SYSFS
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      ret
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
INT32 gpiotest
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halSetDir
 *
 *  DESCRIPTION :
 *      Set GPIO direction in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
 *      dir        -   GPIO direction
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
INT32 gpio_halSetDir
(
    UINT32 gpioPin,
    UINT8  dir
);
#endif
