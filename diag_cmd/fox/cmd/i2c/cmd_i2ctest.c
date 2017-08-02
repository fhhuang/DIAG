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
#include "i2ctest.h"
#include "i2c_hal.h"
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
INT32 do_i2ctest
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS, tesResult=E_TEST_PASS;
    E_I2C_TEST_TYPE i2cTestType = E_I2C_TEST_TYPE_MAX;
    UINT32  comp_num = 0;
    INT32 addLen, getLen, setLen;

    if(argc != 3 && argc != 2)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    /* show all devices */
    if( strcmp(argv[1], "all") == 0)
    {
        for(i2cTestType=E_I2C_TEST_TYPE_EEPROM; i2cTestType < E_I2C_TEST_TYPE_MAX ;i2cTestType++)
        {
            ret = i2ctest(i2cTestType, comp_num);

            /* At least one device fail */
            if( ret != E_TYPE_SUCCESS )
            {
                log_printf("FAIL\r\n");
                tesResult=ret;
            }
            else
                log_printf("PASS\r\n");
        }

        if( tesResult == E_TYPE_SUCCESS )
        {
            log_cmdPrintf(E_LOG_MSG_PASS, "I2C Test All\r\n");
        }
        else
        {
            log_cmdPrintf(E_LOG_MSG_FAIL, "I2C Test All\r\n");
        }

        return tesResult;
    }
    else if( strcmp(argv[1], "eeprom") == 0 )
    {
        i2cTestType = E_I2C_TEST_TYPE_EEPROM;
    }
    else if( strcmp(argv[1], "fpga") == 0 )
    {
        i2cTestType = E_I2C_TEST_TYPE_FPGA;
    }
    else if( strcmp(argv[1], "mcu") == 0 )
    {
        i2cTestType = E_I2C_TEST_TYPE_MCU;
    }
    else if( strcmp(argv[1], "sfp") == 0 )
    {
        if ( argc != 3 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        /* Specify which SFP module to test */
        comp_num = simple_strtoul(argv[2], NULL, 10);
        i2cTestType = E_I2C_TEST_TYPE_SFP;
    }
    else if( strcmp(argv[1], "ioexp") == 0 )
    {
        i2cTestType = E_I2C_TEST_TYPE_IOEXP;
    }
    else
    {
        log_printf("Unsupport to test this device\n");
        ret = E_TYPE_INVALID_CMD;
        goto __CMD_ERROR;
    }

    ret = i2ctest(i2cTestType, comp_num);
    log_printf("\n");

    if( ret != E_TYPE_SUCCESS )
    {
        log_cmdPrintf(E_LOG_MSG_FAIL, "I2C Test - %s\r\n", argv[1]);
    }
    else
    {
        log_cmdPrintf(E_LOG_MSG_PASS, "I2C Test - %s\r\n", argv[1]);
    }

__CMD_ERROR:
    return ret;
}

INT32 do_i2cread
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  bus, dev_addr, addr, data, switch_num;
    INT32   addLen, getLen, setLen;

    if(argc != 7)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    bus = simple_strtoul(argv[1], NULL, 10);
    switch_num = simple_strtoul(argv[2], NULL, 10);
    dev_addr = simple_strtoul(argv[3], NULL, 16);
    addr = simple_strtoul(argv[4], NULL, 16);
    addLen = simple_strtoul(argv[5], NULL, 10);
    getLen = simple_strtoul(argv[6], NULL, 10);

    if( getLen > sizeof(data) )
    {
        log_printf("i2c read length fail (getLen=%d)\n", getLen);
        ret = E_TYPE_INVALID_PARA;
        goto __CMD_ERROR;
    }

    data = 0;
    ret=i2c_halRegGet(dev_addr, addr, addLen, (UINT8 *)&data, getLen);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("FAIL\n");
        goto __CMD_ERROR;
    }

#if 0
    switch (getLen)
    {
        case 1:
            data = data>>24;
            break;
        case 2:
            data = data>>16;
            break;
        default:
        case 4:
            break;
    }
#endif
    log_printf("Read: dev_addr=0x%x, addr=0x%x, data=0x%08x, (al=%d, dl=%d)\n", dev_addr, addr, data, addLen, getLen);

__CMD_ERROR:
    return ret;
}

INT32 do_i2cwrite
(
    IN  cmd_tbl_t * cmdtp,
    IN  INT32       flag,
    IN  INT32       argc,
    IN  INT8 *      argv[]
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  bus, switch_num, dev_addr, addr, data;
    INT32   addLen, getLen, setLen;

    if(argc != 8)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    data = 0;
    bus = simple_strtoul(argv[1], NULL, 10);
    switch_num = simple_strtoul(argv[2], NULL, 10);
    dev_addr = simple_strtoul(argv[3], NULL, 16);
    addr = simple_strtoul(argv[4], NULL, 16);
    addLen = simple_strtoul(argv[5], NULL, 10);
    data = simple_strtoul(argv[6], NULL, 16);
    setLen = simple_strtoul(argv[7], NULL, 10);

    if( setLen > sizeof(data) )
    {
        log_printf("i2c write length fail (getLen=%d)\n", setLen);
        ret = E_TYPE_INVALID_PARA;
        goto __CMD_ERROR;
    }
#if 0
    switch (setLen)
    {
        case 1:
            data = data<<24;
            break;
        case 2:
            data = data<<16;
            break;
        default:
        case 4:
            break;
    }
#endif
    ret=i2c_halRegSet(dev_addr, addr, addLen, (UINT8 *)&data, setLen) ;

    log_printf("Write: dev_addr=0x%x, addr=0x%x, data=0x%08x, (al=%d,dl=%d)\n", dev_addr, addr, data, addLen, setLen);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("FAIL\n");
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    i2ctest,    CONFIG_SYS_MAXARGS,    1,    do_i2ctest,
    "i2ctest \t- Test i2c devices\n",
    "<device_name | all>\n"
    "  - device_name: Specify the i2c device to test. Valid devices are \n"
    "     <mcu|fpga|eeprom|sfp <1~4>|ioexp|all>.\n"
);

U_BOOT_CMD(
    i2cread,    CONFIG_SYS_MAXARGS,    1,    do_i2cread,
    "i2cread \t- Read data from an i2c device\n",
    "<bus> <channel> <dev_addr> <reg_addr> <addr_len> <data_len>\n"
    "  - bus: I2C bus number\n"
    "  - channel: I2C MUX channel index, <value: 0~4>\n"
    "  - dev_addr: device address\n"
    "  - reg_addr: register offset\n"
    "  - addr_len: length of register offset (0~2)\n"
    "  - data_len: length of data\n"
);

U_BOOT_CMD(
    i2cwrite,    CONFIG_SYS_MAXARGS,    1,    do_i2cwrite,
    "i2cwrite \t- Write data to i2c device\n",
    "<bus> <channel> <dev_addr> <reg_addr> <addr_len> <hex_data> <data_len>\n"
    "  - bus: I2C bus number\n"
    "  - channel: I2C MUX channel index, <value: 0~4>\n"
    "  - dev_addr: device address\n"
    "  - reg_addr: register offset\n"
    "  - addr_len: length of register offset (0~2)\n"
    "  - hex_data: data to be written\n"
    "  - data_len: length of data\n"
);


