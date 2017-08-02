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
***             i2c_fpga.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
***             File Creation
***
***************************************************************************/

#ifndef __FPGA_TEST_H_
#define __FPGA_TEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include "cmn_type.h"
#include "port_defs.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Macro Definition Segment
 *
 *==========================================================================
 */

#define FPGA_REG_LEN            1
#define FPGA_VER_ID             0x00
#define FPGA_TEST_REG           0x00
#define FPGA_BOARD_ID_REG       0x01
#define FPGA_HW_VER_REG         0x02
#define FPGA_VER_REG            0x03
#define FPGA_SFP_PRE_REG        0x04
#define FPGA_SFP_LOS_REG        0x05
#define FPGA_SFP_FAULT_REG      0x06
#define FPGA_SFP_DISABLE_REG    0x07
#define FPGA_USB_CTRL_REG       0x08
#define FPGA_INT_MASK_REG       0x09
#define FPGA_INT_STA_REG        0x0A
#define FPGA_RESET_REG          0x0B
#define FPGA_SEC_BOOT_REG0      0x10
#define FPGA_SEC_BOOT_REG1      0x11
#define FPGA_TEST_DATA          0x5A5A
#define FPGA_ERHAI_SB_STATUS_2  0x12
#define FPGA_ERHAI_SB_STATUS_3  0x13
#define FPGA_ERHAI_SB_STATUS_4  0x14
#define FPGA_ERHAI_SB_STATUS_5  0x15
#define FPGA_SFP_SEL_MASK       0xFFCF
//#define FPGA_REG_ADDR_MAX       FPGA_SEC_BOOT_REG1
#define FPGA_REG_ADDR_MAX       FPGA_ERHAI_SB_STATUS_5

/*FPGA eSPI define*/
#define BUF_SIZE                (4096)
#define USER_SPI_ADDR_START     (0x0)
#define USER_SPI_ADDR_END       (0x200000)
#define SIZE_64K                (0x10000)
#define SIZE_4K                 (0x1000)
#define MASK_4K                 (0xFFFFF000)

#define SPI_DIR_TABLE_START     (0x0)
#define SPI_DIR_TABLE_SIZE      (12)
#define BITSTREAM_BUF_SIZE      (0x80000)
#define GOLDEN_FW_START         (0x00001000)
#define UPDATE_FW_START         (0x00080000)
#define GOLDEN_FPGA_START       (0x00100000)
#define UPDATE_FPGA_START       (0x00180000)

#define GOLDEN_IMAGE            (1)
#define UPGRADE_IMAGE           (2)
#define NEW_GOLDEN_VER          (0)
#define NEW_UPDATE_VER          (2)

#define WR_DATA_LEN             (0x400)

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef enum {
    E_FPGA_TYPE_VER,
    E_FPGA_TYPE_SEC,
    E_FPGA_TYPE_USB,
    E_FPGA_TYPE_MCU,
    E_FPGA_TYPE_INT,
    E_FPGA_TYPE_SFP,
    E_FPGA_TYPE_MAX
} E_FPGA_TYPE;

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */

typedef struct {
    UINT32 golden_start;
    UINT16 golden_ver;
    UINT32 update_start;
    UINT16 update_ver;
} __attribute__((packed))dir_tbl_t;

/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaRegRead
 *
 *  DESCRIPTION :
 *      FPGA register read
 *
 *  INPUT :
 *      regAddr - register address
 *
 *  OUTPUT :
 *      regVal - register value
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
INT32 fpgaRegRead
(
    IN UINT32 regAddr,
    OUT UINT16 *regVal
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaRegWrite
 *
 *  DESCRIPTION :
 *      FPGA register write
 *
 *  INPUT :
 *      regAddr - register address
 *      regVal - register value
 *
 *  OUTPUT :
 *      None
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
INT32 fpgaRegWrite
(
    IN UINT32 regAddr,
    IN UINT16 regVal
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sfpClkSel
 *
 *  DESCRIPTION :
 *      FPGA to switch the SFP clock channel
 *
 *  INPUT :
 *      clkNum - sfp clock channel
 *
 *  OUTPUT :
 *      None
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
INT32 fpga_sfpClkSel
(
    IN UINT32 clkNum
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpga_boardIdGet
 *
 *  DESCRIPTION :
 *      FPGA to get board id
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      boardId - board id
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
INT32 fpga_boardIdGet
(
    OUT UINT32 *boardId
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpga_hwVerGet
 *
 *  DESCRIPTION :
 *      FPGA to get HW version
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      hwVer - HW version
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
INT32 fpga_hwVerGet
(
    OUT UINT32 *hwVer
);

#endif /* __FPGA_TEST_H_ */
