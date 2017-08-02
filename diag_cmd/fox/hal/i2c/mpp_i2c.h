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
***         mpp_i2c.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
***             File Creation
***
***************************************************************************/

#ifndef __MPP_I2C_H_
#define __MPP_I2C_H_

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

/* MPP 11 -> I2C_SDA */
#define SDA_BIT     11
/* MPP 10 -> I2C_SCL */
#define SCL_BIT     10
#define SDA         (1<<SDA_BIT)
#define SCL         (1<<SCL_BIT)

/* MPP define */
#define IO_BASE_ADDR    0xF1010000
#define MPP_USE_SEL     (IO_BASE_ADDR + 0x8100)
#define MPP_IO_SEL      (IO_BASE_ADDR + 0x8104)
#define MPP_IN_LVL      (IO_BASE_ADDR + 0x8110)
#define MPP_OUT_LVL     (IO_BASE_ADDR + 0x8100)
#define MPP_ACCESS      32

#define MAP_SPACE_SIZE  0xFFFFF
#define DELAY_TIME      50

/*==========================================================================
 *
 *      Type and Structure Definition Segment
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
 *      External Funtion Segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      I2Cgpio_write
 *
 *  DESCRIPTION :
 *      I2c data write by MPP simulation I2c
 *
 *  INPUT :
 *      dev - devic address
 *      buff - write data buffer
 *      size - write data size
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
UINT32 I2Cgpio_read
(
    IN UINT16   dev,
    IN UINT8   *buff,
    IN UINT32   size
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      I2Cgpio_write
 *
 *  DESCRIPTION :
 *      I2c data read by MPP simulation I2c
 *
 *  INPUT :
 *      dev - devic address
 *      buff - read data buffer
 *      size - read data size
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
UINT32 I2Cgpio_write
(
    IN UINT16 dev,
    IN UINT8 *buff,
    IN UINT32 size
);

#endif /* __MPP_I2C_H_ */
