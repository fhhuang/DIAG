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
***
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2008/12/09, 16:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __I2CTEST_H_
#define __I2CTEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define NUM_1   (1<<0)
#define NUM_2   (1<<1)
#define NUM_3   (1<<2)
#define MCU_TEST_DATA 0xA55A55A5
#define MCU_READ_DATA 0xA5555AA5
#define MCU_TEST_ADDR 0x7800

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef enum {
    E_I2C_TEST_TYPE_EEPROM,
    E_I2C_TEST_TYPE_FPGA,
    E_I2C_TEST_TYPE_MCU,
    E_I2C_TEST_TYPE_SFP,
    E_I2C_TEST_TYPE_IOEXP,
    E_I2C_TEST_TYPE_MAX
} E_I2C_TEST_TYPE;

/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
INT32 i2cprobe
(
    IN E_I2C_TEST_TYPE i2cTestType
);

INT32 i2ctest
(
    IN E_I2C_TEST_TYPE i2cTestType,
    IN UINT32 comp_num   /* used to indicate which component to be tested */
);

#ifdef __cplusplus
}
#endif

#endif /* __I2CTEST_H_ */
