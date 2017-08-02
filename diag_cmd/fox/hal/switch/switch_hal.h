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
***switch_hal.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/20, 16:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __SWITCH_HAL_H_
#define __SWITCH_HAL_H_

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
#include "mvTypes.h"
#include "port_defs.h"
#include "userEventHandler.h"
#include "cpss/generic/phy/cpssGenPhySmi.h"
#include "cpss/generic/phy/cpssGenPhyVct.h"
#include "cpss/generic/port/cpssPortCtrl.h"
#include "cpss/generic/port/cpssPortStat.h"
#include "gtOs/gtOsSem.h"
#include "cpss/generic/config/private/prvCpssConfigTypes.h"
#include "cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h"
#include "cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h"
#include "utf/private/prvUtfExtras.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define MAC_ADDRESS_SIZE                6

#define SWITCH_HANDLER_LOW_PRIORITY     250
#define LINESPEED_DAC_7M_10M_PKT_LOSS_NUM_MAX 3600
/* SFP LED */
#define SFP_LED_GREEN_ON                (1<<8)
#define SFP_LED_ACTIVITY                (1<<16)

#define FPGA_MCU_RESET_BIT              (1<<6)

#define PKT_LOOPBACK_MAX_NUM            1000
#define PKT_LINESPEED_MAX_NUM           8
#define PKT_GOLDSWITCH_MAX_NUM          50

#define FOXCONN_PKT_MIN_SIZE            64
#define FOXCONN_PKT_MAX_SIZE            1522

#define LINESPEED_PACKET_LIMIT          1488

/* 1G PHY */
#define PHY_CNTL_REG_OFFSET         (0x0000)
#define PHY_PAGE_REG_OFFSET         (0x0016)
#define PHY_RESET                   (0x8000)
#define PHY_1000MB                  (0x0040)
#define PHY_100MB                   (0x2000)
#define PHY_AUTON                   (0x1000)
#define PHY_DPLX                    (0x0100)
#define PHY_RESTART_AUTO            (0x0200)

#define SMI_DEVICE_ID                   (0x3)
#define SMI_VENDOR_ID                   (0x2)
#define SMI_TEST_REG                    (0x12)  /* Copper specific interrupt reg, all bit is r/w */
#define SMI_TEST2_REG                   (0xe)   /* MMD access address/data reg, all bit is r/w */

/* define the retry time for PHY LPI */
#define PHY_LPI_RETRY_TIME              100

/* Add only for ac3 demo board, Brian Lu */
#define SWITCH_HAL_AC3_DEVICE            0
#define SWITCH_HAL_AC3_CPSS_BOARDID_REG  0x7c
#define SWITCH_HAL_AC3_MAC_DEVICE_ID_REG 0x4c

#define SWITCH_XCAT_AC3_3233_DEV_ID  0xF413
#define SWITCH_XCAT_AC3_3234_DEV_ID  0xF412
#define SWITCH_XCAT_AC3_3235_DEV_ID  0xF411
#define SWITCH_XCAT_AC3_3236_DEV_ID  0xF410
#define SWITCH_XCAT_AC3_C323_DEV_ID  0xF41b


