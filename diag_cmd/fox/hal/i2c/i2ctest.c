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
***      i2ctest.c
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
#include "cmn_type.h"
#include "porting.h"
#include "i2ctest.h"
#include "i2c_hal.h"
#include "mcu_hal.h"
#include "i2c_fpga.h"
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
static INT32 i2ctest_check
(
    IN INT32 deviceId,
    IN UINT32 reg,
    IN INT32 addrLen,
    IN UINT8 *getValue,
    IN UINT8 *setValue,
    IN UINT8 dataLen
)
{
    UINT8 orgValue[4]={0}, idx=0;
    UINT32 testVal=0, readVal=0;

    if( i2c_halRegGet(deviceId, reg, addrLen, &orgValue[0], dataLen) < E_TYPE_SUCCESS )
    {
        log_dbgPrintf("Read device id %x addr %x fail\n", deviceId, reg);
        return E_TYPE_REG_READ;
    }

    if( i2c_halRegSet(deviceId, reg, addrLen, setValue, dataLen) < E_TYPE_SUCCESS )
    {
        log_dbgPrintf("Write device id %x addr %x fail\n", deviceId, reg);
        return E_TYPE_REG_WRITE;
    }

    udelay(10000); /* must be delayed for eeprom */

    if( i2c_halRegGet(deviceId, reg, addrLen, getValue, dataLen) < E_TYPE_SUCCESS )
    {
        log_dbgPrintf("Read device id %x addr %x fail\n", deviceId, reg);
        return E_TYPE_REG_READ;
    }

    if( i2c_halRegSet(deviceId, reg, addrLen, &orgValue[0], dataLen) < E_TYPE_SUCCESS )
    {
        log_dbgPrintf("Write device id %x addr %x fail\n", deviceId, reg);
        return E_TYPE_REG_WRITE;
    }

    for ( idx=0; idx<dataLen; idx++ )
        testVal = testVal | (setValue[idx]<<(8*idx));

    for ( idx=0; idx<dataLen; idx++ )
        readVal = readVal | (getValue[idx]<<(8*idx));

    log_dbgPrintf("Test data: 0x%X, Read back data: 0x%X \n", testVal, readVal);

    if(memcmp(setValue, getValue, dataLen) != 0)
    {
        return E_TYPE_DATA_MISMATCH;
    }

    return E_TYPE_SUCCESS;
}

INT32 msa_cc_base_check
(
    IN UINT8 sfpDev
)
{
    INT32 i=0, ret=E_TYPE_SUCCESS;
    UINT8 buf[256]={0};
    UINT32 checkSum=0;
    BOOL sfpPlugOut=FALSE;

    /* Check the sfp plug in ?*/
    ret = fpgaBitGet(FPGA_SFP_PRE_REG, (sfpDev-1), &sfpPlugOut);
    if (ret != E_TYPE_SUCCESS)
    {
        log_dbgPrintf("Get the SFP presention failed.\n",sfpDev);
        return E_TYPE_REG_READ;
    }

    if ( sfpPlugOut )
    {
        log_dbgPrintf("SFP device %d not plug in.\n", sfpDev);
        return E_TYPE_SFP_NO_PRESENT;
    }

    i2c_halSfpClkSel(sfpDev);

    ret = i2c_halRegGet(SFP_EEPROM_I2C_ADDR, 0, 1, buf, 256);
    if (ret != E_TYPE_SUCCESS)
    {
        log_dbgPrintf("Get the SFP %d eeprom content failed.\n",sfpDev);
        return E_TYPE_REG_READ;
    }

    /*byte[62:0] BASE ID FIELDS, byte63 is Check code for Base ID Fields*/
    for (i=0; i<63; i++)
        checkSum += buf[i];

    log_dbgPrintf("CheckSum = 0x%02x, buf[63]= 0x%02x.\n", checkSum&0xFF, buf[63]);
    if ( (checkSum&0xFF) != buf[63] )
        ret  = E_TYPE_DATA_MISMATCH;

    return ret;
}

