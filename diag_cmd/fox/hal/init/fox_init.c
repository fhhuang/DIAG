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
***      fox_init.c
***
***    DESCRIPTION :
***      for module init
***
***    HISTORY :
***       - 2009/07/06, 10:30:52, Eden Weng
***             File Creation
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
#include "fox_init.h"

#include "port_utils.h"
#include "sys_utils.h"
#include "poe_hal.h"
#include "gpio_hal.h"
#include "err_type.h"
#include "log.h"
#include "i2c_hal.h"
#include "switch_hal.h"
#include "tempmonitor.h"
#include "fan_hal.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Structrue segmentn
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
INT32 fox_init
(
    void
)
{
    INT32       ret = 0;
    UINT32      intrData = 0;
    E_BOARD_ID  boardId;
    char    *env = NULL;
    UINT32  ppInitEnable = 0;
    UINT8   u8SetData;

    /* Initialize the board information */
    sys_utilsInit();

    /* Initialize the port information */
    port_utilsInit();

    boardId = sys_utilsDevBoardIdGet();

    /* Initialize GPIO */
    log_printf("Initialize GPIO ... \r\n");
    gpio_halInit();
    udelay(1000);

    /* QSGMII parameter fine tune */
    ret = switch_halQsgmiiInit();
    log_printf("QSGMII parameter fine tune %s \r\n", (ret==E_TYPE_SUCCESS)?"Done":"Fail");
    
    /* turn on SYS LED */
    led_halInit();

    /* Initialize Voltage */
    if( (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) ||
        (boardId == E_BOARD_ID_HAYWARDS_16G2G_P) ||
        (boardId == E_BOARD_ID_HAYWARDS_24G4G_P) ||
        (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        log_printf("Initialize PoE ... \r\n");
        if ( poe_halInit() != E_TYPE_SUCCESS)
        {
            log_printf("FAIL!\n");
        }
    }

    switch_halSpawnSfpLedTask();
    udelay(1000);

    return ret;
}

