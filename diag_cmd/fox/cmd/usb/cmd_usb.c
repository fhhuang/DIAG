/***************************************************************************
***
***    Copyright 2014  Hon Hai Precision Ind. Co. Ltd.
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
***      cmd_usbtest.c
***
***    DESCRIPTION :
***      for USB verification
***
***    HISTORY :
***       - 2014/03/05, 14:30:52, Wed Chen
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
#include "usb_hal.h"
#include "porting.h"
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
INT32 do_usb
(
    IN  cmd_tbl_t                   *cmdtp,
    IN  INT32                       flag,
    IN  INT32                       argc,
    IN  INT8                        *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 testPort=0;
    UINT16 en=0;

    if(strcmp(argv[0],"usb"))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
    else
    {
        if(argc<2)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }

    if(!strcmp(argv[1],"probe"))
    {
        ret = usb_halProbeTest();
        log_cmdPrintf(ret, "USB probe test\r\n");
    }
    else if(!strcmp(argv[1],"connect"))
    {
        if(argv[2] == NULL)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        testPort=simple_strtoul(argv[2], NULL, 10);
        ret = usb_halConnectTest(testPort);
        log_cmdPrintf(ret, "USB connect test\r\n");
    }
    else if(!strcmp(argv[1],"test"))
    {
        log_printf("Start USB test, please make sure USB storage is plugged-in already.\r\n");

        /* For only one USB port. Fix port 0 for manufacture test. */
        ret = usb_halConnectTest(0);
        log_cmdPrintf(ret, "USB test (read/write)\r\n");  
    }
    else if(!strcmp(argv[1],"enable"))
    {
        if(argv[2] == NULL)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        
        en=simple_strtoul(argv[2], NULL, 10);
        if( en != 0)
        {
            usb_halPortEnable(1);
            log_printf("Enable usb port.\r\n");
        }
        else
        {
            usb_halPortEnable(0);
            log_printf("disable usb port.\r\n");
        }
    }

    return ret;
__CMD_ERROR:
    log_printf("USB command FAIL\n");
    return ret;
}

INT32 do_mini_usb
(
    IN  cmd_tbl_t                   *cmdtp,
    IN  INT32                       flag,
    IN  INT32                       argc,
    IN  INT8                        *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;

    if(strcmp(argv[0],"miniusbtest"))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
    else
    {
        if(argc<1)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }

    log_printf("Start USB test, please make sure USB storage is plugged-in already.\r\n");
    log_printf("Warnning !!!, dunring the mini usb test , console may not respond around 20 seconds.\r\n");

    /* For only one USB port. Fix port 0 for manufacture test. */
    ret = usb_halMiniUsbLbTest();
    
    udelay(2000000);
    
__CMD_ERROR:
    if( ret != E_TYPE_SUCCESS )
        log_cmdPrintf(E_LOG_MSG_FAIL, "mini USB loopback test\r\n");     
    else
        log_cmdPrintf(E_LOG_MSG_PASS, "mini USB loopback test\r\n");
    return ret;
}

/* 
 * Function: do_mini_usb_detect
 * In PP1/PP2 build H/W changed miniUSB detect in GPIO18
 * Date: 07282016
 */
INT32 do_mini_usb_detect
(
    IN  cmd_tbl_t                   *cmdtp,
    IN  INT32                       flag,
    IN  INT32                       argc,
    IN  INT8                        *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;

    if(strcmp(argv[0],"miniusbdetect"))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
    else
    {
        if(argc<1)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }

    log_printf("Start USB detection test, please make sure USB loopback cable is plugged-in already.\r\n");
    log_printf("Warnning !!!, dunring the mini usb detect test , console may not respond around 10 seconds.\r\n");

    /* For only check USB detection pin in GPIO 18 is actived low. */
    ret = usb_halMiniUsbDetectTest();
    
    udelay(1000000);
    
__CMD_ERROR:
    if( ret != E_TYPE_SUCCESS )
        log_cmdPrintf(E_LOG_MSG_FAIL, "mini USB detection test\r\n");     
    else
        log_cmdPrintf(E_LOG_MSG_PASS, "mini USB detection test\r\n");
    return ret;
}

/* 
 * Function: do_mini_usb_led
 * In Pilot Build, enhance LED test
 * Date: 10172016
 */
INT32 do_mini_usb_led
(
    IN  cmd_tbl_t                   *cmdtp,
    IN  INT32                       flag,
    IN  INT32                       argc,
    IN  INT8                        *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    INT32 timeout = 30; 
    
    if(strcmp(argv[0],"miniusbled"))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
    else
    {
        if(argc<1)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }

    /* check led time out, default are 30 seconnds */
    if (argc >= 2) {
        if (argv[1]!= NULL)
            timeout = simple_strtoul(argv[1], NULL, 10);
    }
                
    log_printf("Start USB LED test.\r\n");
    log_printf("Warnning !!!, dunring the mini usb LED test , console may not respond around %d seconds.\r\n", timeout);

    /* Test miniUSB LED. */
    ret = usb_halMiniUsbLED(timeout);
    
    udelay(1000000);
    
__CMD_ERROR:
    if( ret != E_TYPE_SUCCESS )
        log_cmdPrintf(E_LOG_MSG_FAIL, "mini USB LED test\r\n");     
    else
        log_cmdPrintf(E_LOG_MSG_PASS, "mini USB LED test\r\n");
    return ret;
}

U_BOOT_CMD(
    usb,    CONFIG_SYS_MAXARGS,    1,    do_usb,
    "usb\t\t- USB test and utility\n",
    "<test | probe | connect <usb port> | enable <en>\n"
    "  - test: write/read USB storage\n"
    "  - probe: probe USB devices\n"
    "  - connect: connect USB device on a given USB port\n"
    "  - enable: 1 or 0 for enable and disable port \n"
);

U_BOOT_CMD(
    miniusbtest,    CONFIG_SYS_MAXARGS,   1,    do_mini_usb,
    "miniusbtest\t- mini USB console test\n", 
    " -validate the usb interface to the mini usb \n"
);

U_BOOT_CMD(
    miniusbdetect,    CONFIG_SYS_MAXARGS,   1,    do_mini_usb_detect,
    "miniusbdetect\t- mini USB console detect in gpio18\n", 
    " -validate the mini usb detect in gpio18 \n"
);

U_BOOT_CMD(
    miniusbled,    CONFIG_SYS_MAXARGS,   1,    do_mini_usb_led,
    "miniusbled\t- mini USB LED test\n", 
    "<timeout>\n"
    " timeout: default is 30s\n"
    " -validate the min USB LED \n"
);
