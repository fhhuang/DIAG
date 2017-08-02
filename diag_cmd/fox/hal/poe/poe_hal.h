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
***         poe_hal.h
***
***    DESCRIPTION :
***         type, structure, and constant definiton for SCP packet used in Microsem's PoE module
***
***    HISTORY :
***       - 2010/01/05, Jungle Chen
***             File Creation
***
***************************************************************************/

#ifndef __POE_HAL_H__
#define __POE_HAL_H__


/*==========================================================================
 *
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
#include "err_type.h"

/*==========================================================================
 *
 *      Constant Definition Segment
 *
 *==========================================================================
 */
#ifdef POE_DBG_FLAG
#define POE_DBG_CHECK(x) x
#else
#define POE_DBG_CHECK(x)
#endif

#define POE_DELAY(x)      udelay(x) /*udelay(x), Eric */

#define POE_TIME_OUT        400
/* 20160822, updated Microsemi V2R2 FW v1.8.5 
 * 20160825, updated V2R2 FW v1.8.6 (23018612_0805_015.bin)
 * 20160907, downgrade V2R2 FW v1.8.2 unlocked for Cisco IOS
 * 20161018, upgraded FW v1.8.6 for Cisco IOS MR1
 * 20170106, downgrade V2R2 FW v1.8.2 unlocked for Cisco IOS
 */
#define POE_FW_FILE         "/sbin/23018225_0811_003.bin" 
/* #define POE_FW_FILE         "/sbin/23018612_0805_015.bin" */
#define POE_FW_SIZE         65536
#define POE_FW_LOAD_ADDR    0
#define POE_INIT_VAL        0xFF

#define POE_CHK_INIT(fd) \
    if (fd<0)           \
        return E_TYPE_IO_ERROR;

#define POE_DRAGONITE_FW_LOAD_ADDR_CNS      0
#define POE_ITCM_DIR                        0
#define POE_DTCM_DIR                        1
#define POE_DRAGONITE_IOC_MAGIC             'd'
#define POE_DRAGONITE_IOC_SETMEM_TYPE       _IOW(POE_DRAGONITE_IOC_MAGIC, 0, BOOL)
#define POE_DRAGONITE_IOC_UNRESET           _IOW(POE_DRAGONITE_IOC_MAGIC, 1, BOOL)
#define POE_DRAGONITE_IOC_SENDIRQ           _IOW(POE_DRAGONITE_IOC_MAGIC, 2, BOOL)

#define POE_DRAGONITE_DATA_MSG_LEN          15

/****************/
/*  RX / TX     */
/****************/
#define POE_TX_MO_ADDR                      0x50
#define POE_TX_MO_HOST_OWNERSHIP_CODE       0xA0
#define POE_TX_MO_POE_OWNERSHIP_CODE        0x0A

#define POE_RX_MO_ADDR                      0x100
#define POE_RX_MO_HOST_OWNERSHIP_CODE       0xB0
#define POE_RX_MO_POE_OWNERSHIP_CODE        0x0B

#define POE_TX_BUF_ADDR                     0x54
#define POE_RX_BUF_ADDR                     0x104

#define POE_PORT_NO_PER_DEV     12 /* the number of ports provided in a microsemi's 6912 device */
#define UPOE_PORT_NO_PER_DEV     8 /* the number of ports provided in a microsemi's 69208 device */

#define SCP_PKT_LEN          15
#define SCP_PKT_CHKSUM_LEN   13

#define SCP_MAX_PKT_WAIT     400  /* 400ms */ 
#define SCP_MAX_RESET_WAIT   3000 /* 3 seconds    */

