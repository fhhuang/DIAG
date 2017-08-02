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
***      flashtest.c
***
***    DESCRIPTION :
***      for flash memory test
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
#include "memtest.h"
#include "mem_hal.h"

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
INT32 do_memtest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 memAddr, dataSize, data;
    UINT32 startAddr;
    UINT32 endAddr;
    UINT32 pattern = 0;
    UINT32 test_size=MEM_TEST_SIZE;
    UINT32 free_size=0;
    E_MEM_TEST_TYPE memTestType = E_MEM_TEST_TYPE_MAX;
    UINT32 repeat=0;
    UINT32 defaultArgC;
    BOOL isPhy;

    if( argc == 1 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( strstr(argv[0], "memtest") )
    {
        if( strcmp(argv[1], "quick") == 0 )
        {
            defaultArgC = 2;
            if( argc == (defaultArgC+1) )
                repeat = simple_strtoul(argv[defaultArgC], NULL, 10);
            else if( argc == (defaultArgC+2) )
            {
                test_size =simple_strtoul(argv[defaultArgC], NULL, 10);
                repeat = simple_strtoul(argv[defaultArgC+1], NULL, 10);
            }
            else if( argc != defaultArgC )
            {
                ERR_PRINT_CMD_USAGE(argv[0]);
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }

            memTestType = E_MEM_TEST_TYPE_QUICK;
        }
        else if( strcmp(argv[1], "ram") == 0 )
        {
            defaultArgC = 3;
            if( argc == (defaultArgC+1) )
            {
                pattern = simple_strtoul(argv[(defaultArgC-1)], NULL, 16);
                repeat = simple_strtoul(argv[defaultArgC], NULL, 10);
            }
            else if( argc == (defaultArgC+2) )
            {
                test_size =simple_strtoul(argv[defaultArgC-1], NULL, 10);
                pattern = simple_strtoul(argv[(defaultArgC)], NULL, 16);
                repeat = simple_strtoul(argv[defaultArgC+1], NULL, 10);
            }
            else if( argc != defaultArgC )
            {
                ERR_PRINT_CMD_USAGE(argv[0]);
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }

            memTestType = E_MEM_TEST_TYPE_RAM;
        }
        else if( strcmp(argv[1], "marchc") == 0 )
        {
            defaultArgC = 2;
            if( argc == (defaultArgC+1) )
            {
                repeat = simple_strtoul(argv[defaultArgC], NULL, 10);
            }
            else if( argc == (defaultArgC+2) )
            {
                test_size =simple_strtoul(argv[defaultArgC], NULL, 10);
                repeat = simple_strtoul(argv[defaultArgC+1], NULL, 10);
            }
            else if( argc != defaultArgC )
            {
                ERR_PRINT_CMD_USAGE(argv[0]);
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }
            memTestType = E_MEM_TEST_TYPE_MARCHC;
        }
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        free_size = getFreeMem();
        if(free_size*1024 <= MEM_FREE_SIZE)
        {
            log_printf("Free memory is %u MB, it is not enough for testing\n", KB(free_size));
            ret = E_TYPE_ALLOC_MEM_FAIL;
            goto __CMD_ERROR;
        }

        if(test_size * 1024 > free_size)
        {
            log_printf("Input testsize larger than the current available free memory %u MB \n", KB(free_size));
            test_size = KB(free_size - KB(MEM_FREE_SIZE));
        }

        if(test_size > MB(MEM_FIX_SIZE))
        {
            log_printf("memory test size must less than 512MB.\n");
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

         test_size = test_size * 1024 *1024;

        startAddr = (UINT32)malloc(test_size);
        if(startAddr ==0)
        {
            log_printf("malloc %d bytes fail, not enough free memory size.\n",test_size);
            ret = E_TYPE_ALLOC_MEM_FAIL;
            goto __CMD_ERROR;
        }
        endAddr = startAddr+test_size;

        ret = memtest(memTestType, startAddr, endAddr, pattern, repeat);

        free((void*)startAddr);

        log_cmdPrintf(ret, "Memory Test\r\n");
    }
    else if( strstr(argv[0], "memread") )
    {
        if( argc != 4 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        memAddr = simple_strtoul(argv[1], NULL, 16);
        dataSize = simple_strtoul(argv[2], NULL, 10);
        if( strcmp(argv[3], "phy") == 0 )
            isPhy = TRUE;
        else if( strcmp(argv[3], "virt") == 0 )
            isPhy = FALSE;
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        ret = mem_halRead(memAddr, dataSize, isPhy, &data);
        log_printf("Read 0x%x = ", memAddr);
        log_printf("0x%x (%s address)\n", data, (isPhy==TRUE)?"Physical":"Virtual");
    }
    else if( strstr(argv[0], "memwrite") )
    {
        if( argc != 5 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        memAddr = simple_strtoul(argv[1], NULL, 16);
        dataSize = simple_strtoul(argv[2], NULL, 10);
        data = simple_strtoul(argv[3], NULL, 16);
        if( strcmp(argv[4], "phy") == 0 )
            isPhy = TRUE;
        else if( strcmp(argv[4], "virt") == 0 )
            isPhy = FALSE;
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        ret = mem_halWrite(memAddr, dataSize, isPhy, data);
        log_printf("Write 0x%x = ", memAddr);
        log_printf("0x%x (%s address)\n", data, (isPhy==TRUE)?"Physical":"Virtual");
    }
    return ret;

__CMD_ERROR:

    return ret;
}


U_BOOT_CMD(
     memtest,    CONFIG_SYS_MAXARGS,    1,    do_memtest,
     "memtest \t- To perform memory test\n",
     "<quick |ram <pattern>|marchc> <size> <iterations>\n"
    "  - method: Specify the method to test\n"
    "  - pattern: A 32-bit hexadecimal test pattern\n"
    "  - size: Specify the memory size in megabytes to be tested\n"
    "  - iterations: Specify the number of iterations to run this test\n"
);

U_BOOT_CMD(
     memread,    CONFIG_SYS_MAXARGS,    1,    do_memtest,
     "memread \t- To perform memory read\n",
     "<address> <size> <is physical>\n"
    "  - perform dram memory read\n"
    "    address: Specify the address offset.\n"
    "    size: Specify the data size to read. Valid values are <8|16|32> bits.\n"
    "    physical address: The address is physical address. Valid values are <phy|virt>\n"
);

U_BOOT_CMD(
     memwrite,    CONFIG_SYS_MAXARGS,    1,    do_memtest,
     "memwrite \t- To perform memory write\n",
     "<address> <size> <hex_data> <is physical>\n"
    "  - perform dram memory write\n"
    "    address: Specify the address offset.\n"
    "    size: Specify the data size to read. Valid values are <8|16|32> bits.\n"
    "    hex_data: Specify the register value in hexadecimal format.\n"
    "    physical address: The address is physical address. Valid values are <phy|virt>\n"
);