/*==========================================================================
 *
 *      Macro Definition Segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef GT_U32  GT_TASK;

typedef enum {
    E_LB_TEST_SNAKE_MODE_CPU,
    E_LB_TEST_SNAKE_MODE_PKT_GEN,
    E_LB_TEST_INTERNAL_SNAKE_MODE_PKT_GEN,
    E_LB_TEST_SNAKE_MODE_MAX
} E_LB_TEST_SNAKE_MODE;

typedef enum {
    E_LB_CHECK_RETRY=1,
    E_LB_CHECK_SUCCESS=0,
    E_LB_CHECK_FAIL=-1
} E_LB_CHECK;

typedef enum {
    E_PHY_LPI_DISABLED=0,
    E_PHY_LPI_ENABLED,
    E_PHY_LPI_SHOW
} E_PHY_LPI_MODE;

typedef struct {
    UINT64 txTotalBytes;
    UINT64 txUnicastNum;
    UINT64 txBroadcastNum;
    UINT64 txMulticastNum;
} S_PORT_TX_CNT;

typedef struct {
    UINT64 rxTotalBytes;
    UINT64 rxUnicastNum;
    UINT64 rxBroadcastNum;
    UINT64 rxMulticastNum;
} S_PORT_RX_CNT;

typedef struct {
    UINT64 rxBadBytes;
    UINT64 txCrcNum;
    UINT64 rxOverRunNum;
    UINT64 rxUnderSizeNum;
    UINT64 rxFragmentsNum;
    UINT64 rxOverSizeNum;
    UINT64 rxJabbberNum;
    UINT64 rxErrorNum;
    UINT64 rxCrcNum;
    UINT64 txDeferred;
    UINT64 txExcessiveCollision;
    UINT64 txSentMultiple;
    UINT64 txCollisionNum;
} S_PORT_ERR_CNT;

typedef struct {
    S_PORT_TX_CNT txCnt;
    S_PORT_RX_CNT rxCnt;
    S_PORT_ERR_CNT errCnt;
} S_PORT_CNT;


typedef enum {
    CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E,
    CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E,
    CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E
}CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT;

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef enum {
    E_LB_TEST_TYPE_NONE,
    E_LB_TEST_TYPE_MAC,
    E_LB_TEST_TYPE_PHY,
    E_LB_TEST_TYPE_EXT_S,       /* single port loopback, P1 tx == P1 rx */
    E_LB_TEST_TYPE_EXT_P,       /* port-pair loopback, P1 tx == P2 rx */
    E_LB_TEST_TYPE_SNAKE,
    E_LB_TEST_TYPE_CPU,
    E_LB_TEST_TYPE_MAX
} E_LB_TEST_TYPE;

typedef struct
{
    UINT32 linkStatus;
    UINT32 linkSpeed;
    UINT32 cableStatus[4];
    UINT32 cableLen[4];
}PHY_CABLE_INFO;