#define SCP_COMMAND                 0x00
#define SCP_PROGRAM                 0x01
#define SCP_REQUEST                 0x02
#define SCP_TELEMETRY               0x03
#define SCP_CHANNEL                 0x05
#define SCP_E2                      0x06
#define SCP_GLOBAL                  0x07
#define SCP_PRIORITY                0x0A
#define SCP_SUPPLY                  0x0B
#define SCP_ENDIS                   0x0C
#define SCP_PORTSTATUS              0x0E
#define SCP_SAVECONFIG              0x0F
#define SCP_PRODUCTINFOZ            0x13
#define SCP_SUPPLY1                 0x15
#define SCP_SUPPLY2                 0x16
#define SCP_MAIN                    0x17
#define SCP_MEASUREMENTZ            0x1A
#define SCP_VERSIONZ                0x1E
#define SCP_SOFTWAREVERSION         0x21
#define SCP_PARAMZ                  0x25
#define SCP_MASKZ                   0x2B
#define SCP_RESTOREFACT             0x2D
#define SCP_PORTSSTATUS1            0x31
#define SCP_PORTSSTATUS2            0x32
#define SCP_PORTSSTATUS3            0x33
#define SCP_LATCHES                 0x3A
#define SCP_DETECTTEST              0x3C
#define SCP_SYSTEMSTATUS            0x3D
#define SCP_USERBYTE                0x41
#define SCP_TEMPORARYCHANNELMATRIX  0x43
#define SCP_CHANNELMATRIX           0x44
#define SCP_PORTSSTATUS4            0x47
#define SCP_PORTSSTATUS5            0x48
#define SCP_LATCHES2                0x49
#define SCP_PORTFULLINIT            0x4A
#define SCP_PORTSPOWER1             0x4B
#define SCP_PORTSPOWER2             0x4C
#define SCP_PORTSPOWER3             0x4D
#define SCP_PORTSPOWER4             0x4F
#define SCP_PORTSPOWER5             0x50
#define SCP_FORCEPOWER              0x51
#define SCP_REPORT                  0x52
#define SCP_RESET                   0x55
#define SCP_INDIVIDUAL_MASK         0x56
#define SCP_POWERBUDGET             0x57
#define SCP_UDLCOUNTER1             0x59
#define SCP_UDLCOUNTER2             0x5A
#define SCP_POEDEVICEVERSION        0x5E
#define SCP_POWERMANAGEMODE         0x5F
#define SCP_EXPENDEDPOWERINFO       0x60
#define SCP_ALLPORTCLASS            0x61
#define SCP_TMPSET                  0x62
#define SCP_IRQMASK                 0x63
#define SCP_ALLCHANNELS             0X80
#define SCP_DEVICESTATUS            0x87
#define SCP_FLASH                   0xFF

#define POE_REG_TEST_PATTER         0x55
#define POE_PD_POWER_UPPER_LIMIT    60                      /* The definition is from hardware */
#define POE_PD_POWER_LOWER_LIMIT    45 
#define POE_SCP_MAX_RETRY           1
#define POE_PD_DEFAULT_TYPE         1   /* standard */
#define POE_PD_DEFAULT_CLASS        0   /* class 0 */
#define MAX_POE_BANK                16
#define POE_DEFINE_TEST_MAX_CURRENT      600                /* The definition is from hardware */
#define POE_DEFINE_TEST_MIN_CURRENT      10
#define POE_DEFINE_TEST_MAX_CONSUMPTION  32400              /* The definition is from hardware */
#define POE_DEFINE_TEST_MIN_CONSUMPTION  1

#define POE_DETECTION_STAGE       (1<<0)
#define POE_CLASSIFICATION_STAGE  (1<<1)
#define POE_POWERUP_STAGE         (1<<2)
#define POE_DISCONNECT_STAGE      (1<<3)
#define POE_RESET_STAGE           (1<<4)
#define POE_VOPCHECK_STAGE       (1<<5)
#define POE_ALL_STAGES            (POE_DETECTION_STAGE|POE_CLASSIFICATION_STAGE|POE_POWERUP_STAGE|POE_DISCONNECT_STAGE|POE_RESET_STAGE|POE_VOPCHECK_STAGE)

#define POE_17W         17000
#define POE_7W           7000
#define POE_8W           8000
#define POE_18W         18000
#define POE_28W         28000

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef struct SCPPkt
{
    UINT8 key;
    UINT8 echo;
    UINT8 subject;
    UINT8 subject1;
    UINT8 subject2;
    UINT8 data[8];
    UINT8 csumh;
    UINT8 csuml;
} S_SCP_PKT;

typedef enum SCPRetCode
{
    E_SCP_SUCCESS          = 0,
    E_SCP_TIMEOUT_ERROR    = 1,
    E_SCP_ECHO_ERROR       = 2,
    E_SCP_CHECKSUM_ERROR   = 3,
    E_SCP_RE_SYNC_ERROR    = 4,
    E_SCP_STATUS_RECEIVE_ERROR = 5,
    E_SCP_STATUS_TRANSMIT_ERROR = 6,
} E_SCP_RET_CODE;

