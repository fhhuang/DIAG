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
***      cmd_fantest.c
***
***    DESCRIPTION :
***      for fan test
***
***    HISTORY :
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include "foxCommand.h"
#include "cmn_type.h"
#include "porting.h"
#include "fan_hal.h"
#include "err_type.h"
#include "log.h"
#include "sys_utils.h"

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

INT32 do_fantest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32       ret=E_TYPE_SUCCESS;
    UINT32      fan_duty;
    E_BOARD_ID  boardId = sys_utilsDevBoardIdGet();

    if( boardId != E_BOARD_ID_HAYWARDS_48G4G_P )
    {
        log_printf("This SKU doesn't support FAN read/write commands.\n");
        goto __CMD_ERROR;
    }

    if(!strcasecmp(argv[0], "fanwrite") )
    {
        if (argc != 2)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        fan_duty = simple_strtoul(argv[1], NULL, 10);

        if (fan_duty > 0 && fan_duty < 100)
        {
            fan_duty = 100 - fan_duty;
            ret = fan_halSetSpeed(fan_duty);
            if(ret!=E_TYPE_SUCCESS)
            {
                log_printf("Failed to set the duty cycle percentage of Fan.\n");
                goto __CMD_ERROR;
            }
            log_printf("Succeed to set FAN %d%% duty cycle.\n", 100 - fan_duty);
        }
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }
    else if(!strcasecmp(argv[0], "fanread") )
    {
        ret = fan_halGetSpeed(&fan_duty);
        if(ret!=E_TYPE_SUCCESS)
        {
            log_printf("Failed to get the duty cycle percentage of FAN.\n");
            goto __CMD_ERROR;
        }
        log_printf("The duty cycle percentage of FAN is %d%%.\n", 100 - fan_duty);
    }

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    fanwrite,    CONFIG_SYS_MAXARGS,    1,    do_fantest,
    "fanwrite \t- To configure the duty cycle percentage of FAN\n",
    "<percent>\n"
    "    percent: Specify the duty cycle percentage of fan. Valid values are <1-99, from slow to fast>\n"
);

U_BOOT_CMD(
    fanread,    CONFIG_SYS_MAXARGS,    1,    do_fantest,
    "fanread \t- To read the duty cycle percentage of FAN\n",
    "\n"
);