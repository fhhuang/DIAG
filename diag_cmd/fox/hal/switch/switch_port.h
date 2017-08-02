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
***            switch_port.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/21, 14:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __SWITCH_PORT_H_
#define __SWITCH_PORT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "i2c_hal.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define SFP_INFO_BUFF_SIZE        (256+1)
#define SFP_DIAG_MON_TYPE 92
#define SFP_DIAG_TEMP_MSB 96
#define SPF_DIAG_TEMP_LSB 97

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef enum {
    E_SWITCH_PORT_LINK_DOWN,
    E_SWITCH_PORT_LINK_UP,
    E_SWITCH_PORT_LINK_FAILED,
    E_SWITCH_PORT_LINK_REMOTE_FAILED
} E_SWITCH_PORT_LINK;

typedef enum {
    E_SWITCH_PORT_ADMIN_OFF,
    E_SWITCH_PORT_ADMIN_ON
} E_SWITCH_PORT_ADMIN;

typedef enum {
    E_SWITCH_PORT_FLOWCONTROL_OFF,
    E_SWITCH_PORT_FLOWCONTROL_ON
} E_SWITCH_PORT_FLOWCONTROL;

typedef enum {
    E_SWITCH_PORT_AUTONEG_OFF,
    E_SWITCH_PORT_AUTONEG_ON
} E_SWITCH_PORT_AUTONEG;

typedef enum {
    E_SWITCH_PORT_LOOPBACK_MAC,
    E_SWITCH_PORT_LOOPBACK_PHY,
    E_SWITCH_PORT_LOOPBACK_NONE
} E_SWITCH_PORT_LOOPBACK;

typedef enum {
    E_SWITCH_PORT_SPEED_10M,
    E_SWITCH_PORT_SPEED_100M,
    E_SWITCH_PORT_SPEED_1G,
    E_SWITCH_PORT_SPEED_2G5,
    E_SWITCH_PORT_SPEED_10G,
    E_SWITCH_PORT_SPEED_16G,
    E_SWITCH_PORT_SPEED_20G,
    E_SWITCH_PORT_SPEED_40G,
    E_SWITCH_PORT_SPEED_100G
} E_SWITCH_PORT_SPEED;

typedef enum {
    E_SWITCH_PORT_DUPLEX_HALF,
    E_SWITCH_PORT_DUPLEX_FULL
} E_SWITCH_PORT_DUPLEX;

typedef struct {
    E_SWITCH_PORT_LINK link;
    E_SWITCH_PORT_SPEED speed;
    E_SWITCH_PORT_DUPLEX duplex;
    E_SWITCH_PORT_AUTONEG autoneg;
    E_SWITCH_PORT_LOOPBACK loopback;
    E_SWITCH_PORT_ADMIN admin;
    S_SFP_MSA sfpInfo;
} S_SWITCH_PORT_STATUS;

typedef enum {
    E_SWITCH_SFP_TYPE_LRM_10GE = 1,
    E_SWITCH_SFP_TYPE_SR_10GE = 2,
    E_SWITCH_SFP_TYPE_LR_10GE =3,
    E_SWITCH_SFP_TYPE_SFPP_DIR_ATT_CABLE = 4,
    E_SWITCH_SFP_TYPE_SX_1GE = 5,
    E_SWITCH_SFP_TYPE_LX_1GE = 6,
    E_SWITCH_SFP_TYPE_INVALID_MODULE = 7,
    E_SWITCH_SFP_TYPE_1000BASE_CX = 8,
    E_SWITCH_SFP_TYPE_1000BASE_T = 9,
    E_SWITCH_SFP_TYPE_RESERVED,
    E_SWITCH_SFP_TYPE_MAX
} E_SWITCH_SFP_TYPE;

typedef enum  {
    E_SFP1_IOEXP0  = 0,
    E_SFP0_IOEXP2  = 2,
    E_SFP3_IOEXP4  = 4,
    E_SFP2_IOEXP6  = 6
} E_SWITCH_SFP_RATE_IOEXP_OFFSET;

typedef enum {
    E_SWITCH_SFP_RATE_LO   = 0,
    E_SWITCH_SFP_RATE_HI   = 3
} E_SWITCH_SFP_RATE_LEVEL;

enum SFP_PORT_STATUS {
    SFP_PORT_NO_ALL = 0,
    SFP_PORT_ALL    = 1,
};

typedef struct {
    UINT32 macLink;
    UINT32 extPhyLink;
} S_PORT_LINK;

