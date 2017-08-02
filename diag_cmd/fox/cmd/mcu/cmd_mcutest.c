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
***      cmd_i2ctest.c
***
***    DESCRIPTION :
***      for i2c test
***
***    HISTORY :
***       - 2015/08/01, 16:30:52, Wed Chen
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
#include "mcu_hal.h"
#include "sys_utils.h"
#include "err_type.h"
#include "log.h"
#include <dirent.h>
#include <sys/stat.h>

INT32 do_mcuwrite
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  cmd, addr, data = 0;

    if(argc != 4)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    cmd = simple_strtoul(argv[1], NULL, 16);
    addr = simple_strtoul(argv[2], NULL, 16);
    data = simple_strtoul(argv[3], NULL, 16);

    if ((addr >= MCU_MAX_ADDR) || (addr < 0))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    ret = mcu_halDataSet(cmd, addr, data);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to set MCU data.\n");
        goto __CMD_ERROR;
    }

    log_printf("Write: cmd=0x%X, data=0x%08X\n", cmd, data);

__CMD_ERROR:
    return ret;
}

INT32 do_mcuread
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  cmd, addr, setLen, pktsize, data = 0;
    UINT8   packet[MCU_MAX_SIZE];

    if(argc != 3)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    cmd = simple_strtoul(argv[1], NULL, 16);
    addr = simple_strtoul(argv[2], NULL, 16);

    if ((addr >= MCU_MAX_ADDR) || (addr < 0))
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    ret = mcu_halDataGet(cmd, addr, &data);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to get MCU data.\n");
        goto __CMD_ERROR;
    }

    log_printf("Read: cmd=0x%x, data=0x%08x\n", cmd, data);

__CMD_ERROR:
    return ret;
}

INT32 do_mcuerase
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  page, read_data;

    if(argc != 2)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    page = simple_strtoul(argv[1], NULL, 10);

    // Get MCU version
    ret = mcu_halDataSet(MCU_GET_VER, 0, 0);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to set MCU data.\n");
        goto __CMD_ERROR;
    }
    ret = mcu_halDataGet(0x1, 0, &read_data);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to get MCU version.\n");
        goto __CMD_ERROR;
    }
    log_printf("MCU version is v%d.%d.%d (%s).\n", (read_data >> 16) & 0xFF, (read_data >> 8) & 0xFF, (read_data & 0xFF), ((((read_data >> 24) & 0xFF) == MCU_IAP_MODE) ? "IAP" : "AP"));

    if (page == 0)
    {
        // Old version (v1.0.0 ~ v1.1.0) of MCU doesn't support page erase function
        if ((read_data & 0xFFFFFF) <= 0x010100)
        {
            ret = i2c_halRegSet(MCU_DEV_ADDR, MCU_OPC_ERUSM, 1, (UINT8 *)&page, 1) ;
            log_printf("Write: dev_addr=0x%x, cmd=0x%x, page = ALL (Erase from : 0x%08X)\n",
                        MCU_DEV_ADDR, MCU_OPC_ERPG, MCU_AP_ADDR);
        }
        else
        {
            ret = mcu_halDataSet(MCU_OPC_ERPG, MCU_AP_ADDR, 15);
            log_printf("Write: dev_addr=0x%x, cmd=0x%x, page = %d (Erase from : 0x%08X)\n",
                        MCU_DEV_ADDR, MCU_OPC_ERPG, 15, MCU_AP_ADDR);
        }
    }
    else if ((page <= 10) && (page > 0))
    {
        ret = mcu_halDataSet(MCU_OPC_ERPG, MCU_AP_ADDR, page);
        log_printf("Write: dev_addr=0x%x, addr=0x%x, page = %d (Erase from : 0x%08X)\n",
             MCU_DEV_ADDR, MCU_OPC_ERPG, page, MCU_AP_ADDR);
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to erase MCU flash.\n");
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;
}

