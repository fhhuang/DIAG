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
***      cmd_gpiotest.c
***
***    DESCRIPTION :
***      for gpio test
***
***    HISTORY :
***       - 2009/05/06, 16:30:52, Eden Weng
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
#include "gpio_hal.h"
#include "err_type.h"
#include "log.h"


INT32 do_gpiotest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT8 pinStatus = 0;
    BOOLEAN doWrite = FALSE;
    int pin;

    if( strcmp("gpioread", argv[0]) == 0 )
    {
        doWrite = FALSE;
    }
    else if( strcmp("gpiowrite", argv[0]) == 0 )
    {
        doWrite = TRUE;
    }
    else if(strcmp("gpioinit", argv[0]) == 0)
    {
        return gpio_halInit();
    }
    else if(strcmp("gpioreset", argv[0]) == 0)
    {
        return gpio_halReset();
    }
    else if(strcmp("gpiodefault", argv[0]) == 0)
    {
        gpio_halGetVal(13, &pinStatus);
        log_printf("factory Button is %d \n",pinStatus);
        return ret;
    }
    else if(strcmp("gpiotest", argv[0]) == 0)
    {
        ret=gpiotest();

        if(ret)
        {
            log_printf("GPIO test fail %d \n",ret);
        }
        return ret;
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( argc != (doWrite?(3):(2)) )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    pin = simple_strtoul(argv[1], NULL, 10);

    if(doWrite)
    {
        pinStatus = simple_strtoul(argv[2], NULL, 16);
    }

    if(doWrite)
    {
        ret = gpio_halSetVal(pin, pinStatus);
    }
    else
    {
        ret = gpio_halGetVal(pin, &pinStatus);
    }

    log_printf("%s Pin%d - ", doWrite?"Writing":"Reading", pin);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail\n");
        goto __CMD_ERROR;
    }

    log_printf("0x%x\n", pinStatus);

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    gpioread,    CONFIG_SYS_MAXARGS,    1,    do_gpiotest,
    "gpioread \t- To read GPIO pin status\n",
    "gpioread <pin> \n"
    "  - Read pin status\n"
    "     pin: Specify gpio pin. Valid values are 5~13, 17 and 18.\n"
);

U_BOOT_CMD(
    gpiowrite,    CONFIG_SYS_MAXARGS,    1,    do_gpiotest,
    "gpiowrite \t- To write gpio pin status\n",
    "gpiowrite <pin> <data>\n"
    "  - Write pin status\n"
    "     pin: Specify gpio pin. Valid values are 5~13, 17 and 18.\n"
    "     data: 1-byte value in hexadecimal to write\n"
);

U_BOOT_CMD(
    gpioinit,    CONFIG_SYS_MAXARGS,    1,    do_gpiotest,
    "gpioinit \t- To init GPIO\n",
    "  - To verify gpio\n"
);

U_BOOT_CMD(
    gpiotest,    CONFIG_SYS_MAXARGS,    1,    do_gpiotest,
    "gpiotest \t- To test used by GPIO pin\n",
    "  - To chip reset\n"
);

U_BOOT_CMD(
    gpioreset,    CONFIG_SYS_MAXARGS,    1,    do_gpiotest,
    "gpioreset \t- To reset used by GPIO pin\n",
    "  - To chip reset\n"
);

U_BOOT_CMD(
    gpiodefault,    CONFIG_SYS_MAXARGS,    1,    do_gpiotest,
    "gpiodefault \t- To read default button\n",
    "  - To chip reset\n"
);