INT32 mcuTest()
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 data=0;

    /* set the test data to mcu */
    ret = mcu_halDataSet(MCU_ERASE_WRITE, MCU_TEST_ADDR, MCU_TEST_DATA);
    if (ret != E_TYPE_SUCCESS)
    {
        log_dbgPrintf("Set the mcu data failed.\n");
        return E_TYPE_REG_WRITE;
    }

    /* get the test data from mcu */
    ret = mcu_halDataGet(MCU_GET_DATE, MCU_TEST_ADDR, &data);
    if (ret != E_TYPE_SUCCESS)
    {
        log_dbgPrintf("Get the mcu data failed.\n");
        return E_TYPE_REG_WRITE;
    }

    /* MCU write would write one bytes each time, at first, send low bytes, then high bytes */
    /* MCU_TEST_ADDR(0xA55A55A5) read back should be converse(MCU_READ_DATA 0xA5555AA5) */
    log_dbgPrintf("test data 0x%08X, read back data(0x%08X)\n", MCU_READ_DATA, data);

    if ( data != MCU_READ_DATA )
        ret  = E_TYPE_DATA_MISMATCH;

    return ret;
}

INT32 i2ctest
(
    IN E_I2C_TEST_TYPE i2cTestType,
    IN UINT32 comp_num   /* used to indicate which component to be tested */
)
{
    INT32 ret=E_TYPE_SUCCESS, rc=E_TYPE_SUCCESS;
    UINT8 val[2] = {0};
    UINT16 hwRev;
    UINT32 readDevMACId = 0;
    UINT8 sfpDev=0, idx = 0, getValue[4]={0}, setValue[4]={0x5a, 0x55, 0xaa, 0xa5};
    S_BOARD_INFO    boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if( i2cTestType > E_I2C_TEST_TYPE_MAX )
    {
        log_printf("Unsupport to test this device\n");
        return E_TYPE_INVALID_PARA;
    }

    switch (i2cTestType)
    {
        case E_I2C_TEST_TYPE_EEPROM:
            log_printf("SYSTEM EEPROM: ");
            ret = i2ctest_check(SYSTEM_EEPROM_I2C_ADDR, EEPROM_TEST_ADDR, 2, &getValue[0], &setValue[0], 4);
            break;

        case E_I2C_TEST_TYPE_FPGA:
            log_printf("FPGA: ");
            ret = i2ctest_check(FPGA_I2C_ADDR, FPGA_TEST_REG, FPGA_REG_LEN, &getValue[0], &setValue[0], 2);
            break;

        case E_I2C_TEST_TYPE_MCU:
            log_printf("ST MCU: ");
            ret = mcuTest();
            break;

        case E_I2C_TEST_TYPE_SFP:
            /*Test all*/
            if (comp_num == 0)
            {
                for (idx=1; idx <= (boardInfo.lPortMaxNum-boardInfo.copperMaxNum); idx++)
                {
                    ((boardInfo.lPortMaxNum-boardInfo.copperMaxNum)==2)? (sfpDev=idx+2):(sfpDev=idx);
                    ret = msa_cc_base_check(sfpDev);
                    log_printf("SFP #%d: %s\n", idx, ret? "FAIL":"PASS");
                    if ( ret != E_TYPE_SUCCESS )
                        rc = ret;
                }
                return rc;
            }
            log_printf("SFP #%d: ", comp_num);
            ((boardInfo.lPortMaxNum-boardInfo.copperMaxNum)==2)? (sfpDev=comp_num+2):(sfpDev=comp_num);
            ret = msa_cc_base_check(sfpDev);
            break;

        case E_I2C_TEST_TYPE_IOEXP:
            /* Get Hw Revision by reading FPGA register */
            ret = i2c_halRegGet(FPGA_I2C_ADDR, 0x1, FPGA_REG_LEN, val, 2);
            if ( ret != E_TYPE_SUCCESS )
            {
                log_printf("Fail to get the HW Revision.");
                return ret;
            }
            hwRev = (( val[0]>>3) & 0x3);
            log_dbgPrintf("HwRev: 0x%02x\n",  hwRev);   
            log_dbgPrintf("Hardware Revision : P%d\n",  hwRev);
            
            /*20170626 - Try to read device MAC chip Id information */
            readDevMACId  = sys_utilsDevMACChipIdGet();
            
            /* Add Haywards2 MAC chip is 98DX3236 */
            if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) 
            {  
                log_printf("I/O Expander: ");
                if (hwRev > 0) {
                    ret = i2ctest_check(IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR, 1, &getValue[0], &setValue[0], 1);
                } else {
                    log_printf("PASS\nHW Revision is P0. Skip the I/O Expander Test ");
                }
            }
            else
            {
                log_printf("PASS\nHW is Haywards SKU. Skip the I/O Expander Test ");
            }
            break;

        default:
            return E_TYPE_INVALID_PARA;
    }

    return ret;
}


