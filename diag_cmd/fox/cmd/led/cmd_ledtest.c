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
***      cmd_ledtest.c
***
***    DESCRIPTION :
***      for led test
***
***    HISTORY :
***       - 2015/11/18, 14:43:52, Eric Hsu
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
#include "foxCommand.h"
#include "porting.h"
#include "sys_utils.h"
#include "port_utils.h"
#include "err_type.h"
#include "log.h"
#include "led_hal.h"

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
INT32 do_ledtest
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32 ret = 0, repeat=1;
    E_LED_TEST_MODE mode;

    if ( strcmp("ledtest", argv[0]) == 0 )
    {
        if ( (argc != 1) && (argc != 2) )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if ( argc == 2 || argc == 3)
        {
            if ( strcmp("auto", argv[1]) == 0 )
                mode = LED_TEST_AUTO;
            else if ( strcmp("manual", argv[1]) == 0 )
                mode = LED_TEST_MANUAL;
            else
            {
                log_printf("Unknown mode, default uses manual mode\n");
            }

            if ( argv[2] == NULL )
                repeat =1;
            else
                repeat = simple_strtoul(argv[2], NULL, 10);     
        }
       

        log_printf("LED %s Test\n", mode?"Auto": "Maual");
        ret = ledtest(mode);

        if( ret != E_TYPE_SUCCESS )
        {
            log_cmdPrintf(E_LOG_MSG_FAIL, "LED Test - FAIL\r\n");
        }
        else
        {
            log_cmdPrintf(E_LOG_MSG_PASS, "LED Test - PASS\r\n");
        }
    }
    else
    {
        ret = E_TYPE_INVALID_CMD;
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;
}

INT32 do_led_cmd
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = 0;

    if ( argc != 2 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

   if ( strcmp("off", argv[1]) == 0 )
    {
        if ( (ret = ledAllOff(LED_TYPE_ALL)) < 0 )
        {
            log_printf("led all off fail (ret=%d)\n", ret);
            ret = -2;
            goto __CMD_ERROR;
        }
    }
    else if ( strcmp("green", argv[1]) == 0 )
    {
        if ( (ret = ledChangeColor(LED_TYPE_ALL, LED_TYPE_GREEN)) < 0 )
        {
            log_printf("led all on fail (ret=%d)\n", ret);
            ret = -2;
            goto __CMD_ERROR;
        }
    }
    else if ( strcmp("amber", argv[1]) == 0 )
    {
        if ( (ret = ledChangeColor(LED_TYPE_ALL, LED_TYPE_AMBER)) < 0 )
        {
            log_printf("led all on fail (ret=%d)\n", ret);
            ret = -2;
            goto __CMD_ERROR;
        }
    }
    else if ( strcmp("reset", argv[1]) == 0 )
    {
        if( (ret = ledReset(LED_TYPE_ALL)) < 0 )
        {
            log_printf("led all reset fail (ret=%d)\n", ret);
            ret = -2;
            goto __CMD_ERROR;
        }
    }
    else
    {
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    ledtest,	CONFIG_SYS_MAXARGS,	1,	do_ledtest,
    "ledtest \t- This test validates the LED control\n",
    "<mode> \n"
    "<repeat> \n"
    "  - This test validates the LED control\n"
    "    mode : Specify the LED test mode. Valid values are <auto|manual|>. Where auto indicates the LEDs\n"
    "    will be test in auto mode, respectively, manual is manual mode. Default is manual mode\n"
    "    repeat: specify the repeat oounts\n"
);

U_BOOT_CMD(
    ledcmd,	CONFIG_SYS_MAXARGS,	1,	do_led_cmd,
        "ledcmd \t\t- This command execute the LED control\n",
        "<mode>\n"
        "  - This command execute the LED control\n"
        "    mode : Specify the LED mode. Valid values are <green|amber|off|reset>.\n"
        "    green: force all led to green\n"
        "    amber: force all led to amber\n"
        "    off: force all led off\n"
        "    reset: reset led to original default state\n"
);

