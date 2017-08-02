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
***      cmd_tempmonitor.c
***
***    DESCRIPTION :
***      for temperature monitor
***
***    HISTORY :
***       - 2009/05/11, 17:30:52, Eden Weng
***             File Creation
***
***       - 2013/11/01, Wed Chen
***             Modified to monitor 2 temperature inputs
***             <MAC|PHY>
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
#include "tempmonitor.h"

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
INT32 do_tempread
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS, source, alarm[3] = {95, 95, 75};
    UINT8 temp_int, temp_decimal, sign_flag, temp[3][4]= {"MAC\0", "PHY\0", "DDR\0"};

    if ( (argc != 2) || (strcasecmp(argv[1], "all") != 0 ))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    for (source = MCU_GET_MAC_TEMP; source <= MCU_GET_DDR_TEMP; source++)
    {
        ret = temp_read(source, &temp_int, &temp_decimal, &sign_flag);

        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Fail to get temperature \n");
            goto __CMD_ERROR;
        }

        log_printf("%s Temperature: %c%02d.%02d (%s)\n", temp[source - MCU_GET_MAC_TEMP], (sign_flag)?'-':'+', temp_int, temp_decimal,\
            ((sign_flag) ? ((temp_int > 5) ? "Out of Limit" : "Normal"):((temp_int > alarm[source - MCU_GET_MAC_TEMP])? "Out of Limit" : "Normal")));
    }

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    tempread,    CONFIG_SYS_MAXARGS,    1,    do_tempread,
    "tempread \t- To read temperature\n",
    "<source>\n"
    "  - Read all temperature\n"
    "    source: Specify the temperature source to read. Valid values are <all>\n"
);
