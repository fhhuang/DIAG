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
***      cmd_fpga.c
***
***    DESCRIPTION :
***      for fpga test
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
#include "i2c_hal.h"
#include "i2c_fpga.h"
#include "act2test.h"
#include "sys_utils.h"
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

INT32 do_fpgaReadWrite
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 regAddr = 0;
    UINT16 regVal = 0;


    if(argc != 3 && argc != 2)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    regAddr = simple_strtoul(argv[1], NULL, 16);
    if ( regAddr < 0 || regAddr > FPGA_REG_ADDR_MAX )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( strstr(argv[0], "fpgaread") )
    {
        if( argc != 2 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        ret = fpgaRegRead(regAddr, &regVal);
        log_printf("Read: 0x%x = 0x%04X\n", regAddr, regVal);
    }
    else if( strstr(argv[0], "fpgawrite") )
    {
        if( argc != 3 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        regVal = simple_strtoul(argv[2], NULL, 16);
        ret = fpgaRegWrite(regAddr, regVal);
        log_printf("Write: 0x%x = 0x%04X\n", regAddr, regVal);
    }
    log_printf("\n");

    return ret;

__CMD_ERROR:

    return ret;
}

INT32 do_act2Test
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    ACT2_TEST_TYPE    act2Type = ACT2_MAX;
    uint32_t debug = 0;

    gSpiFlag = 0;

    if( (argc > 3) || (argc < 2) )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( strcmp(argv[1], "probe") == 0)
    {
        act2Type = ACT2_PROBE;
    }
    else if ( strcmp(argv[1], "probe_i2c") == 0)
    {
        act2Type = ACT2_PROBE_I2C;
    }
    else if ( strcmp(argv[1], "program") == 0)
    {
        act2Type = ACT2_PROGRAM;
    }
    else if ( strcmp(argv[1], "program_i2c") == 0)
    {
        act2Type = ACT2_PROGRAM_I2C;
    }    
    else if ( strcmp(argv[1], "auth") == 0)
    {
        act2Type = ACT2_ATHEN;
    }
    else if ( strcmp(argv[1], "auth_i2c") == 0)
    {
        act2Type = ACT2_ATHEN_I2C;
    }
    else if ( strcmp(argv[1], "ecskmp") == 0)
    {
        act2Type = ACT2_ECSKMP;
    }
    else if ( strcmp(argv[1], "ecskmp_i2c") == 0)
    {
        act2Type = ACT2_ECSKMP_I2C;
    }    
    else if ( strcmp(argv[1], "spitest") == 0)
    {
        act2Type = ACT2_SPITEST;
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( (argc == 3) && (strcmp(argv[2], "-v") == 0))
    {
        debug = 1;
    }

    if (!strstr(argv[1], "_i2c")) {
        gSpiFlag = 1;
        act2_hal_memMapInit();
    }
    
    ret = fpgaAct2Test(act2Type, debug);
    if( ret != E_TYPE_SUCCESS )
    {
        log_cmdPrintf(E_LOG_MSG_FAIL, "ACT2 %s Test\r\n", argv[1]);
    }
    else
    {
        log_cmdPrintf(E_LOG_MSG_PASS, "ACT2 %s Test\r\n", argv[1]);
    }
    
__CMD_ERROR:
    if (gSpiFlag)
        act2_hal_memMapClose();
    
    return ret;
}

INT32 do_fpgaUpgrade
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  startOffset=0, size = 0;
    UINT16  goldenVer=0, updateVer=0;

    gSpiFlag = 0;
    gDebug = 0;
    
    if ( (strcmp("fpgaupgrade", argv[0]) == 0) || (strcmp("fpgaupgrade_i2c", argv[0]) == 0) )
    {
        if(argc < 3)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if (!strstr(argv[0], "_i2c")) {
            gSpiFlag = 1;
            act2_hal_memMapInit();
            
            if(strcmp(argv[argc -1], "-v") == 0)
                gDebug = 1;
        }
        
        ret = fpgaUpgrade(argv[1], argv[2]);
        if( ret != E_TYPE_SUCCESS )
        {
            log_cmdPrintf(E_LOG_MSG_FAIL, "FPGA upgrade.\r\n");
        }
        else
        {
            log_cmdPrintf(E_LOG_MSG_PASS, "FPGA upgrade.\r\n");
        }
    }
    else if ( (strcmp("fpgaspi", argv[0]) == 0) || (strcmp("fpgaspi_i2c", argv[0]) == 0) )
    {
        if( argc < 4 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if (!strstr(argv[0], "_i2c")) {
            gSpiFlag = 1;
            act2_hal_memMapInit();

            if(strcmp(argv[argc -1], "-v") == 0)
                gDebug = 1;
        }
        
        if( strcmp(argv[1], "dump") == 0)
        {
            startOffset = simple_strtoul(argv[2], NULL, 16);
            size = simple_strtoul(argv[3], NULL, 16);
            ret = fpgaSpiDump(startOffset, size);
        }
        else if( strcmp(argv[1], "updatedir") == 0)
        {
            goldenVer = simple_strtoul(argv[2], NULL, 16);
            updateVer = simple_strtoul(argv[3], NULL, 16);
            ret = fpgaUpdateSpiDir(goldenVer, updateVer);
        }
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD;
    }

__CMD_ERROR:
    if (gSpiFlag)
        act2_hal_memMapClose();
    
    return ret;
}

INT32 do_fpgaAikido
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  spireg =0, wdata=0, gpio5=0;
    ACT2_TEST_TYPE    act2Type = ACT2_MAX;

    gDebug = 0;
    gSpiFlag = 0;

    if(argc < 2)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if (strcmp("spiwrite", argv[1]) == 0 )
    {
        if(argc < 4)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        gSpiFlag = 1;
        /* create memory map to access SPI register */
        act2_hal_memMapInit();
         
        if(strcmp(argv[argc -1], "-v") == 0)
                gDebug = 1;

             
         spireg = simple_strtoul(argv[2], NULL, 16);
         wdata = simple_strtoul(argv[3], NULL, 16);
         act2Type = ACT2_SPIWRITE;
         ret = fpgaAikidoTest(act2Type, gDebug, 0x0, spireg, wdata);
            
    }
    else if ( (strcmp("spiread", argv[1]) == 0) || (strcmp("gpio5", argv[1]) == 0) )
    {
        if( argc < 3)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        gSpiFlag = 1;
        /* create memory map to access SPI register */
        act2_hal_memMapInit();

        if(strcmp(argv[argc -1], "-v") == 0)
            gDebug = 1;

        if( strcmp(argv[1], "spiread") == 0)
        {
            act2Type = ACT2_SPIREAD;
            spireg = simple_strtoul(argv[2], NULL, 16);
            ret = fpgaAikidoTest(act2Type, gDebug, 0x0, spireg, 0x0);
        }
        else if( strcmp(argv[1], "gpio5") == 0)
        {
            act2Type = ACT2_SPIGPIO5;
            gpio5 = simple_strtoul(argv[2], NULL, 16);
            ret = fpgaAikidoTest(act2Type, gDebug, gpio5, 0x0, 0x0);
        }
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD;
    }

__CMD_ERROR:
     if (gSpiFlag)
         act2_hal_memMapClose();
    
    return ret;
}

