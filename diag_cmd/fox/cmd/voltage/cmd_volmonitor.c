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
***            cmd_volmonintor.c
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/11, 10:02:52, Eden Weng
***             File Creation
***
***       - 2010/11/01, Wed Chen
***             Modified to monitor 2 voltage inputs
***             <2.5|3.3|all>
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
#include "volmonitor.h"

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
INT32 do_voltageread
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT8 vol_int, vol_decimal;
    E_VOL_SOURCE_TYPE source_type, i;
    float percent = 0.0;

    if( argc != 2)
    {
        log_printf("Wrong commands\n");
        ret = E_TYPE_INVALID_CMD;
        goto __CMD_ERROR;
    }

    if(strcasecmp(argv[1], "p3v3") == 0)
    {
        source_type = VOL_3_3_SOURCE;
    }
    else if(strcasecmp(argv[1], "p2v5") == 0)
    {
        source_type = VOL_2_5_SOURCE;
    }
    else if(strcasecmp(argv[1], "p1v8") == 0)
    {
        source_type = VOL_1_8_SOURCE;
    }
    else if(strcasecmp(argv[1], "p1v5") == 0)
    {
        source_type = VOL_1_5_SOURCE;
    }
    else if(strcasecmp(argv[1], "p0v99") == 0)
    {
        source_type = VOL_0_99_SOURCE;
    }
    else if(strcasecmp(argv[1], "p0v9") == 0)
    {
        source_type = VOL_0_9_SOURCE;
    }
    else if(strcasecmp(argv[1], "all") == 0)
    {
        source_type = VOL_ALL_SOURCE;
    }
    else
    {
       ret = E_TYPE_INVALID_CMD;
       goto __CMD_ERROR;
    }

    if( source_type != VOL_ALL_SOURCE )
    {
        if((ret = voltage_read(&vol_int, &vol_decimal, source_type)) != E_TYPE_SUCCESS )
            goto __CMD_ERROR;

        switch (source_type)
        {
              case 1:percent = -(3300 - (vol_int * 1000 + vol_decimal * 10)) / 3300.0 * 100;
                     log_printf(" 3.3V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 2:percent = -(2500 - (vol_int * 1000 + vol_decimal * 10)) / 2500.0 * 100;
                     log_printf(" 2.5V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 3:percent = -(1800 - (vol_int * 1000 + vol_decimal * 10)) / 1800.0 * 100;
                     log_printf(" 1.8V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 4:percent = -(1500 - (vol_int * 1000 + vol_decimal * 10)) / 1500.0 * 100;
                     log_printf(" 1.5V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 5:percent = -(990 - (vol_int * 1000 + vol_decimal * 10)) / 990.0 * 100;
                     log_printf(" 0.99V: %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 6:percent = -(900 - (vol_int * 1000 + vol_decimal * 10)) / 900.0 * 100;
                     log_printf(" 0.9V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
        }
    }
    else
    {
        for( i = VOL_3_3_SOURCE; i< VOL_ALL_SOURCE; i++ )
        {
            if((ret = voltage_read(&vol_int, &vol_decimal, i)) != E_TYPE_SUCCESS )
                goto __CMD_ERROR;

            switch (i)
            {
              case 1:percent = -(3300 - (vol_int * 1000 + vol_decimal * 10)) / 3300.0 * 100;
                     log_printf(" 3.3V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 2:percent = -(2500 - (vol_int * 1000 + vol_decimal * 10)) / 2500.0 * 100;
                     log_printf(" 2.5V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 3:percent = -(1800 - (vol_int * 1000 + vol_decimal * 10)) / 1800.0 * 100;
                     log_printf(" 1.8V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 4:percent = -(1500 - (vol_int * 1000 + vol_decimal * 10)) / 1500.0 * 100;
                     log_printf(" 1.5V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 5:percent = -(990 - (vol_int * 1000 + vol_decimal * 10)) / 990.0 * 100;
                     log_printf(" 0.99V: %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
              case 6:percent = -(900 - (vol_int * 1000 + vol_decimal * 10)) / 900.0 * 100;
                     log_printf(" 0.9V : %d.%02dV (%s)\n", vol_int, vol_decimal, (((percent > 10.0) || (percent < -10.0)) ? "OUT of Limit" : "Normal"));
                     break;
            }
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_voltagewrite
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT8 vol_level;
    E_VOL_SOURCE_TYPE source_type;

    if( argc != 3)
    {
        log_printf("Wrong commands\n");
        ret = E_TYPE_INVALID_CMD;
        goto __CMD_ERROR;
    }

    if(strcasecmp(argv[1], "p3v3") == 0)
    {
        source_type = VOL_3_3_SOURCE;
    }
    else if(strcasecmp(argv[1], "p1v8") == 0)
    {
        source_type = VOL_1_8_SOURCE;
    }
    else if(strcasecmp(argv[1], "p1v5") == 0)
    {
        source_type = VOL_1_5_SOURCE;
    }
    else if(strcasecmp(argv[1], "p0v9") == 0)
    {
        source_type = VOL_0_9_SOURCE;
    }
    else if(strcasecmp(argv[1], "all") == 0)
    {
        source_type = VOL_ALL_SOURCE;
    }
    else
    {
       ret = E_TYPE_INVALID_CMD;
       goto __CMD_ERROR;
    }

    if(strcasecmp(argv[2], "normal") == 0)
    {
        vol_level = VOL_NORMAL;
    }
    else if(strcasecmp(argv[2], "low") == 0)
    {
        vol_level = VOL_LOW;
    }
    else if(strcasecmp(argv[2], "high") == 0)
    {
        vol_level = VOL_HIGH;
    }
    else
    {
       ret = E_TYPE_INVALID_CMD;
       goto __CMD_ERROR;
    }

    if((ret = voltage_write(vol_level, source_type)) != E_TYPE_SUCCESS )
            goto __CMD_ERROR;


__CMD_ERROR:
    return ret;
}


U_BOOT_CMD(
     voltageread,    CONFIG_SYS_MAXARGS,    1,    do_voltageread,
    "voltageread \t- To read voltage source\n",
    "<source>\n"
    "  - Read voltage source\n"
    "    source: Specify the voltage source to read. Valid values are\n"
    "            <p3v3|p2v5|p1v8|p1v5|p0v99|p0v9|all>. Use \"all\" for all devices.\n"
);

U_BOOT_CMD(
     voltagewrite,    CONFIG_SYS_MAXARGS,    1,    do_voltagewrite,
    "voltagewrite \t- To write voltage source\n",
    "<source> <level>\n"
    "  - Write voltage source\n"
    "    source: Specify the voltage source to write. Valid values are\n"
    "            <p3v3|p1v8|p1v5|p0v9|all>. Use \"all\" for all devices.\n"
    "    level: Specify the voltage level. Valid values are <normal|low|high>.\n"
);
