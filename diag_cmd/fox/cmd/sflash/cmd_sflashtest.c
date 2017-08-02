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
***      cmd_sflashtest.c
***
***    DESCRIPTION :
***      for SPI device test
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
#include "sflashtest.h"
#include "sflash_hal.h"

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
INT32 do_sflashtest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 bankNum;
    UINT32 firstBlock=1;
    UINT32 blockCount=0;
    UINT32 repeat=1;
    UINT32 i=1;
    
    switch (argc)
    {
        case 5:
            bankNum = simple_strtoul(argv[1], NULL, 10);
            if( (bankNum >CONFIG_SYS_MAX_SFLASH_DEVICE) || !bankNum)
            {
                ERR_PRINT_CMD_USAGE(argv[0]);
                ret = E_TYPE_OUT_RANGE;
                goto __CMD_ERROR;
            }

            firstBlock = simple_strtoul(argv[2], NULL, 10);
            blockCount = simple_strtoul(argv[3], NULL, 10);
            if( strcmp(argv[4], "loop") == 0 )
            {
                repeat = 0;
            }
            else
            {
                repeat = simple_strtoul(argv[4], NULL, 10);
            }
            break;

        default:
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
    }

    do
    {
        ret=sflashtest(bankNum, firstBlock, blockCount);
        log_printf("SPI flash test (%u) - ", i);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("FAIL\n\n");
            goto __CMD_ERROR;
        }

        log_printf("PASS\n\n");
#if 0
        if( ctrlc() )
        {
            break;
        }
#endif
        i++;

        if( repeat == 0 )
        {
            continue;
        }

        if( i > repeat )
        {
            break;
        }
    } while(1);

    return 0;
__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
     sflashtest,    CONFIG_SYS_MAXARGS,    1,    do_sflashtest,
     "sflashtest \t- To perform SFLASH test\n",
     "<bank> <block> <cnt> <repeat>\n"
     "  - perform SPI flash test\n"
     "    bank: specify the bank of SPI flash to test. Valid values are <1>\n"
     "    block: the first block# for the test\n"
     "    cnt: the number of blocks to test (0 means from first block# to last block.)\n"
     "    repeat: repeat the SFLASH tests. (0 or \"loop\" is infinite test.)\n"
);
