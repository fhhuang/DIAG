/***************************************************************************
***
***    Copyright 2014  Hon Hai Precision Ind. Co. Ltd.
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
***            usb_hal.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2014/03/05, 16:02:52, Wed Chen
***             File Creation
***
***************************************************************************/
#ifndef __USB_TEST_H
#define __USB_TEST_H

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include "list.h"

/*==========================================================================
 *
 *      Macro Definition Segment
 *
 *==========================================================================
 */
#define NON_WIFI_TAG        "without"

#define STR_MAX_LENGTH  128
#define PROC_INFO_LENGTH    2048
#define CMD_MAX_LENGTH  60
#define DEV_NODE_LENGTH     15
#define BUFFER_MAX_LENGTH   4096
#define PORT_NUM_LENGTH     5
#define DEV_NAME_LENGTH     7

#define USB_NAND_BUS_NO     1
#define USB_NAND_PORT_NO    7
#define USB_BT_BUS_NO       5
#define USB_BT_PORT_NO      2
#define USB_TEST_RETRY      5


#define FILE_NAME_LENGTH    100
#define PORT_NUM_LEN        5
#define STRING_MAX_LENGTH   50
#define DEV_NAME_LEN        5

#define NODE_TAG          "max_sectors"    /* used for parse dmesg */

/* 20160701 H/W changed miniusb mux with gpio 6 */
#define MINI_USB_MUX_GPIO 6
/* Add miniUSB detect with gpio 18 */
#define MINI_USB_DETECT_GPIO 18


/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef struct {
    int busId;
    int level;
    int portId;
    char devInfo[DEV_NODE_LENGTH];
    struct list_head list;
} S_USB_PROBE_DEV_NODE;

typedef struct {
    int portId;                                                     /* port id */
    char dev[DEV_NAME_LEN];                                         /* logical device, sda, ... */
    int testResult;                                                 /* r/w test result, TRUE, FALSE */
    struct list_head list;
} S_USB_FLASH_NODE;

/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halCheckProcStat
 *
 *  DESCRIPTION :
 *      Check if USB mount into proc/bus/usb
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
INT32 usb_halCheckProcStat
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeListDump
 *
 *  DESCRIPTION :
 *      Dump node info in usb dev list
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
void usb_halProbeListDump
(
    IN S_USB_PROBE_DEV_NODE *pList
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeListAdd
 *
 *  DESCRIPTION :
 *      Add node to usb dev list
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
INT32 usb_halProbeListAdd
(
    IN int bus,
    IN int lev,
    IN int port,
    IN char *devInfo
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeDevInfo
 *
 *  DESCRIPTION :
 *      Probe current USB device and add them into the list
 *      Log is as below:
 *
 *        T:   Bus=01 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#= 2 Spd=12 Mxch= 0
 *        D:   Ver= 2.00 Cls=00(>ifc) Sub=00 Prot=00 MxPS=64 $Cfgs= 1
 *        P:   Vendor=10c4 ProdID=ea60 Rev= 1.00
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
INT32 usb_halProbeDevInfo
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halProbeTest
 *
 *  DESCRIPTION :
 *      probe USB devices
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
INT32 usb_halProbeTest
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halConnectTest
 *
 *  DESCRIPTION :
 *      Connect to USB devices
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
INT32 usb_halConnectTest
(
    UINT32 testPortId
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbLbTest
 *
 *  DESCRIPTION :
 *      Connect to mini USB devices
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
INT32 usb_halMiniUsbLbTest
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbDetectTest
 *
 *  DESCRIPTION :
 *      Check mini USB device is detected in GPIO18
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
INT32 usb_halMiniUsbDetectTest
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halPortEnable
 *
 *  DESCRIPTION :
 *      enable usb port
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
INT32 usb_halPortEnable
(
    UINT16 en
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usb_halMiniUsbMux
 *
 *  DESCRIPTION :
 *      enable mini usb port mux with gpio 6
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
INT32 usb_halMiniUsbMux
(
   UINT16 en
);
#endif