/*==========================================================================
 *                                                                          
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portStatusDump
 *
 *  DESCRIPTION :
 *      dump port staus
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port status
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
INT32 switch_portStatusDump
(
    IN UINT32 lPort,
    OUT S_SWITCH_PORT_STATUS *portStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortStatusDump
 *
 *  DESCRIPTION :
 *      dump cascade port staus
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port status
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
INT32 switch_cascadePortStatusDump
(
    IN UINT32 lPort,
    OUT S_SWITCH_PORT_STATUS *portStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSFPInfoDump
 *
 *  DESCRIPTION :
 *      dump port SFP information
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port sfp info
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
INT32 switch_portSFPInfoDump
(
    IN  UINT32                  lPort,
    OUT S_SWITCH_PORT_STATUS *  portStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSFPDiagMonitorTemp
 *
 *  DESCRIPTION :
 *      Show port SFP/SFP+ Diagnostic Monioring data of temperature
 *
 *  INPUT :
 *      lPort      - logical port
 *     portStatus - port sfp info
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
INT32 switch_portSFPDiagMonitorTemp
(
    IN  UINT32                  lPort,
    OUT S_SWITCH_PORT_STATUS *  portStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSFPSetRXSerdesToDAC
 *
 *  DESCRIPTION :
 *      read port SFP information if DAC7m and DAC10m and set RX serdes parameters
 *
 *  INPUT :
 *      lPort      - logical port
 *      portStatus - port sfp info
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
INT32 switch_portSFPSetRXSerdesToDAC
(
    IN  UINT32                  lPort,
    OUT S_SWITCH_PORT_STATUS *  portStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portCounterDump
 *
 *  DESCRIPTION :
 *      dump port counter
 *
 *  INPUT :
 *      lPort       - logical port
 *      portCounter - port statistic counter
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
INT32 switch_portCounterDump
(
    IN  UINT32          lPort,
    OUT S_PORT_CNT *    portCounter
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortCounterDump
 *
 *  DESCRIPTION :
 *      dump port counter
 *
 *  INPUT :
 *      lPort       - logical port
 *      portCounter - port statistic counter
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
INT32 switch_cascadePortCounterDump
(
    IN  UINT32          lPort,
    OUT S_PORT_CNT *    portCounter
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portLinkGet
 *
 *  DESCRIPTION :
 *      get port link status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      linkStatus - UP/DOWN
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
INT32 switch_portLinkGet
(
    IN UINT32 lPort,
    OUT UINT32 *linkStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSpeedGet
 *
 *  DESCRIPTION :
 *      get port speed
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      portSpeed - 1G/10G/40G
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
INT32 switch_portSpeedGet
(
    IN UINT32 lPort,
    OUT UINT32 *portSpeed
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSpeedSet
 *
 *  DESCRIPTION :
 *      set port speed
 *
 *  INPUT :
 *      lPort - logical port
 *      portSpeed - 1G/10G/40G
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
INT32 switch_portSpeedSet
(
    IN UINT32 lPort,
    IN UINT32 portSpeed
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portDuplexGet
 *
 *  DESCRIPTION :
 *      get port duplex
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      portDuplex - Half/Full
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
INT32 switch_portDuplexGet
(
    IN UINT32 lPort,
    OUT UINT32 *portDuplex
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portDuplexSet
 *
 *  DESCRIPTION :
 *      set port duplex
 *
 *  INPUT :
 *      lPort - logical port
 *      portDuplex - Half/Full
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
INT32 switch_portDuplexSet
(
    IN UINT32 lPort,
    IN UINT32 portDuplex
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portAutoNegGet
 *
 *  DESCRIPTION :
 *      get port autonego status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      portAutoneg - ON/OFF
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
INT32 switch_portAutoNegGet
(
    IN UINT32 lPort,
    OUT UINT32 *portAutoneg
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portAutoNegSet
 *
 *  DESCRIPTION :
 *      set port autonego status
 *
 *  INPUT :
 *      lPort - logical port
 *      portAutoneg - ON/OFF
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
INT32 switch_portAutoNegSet
(
    IN UINT32 lPort,
    IN UINT32 portAutoneg
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portForceLinkDownEnableGet
 *
 *  DESCRIPTION :
 *      get port admin status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      status - enable/disable
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
INT32 switch_portForceLinkDownEnableGet
(
    IN UINT32 lPort,
    OUT UINT8 *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portForceLinkDownEnableSet
 *
 *  DESCRIPTION :
 *      set port admin status
 *
 *  INPUT :
 *      lPort - logical port
 *      portEnable - enable/disable
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
INT32 switch_portForceLinkDownEnableSet
(
    IN UINT32 lPort,
    IN UINT8 portEnable
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portLoopbackGet
 *
 *  DESCRIPTION :
 *      get port loopback status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      loopbackStatus - MAC/PHY/NONE
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
INT32 switch_portLoopbackGet
(
    IN UINT32 lPort,
    OUT UINT32 *loopbackStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portLoopbackSet
 *
 *  DESCRIPTION :
 *      set port loopback status
 *
 *  INPUT :
 *      lPort - logical port
 *      loopbackStatus - MAC/PHY/NONE
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
INT32 switch_portLoopbackSet
(
    IN UINT32 lPort,
    IN UINT32 loopbackStatus
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFrameSizeGet
 *
 *  DESCRIPTION :
 *      Get port max frame size
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      frameSize - 1518 bytes ~ 16360 bytes
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
INT32 switch_portFrameSizeGet
(
	IN  UINT32  lPort,
	OUT  UINT32  *frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFrameSizeSet
 *
 *  DESCRIPTION :
 *      set port max frame size
 *
 *  INPUT :
 *      lPort - logical port
 *      frameSize - 1518 bytes ~ 16360 bytes
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
INT32 switch_portFrameSizeSet
(
	IN  UINT32  lPort,
	IN  UINT32  frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFrameSizeGet
 *
 *  DESCRIPTION :
 *      Get cascade port max frame size
 *
 *  INPUT :
 *      lPort - logical cascade port
 *
 *  OUTPUT :
 *      frameSize - 1518 bytes ~ 16360 bytes
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
INT32 switch_cascadePortFrameSizeGet
(
    IN  UINT32  lPort,
    OUT  UINT32  *frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFrameSizeSet
 *
 *  DESCRIPTION :
 *      Get cascade port max frame size
 *
 *  INPUT :
 *      lPort - cascade port
 *      frameSize - 1518 bytes ~ 16360 bytes
 *
 *  OUTPUT :
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
INT32 switch_cascadePortFrameSizeSet
(
    IN  UINT32  lPort,
    IN  UINT32  frameSize
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFlowControlGet
 *
 *  DESCRIPTION :
 *      get port flow control status
 *
 *  INPUT :
 *      lPort - logical port
 *
 *  OUTPUT :
 *      status - enabled/disabled
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
INT32 switch_portFlowControlGet
(
    IN  UINT32  lPort,
    OUT  UINT8  *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portFlowControlSet
 *
 *  DESCRIPTION :
 *      set port flow control status
 *
 *  INPUT :
 *      lPort - logical port
 *      status - enabled/disabled
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
INT32 switch_portFlowControlSet
(
    IN  UINT32  lPort,
    IN  UINT8  status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFlowControlGet
 *
 *  DESCRIPTION :
 *      Get cascade port flow control status
 *
 *  INPUT :
 *      lPort - logical cascade port
 *
 *  OUTPUT :
 *      status - enabled/disabled
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
INT32 switch_cascadePortFlowControlGet
(
    IN   UINT32  lPort,
    OUT  UINT8   *status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortFlowControlSet
 *
 *  DESCRIPTION :
 *      Get cascade port flow control status
 *
 *  INPUT :
 *      lPort - cascade port
 *      status - enabled/disabled
 *
 *  OUTPUT :
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
INT32 switch_cascadePortFlowControlSet
(
    IN  UINT32  lPort,
    IN  UINT8   status
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portClearCounter
 *
 *  DESCRIPTION :
 *      clear statistics of the specific port
 *
 *  INPUT :
 *      lPort - logical port
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
INT32 switch_portClearCounter
(
    IN UINT32 lPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_cascadePortClearCounter
 *
 *  DESCRIPTION :
 *      clear statistics of the cascade port
 *
 *  INPUT :
 *      lPort - logical port
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
INT32 switch_cascadePortClearCounter
(
    IN UINT32 lPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSfpRateSet
 *
 *  DESCRIPTION :
 *      port sfp+ rate select high or low
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
INT32 switch_portSfpRateSet
(
    IN UINT32 lPort,
    IN UINT32 rateLevel
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_portSfpRateTest
 *
 *  DESCRIPTION :
 *      Test sfp port rate select 
 *
 *  INPUT :
 *      lPort      - logical port
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
INT32 switch_portSfpRateTest
(
    IN  UINT32                  lPort
);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_PORT_H_ */
