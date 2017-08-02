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
***      cmd_volmonitor.c
***
***    DESCRIPTION :
***      for voltage monitor
***
***
***    HISTORY :
***       - 2011/10/20, 10:50:52, Eric Hsu
***             File Creation
***
***       - 2012/11/09, 16:50:00, Wed Chen
***             Revision
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */

#include "foxCommand.h"
#include "porting.h"
#include "cmn_type.h"
#include "mcu_hal.h"

#include "err_type.h"
#include "log.h"
#include "rtctest.h"

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
 UINT8 Week[7][4]   = {"Mon\0", "Tue\0", "Wed\0", "Thu\0", "Fri\0", "Sat\0", "Sun\0"};
 UINT8 Month[12][4] = {"Jan\0", "Feb\0", "Mar\0", "Apr\0", "May\0", "Jun\0", "Jul\0", "Aug\0", "Sep\0", "Oct\0", "Nov\0", "Dec\0"};

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
INT32 do_rtctest
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;

    if ( argc != 1 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    log_printf("RTC Test - \n");
    ret = rtctest();

    if ( ret != E_TYPE_SUCCESS )
    {
        log_cmdPrintf(E_LOG_MSG_FAIL, "RTC Test\r\n");
        goto __CMD_ERROR;
    }

    log_cmdPrintf(E_LOG_MSG_PASS, "RTC Test\r\n");

__CMD_ERROR:
    return ret;
}

INT32 do_rtcread
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 read_data;

    if ( argc != 2 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if (!strcasecmp(argv[1], "date"))
    {
        ret = mcu_halDataSet(MCU_GET_DATE, 0, 0);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU RTC info.\n");
            goto __CMD_ERROR;
        }

        ret = mcu_halDataGet(0x1, 0, &read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU date.\n");
            goto __CMD_ERROR;
        }

        if (((read_data >> 8) & 0xFF) > 0x9)
            read_data -= 0x600;
        
        log_printf("%s %s %02X 20%02X\n", Week[((read_data >> 24) & 0xFF) - 1], Month[((read_data >> 8) & 0xFF) - 1], \
                   read_data & 0xFF, (read_data >> 16) & 0xFF);
    }
    else if (!strcasecmp(argv[1], "time"))
    {
        ret = mcu_halDataSet(MCU_GET_TIME, 0, 0);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU RTC info.\n");
            goto __CMD_ERROR;
        }

        ret = mcu_halDataGet(0x1, 0, &read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU time.\n");
            goto __CMD_ERROR;
        }
        log_printf("%02X:%02X:%02X\n", (read_data >> 16) & 0xFF, (read_data >> 8) & 0xFF, read_data & 0xFF);
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;
}

INT32 do_rtcwrite
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 value = 0, valid = 0;

    if ( argc != 3  )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    value = simple_strtoul(argv[2], NULL, 16);

    //Check the parameters
    if (!strcasecmp(argv[1], "date") && (atoi(argv[2]) >= 1000000))
    {
        //BCD: Weekday: 0x1-0x7, Year: 0x0-0x99, Month: 0x1-0x12, Day: 0x1-0x31
        if (((value & 0xFF) <= 0x31) && ((value & 0xFF) >= 0x01))
            if ((((value >> 8) & 0xFF) <= 0x12) && (((value >> 8) & 0xFF) >= 0x1))
                if ((((value >> 16) & 0xFF) <= 0x99) && (((value >> 16) & 0xFF) >= 0x0))
                    if ((((value >> 24) & 0xFF) <= 0x7) && (((value >> 24) & 0xFF) >= 0x1))
                        valid = 1;
        if (!valid)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            return ret = E_TYPE_INVALID_CMD_FORMAT;
        }

        ret = mcu_halDataSet(MCU_SET_DATE, 0, value);
    }
    else if (!strcasecmp(argv[1], "time") && (atoi(argv[2]) >= 10000))
    {
        //BCD: Hour: 0x0-0x23, Minute: 0x0-0x59, Second: 0x0-0x59
        if (((value & 0xFF) <= 0x59) && (value & 0xFF) >= 0x00)
            if ((((value >> 8) & 0xFF) <= 0x59) && (((value >> 8) & 0xFF) >= 0x00))
                if ((((value >> 16) & 0xFF) <= 0x23) && (((value >> 16) & 0xFF) >= 0x00))
                    valid = 1;

        if (!valid)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            return ret = E_TYPE_INVALID_CMD_FORMAT;
        }

        ret = mcu_halDataSet(MCU_SET_TIME, 0, value);
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( ret != E_TYPE_SUCCESS )
        log_printf("Failed to set RTC information.\n");
    else
        log_printf("Succeed to set RTC information.\n");

__CMD_ERROR:
    return ret;
}
U_BOOT_CMD(
     rtctest,    CONFIG_SYS_MAXARGS,    1,    do_rtctest,
    "rtctest \t- test the rtc function\n",
    " - test the rtc function\n"
);

U_BOOT_CMD(
     rtcread,    CONFIG_SYS_MAXARGS,    1,    do_rtcread,
    "rtcread \t- read the date/time information from MCU RTC\n",
    "<info>\n"
    " - info: RTC information, Valid values are <date|time>\n"
);

U_BOOT_CMD(
     rtcwrite,    CONFIG_SYS_MAXARGS,    1,    do_rtcwrite,
    "rtcwrite \t- write the date/time information to MCU RTC\n",
    "<info> <value>\n"
    " - info : RTC information, Valid values are <date|time>\n"
    " - value: Set the date or time, the format are listed below,\n"
    "          For date format, [wwyymmdd]\n"
    "          For time format, [hhmmss]\n"
    "          (ww: weekday, value is 01-07)\n"
    "          (yy: year,    value is 00-99)\n"
    "          (mm: month,   value is 01-12)\n"
    "          (dd: day,     value is 01-31)\n"
    "          (hh: hour,    value is 00-23)\n"
    "          (mm: minute,  value is 00-59)\n"
    "          (ss: second,  value is 00-59)\n"
);