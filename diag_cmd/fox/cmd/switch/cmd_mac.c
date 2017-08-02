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
***      cmd_mac.c
***
***    DESCRIPTION :
***      for switch mac
***
***    HISTORY :
***       - 2009/05/20, 16:30:52, Eden Weng
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *                                                                          
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
#include "porting.h"
#include "foxCommand.h"
#include "cmn_type.h"
#include "err_type.h"
#include "log.h"

#include "switch_hal.h"

#include "port_utils.h"

#include "mac_test.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define DBG_PRINTF log_dbgPrintf

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
INT32 do_macrw
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = 0;
    UINT32  device, offset, data = 0;
    UINT32  repeat;

    if ( strcmp("boardidtest", argv[0]) == 0 )
    {
        if ( argc != 1 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        ret = switch_halBoardIdTest();

        if ( ret != E_TYPE_SUCCESS )
        {
            log_cmdPrintf(E_LOG_MSG_FAIL, "Board ID test\r\n");
            goto __CMD_ERROR;
        }
        log_cmdPrintf(E_LOG_MSG_PASS, "Board ID test\r\n");
    }
    else if( strcmp("macread", argv[0]) == 0 )
    {
        if( argc != 3 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        device = simple_strtoul(argv[1], NULL, 16);
        offset = simple_strtoul(argv[2], NULL, 16);

        if ( (0 > device) || (device >= port_utilsTotalDevGet()) )
        {
            ret = E_TYPE_UNKNOWN_DEV;
            goto __CMD_ERROR;
        }

        if ( (ret = switch_halMACRegGet(device, offset, &data)) < 0 )
        {
            ret = E_TYPE_IO_ERROR;
            goto __CMD_ERROR;
        }

        log_printf("Read MAC CFG: Device=0x%x, Register 0x%x = 0x%08x\n", device, offset, data);
    }
    else if( strcmp("macwrite", argv[0]) == 0 )
    {
        if( argc != 4 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        device  = simple_strtoul(argv[1], NULL, 16);
        offset  = simple_strtoul(argv[2], NULL, 16);
        data    = simple_strtoul(argv[3], NULL, 16);

        if ( (0 > device) || (device >= port_utilsTotalDevGet()) )
        {
            ret = E_TYPE_UNKNOWN_DEV;
            goto __CMD_ERROR;
        }

        if( (ret=switch_halMACRegSet(device, offset, data)) < 0 )
        {
            ret = E_TYPE_IO_ERROR;
            goto __CMD_ERROR;
        }

        log_printf("Write MAC CFG: Device=0x%x, Register 0x%x = 0x%08x\n", device, offset, data);
    }

    return ret;

__CMD_ERROR:

    return ret;
}

U_BOOT_CMD(
     boardidtest,   CONFIG_SYS_MAXARGS,    1,  do_macrw,
     "boardidtest \t- Validates the boardId if match MAC chip ID.\n",
     "  - Validates the boardId if match MAC chip ID.\n"
);

U_BOOT_CMD(
 	macread,	CONFIG_SYS_MAXARGS,	1,	do_macrw,
 	"macread \t- Read MAC device configuration register.\n",
 	"<dev> <offset>\n"
 	"  - Read MAC device configuration register.\n"
 	"    dev    : Specify the device to read. Valid range is 0.\n"
	"    offset : Specify the register offset.\n"
);

U_BOOT_CMD(
 	macwrite,	CONFIG_SYS_MAXARGS,	1,	do_macrw,
 	"macwrite \t- Write MAC device configuration register.\n",
 	"<dev> <offset> <hex_data>\n"
 	"  - Write MAC device configuration register.\n"
 	"    dev      : Specify the device to read. Valid range is 0.\n"
 	"    offset   : Specify the register offset.\n"
	"    hex_data : Specify the register value in hexadecimal format.\n"
);