typedef E_LB_CHECK switch_lbTestFun(volatile UINT8 *, UINT32 );
/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSfpLedTaskFlagGet
 *
 *  DESCRIPTION :
 *      Get flag status of SFP LED task
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      flagStatus
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
INT32 switch_halSfpLedTaskFlagGet
(
    OUT  UINT8      *flagStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSfpLedTaskFlagSet
 *
 *  DESCRIPTION :
 *      Enable / Disable SFP LED Task
 *
 *  INPUT :
 *      flagStatus - Flag status of SFP LED task
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
INT32 switch_halSfpLedTaskFlagSet
(
    IN  UINT8      flagStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halXSMIRegGet
 *
 *  DESCRIPTION :
 *      get xsmi register
 *
 *  INPUT :
 *      device - device number
 *      xsmiAddr - address of xsmi interface
 *      devAddr - device type of xsmi
 *      regNum - register offset
 *      regValue - value of register
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
INT32 switch_halXSMIRegGet
(
    IN  UINT32      lPort,
    IN  UINT32      devAddr,
    IN  UINT32      regNum,
    OUT UINT16 *    regValue
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halXSMIRegSet
 *
 *  DESCRIPTION :
 *      set value to xsmi register
 *
 *  INPUT :
 *      device - device number
 *      xsmiAddr - address of xsmi interface
 *      devAddr - device type of xsmi
 *      regNum - register offset
 *      regValue - value of register
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
INT32 switch_halXSMIRegSet
(
    IN  UINT32  lPort,
    IN  UINT32  devAddr,
    IN  UINT32  regNum,
    IN  UINT16  regValue
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPhyPageNumSet
 *
 *  DESCRIPTION :
 *      set page number of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      pageNum - page number
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
INT32 switch_halPhyPageNumSet
(
    UINT32  lPort,
    UINT32  pageNum
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortCntGet
 *
 *  DESCRIPTION :
 *      Get counter of a logical port
 *
 *  INPUT :
 *      lPort - logical port number
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
INT32 switch_halPortCntGet
(
    IN  UINT32          lPort,
    OUT S_PORT_CNT *    portCnt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortCntGet
 *
 *  DESCRIPTION :
 *      Get counter of a logical port
 *
 *  INPUT :
 *      lPort - logical port number
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
INT32 switch_halCascadePortCntGet
(
    IN UINT32 lPort,
    OUT S_PORT_CNT *portCnt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halArpToCpuStatusSet
 *
 *  DESCRIPTION :
 *      set ARP to CPU trap status of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      status - trap status
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
INT32 switch_halArpToCpuStatusSet
(
    IN  UINT32  lPort,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortForceLinkDownEnableGet
 *
 *  DESCRIPTION :
 *      Get port admin status
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      status - admin status
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
INT32 switch_halPortForceLinkDownEnableGet
(
    IN UINT32 lPort,
    OUT UINT8* status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortForceLinkDownEnableSet
 *
 *  DESCRIPTION :
 *      set admin status of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      status - admin status
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
INT32 switch_halPortForceLinkDownEnableSet
(
    IN  UINT32  lPort,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortLoopbackSet
 *
 *  DESCRIPTION :
 *      Set port loopback mode
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      loopback - loopback mode
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
INT32 switch_halPortLoopbackSet
(
    IN UINT32 lPort,
    IN UINT32 loopback
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortLoopbackGet
 *
 *  DESCRIPTION :
 *      Get port loopback mode
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      loopback - loopback mode
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
INT32 switch_halPortLoopbackGet
(
    IN UINT32 lPort,
    OUT UINT32* loopback
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortAutoNegGet
 *
 *  DESCRIPTION :
 *      Get port autoneg status
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      status - autoneg status
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
INT32 switch_halPortAutoNegGet
(
    IN  UINT32 lPort,
    OUT UINT32  *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortAutoNegSet
 *
 *  DESCRIPTION :
 *      set autoneg status of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      status - autoneg status
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
INT32 switch_halPortAutoNegSet
(
    IN  UINT32  lPort,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCallbackRegister
 *
 *  DESCRIPTION :
 *      register callback function
 *
 *  INPUT :
 *      rcv_fun - callback function
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
INT32 switch_halCallbackRegister
(
    IN GT_STATUS (*rcv_fun)(UINT8,  UINT8,  UINT32,  UINT8 **, UINT32 *, void *)
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halLbInit
 *
 *  DESCRIPTION :
 *      switch loopback test init
 *
 *  INPUT :
 *      txPort - logical port number
 *      rxPort - logical port number
 *      lbTestType - loopback test type
 *      speed - test speed
 *      issfp - flag to check logical type if sfp or not
 *      rcv_fun - callback function
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
INT32 switch_halLbInit
(
    IN  UINT32                      txPort,
    IN  UINT32                      rxPort,
    IN  E_LB_TEST_TYPE              lbTestType,
    IN  UINT32                      speed,
    IN  BOOL                        issfp,
    IN  GT_STATUS (*rcv_fun)(UINT8,  UINT8,  UINT32,  UINT8 **, UINT32 *, void *)
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halLbReInit
 *
 *  DESCRIPTION :
 *      Re-init switch configuration before test
 *
 *  INPUT :
 *      txPort - logical port index
 *      rxPort - logical port index
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
INT32 switch_halLbReInit
(
    IN UINT32                   txPort,
    IN UINT32                   rxPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortCounterClearFlagSet
 *
 *  DESCRIPTION :
 *      Set clear counter flag of a logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      status - flag to clear port counter after read
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
INT32 switch_halPortCounterClearFlagSet
(
    IN  UINT32  lPort,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortPVESet
 *
 *  DESCRIPTION :
 *      set PVE to inter-link port
 *
 *  INPUT :
 *      src_lPort  - source port number
 *      dest_lPort - destnation port number
 *      status     - enable/disable
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
INT32 switch_halPortPVESet
(
    IN  UINT32  src_lPort,
    IN  UINT32  dest_lPort,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halClearPortCounter
 *
 *  DESCRIPTION :
 *      Clear counter of a logical port
 *
 *  INPUT :
 *      lPort - logical port number
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
INT32 switch_halClearPortCounter
(
    IN  UINT32  lPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halClearCascadePortCounter
 *
 *  DESCRIPTION :
 *      Clear counter of a cascade port
 *
 *  INPUT :
 *      lPort - logical port number
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
INT32 switch_halClearCascadePortCounter
(
    IN  UINT32  lPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halBistTcamConfigure
 *
 *  DESCRIPTION :
 *      Configure build-in self test for TCAM
 *
 *  INPUT :
 *      lPort - logical port index
 *      srcMac - soure MAC
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
INT32 switch_halBistTcamConfigure
(
    IN UINT32 lPort,
    IN UINT8 *srcMac
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halBistTcamReinit
 *
 *  DESCRIPTION :
 *      Reinit PCL in TCAM
 *
 *  INPUT :
 *      lPort - logical port index
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
INT32 switch_halBistTcamReinit
(
    IN UINT32 lPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halEthInit
 *
 *  DESCRIPTION :
 *      Eth Tx init
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
INT32 switch_halEthInit
(
    IN  void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSpawnSfpLedTask
 *
 *  DESCRIPTION :
 *      Spawn task for SFP LED
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
INT32 switch_halSpawnSfpLedTask
(
    IN  void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halTrapStatusSet
 *
 *  DESCRIPTION :
 *      set trap to cpu status
 *
 *  INPUT :
 *      devNum    - Device index
 *      status    - enable/disable
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
INT32 switch_halTrapStatusSet
(
    IN  UINT32  devNum,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halEthTx
 *
 *  DESCRIPTION :
 *      Send packet to a logical port
 *
 *  INPUT :
 *      lPort        - logical port number
 *      txCnt        - rx counter structure
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
INT32 switch_halEthTx
(
    IN  UINT32  lPort,
    IN  UINT8 * pktBuf,
    IN  UINT32  pktLen
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortAllCntGet
 *
 *  DESCRIPTION :
 *      Get tx counter of a logical port
 *
 *  INPUT :
 *      lPort        - logical port number
 *      txCnt        - rx counter structure
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
INT32 switch_halPortAllCntGet
(
    IN  UINT32          lPort,
    OUT S_PORT_TX_CNT * txCnt,
    OUT S_PORT_RX_CNT * rxCnt,
    OUT S_PORT_ERR_CNT * errCnt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortAllCntGet
 *
 *  DESCRIPTION :
 *      Get counter of a logical port
 *
 *  INPUT :
 *      lPort        - logical port number
 *      Cnt          - counter structure
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
INT32 switch_halCascadePortAllCntGet
(
    IN  UINT32          lPort,
    OUT S_PORT_TX_CNT * txCnt,
    OUT S_PORT_RX_CNT * rxCnt,
    OUT S_PORT_ERR_CNT * errCnt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halMacAddrGet
 *
 *  DESCRIPTION :
 *      Get MAC address
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      srcMac - MAC address
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
INT32 switch_halMacAddrGet
(
    OUT UINT8 * srcMac
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSetCpuMac
 *
 *  DESCRIPTION :
 *      Set CPU MAC address 
 *
 *  INPUT :
 *      srcMac - MAC address
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
INT32 switch_halSetCpuMac
(
    IN UINT8 *srcMac
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halInvalidCpuMac
 *
 *  DESCRIPTION :
 *      Invalid CPU MAC address entry
 *
 *  INPUT :
 *      srcMac - CPU MAC
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
INT32 switch_halInvalidCpuMac
(
    IN UINT8 *srcMac
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortForceLinkDownEnableGet
 *
 *  DESCRIPTION :
 *      Get cascade port admin status
 *
 *  INPUT :
 *      devNum - device ID
 *      phyPort - physical port
 *
 *  OUTPUT :
 *      status - admin status
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
INT32 switch_halCascadePortForceLinkDownEnableGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT8* status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortAutoNegGet
 *
 *  DESCRIPTION :
 *      Get cascade port autoneg status
 *
 *  INPUT :
 *      devNum - device ID
 *      phyPort - physical port
 *
 *  OUTPUT :
 *      status - autoneg status
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
INT32 switch_halCascadePortAutoNegGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT32 *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortDuplexGet
 *
 *  DESCRIPTION :
 *      get duplex of cascade port
 *
 *  INPUT :
 *      devNum - device ID
 *      phyPort - physical port
 *
 *  OUTPUT :
 *      duplex - duplex of cascade port
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
INT32 switch_halCascadePortDuplexGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT32 *duplex
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortSpeedGet
 *
 *  DESCRIPTION :
 *      get speed of cascade port
 *
 *  INPUT :
 *      devNum - device ID
 *      phyPort - physical port
 *
 *  OUTPUT :
 *      speed - speed of cascade port
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
INT32 switch_halCascadePortSpeedGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT32 *speed
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortMACLinkStatusGet
 *
 *  DESCRIPTION :
 *      Get link status of mac port
 *
 *  INPUT :
 *      devNum - device Id
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      link - link status of logical port
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
INT32 switch_halCascadePortMACLinkStatusGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT32 *link
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortMACLinkStatusGet
 *
 *  DESCRIPTION :
 *      Get link status of mac port
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      link - link status of logical port
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
INT32 switch_halPortMACLinkStatusGet
(
    IN UINT32 lPort,
    OUT UINT32 *link
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortLinkStatusGet
 *
 *  DESCRIPTION :
 *      link status of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      link - link status of logical port
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
INT32 switch_halPortLinkStatusGet
(
    IN UINT32 lPort,
    OUT UINT32 *link
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortFrameSizeGet
 *
 *  DESCRIPTION :
 *      get port max frame size
 *
 *  INPUT :
 *      lPort - logiclal port number
 *
 *  OUTPUT :
 *      frameSize - frame size limit of logical port
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
INT32 switch_halPortFrameSizeGet
(
    IN UINT32 lPort,
    OUT UINT32 *frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortFrameSizeSet
 *
 *  DESCRIPTION :
 *      set port max frame size
 *
 *  INPUT :
 *      lPort - logiclal port number
 *      frameSize - 1518 bytes ~ 16360 bytes
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
INT32 switch_halPortFrameSizeSet
(
    IN UINT32 lPort,
    IN UINT32 frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortFrameSizeGet
 *
 *  DESCRIPTION :
 *      get cascade port max frame size
 *
 *  INPUT :
 *      devNum  - device number
 *      phyPort - cascade port number
 *
 *  OUTPUT :
 *      frameSize - frame size limit of logical port
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
INT32 switch_halCascadePortFrameSizeGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT32 *frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortFrameSizeSet
 *
 *  DESCRIPTION :
 *      set cascade port max frame size
 *
 *  INPUT :
 *      devNum    - device number
 *      phyPort   - cascade port number
 *      frameSize - 1518 bytes ~ 16360 bytes
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
INT32 switch_halCascadePortFrameSizeSet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    IN UINT32 frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortFlowControlGet
 *
 *  DESCRIPTION :
 *      get port flow control status
 *
 *  INPUT :
 *      lPort - logiclal port number
 *
 *  OUTPUT :
 *      status - flow control status of logical port
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
INT32 switch_halPortFlowControlGet
(
    IN UINT32 lPort,
    OUT UINT8 *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortFlowControlSet
 *
 *  DESCRIPTION :
 *      set port flow control status
 *
 *  INPUT :
 *      lPort - logiclal port number
 *      status - flow control status
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
INT32 switch_halPortFlowControlSet
(
    IN UINT32 lPort,
    IN UINT8  status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortFlowControlGet
 *
 *  DESCRIPTION :
 *      get cascade port flow control status
 *
 *  INPUT :
 *      devNum  - device number
 *      phyPort - cascade port number
 *
 *  OUTPUT :
 *      status - flow control status of cascade port
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
INT32 switch_halCascadePortFlowControlGet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    OUT UINT8 *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortFlowControlSet
 *
 *  DESCRIPTION :
 *      set cascade port flow control status
 *
 *  INPUT :
 *      devNum  - device number
 *      phyPort - physical port number
 *      status  - flow control status
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
INT32 switch_halCascadePortFlowControlSet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    IN UINT8  status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSMIRegGet
 *
 *  DESCRIPTION :
 *      get smi register
 *
 *  INPUT :
 *      lPort - liner port
 *      regNum - register offset
 *
 *  OUTPUT :
 *      regValue - value of register
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
INT32 switch_halSMIRegGet
(
    IN  UINT32      lPort,
    IN  UINT32      regNum,
    OUT UINT16      *regValue
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSMIRegSet
 *
 *  DESCRIPTION :
 *      set value to smi register
 *
 *  INPUT :
 *      lPort - liner port
 *      regNum - register offset
 *      regValue - value of register
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
INT32 switch_halSMIRegSet
(
    IN  UINT32  lPort,
    IN  UINT32  regNum,
    IN  UINT16  regValue
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortSpeedGet
 *
 *  DESCRIPTION :
 *      get speed of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      speed - speed of logical port
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
INT32 switch_halPortSpeedGet
(
    IN  UINT32      lPort,
    OUT UINT32 *    speed
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortSpeedSet
 *
 *  DESCRIPTION :
 *      set speed of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      speed - speed of logical port
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
INT32 switch_halPortSpeedSet
(
    IN  UINT32  lPort,
    IN  UINT32  speed,
    IN  E_LB_TEST_TYPE lbTestType
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halMACRegGet
 *
 *  DESCRIPTION :
 *      Get register info from MAC
 *
 *  INPUT :
 *      device - MAC index
 *      regNum - Regiser number
 *
 *  OUTPUT :
 *      regValue - register value
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
INT32 switch_halMACRegGet
(
    IN  UINT32      device,
    IN  UINT32      regNum,
    OUT UINT32      *regValue
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halMACRegSet
 *
 *  DESCRIPTION :
 *      Set register info into MAC
 *
 *  INPUT :
 *      device - MAC index
 *      regNum - Regiser number
 *      regValue - register value
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
INT32 switch_halMACRegSet
(
    IN  UINT32  device,
    IN  UINT32  regNum,
    IN  UINT32  regValue
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortDuplexGet
 *
 *  DESCRIPTION :
 *      get duplex of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      duplex - duplex of logical port
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
INT32 switch_halPortDuplexGet
(
    IN UINT32  lPort,
    OUT UINT32 *duplex
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortDuplexSet
 *
 *  DESCRIPTION :
 *      set duplex of logical port
 *
 *  INPUT :
 *      lPort - logical port number
 *      duplex - duplex of logical port
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
INT32 switch_halPortDuplexSet
(
    IN  UINT32  lPort,
    IN  UINT32  duplex
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halStr2MacAddr
 *
 *  DESCRIPTION :
 *      Transfer a mac address with string format into one with array format.
 *
 *  INPUT :
 *      macStr - a string with a format "xx:xx:xx:xx:xx:xx"
 *
 *  OUTPUT :
 *      macAddr - a mac address in array
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
INT32 switch_halStr2MacAddr
(
    IN  UINT8 * macStr,
    OUT UINT8 * macAddr
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halVlanCreate
 *
 *  DESCRIPTION :
 *      Create a valid VLAN
 *
 *  INPUT :
 *      devNum - device ID
 *      vid - vlan id
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
INT32 switch_halVlanCreate
(
    IN UINT32 devNum,
    IN UINT32 vid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halVlanInvalid
 *
 *  DESCRIPTION :
 *      Invalid a valid VLAN
 *
 *  INPUT :
 *      devId - Device ID
 *      vid - vlan id
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
INT32 switch_halVlanInvalid
(
    IN UINT32 devId,
    IN UINT32 vid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halAddPortToVlan
 *
 *  DESCRIPTION :
 *      add port to special vlan member
 *
 *  INPUT :
 *      lPort - logical port number
 *      vid - vlan id
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
INT32 switch_halAddPortToVlan
(
    IN UINT32 lPort,
    IN UINT32 vid,
    IN BOOL   isTagged
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halRemovePortFromVlan
 *
 *  DESCRIPTION :
 *      delete port to special vlan member
 *
 *  INPUT :
 *      lPort - logical port number
 *      vid - vlan id
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
INT32 switch_halRemovePortFromVlan
(
    IN UINT32 lPort,
    IN UINT32 vid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortPvidGet
 *
 *  DESCRIPTION :
 *      Get port vid
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      pvid - port vid
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
INT32 switch_halPortPvidGet
(
    IN UINT32 lPort,
    OUT UINT16 *pvid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortPvidSet
 *
 *  DESCRIPTION :
 *      set port vid
 *
 *  INPUT :
 *      lPort - logical port number
 *      pvid - port vid
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
INT32 switch_halPortPvidSet
(
    IN UINT32 lPort,
    IN UINT16 pvid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halAddCascadePortToVlan
 *
 *  DESCRIPTION :
 *      add cascade port to special vlan member
 *
 *  INPUT :
 *      devNum - device ID
 *      lPort - logical port number
 *      vid - vlan id
 *      isTagged - specific tagged frame or not
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
INT32 switch_halAddCascadePortToVlan
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    IN UINT32 vid,
    IN BOOL   isTagged
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halRemoveCascadePortFromVlan
 *
 *  DESCRIPTION :
 *      delete cascade port to special vlan member
 *
 *  INPUT :
 *      phyPort - physical port number
 *      vid - vlan id
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
INT32 switch_halRemoveCascadePortFromVlan
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    IN UINT32 vid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCascadePortPvidSet
 *
 *  DESCRIPTION :
 *      set port vid
 *
 *  INPUT :
 *      devNum  - device ID
 *      phyPort - physical port number
 *      pvid - port vid
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
INT32 switch_halCascadePortPvidSet
(
    IN UINT32 devNum,
    IN UINT32 phyPort,
    IN UINT16 pvid
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halVlanReInit
 *
 *  DESCRIPTION :
 *      Re-init a valid VLAN
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
INT32 switch_halVlanReInit
(
    IN void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSnakeLbTestInit
 *
 *  DESCRIPTION :
 *      init port before snake test
 *
 *  INPUT :
 *      snakeMode - snake test mode
 *      portList - test port list
 *      numPort - port number
 *      speed   - test speed
 *      rcv_fun - callback function
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
INT32 switch_halSnakeLbTestInit
(
    IN  E_LB_TEST_SNAKE_MODE    snakeMode,
    IN  UINT32                  startPort,
    IN  UINT32                  endPort,
    IN  UINT32                  speed,
    IN  GT_STATUS (*rcv_fun)(UINT8,  UINT8,  UINT32,  UINT8 **, UINT32 *, void *)
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSnakeLbTestReinit
 *
 *  DESCRIPTION :
 *      init port after snake test
 *
 *  INPUT :
 *      snakeMode - snake test mode
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
INT32 switch_halSnakeLbTestReinit
(
    IN  E_LB_TEST_SNAKE_MODE    snakeMode
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSnakeLbTestCheck
 *
 *  DESCRIPTION :
 *      check before snake loopback test
 *
 *  INPUT :
 *      snakeMode - snake test mode
 *      portList - test port list
 *      numPort - port number
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
INT32 switch_halSnakeLbTestCheck
(
    IN  E_LB_TEST_SNAKE_MODE    snakeMode
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halBoardIDInit
 *
 *  DESCRIPTION :
 *      set xcat3 switch board id
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
INT32 switch_halBoardIDInit
(
    IN  void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halFpgaAllReset
 *
 *  DESCRIPTION :
 *     FPGA Reset Control Register 0xB (USB, MCU, PHY) 0x01FF
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
INT32 switch_halFpgaAllReset
(
    IN  void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halCableDiag
 *
 *  DESCRIPTION :
 *      switch cable diagnostic test
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      cableStatus - cable status information
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
INT32 switch_halCableDiag
(
    IN UINT32 lPort,
    OUT PHY_CABLE_INFO *cableStatus;
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halEEESet
 *
 *  DESCRIPTION :
 *      switch eee function set
 *
 *  INPUT :
 *      lPort - logical port number
 *      mode   - enable/disable
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
INT32 switch_halEEESet
(
    IN UINT32 lPort,
    IN E_PHY_LPI_MODE lpiMode
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halEEEGet
 *
 *  DESCRIPTION :
 *      switch eee function get
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      lpiMode - EEE mode status
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
INT32 switch_halEEEGet
(
    IN UINT32 lPort,
    OUT E_PHY_LPI_MODE *lpiMode
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halBoardIdTest
 *
 *  DESCRIPTION :
 *      API to validate MAC chip ID
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
INT32 switch_halBoardIdTest
(
    IN void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halQsgmiiInit
 *
 *  DESCRIPTION :
 *      Initialize qsgmii for different skus 
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
INT32 switch_halQsgmiiInit
(
    IN void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortSfpRateSet
 *
 *  DESCRIPTION :
 *      select port sfp+ rate
 *
 *  INPUT :
 *      lPort       - logical port
 *      rateLevel - sfp port rate select
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
INT32 switch_halPortSfpRateSet
(
    IN UINT32 lPort,
    IN UINT32 rateLevel
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortSfpRateTest
 *
 *  DESCRIPTION :
 *      test sfp+ port rate select, read ioexp as input result
 *
 *  INPUT :
 *      lPort       - logical port
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
INT32 switch_halPortSfpRateTest
(
    IN UINT32 lPort
);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_HAL_H_ */