U_BOOT_CMD(
    fpgaread,    CONFIG_SYS_MAXARGS,    1,    do_fpgaReadWrite,
    "fpgaread \t- FPGA register read\n",
    "<regAddr>\n"
    "  - regAddr: register address. <valid value 0x0~0x11>\r\n"
);

U_BOOT_CMD(
    fpgawrite,    CONFIG_SYS_MAXARGS,    1,    do_fpgaReadWrite,
    "fpgawrite \t- FPGA register write\n",
    "<regAddr> <regVal>\n"
    "  - regAddr: register address. <valid value 0x0~0x11>\r\n"
    "  - regVal: set value data\r\n"
);

U_BOOT_CMD(
    act2,    CONFIG_SYS_MAXARGS,    1,    do_act2Test,
    "act2 \t\t- ACT2 probe and program test\n",
    "<probe|program|auth|ecskmp|probe_i2c|program_i2c|auth_i2c|ecskmp_i2c>\n"
    "  - probe: probe the ACT2 fw version and chip serial number.(SPI)\n"
    "  - program: program ACT2 chip for mfg process.(SPI)\n"
    "  - auth: authenticate ACT2 chip after program.(SPI)\n"
    "  - ecskmp: generate the ecskmp.bin file.(SPI)\n"
    "  - probe_i2c: probe the ACT2 fw version and chip serial number.(I2C)\n"
    "  - program_i2c: program ACT2 chip for mfg process.(I2C)\n"
    "  - auth_i2c: authenticate ACT2 chip after program.(I2C)\n"
    "  - ecskmp_i2c: generate the ecskmp.bin file.(I2C)\n"
);

U_BOOT_CMD(
    fpgaupgrade,    CONFIG_SYS_MAXARGS,    1,    do_fpgaUpgrade,
    "fpgaupgrade \t- FPGA FW upgrade by SPI\n",
    "<fpga_file> <fw_file>\n"
    "  - fpga_file: fpga file.\n"
    "  - fw_file: fpga fw file.\n"
);

U_BOOT_CMD(
    fpgaupgrade_i2c,    CONFIG_SYS_MAXARGS,    1,    do_fpgaUpgrade,
    "fpgaupgrade_i2c\t- FPGA FW upgrade by I2C\n",
    "<fpga_file> <fw_file>\n"
    "  - fpga_file: fpga file.\n"
    "  - fw_file: fpga fw file.\n"
);

U_BOOT_CMD(
    fpgaspi,    CONFIG_SYS_MAXARGS,    1,    do_fpgaUpgrade,
    "fpgaspi \t- FPGA SPI flash program by SPI\n",
    "<dump|updatedir>\n"
    "  - dump: dump <startOffset> <size>.\n"
    "  - updatedir: updatedir <golden version> <update version>.\n"
);

U_BOOT_CMD(
    fpgaspi_i2c,    CONFIG_SYS_MAXARGS,    1,    do_fpgaUpgrade,
    "fpgaspi_i2c \t- FPGA SPI flash program by I2C\n",
    "<dump|updatedir>\n"
    "  - dump: dump <startOffset> <size>.\n"
    "  - updatedir: updatedir <golden version> <update version>.\n"
);

U_BOOT_CMD(
    fpgaaikido,    CONFIG_SYS_MAXARGS,    1,    do_fpgaAikido,
    "fpgaaikido \t- FPGA Aikido access by SPI\n",
    "<gpio5|spiread|spiwrite>\n"
    "  - gpio5: gpio5 <0|1>.\n"
    "  - spiread: spiread <reg>.\n"
    "  - spiwrite: spiwrite <reg> <wdata>.\n"
);