INT32 do_mcuupgrade
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  type, cmd, addr, ptr, size, cmdLen, setLen, read_data, mode;
    UINT8   filename[50], *data, packet[MCU_PACKET_SIZE], *data2;
    FILE    *fptr;
    struct stat st;
    INT8   src[64]; 

    if(argc != 2)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    strcpy(filename, argv[1]);

    sprintf(src, "/usr/bin/%s", filename);

    if ((fptr = fopen(src, "r")) == NULL)
    {
        log_printf("Failed to open file %s.\n", src);
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    stat(src, &st);
    size = st.st_size;
    log_printf("\nfile (%s) size = %d Byte\n", src, size);

    data = (UINT8 *)malloc(sizeof(UINT8) * size + 1);
    memset(data,0,sizeof(data));
    if((fread(data, size, 1, fptr)) == 0)
    {
        log_printf("Failed to read file %s.\n", src);
        fclose(fptr);
        free(data);
        ret = E_TYPE_REG_READ;
        goto __CMD_ERROR;
    }

    fclose(fptr);

    switch (data[5])
    {
        case 0x00:log_printf("It's IAP image.\n\n");
                  addr = MCU_BASE_ADDR;
                  break;
        case 0x40:log_printf("It's AP image.\n\n");
                  addr = MCU_AP_ADDR;
                  break;
        default:  log_printf("It's a invalid image.\n");
                  free(data);
                  goto __CMD_ERROR;
    }

    // Check MCU MODE
    log_printf("Checking MCU Mode ...\n");
    ret = mcu_halModeGet(&read_data);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to get MCU running mode.\n");
        free(data);
        goto __CMD_ERROR;
    }
    log_printf("==> Running %s Mode ...\n", ((read_data == MCU_IAP_MODE) ? "IAP" : "AP"));

    // If the image is for IAP, need to jump to AP mode ; otherwise jump to IAP mode
    mode = (addr == MCU_BASE_ADDR) ? MCU_AP_MODE : MCU_IAP_MODE;
    if (read_data != mode)
    {
        log_printf("Configuring MCU Mode ...\n");
        ret = mcu_halModeSet(mode);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to set MCU running mode.\n");
            free(data);
            goto __CMD_ERROR;
        }

        udelay(1000000);
        log_printf("Checking MCU Mode ...\n");
        ret = mcu_halModeGet(&read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU running mode.\n");
            free(data);
            goto __CMD_ERROR;
        }

        if (read_data != mode)
        {
            log_printf("Failed to configure MCU running mode. Current running mode is %s mode.\n", ((read_data == MCU_IAP_MODE) ? "IAP" : "AP"));
            free(data);
            goto __CMD_ERROR;
        }
        log_printf("==> Running %s Mode ...\n", ((read_data == MCU_IAP_MODE) ? "IAP" : "AP"));
    }

    // Erase Flash Memory 15KB (15 pages)
    log_printf("Erasing MCU Flash Memory ...\n");
    ret = mcu_halDataSet(MCU_OPC_ERPG, addr, 15);
    log_dbgPrintf("Write: dev_addr=0x%x, addr=0x%x, page = %d\n", MCU_DEV_ADDR, addr, 15);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to erase MCU flash memory.\n");
        free(data);
        goto __CMD_ERROR;
    }
    log_printf("Succeed to erase MCU flash memory.\n");

    log_printf("Upgrading MCU firmware ...\n");
    log_printf("Please DO NOT break, power off or reboot the device during firmware upgrade.\n");
    cmdLen = MCU_BUF_SIZE;
    for (ptr = 0; ptr < size; ptr+=cmdLen)
    {
        memset(packet, 0, sizeof(packet));
        if(cmdLen + ptr > size) cmdLen = size - ptr;
            mcu_halPacketGen(MCU_OPC_UPGRADE, MCU_OPC_UPGRADE, addr + ptr, data + ptr, packet, &cmdLen);

        ret=i2c_halRegSet(MCU_DEV_ADDR, MCU_OPC_WRITE, 1, packet, cmdLen + 9);
        udelay(100000);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to upgrade MCU firmware.\n");
            free(data);
            goto __CMD_ERROR;
        }
    }

    data2 = (UINT8 *)malloc(sizeof(UINT8) * size + 1);
    memset(data2,0,sizeof(data2));

    log_printf("Verifying MCU firmware ...\n");
    //Reset MCU Read Flag
    ret = mcu_halDataSet(MCU_RESET_READ, 0, 0);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to reset MCU read flag.\n");
        free(data);
        free(data2);
        goto __CMD_ERROR;
    }

    for (ptr = 0; ptr < size; ptr+=4)
    {
        ret = mcu_halDataGet(0x1, addr + ptr, &read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU data.\n");
            free(data);
            free(data2);
            goto __CMD_ERROR;
        }
        data2[ptr] = read_data & 0xFF;
        data2[ptr + 1] = (read_data >> 8) & 0xFF;
        data2[ptr + 2] = (read_data >> 16) & 0xFF;
        data2[ptr + 3] = (read_data >> 24) & 0xFF;
        udelay(1000);
        log_dbgPrintf("Addr = 0x%08X, data = 0x%02X%02X%02X%02X\n", addr + ptr, data2[ptr], data2[ptr+1], data2[ptr+2], data2[ptr+3] );
    }

    if (memcmp(data, data2,size) == 0)
    {
        log_printf("The MCU images are the same.\n");
        log_printf("Upgrade firmware completed.\n");
    }
    else
    {
        log_printf("The MCU images are different.\n");

        // Erase Flash Memory 15KB (15 pages)
        ret = mcu_halDataSet(MCU_OPC_ERPG, addr, 15);
        log_printf("Failed to upgrade firmware.\n");
    }

    free(data);
    free(data2);

    log_printf("Configuring MCU Mode ...\n");
    ret = mcu_halModeSet(!mode);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to set MCU running mode.\n");
        goto __CMD_ERROR;
    }

    udelay(1000000);
    log_printf("Checking MCU Mode ...\n");
    ret = mcu_halModeGet(&read_data);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to get MCU running mode.\n");
        goto __CMD_ERROR;
    }

    log_printf("Current running mode is %s mode.\n", ((read_data == MCU_IAP_MODE) ? "IAP" : "AP"));