typedef enum POE_PDTYPE
{
    E_POE_PD_LEGACY          = 0,
    E_POE_PD_STANDARD    = 1
} E_POE_PD_TYPE;

typedef enum POE_SYSTEM_MASK_BIT_E
{
    E_BIT0_POWER_DISCONNECT    = 0,
    E_BIT1_CAPACITOR_DETECT    = 1
} E_POE_SYSTEM_MASK_BIT;

typedef struct POE_POWERINFO_S {
    UINT32 vmainVoltage;
    UINT32 caculatedCurrent;
    UINT32 powerConsumption;
    UINT32 vportVoltage;
}POE_POWERINFO_T;

typedef struct POE_BUDGE_S {
    UINT32 powerBank;
    UINT32 powerLimit;
    UINT32 powerMaxVoltage;
    UINT32 powerMinVoltage;
}POE_BUDGE_T;

/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktPrint
 *
 *  DESCRIPTION :
 *      a global API to print the content of a SCP packet, solely for debugging
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void poe_halSCPPktPrint
(
    IN S_SCP_PKT *pkt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktSyncLossHandler
 *
 *  DESCRIPTION :
 *      a API to do synchronization during communication loss
 *
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code 
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPPktSyncLossHandler
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktReceive
 *
 *  DESCRIPTION :
 *      receive a SCP packet
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *      max_wait - before receiving a SCP packet via i2c, how long will it hold on.
 *
 *  OUTPUT :
 *      *pkt - points to the received packet
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code 
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPPktReceive
(
    OUT S_SCP_PKT *pkt, 
    IN UINT32 max_wait
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPPktTransmit
 *
 *  DESCRIPTION :
 *      transmit a SCP packet
 *
 *
 *  INPUT :
 *      *pkt - points to a SCP packet
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPPktTransmit
(
    IN S_SCP_PKT *pkt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSCPDebugCmd
 *
 *  DESCRIPTION :
 *      a API for communicating with PoE system, which transmits a command/request SCP packet
 *      and then receives a report/telemetry SCP packet.
 *
 *
 *  INPUT :
 *
 *     pkt - points to a SCP packet, which will be transmitted
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_SCP_RET_CODE - error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
E_SCP_RET_CODE poe_halSCPDebugCmd 
(
    IN S_SCP_PKT *req_pkt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoECtrlStateGet
 *
 *  DESCRIPTION :
 *      a API to get the state of PoE module ctrl in CPLD
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      state - disable/enable
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoECtrlStateGet
(
    OUT BOOL *state
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoECtrlStateSet
 *
 *  DESCRIPTION :
 *      a API to set the state of PoE module ctrl in CPLD
 *
 *  INPUT :
 *      state - disable/enable
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoECtrlStateSet
(
    IN BOOL state
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSCPHandshake
 *
 *  DESCRIPTION :
 *      a API to do SCP handshaking
 *
 *
 *  INPUT :
 *      *pReqPkt - a request/command/program SCP packet
 *      
 *
 *  OUTPUT :
 *      *pRspPkt - a report/telemetry SCP packet
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSCPHandshake
(
    IN  S_SCP_PKT  *pReqPkt, 
    OUT S_SCP_PKT  *pRspPkt
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDragoniteRegRead
 *
 *  DESCRIPTION :
 *      Read value from Dragonite register
 *
 *  INPUT :
 *      addr - address of Dragonite
 *
 *  OUTPUT :
 *      val  - value
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDragoniteRegRead
(
    UINT32 addr, 
    UINT32 *val
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDragoniteRegWrite
 *
 *  DESCRIPTION :
 *      Write Dragonite register
 *
 *  INPUT :
 *      addr - address of Dragonite
 *      val  - value
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDragoniteRegWrite
(
    UINT32 addr, 
    UINT32 val
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDownloadFirmware
 *
 *  DESCRIPTION :
 *      Download dragonite fw from filesystem
 *
 *  INPUT :
 *      f_name
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDownloadFirmware
(
    char *f_name, 
    UINT32 size
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halLoadFirmware
 *
 *  DESCRIPTION :
 *      Load dragonite fw
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halLoadFirmware
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halInitDragonite
 *
 *  DESCRIPTION :
 *      Open dragonite fd
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void poe_halInitDragonite
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halGetSoftwareVersion
 *
 *  DESCRIPTION :
 *      Get software version
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halGetSoftwareVersion
(
    OUT UINT8 *softwareVersion
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halPowerShow
 *
 *  DESCRIPTION :
 *      Show the power info of a PoE port
 *
 *
 *  INPUT :
 *      lPort - the specify port to show
 *
 *  OUTPUT :
 *      powerInfo - power information
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPowerShow
(
    IN  UINT32  lPort,
    OUT POE_POWERINFO_T *powerInfo
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSetPortForcePowerOn
 *
 *  DESCRIPTION :
 *      Force enable or disable a PoE port.
 *
 *  INPUT :
 *      lPort - PoE port
 *      state - enable or disable
 *
 *  OUTPUT :
 *       none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetPortForcePowerOn
(
    IN  UINT32  lPort,
    IN  UINT8   forceState
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halSetPortMode
 *
 *  DESCRIPTION :
 *      Configure mode of a PoE port.
 *
 *  INPUT :
 *      lPort - PoE port
 *      af_mode - at or af
 *      state - enable or disable
 *
 *  OUTPUT :
 *       none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetPortMode
(
    UINT32 lPort,
    BOOL   af_mode,
    UINT8  state
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSaveSystemSetting
 *
 *  DESCRIPTION :
 *      a API to save system settings.
 *
 *  INPUT :
 *      none.
 *
 *  OUTPUT :
 *      none.
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSaveSystemSetting
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSetDeratingData
 *
 *  DESCRIPTION :
 *      a API to set the power derating parameters.
 *
 *  INPUT :
 *      POE_BUDGE_T - power derating parameters.
 *
 *
 *  OUTPUT :
 *      GT_OK; GT_FAIL
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetDeratingData
(
    IN  POE_BUDGE_T deratingData
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoESetPmMethod
 *
 *  DESCRIPTION :
 *      a API to set the power management mode of operation
 *
 *  INPUT :
 *      pm1, method to calculate system power
 *      pm2, port power limit
 *      pm3, start up condition
 *
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoESetPmMethod
(
    IN  UINT8 pm1,
    IN  UINT8 pm2,
    IN  UINT8 pm3
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halPoESetSystemMask
 *
 *  DESCRIPTION :
 *      a API to set the system mask for power management disconnect method 
 *      and capacitor detect
 *
 *  INPUT :
 *      maskBits
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPoESetSystemMask
(
    IN  UINT32  maskBits
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME:
 *      poe_halSetIndividualMask
 *
 *  DESCRIPTION :
 *      a API to set the individual mask
 *
 *  INPUT :
 *      maskKeyNum
 *      state - disable/enable
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halSetIndividualMask
(
    IN  UINT32  maskKeyNum,
    IN  UINT8   state
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halInit
 *
 *  DESCRIPTION :
 *      Init procedure of PoE
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      err_code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halInit
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDetectTest
 *
 *  DESCRIPTION :
 *      a API to test detection of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *      pdType - which type the PD under test belongs to
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDetectTest
(
    IN  UINT32  lPort, 
    IN  E_POE_PD_TYPE pdType
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halVopCheckTest
 *
 *  DESCRIPTION :
 *      a API to test vopcheck functionality of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halVopCheckTest 
(
    IN  UINT32  lPort,
    IN  UINT32  vop_margin
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halClassifyTest
 *
 *  DESCRIPTION :
 *      a API to test classification of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halClassifyTest
(
    IN  UINT32  lPort,
    IN  UINT32  pdClass
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halPowerUpTest
 *
 *  DESCRIPTION :
 *      a API to test powering of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halPowerUpTest
(
    IN  UINT32  lPort
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halDisconnetTest
 *
 *  DESCRIPTION :
 *      a API to test disconnection functionality of PoE
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halDisconnetTest 
(
    IN  UINT32  lPort
);


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halGetPdClass
 *
 *  DESCRIPTION :
 *      a API to get the classified pd class
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *     pd class
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halGetPdClass 
(
    IN     UINT32  lPort,
    OUT  UINT8   *pdclass
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      poe_halGetTotalPower
 *
 *  DESCRIPTION :
 *      a API to get total power consumption in watts
 *
 *
 *  INPUT :
 *      lPort - the logical port to test
 *
 *  OUTPUT :
 *     total power consumption in watts
 *
 *  RETURN :
 *      error code
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 poe_halGetTotalPower
(
    OUT  UINT32   *total_power
);

#endif
