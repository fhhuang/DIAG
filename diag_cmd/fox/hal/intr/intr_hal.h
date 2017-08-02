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
***            intrtest.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/20, 10:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __INTR_HAL_H_
#define __INTR_HAL_H_

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
/* System Library */
#ifdef FOX_KERNEL
#include <linux/ioctl.h>
#endif
/* User-defined Library */

/*==========================================================================
 *
 *      Constant Definition Segment
 *
 *==========================================================================
 */
#define INTR_RSV0_INT_LINE      0
#define INTR_RM_INT_LINE        1
#define INTR_HWMON_INT_LINE        2
#define INTR_MAC0_INT_LINE        3
#define INTR_MAC1_INT_LINE        4
#define INTR_PHY_INT_LINE      5
#define INTR_POE_INT_LINE      6
#define INTR_IOEXP_INT_LINE        7
#define INTR_PCIE_INT_LINE        8
#define INTR_MGNT_INT_LINE        9
#define INTR_PSU_INT_LINE      10
#define INTR_STK_INT_LINE      11

#ifdef FOX_KERNEL
#define INTR_MAJOR_NUM 38
#define INTR_DEVICE_FILE_NAME "intrDev"

#define IOCTL_SET_PID_MSG _IOW(INTR_MAJOR_NUM, 0, int)
#define IOCTL_ENABLE_IRQ_MSG _IOW(INTR_MAJOR_NUM, 1, int)
#define IOCTL_DISABLE_IRQ_MSG _IOW(INTR_MAJOR_NUM, 2, int)
#define IOCTL_IRQ_TEST_MSG _IOW(INTR_MAJOR_NUM, 3, int)
#endif

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef enum {
    E_EXT_INTR_MIN = 0,
    E_EXT_INTR_SFP_PRESENT,
    E_EXT_INTR_SFP_RX_LOSS,
    E_EXT_INTR_SFP_TX_FAULT,
    E_EXT_INTR_SB,
    E_EXT_INTR_MCU,
    E_EXT_INTR_PUSH_BUTTON,
    E_EXT_INTR_MAX
} E_EXT_INTR;

typedef enum {
    SFP1_PRESENT = 0,
    SFP2_PRESENT,
    SFP3_PRESENT,
    SFP4_PRESENT,
} E_FPGA_SFP_PRESENT;
/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_halIntStatusGet
 *
 *  DESCRIPTION :
 *      special interrupt source has trig
 *
 *  INPUT :
 *      extIntr -interrupt source
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
INT32 intr_halIntStatusGet
(
    IN  E_EXT_INTR                  extIntr,
    OUT UINT32                      *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_halExtIntrClear
 *
 *  DESCRIPTION :
 *      clear external interrupt in an assigned time
 *
 *  INPUT :
 *      extIntr -interrupt source
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
INT32 intr_halIntrClear
(
    IN E_EXT_INTR                   extIntr
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_halIntrGen
 *
 *  DESCRIPTION :
 *      trigger interrupt
 *
 *  INPUT :
 *      none
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
INT32 intr_halIntrGen
(
    IN  E_EXT_INTR                  extIntr
);

#ifdef __cplusplus
}
#endif 

#endif /* __INTR_HAL_H_ */
