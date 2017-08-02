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
***			led_hal.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2015/12/22, 10:02:52, Eric Hsu
***             File Creation
***
***************************************************************************/

#ifndef __LED_HAL_H_
#define __LED_HAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cmn_type.h"
#include "port_utils.h"
#include "err_type.h"
#include "log.h"
#include "porting.h"
/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define MAC_DEV_GET(x)  (x.devId)
#define MAX_RAND_NUM    6
/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef struct
{
    UINT32 pexNum;      /* pex number */
    UINT32 portNum;     /* port number as defined in uboot code */
    INT32  fpPortNum;   /* front panel port */
    UINT32 phyAddr;     /* phy address */
    UINT32 smiBus;      /* smi bus number */
    UINT32 poeLedNum;   /* led number in phy to indicate POE */
}GT_CurlCat_PORT_DATA;

typedef struct
{
    UINT32 pexNum;      /* pex number */
    UINT32 poePort;     /* port number as defined in uboot code */
    INT32  fpPortNum;   /* front panel port */
    UINT32 phyAddr;     /* phy address */
    UINT32 smiBus;      /* smi bus number */
    UINT32 poeLedNum;   /* led number in phy to indicate POE */
}GT_CurlCat_POE_PORT_DATA;


typedef enum {
    LED_TEST_MANUAL,
    LED_TEST_AUTO,
}E_LED_TEST_MODE;

typedef enum {
    LED_TYPE_SYS = 0x1,
    LED_TYPE_SFP = 0x2,
    LED_TYPE_COPPER_PORT = 0x4,
    LED_TYPE_ALL = 0x7,
}E_LED_TYPE;

typedef enum {
    LED_TYPE_OFF,
    LED_TYPE_GREEN,
    LED_TYPE_YELLOW, 
    LED_TYPE_AMBER
}E_LED_COLOR;

typedef enum {
    LED_TYPE_SFP_PORT_1 = 0x1,
    LED_TYPE_SFP_PORT_2 = 0x2,
    LED_TYPE_SFP_PORT_3 = 0x3,
    LED_TYPE_SFP_PORT_4 = 0x4,
}E_LED_SFP_PORT;

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      led_halSet
 *
 *  DESCRIPTION :
 *      config the led behavior into MCU 
 *
 *  INPUT :
 *      port_num    -   0: system led, 1-4 maps to SFP 1-4
 *      color    -      0: off, 1:green, 2:amber
 *      blink    -      0: no blink, 1:blink
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
INT32 led_halSet
(
  IN UINT8 port_num, 
  IN E_LED_COLOR color, 
  IN BOOL blink
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      led_halInit
 *
 *  DESCRIPTION :
 *      config the default led behavior
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
INT32 led_halInit
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* __LED_HAL_H_ */
