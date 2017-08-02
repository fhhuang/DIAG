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
***      cmd_intrtest.c
***
***    DESCRIPTION :
***      for interrupt test
***
***    HISTORY :
***       - 2009/05/20, 10:30:52, Eden Weng
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *                                                                          
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
#ifndef FOX_KERNEL
#include "exports.h"
#include "config.h"
#include "common.h"
#include "command.h"
#else
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <sys/ioctl.h>
#ifndef INDEPENDENCE_PROC
#include "foxCommand.h"
#endif
#endif

#include "cmn_type.h"
#include "porting.h"
#include "sys_utils.h"
#include "intr_hal.h"
#include "intrtest.h"
#include "err_type.h"
#include "log.h"
/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
static const INT8 *                 intrName[E_EXT_INTR_MAX] = { "min", "sfp_present", "sfp_rxloss", "sfp_txfault", "sb_intr", "mcu_intr", "push_button", "max"};

static const INT8 *                 intrTxt[E_EXT_INTR_MAX] = {  "min", "SFP Present", "SFP Rx_Loss", "SFP Tx_Fault",
                                                                 "Secure Boot", "MCU", "Push Button", "max"};

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
INT32 do_intrtest
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    BOOL                            intrFound = FALSE;
    INT32                           ret = E_TYPE_SUCCESS, rcode = E_TYPE_SUCCESS;
    S_BOARD_INFO                    boardInfo;
    UINT32                          itemMap = 0;
    E_EXT_INTR                      extIntr;
    INT32                            timeout;
    
    sys_utilsBoardInfoGet(&boardInfo);

    if (strcmp("intrtest", argv[0]) == 0)
    {
        if (argc != 2 && argc != 3) 
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if ( strcmp("all", argv[1]) == 0 )
        {
            for (extIntr = E_EXT_INTR_MIN ; extIntr < E_EXT_INTR_MAX ; extIntr++)
            {
                itemMap |= 0x1 << extIntr;
            }
        }
        else
        {
            for (extIntr = E_EXT_INTR_MIN ; extIntr < E_EXT_INTR_MAX ; extIntr++)
            {
                if (strncmp(intrName[extIntr], argv[1], 4) == 0)
                {
                    itemMap |= 0x1 << extIntr;
                    intrFound = TRUE;
                    break;
                }
            }

            if (!intrFound)
            {
                log_printf("Unsupport parameter\n");
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }
            
        }

        for (extIntr = E_EXT_INTR_MIN ; extIntr < E_EXT_INTR_MAX; extIntr ++)
        {
            if (!(itemMap & (0x1 << extIntr)))
                continue;

            if ( argv[2] != NULL )
                timeout = simple_strtoul(argv[2], NULL, 10);
            else
                timeout = 5;
            
            rcode = intrtest(extIntr, timeout);

            if (rcode == E_TYPE_INT_NO_SUPPORT)
            {
                /* Should not execute here, the unsupport interrupt should be filtered */
                log_printf("%s test is not supported\n", intrTxt[extIntr]);
            }
            else if (rcode != E_TYPE_SUCCESS)
            {
                log_printf("%s test - FAIL\n", intrTxt[extIntr]);
                ret = rcode;
            }
            else    
                log_printf("%s test - PASS\n", intrTxt[extIntr]);
        }
    }
    else
    {
        log_printf("Unsupport cmd type\n");
        ret = E_TYPE_INVALID_CMD;
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;

}

U_BOOT_CMD(
     intrtest,    CONFIG_SYS_MAXARGS,    1,    do_intrtest,
     "intrtest \t- This test interrupt pins from devices to CPU\n",
     "<int_name> <timeout>\n"
     "  - This test interrupt pins from devices to CPU\n"
     "    int_name : Valid values are <all |sfp_present   \n"
     "              mcu_intr | push_button>.\n" 
     "    timeout: default is 5s\n"
     "    Use \"all\" for testing all interrupts.\n"
    );
