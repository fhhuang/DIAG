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

#ifndef __I2C_HAL_H_
#define __I2C_HAL_H_

#include "../common/cmn_type.h"

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
#define I2C_SHIFT_NUM    (1)

/* I2C 0 */
#define I2C_DEVICE_FILE_NAME "i2c-%d"

#define I2C_EEPROM_NUM      1
#define I2C_MUX_NUM         1

#define I2C_SHIFT_NUM    (1)

#define FPGA_I2C_ADDR                         (0x22>>I2C_SHIFT_NUM)
#define ST_MCU_I2C_ADDR                       (0x20>>I2C_SHIFT_NUM)
#define SFP_EEPROM_I2C_ADDR                   (0xA0>>I2C_SHIFT_NUM)
#define SFP_EEPROM_I2C_ADDR_A2                (0xA2>>I2C_SHIFT_NUM)
#define SYSTEM_EEPROM_I2C_ADDR                (0xAE>>I2C_SHIFT_NUM)
#define IO_EXP_I2C_ADDR                       (0x40>>I2C_SHIFT_NUM)
#define CISCO_ACT2_ADDR                       (0xE0>>I2C_SHIFT_NUM)


#define EEPROM_TEST_ADDR    0x400
#define IOEXP_IN_ADDR       0x0 
#define IOEXP_OUT_ADDR      0x2  
#define IOEXP_CFG_ADDR      0x6

#define IO_EXP_TEST_REG     0x7
#define MONITOR_NCT7802Y_TEST_REG    0xfd

#define MONITOR_NCT7802Y_BANK_REG    0x00

#define MONITOR_NCT7802Y_VENDOR_ID   0x50

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef enum {
    E_I2C_BUS_0,
    E_I2C_BUS_1,
    E_I2C_BUS_MAX
} E_I2C_BUS;

typedef enum {
    E_I2C_MUX_0,
    E_I2C_MUX_1,
    E_I2C_MUX_2,
    E_I2C_MUX_3,
    E_I2C_MUX_4,
    E_I2C_MUX_5,
    E_I2C_MUX_6,
    E_I2C_MUX_7,
    E_I2C_MUX_MAX
} E_I2C_MUX;

typedef struct {
    union
    {
        struct
        {
            UINT8   sfpInfoA0[256];
        } raw_data;
        struct
        {
            UINT8   identifier;
            UINT8   ext_id;
            UINT8   connector;
            UINT8   transceiver[8];
            UINT8   encoding;
            UINT8   baudrate;
            UINT8   reserved1;
            UINT8   length_9u_km;
            UINT8   length_9u;
            UINT8   length_50u;
            UINT8   length_62_5u;
            UINT8   length_Cu;
            UINT8   reserved2;
            UINT8   vendor_name[16];
            UINT8   reserved3;
            UINT8   vendor_OUI[3];
            UINT8   vendor_PN[16];
            UINT8   vendor_Rev[4];
            UINT8   wavelength[2];
            UINT8   reserved4;
            UINT8   cc_base;
            UINT8   options[2];
            UINT8   br_max;
            UINT8   br_min;
            UINT8   serial_no[16];
            UINT8   date_code[8];
            UINT8   reserved5[3];
            UINT8   cc_ext;
            UINT8   vendor_specific[161];
        } parsed_data;
    };
    UINT8   sfpInfoA2[256];
} S_SFP_MSA;

/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
INT32 i2c_halRegGet
(
    /* Modified by Foxconn Alex, 2015/10/15 */
    IN UINT8 dev_addr,
    IN UINT32 reg_addr,
    IN INT32 addLen,
    OUT UINT8 *value,
    IN INT32 getLen
);

INT32 i2c_halRegSet
(
    /* removed by Foxconn Alex, 2015/10/15 */
    IN UINT8 dev_addr,
    IN UINT32 reg_addr,
    IN INT32 addLen,
    IN UINT8 *data,
    IN INT32 setLen
);

INT32 i2c_halSfpInfoGet
(
    IN  UINT32      lPort,
    OUT S_SFP_MSA * info
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_halSfpClkSel
 *
 *  DESCRIPTION :
 *      1x4 SFP clock channel select
 *
 *  INPUT :
 *      i2cSfpClk - i2c SFP clock channel number.(valid value 1-4)
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - action successfully
 *      Other - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 i2c_halSfpClkSel
(
    IN UINT32 i2cSfpClk
);

INT32 mcu_halRegGet
(
    IN UINT8 dev_addr,
    IN UINT32 reg_addr,
    IN INT32 addLen,
    OUT UINT8 *value,
    IN INT32 getLen
);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_HAL_H_ */
