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
***      cmd_nandtest.c
***
***    DESCRIPTION :
***      for nand device test
***
***************************************************************************/

/*==========================================================================
 *                                                                          
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include "foxCommand.h"
#include "porting.h"
#include "cmn_type.h"
#include "nandtest.h"
#include "nand_hal.h"

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

#ifndef CONFIG_SYS_MAX_NAND_DEVICE
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#endif


INT32 do_nandtest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
#ifdef NAND_BLOCK_TEST
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
            if( (bankNum >CONFIG_SYS_MAX_NAND_DEVICE) || !bankNum)
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
        ret=nandtest(bankNum, firstBlock, blockCount);
        
	if(repeat>1)
	    log_printf("NAND flash test by Loop (%u)\n", i);
        
	if( ret != E_TYPE_SUCCESS )
        {
	    log_cmdPrintf(E_LOG_MSG_FAIL, "NAND Test\r\n");		
            goto __CMD_ERROR;
        }

        log_cmdPrintf(E_LOG_MSG_PASS, "NAND Test\r\n");

#if 0 /* remove the ctrlc function */
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
#else
    /* while Diag image is builded as initramfs format and boot from Rommon. 
         Nand test should be changed to read/write file .
      */
    INT32 ret=E_TYPE_SUCCESS;
    INT32 fd, count=128;
    INT8   src[32]; 
    INT8   *trgt = "/mnt/flash";
    INT8   *type = "yaffs2";
    unsigned long mntflags = 0;
    INT32  result;
    INT8 *file_name="/mnt/flash/nand_test.txt";  /* Need to define this */
    fd_set readfs;    /* file descriptor set */
    struct timeval Timeout;
    INT8 i=0, j[2];
    UINT32 mtdBlockNum = 2;

    if(argc !=  2 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    mtdBlockNum = simple_strtoul(argv[1], NULL, 10);

    if( mtdBlockNum < 1 ) {
        ERR_PRINT_CMD_USAGE(argv[0]);
        log_printf("mtdBlock num can't less than 1 \n");
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
        

    sprintf(src, "/dev/mtdblock%d", mtdBlockNum);

    /* mount yaffs2 file system */
    result = mount(src, trgt, type, mntflags, NULL);

    /* in case the size of mout filesystem is too large. */
    udelay(3000000);

    if( result != 0 ) {
        log_printf("fail to mount yaffs2 \n");
        ret = -1;
        goto __CMD_ERROR;
    }    
    
    if ((fd = open (file_name, O_CREAT | O_RDWR)) < 0) {
        perror(file_name);
        ret = -1;
        goto __CMD_ERROR;
    }

    lseek(fd, 0, SEEK_SET);
    
    for (i=0; i <= count ; i++) {
        ret=write(fd,&i, 1);
        if (ret != 1) {
            log_printf("write error!\n");
            close(fd);
            ret = -1;
            goto __CMD_ERROR;
        }
    }

    sync();

    lseek(fd, 0, SEEK_SET);
    
    for (i=0; i <= count ; i++) {
        FD_SET(fd, &readfs);  /* set testing source */

        /* set timeout value within input loop */
        Timeout.tv_usec = 0;  /* milliseconds */
        Timeout.tv_sec  = 10;  /* seconds */
        ret = select(fd+1, &readfs, NULL, NULL, &Timeout);
        if (ret==0){
            log_printf("read timeout error!\n");
            close(fd);
            ret = -1;
            goto __CMD_ERROR;
        }
        else
            ret=read(fd, &j, 1);

        if (ret != 1) {
            log_printf("read error!\n");
            close(fd);
            ret = -1;
            goto __CMD_ERROR;
        }

        if (  i != j[0]  ) {
            log_printf("read data error: wrote 0x%x read 0x%x\n",i,j[0]);
            close(fd);
            ret = -1;
            goto __CMD_ERROR;
        }
    }

    log_printf("\n");

    close(fd);
    
    ret = remove(file_name);

    if (ret != 0) {
        log_printf("can't remove file \n");
        ret = -1;
        goto __CMD_ERROR;
    }
    
    ret = E_TYPE_SUCCESS;

__CMD_ERROR:
    if( ret != E_TYPE_SUCCESS )
        log_cmdPrintf(E_LOG_MSG_FAIL, "NAND Test\r\n");     
    else
        log_cmdPrintf(E_LOG_MSG_PASS, "NAND Test\r\n");

    umount(trgt);
    return ret;
#endif    
}

#ifdef NAND_BLOCK_TEST
U_BOOT_CMD(
     nandtest,    CONFIG_SYS_MAXARGS,    1,    do_nandtest,
     "nandtest \t- To perform NAND test\n",
     "<bank> <block> <cnt> <repeat>\n"
     "  - perform NAND flash test\n"
     "    bank: specify the bank of NAND flash to test. Valid values are <1>\n"
     "    block: the first block# for the test\n"
     "    cnt: the number of blocks to test (0 means from first block# to last block.)\n"
     "    repeat: repeat the NAND tests. (0 or \"loop\" is infinite test.)\n"
);
#else
U_BOOT_CMD(
     nandtest,    CONFIG_SYS_MAXARGS,    1,    do_nandtest,
     "nandtest \t- To perform NAND test\n",
     "<mtdBlockNum>. Valid values depends on mtd parts 1,2 etc.\n"
     "  - perform NAND flash test\n"
     "    create file, write file , and read file\n"
);
#endif