__CMD_ERROR:
    return ret;
}

INT32 do_mcumode
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  page, read_data, mode;

    if(argc != 2)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if (!strcasecmp(argv[1], "show"))
    {
        // Get MCU version and running mode
        ret = mcu_halDataSet(MCU_GET_VER, 0, 0);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to set MCU data.\n");
            goto __CMD_ERROR;
        }
        ret = mcu_halDataGet(0x1, 0, &read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU version.\n");
            goto __CMD_ERROR;
        }
        log_printf("MCU version is v%d.%d.%d (%s).\n", (read_data >> 16) & 0xFF, (read_data >> 8) & 0xFF, (read_data & 0xFF), ((((read_data >> 24) & 0xFF) == MCU_IAP_MODE) ? "IAP" : "AP"));
    }
    else if (!strcasecmp(argv[1], "ap") || !strcasecmp(argv[1],"iap"))
    {
        mode = (!strcasecmp(argv[1], "ap")) ? MCU_AP_MODE : MCU_IAP_MODE;
        ret = mcu_halModeGet(&read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU running mode.\n");
            goto __CMD_ERROR;
        }
        log_printf("Current running %s Mode (Before).\n", ((read_data == MCU_IAP_MODE) ? "IAP" : "AP"));

        log_printf("Configuring MCU running mode ...\n");
        ret = mcu_halModeSet(mode);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to set MCU running mode.\n");
            goto __CMD_ERROR;
        }

        udelay(1000000);
        log_printf("Checking MCU running mode ...\n");
        ret = mcu_halModeGet(&read_data);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to get MCU running mode.\n");
            goto __CMD_ERROR;
        }
        log_printf("Current running %s Mode (After).\n", ((read_data == MCU_IAP_MODE) ? "IAP" : "AP"));

        (mode == read_data) ? log_printf("\nSucceed to configure MCU running mode.\n") : \
                              log_printf("\nFailed to configure MCU running mode.\n");
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

U_BOOT_CMD(
    mcuwrite,    CONFIG_SYS_MAXARGS,    1,    do_mcuwrite,
    "mcuwrite \t- Write data to MCU\n",
    "<cmd> <mem_addr> <hex_data>\n"
    "  - cmd: mcu write command\n"
    "  - mem_addr: memory offset, <value:0x0-0x7FFF>\n"
    "  - hex_data: data to be written\n"
);

U_BOOT_CMD(
    mcuread,    CONFIG_SYS_MAXARGS,    1,     do_mcuread,
    "mcuread \t- Read data from MCU\n",
    "<cmd> <mem_addr>\n"
    "  - cmd: mcu read command\n"
    "  - mem_addr: memory offset, <value:0x0-0x7FFF>\n"
);

U_BOOT_CMD(
    mcuerase,    CONFIG_SYS_MAXARGS,    1,    do_mcuerase,
    "mcuerase \t- Erase MCU user space memory\n",
    "<page>\n"
    "  - page: erase page number of MCU user space memory, <value: 0~10>, 0: all\n"
);

U_BOOT_CMD(
    mcuupgrade,    CONFIG_SYS_MAXARGS,    1,   do_mcuupgrade,
    "mcuupgrade \t- Upgrade MCU Application\n",
    "<file>\n"
    "  - file: MCU file name (*.bin)\n"
);

U_BOOT_CMD(
    mcumode,    CONFIG_SYS_MAXARGS,    1,   do_mcumode,
    "mcumode \t- Show and configure current MCU running mode\n",
    "<mode>\n"
    "  - mode: Get/Set the MCU running mode. Valid values are <show|ap|iap>\n"
);
