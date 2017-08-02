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
***      switch_lb.c
***
***    DESCRIPTION :
***      for switch loopback test
***
***    HISTORY :
***       - 2009/05/12, 10:30:52, Eden Weng
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
#include "port_defs.h"
#include "switch_lb.h"
#include "err_type.h"
#include "port_utils.h"
#include "foxCommand.h"

#include "switch_hal.h"
#include "sys_utils.h"
#include "mcu_hal.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define DBG_PRINTF log_dbgPrintf
#define PORT_LB_PKT_LEN    1600
#define CPU_LB_PKT_ID      0x36

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
UINT32  g_pattern;
UINT32  g_lbPktId;
UINT32  g_lbTestStatus;
UINT32  g_lbRxCounter;
UINT8   g_pktBuf[PORT_LB_PKT_LEN+16];
static  BOOL    g_lineSpeedRxPkt[PKT_LINESPEED_MAX_NUM] = {0};

/* global variable */
UINT32 g_pktlen;

/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
static UINT32 dataCJPAT[376] =
{   0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e, 0x7e7e7e7e,
    0x7e7e7e7e, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,
    
};

/*==========================================================================
 *
 *      Local Function Body segment
 *
 *==========================================================================
 */
static void switchBuildTestPacket
(
    IN      UINT8 * srcMac,
    INOUT   UINT8 * pktBuf,
    IN      UINT32  pattern,
    IN      UINT32  pktLen,
    IN      UINT32  pktId
)
{
    UINT8   uniMac[6]   = { 0x00, 0x00, 0x12, 0x34, 0x56, 0x78 };
    UINT8   mcMac[6]    = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
    UINT32  selPattern, i, j;
    UINT8   *pktPattern, patternByteHigh=0, patternByteLow=0;
    UINT8   random_pkt;

    if ( mcMac || uniMac ) ;

    /* Fill DA */
    memcpy(&pktBuf[0], mcMac, 6);

    /* Fill SA */
    memcpy(&pktBuf[6], srcMac, 6);

    pktBuf[12] = pktLen >> 8;
    pktBuf[13] = pktLen;
    /* Add LLC protocol for gold switch */
    pktBuf[14] = 0x42;
    pktBuf[15] = 0x42;
    pktBuf[16] = 0x03;

    pktBuf[17] = (pktId >> 24) & 0xff;
    pktBuf[18] = (pktId >> 16) & 0xff;
    pktBuf[19] = (pktId >> 8) & 0xff;
    pktBuf[20] = pktId & 0xff;

    /* Fill the rest of data */
    if ( pattern <= 0xffff )
    {
        if(pattern == E_SW_LB_PATTERN_CJPAT)
        {
            pktPattern = (UINT8 *)dataCJPAT;
            for ( i = 21, j = 0; i < pktLen; i ++ )
            {
                pktBuf[i] = pktPattern[j];

                j++;
                if ( j >= sizeof(dataCJPAT) )
                    j = 0;
            }
        }
        else if(pattern == E_SW_LB_PATTERN_RANDOM)
        {
            for ( i = 21, j = 0; i < pktLen; i ++ )
            {
                random_pkt=(UINT8)rand();
                pktBuf[i] = random_pkt;
                j++;
                if ( j >= sizeof(UINT32) )
                    j = 0;
            }
        }
        else
        {
            if(pattern > 0xff)
            {
                patternByteLow = pattern & 0xff;
                patternByteHigh = (pattern>>8) & 0xff;
                for ( i = 21; i < pktLen; i++ )
                {
                    pktBuf[i] = patternByteHigh;
                    pktBuf[i+1] = patternByteLow;
                    i++;
                }
            }
            else
            {
                memset(pktBuf + 21, pattern, pktLen-21);
            }
        }
    }
    else
    {
        memset(pktBuf + 21, pattern, pktLen-21);
    }
}

INT32 switch_lbLinkCheck
(
    IN UINT32 txLPort,
    IN UINT32 rxLPort,
    IN E_LB_TEST_TYPE lbTestType
)
{
    INT8 portTypeStr[20]="N/A";
    INT32 ret = 0;
    UINT16 regValue=0;
    UINT32 startPortId, endPortId;
    UINT32 txPortPHYLink, txPortMACLink;
    UINT32 rxPortPHYLink, rxPortMACLink;
    E_LINER_PORT_TYPE startLPortType, endLPortType;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    /* Check valid lPort and portType */
    if( ( port_utilsLPort2PortTypeGet(txLPort, &startLPortType, &startPortId)) != E_TYPE_SUCCESS )
    {
        return E_TYPE_DATA_GET;
    }

    switch (startLPortType)
    {
        case E_LINER_PORT_TYPE_FIXED:
            strcpy(portTypeStr, "Port");
            break;
        case E_LINER_PORT_TYPE_STACKING:
            strcpy(portTypeStr, "Stacking Port");
            break;
        case E_LINER_PORT_TYPE_EXPANSION:
            return E_TYPE_UNSUPPORT_DEV;
        case E_LINER_PORT_TYPE_INTER_LINK:
            strcpy(portTypeStr, "Inter-Link Port");
            break;
        default:
            return E_TYPE_UNSUPPORT_DEV;
    }

    if(txLPort<=boardInfo.copperMaxNum)
    {
        if ( ((ret = switch_halPortLinkStatusGet(txLPort, &txPortPHYLink)) != E_TYPE_SUCCESS ) ||
             ((ret = switch_halPortMACLinkStatusGet(txLPort, &txPortMACLink)) != E_TYPE_SUCCESS ))
        {
            log_dbgPrintf("switch_halPortLinkStatusGet: fail, %ld\n", ret);
            return ret;
        }
    }
    else
    {
        if ( (ret = switch_halPortMACLinkStatusGet(txLPort, &txPortMACLink)) != E_TYPE_SUCCESS )
        {
            log_dbgPrintf("switch_halPortMACLinkStatusGet: fail, %ld\n", ret);
            return ret;
        }
    }

    switch (lbTestType)
    {
        case E_LB_TEST_TYPE_PHY:
        case E_LB_TEST_TYPE_MAC:
            /* For MAC and PHY loopback test, it checks MAC link status only. */
            txPortPHYLink = 1;
            break;
        default:
            break;
    }

    if(txLPort<=boardInfo.copperMaxNum)
    {
        if( (txPortPHYLink==0) || (txPortMACLink==0))
        {
            DBG_PRINTF("%s %d Link: MAC DOWN, PHY DOWN\n", portTypeStr, txLPort);
            return E_TYPE_PORT_LINK_DOWN;
        }
    }
    else
    {
        if( txPortMACLink==0 )
        {
            DBG_PRINTF("%s %d Link: MAC DOWN\n", portTypeStr, txLPort);
            return E_TYPE_PORT_LINK_DOWN;
        }        
    }

    if(txLPort<=boardInfo.copperMaxNum)
    {
        if( (txPortPHYLink==1) && (txPortMACLink==1))
        {
            DBG_PRINTF("%s %d Link: MAC UP, PHY UP\n", portTypeStr, txLPort);
        }
    }
    else
    {
        if( txPortMACLink==1 )
        {
            DBG_PRINTF("%s %d Link: MAC UP\n", portTypeStr, txLPort);
        }
    }

    if( txLPort != rxLPort )
    {
        if(rxLPort<=boardInfo.copperMaxNum)
        {
            if ( ((ret = switch_halPortLinkStatusGet(rxLPort, &rxPortPHYLink)) != E_TYPE_SUCCESS ) ||
                 ((ret = switch_halPortMACLinkStatusGet(rxLPort, &rxPortMACLink)) != E_TYPE_SUCCESS))
            {
                log_dbgPrintf("switch_halPortLinkStatusGet: fail, ret %ld\n", ret);
                return ret;
            }
        }
        else
        {
            if ((ret = switch_halPortMACLinkStatusGet(rxLPort, &rxPortMACLink)) != E_TYPE_SUCCESS)
            {
                log_dbgPrintf("switch_halPortLinkStatusGet: fail, ret %ld\n", ret);
                return ret;
            }
        }

        switch (lbTestType)
        {
            case E_LB_TEST_TYPE_PHY:
            case E_LB_TEST_TYPE_MAC:
                /* For MAC and PHY loopback test, it checks MAC link status only. */
                rxPortPHYLink = 1;
                break;
            default:
                break;
        }

        if(rxLPort<=boardInfo.copperMaxNum)
        {
            if( (rxPortMACLink==0) || (rxPortPHYLink==0) )
            {
                DBG_PRINTF("%s %d Link: MAC DOWN, PHY DOWN\n", portTypeStr, rxLPort);
                return E_TYPE_PORT_LINK_DOWN;
            }

            if( (rxPortPHYLink==1) && (rxPortMACLink==1))
            {
                DBG_PRINTF("%s %d Link: MAC UP, PHY UP\n", portTypeStr, rxLPort);
            }
        }
        else
        {
            if(rxPortMACLink==0)
            {
                DBG_PRINTF("%s %d Link: MAC DOWN\n", portTypeStr, rxLPort);
                return E_TYPE_PORT_LINK_DOWN;
            }

            if(rxPortMACLink==1)
            {
                DBG_PRINTF("%s %d Link: MAC UP\n", portTypeStr, rxLPort);
            }
        }
    }

    return E_TYPE_SUCCESS;
}

static GT_STATUS portlbtestCB
(
    IN UINT8      devNum,
    IN UINT8      queueIdx,
    IN UINT32     numOfBuff,
    IN UINT8      *packetBuffs[],
    IN UINT32     buffLen[],
    IN void       *rxParamsPtr
)
{
    volatile UINT8 *pkt_buf = packetBuffs[0];
    UINT8 *pktPattern, patternByteHigh=0, patternByteLow=0;
    UINT32 recvPktId = 0, size = (buffLen[0]-4);
    UINT32 i, j, selPattern;;

    recvPktId = (pkt_buf[17] << 24) + (pkt_buf[18] << 16) + (pkt_buf[19] << 8) + (pkt_buf[20]);

    if ( recvPktId == g_lbPktId )
    {
        if( g_pattern <= 0xffff )
        {
            if(g_pattern == E_SW_LB_PATTERN_RANDOM)
            {
            }
            else if(g_pattern == E_SW_LB_PATTERN_CJPAT)
            {
                pktPattern = (UINT8*)dataCJPAT;
                /* For Marvell AC3, the last 4 bytes will be modified. */
                size-=4;
                for(i=21, j=0; i < size ;i++)
                {
                    if( pkt_buf[i] != pktPattern[j] )
                    {
                        log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x, selPattern=%d\r\n", i, pkt_buf[i], pktPattern[j], selPattern);
                        g_lbTestStatus = 0;
                        return E_LB_CHECK_FAIL;
                    }
                    j++;
                    if( j >= sizeof(dataCJPAT) )
                    {
                        j = 0;
                    }
                }
            }
            else
            {
                if(g_pattern > 0xff)
                {
                    patternByteLow = g_pattern & 0xff;
                    patternByteHigh = (g_pattern>>8) & 0xff;

                    for(i=21; i < size ;i++)
                    {
                        if(i == size-1)
                        {
                            if(pkt_buf[i] != patternByteHigh)
                            {
                                log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], patternByteHigh);
                                g_lbTestStatus = 0;
                                return E_LB_CHECK_FAIL;
                             }
                        }
                        else
                        {
                            if( ( pkt_buf[i] != patternByteHigh) || ( pkt_buf[i+1] != patternByteLow) )
                            {
                                log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], patternByteHigh, pkt_buf[i+1], patternByteLow);
                                g_lbTestStatus = 0;
                                return E_LB_CHECK_FAIL;
                            }
                        }
                        i++;
                    }
                }
                else
                {
                    for(i=21; i < size ;i++)
                    {
                        if( pkt_buf[i] != (UINT8)g_pattern )
                        {
                            log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], g_pattern);
                            g_lbTestStatus = 0;
                            return E_LB_CHECK_FAIL;
                        }
                    }
                }
            }
        }
        else
        {
            for(i=21; i < size ;i++)
            {
                if( pkt_buf[i] != (UINT8)g_pattern )
                {
                    log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], g_pattern);
                    g_lbTestStatus = 0;
                    return E_LB_CHECK_FAIL;
                }
            }
        }
        g_lbTestStatus = 1;
        g_lbRxCounter++;
    }
    else
    {
        g_lbTestStatus = 0;
        return E_LB_CHECK_FAIL;
    }

    return E_LB_CHECK_SUCCESS;
}

static GT_STATUS portLinespeedTestCB
(
    IN UINT8      devNum,
    IN UINT8      queueIdx,
    IN UINT32     numOfBuff,
    IN UINT8      *packetBuffs[],
    IN UINT32     buffLen[],
    IN void       *rxParamsPtr
)
{
    volatile UINT8 *pkt_buf = packetBuffs[0];
    UINT8   *pktPattern, patternByteHigh=0, patternByteLow=0;
    UINT32   recvPktId = 0, size = (buffLen[0]-4);
    UINT32   i, j, selPattern;

    recvPktId = (pkt_buf[17]  << 24) +  (pkt_buf[18]  << 16)  +  (pkt_buf[19]  << 8)  +  (pkt_buf[20]);

    if( g_pattern <= 0xffff )
    {
        if(g_pattern == E_SW_LB_PATTERN_RANDOM)
        {
        }
        else if(g_pattern == E_SW_LB_PATTERN_CJPAT)
        {
            pktPattern = (UINT8*)dataCJPAT;
            /* For Marvell AC3, the last 4 bytes will be modified. */
            size-=4;
            for(i=21, j=0; i < size ;i++)
            {
                if( pkt_buf[i] != pktPattern[j] )
                {
                    log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x, selPattern=%d\r\n", i, pkt_buf[i], pktPattern[j], selPattern);
                    g_lbTestStatus = 0;
                    return E_LB_CHECK_FAIL;
                }
                j++;
                if( j >= sizeof(dataCJPAT) )
                {
                    j = 0;
                }
            }
        }
        else
        {
            if(g_pattern > 0xff)
            {
                patternByteLow = g_pattern & 0xff;
                patternByteHigh = (g_pattern>>8) & 0xff;
                for(i=21; i < size ;i++)
                {
                    if(i == size-1)
                    {
                        if(pkt_buf[i] != patternByteHigh)
                        {
                            log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], patternByteHigh);
                            g_lbTestStatus = 0;
                            return E_LB_CHECK_FAIL;
                        }
                    }
                    else
                    {
                        if( ( pkt_buf[i] != patternByteHigh) || ( pkt_buf[i+1] != patternByteLow) )
                        {
                            log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], patternByteHigh, pkt_buf[i+1], patternByteLow);
                            g_lbTestStatus = 0;
                            return E_LB_CHECK_FAIL;
                        }
                    }
                    i++;
                }
            }
            else
            {
                for(i=21; i < size ;i++)
                {
                    if( pkt_buf[i] != (UINT8)g_pattern )
                    {
                        log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], g_pattern);
                        g_lbTestStatus = 0;
                        return E_LB_CHECK_FAIL;
                    }
                }
            }
        }
    }
    else
    {
        for(i=21; i < size ;i++)
        {
            if( pkt_buf[i] != (UINT8)g_pattern )
            {
                log_dbgPrintf("Data mismatch, pkt_buf[%d]=%x, pattern=%x\r\n", i, pkt_buf[i], g_pattern);
                g_lbTestStatus = 0;
                return E_LB_CHECK_FAIL;
            }
        }
    }

    g_lineSpeedRxPkt[recvPktId] = TRUE;

    g_lbTestStatus = 1;
    g_lbRxCounter++;

    return E_LB_CHECK_SUCCESS;
}

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_lbTest
 *
 *  DESCRIPTION :
 *      switch loopback test
 *
 *  INPUT :
 *      txLPort      - logical port number of transmit
 *      rxLPort      - logical port number of receive
 *      lbTestType   - loopback test type
 *      pattern      - test pattern
 *      size         - test packet size
 *      numPkt       - test packet amount
 *      speed        - test speed
 *      issfp        - Flag to indicate port if SFP or not
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
INT32 switch_lbTest
(
    IN UINT32 txLPort,
    IN UINT32 rxLPort,
    IN E_LB_TEST_TYPE lbTestType,
    IN UINT32 pattern,
    IN UINT32 size,
    IN UINT32 numPkt,
    IN UINT32 speed,
    IN BOOL issfp,
    OUT S_PORT_CNT *txCounter,
    OUT S_PORT_CNT *rxCounter
)
{
    INT32 ret = E_TYPE_SUCCESS, retValue = E_TYPE_SUCCESS;
    UINT8 *pktBuf = g_pktBuf;
    UINT8 srcMac[MAC_ADDRESS_SIZE]={0x00, 0x02, 0x03, 0x04, 0x05, 0x06};
    UINT8 l2TestMac[MAC_ADDRESS_SIZE]={0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
    UINT16 regValue;
    UINT32 txPortPHYLink, txPortMACLink, data=0, sfp_port_index;
    UINT32 i, lbTxCounter = 0, rx_retry_count = 0;
    S_PORT_INFO *txPortInfo, *rxPortInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    txPortInfo=port_utilsLPortInfoGet(txLPort);
    if( txPortInfo == NULL )
    {
        log_dbgPrintf("%s get port %d map data fail\n", __FUNCTION__, txLPort);
        return E_TYPE_DATA_GET;
    }

    rxPortInfo=port_utilsLPortInfoGet(rxLPort);
    if( rxPortInfo == NULL )
    {
        log_dbgPrintf("%s get port %d map data fail\n", __FUNCTION__, rxLPort);
        return E_TYPE_DATA_GET;
    }

    g_pattern = pattern;

    if(txPortInfo->devId != rxPortInfo->devId)
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, TRUE);
        udelay(100);
        switch_halTrapStatusSet(rxPortInfo->devId, TRUE);
    }
    else
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, TRUE);
        udelay(100);
    }

    if( (ret=switch_halLbInit(txLPort, rxLPort, lbTestType, speed, issfp, portlbtestCB)) != E_TYPE_SUCCESS)
    {
        log_dbgPrintf("switch_halLbInit fail.");
        goto __FUN_RET;
    }

    udelay(50000); /* delay 50ms */

    /* check link status */
    for(i=0; i < LINK_CHECK_RETRY_COUNT ;i++)
    {
        udelay(1000000); /* delay 1000ms in extp case */
        if( (ret=switch_lbLinkCheck(txLPort, rxLPort, lbTestType)) == E_TYPE_SUCCESS )
        {
            break;
        }
    }

    if( i >= LINK_CHECK_RETRY_COUNT )
    {
       log_dbgPrintf("Port link retry timeout. Still link down.\r\n");
       ret = E_TYPE_PORT_LINK_DOWN;
       goto __FUN_RET;
    }

    /* check the link of the copper interface to fix the bug unplug loopback cable but copper interface
     * still pass , Eric Hsu
     */
    if ( (lbTestType == E_LB_TEST_TYPE_EXT_S) && ( !issfp ) )
    {
        switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
        switch_halSMIRegGet(txLPort, 0x1, &regValue);
        udelay(1000);
        switch_halSMIRegGet(txLPort, 0x1, &regValue);
        if ( (regValue & 0x4) == 0 ) /* Copper Status Register - copper link status */
        {
            log_dbgPrintf("copper not link, regValue=0x%x.\n", regValue);
            ret = E_TYPE_DATA_MISMATCH;
            goto __FUN_RET;
        }
    }

    if(txLPort >= boardInfo.firstfiberNum)
    {
        sfp_port_index = txLPort - boardInfo.copperMaxNum;

        if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
        {
            sfp_port_index = sfp_port_index+2;
		}

        data |= (SFP_LED_ACTIVITY | SFP_LED_GREEN_ON);
        data += sfp_port_index;
        /* Temporary workaround for SFP LED in case of traffic test */
        mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
    }

    if(txLPort != rxLPort)
    {
        data=0;
        if(rxLPort >= boardInfo.firstfiberNum)
        {
            sfp_port_index = rxLPort - boardInfo.copperMaxNum;

            if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
                ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
                ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
                ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
            {
	            sfp_port_index = sfp_port_index+2;
		    }
            data |= (SFP_LED_ACTIVITY | SFP_LED_GREEN_ON);
            data += sfp_port_index;
            /* Temporary workaround for SFP LED in case of traffic test */
            mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
        }
    }

    /* Prepare to build packet */
    switch_halMacAddrGet(srcMac);
    if(srcMac[0] & 0x1)
    {
        memcpy(&srcMac[0], l2TestMac, 6);
    }

    srcMac[5] += txLPort;

    g_lbRxCounter = 0;

    /* Must add delay here to wait phy port stable. */
    udelay(500000);

    if(pattern == E_SW_LB_PATTERN_CJPAT)
    {
        log_dbgPrintf("pattern-8 CJPAT is used\n");
    }
    else if(pattern == E_SW_LB_PATTERN_RANDOM)
    {
        log_dbgPrintf("True random pattern is used\n");
    }


    /*Make sure packet generator normally before test*/
    for(g_lbPktId=0; g_lbPktId < 10; g_lbPktId++)
    {
        /* Build test packet */
        switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

        /* Send packet to logical port */
        if( (ret=switch_halEthTx(txLPort, pktBuf, size)) < 0 )
        {
            log_dbgPrintf("Loopback test1 TX fail on packet%d\r\n", g_lbPktId);
            ret = E_TYPE_PORT_TX_ERROR;
            goto __FUN_RET;
        }

        lbTxCounter++;

        /* Add delay to wait packet received. */
        udelay(10000);

        while ( g_lbTestStatus != 1 && rx_retry_count++ < 20 )
        {
            udelay(10000); /* delay 10ms */
        }

        if( g_lbTestStatus != 1 )
        {
            continue;
        }

        g_lbTestStatus = 0;
        if ( rx_retry_count > 1 )
        {
            rx_retry_count = 0;
            continue;
        }
        
        break;
    }

    if (lbTxCounter == 10)
    {
       log_dbgPrintf("Loopback test1 TX retry 10 times fail\r\n");
       ret = E_TYPE_PORT_TX_ERROR;
       goto __FUN_RET;
    }      

    if( lbTestType == E_LB_TEST_TYPE_EXT_P )
    {
        g_lbRxCounter = 0;
        lbTxCounter = 0;
        srcMac[5] += rxLPort;
        
        /*Make sure packet generator normally before test*/
        for(g_lbPktId=0; g_lbPktId < 10; g_lbPktId++)
        {
            /* Build test packet */
            switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);
        
            /* Send packet to logical port */
            if( (ret=switch_halEthTx(rxLPort, pktBuf, size)) < 0 )
            {
                log_dbgPrintf("Loopback test2 TX fail on packet%d\r\n", g_lbPktId);
                ret = E_TYPE_PORT_TX_ERROR;
                goto __FUN_RET;
            }
        
            lbTxCounter++;
        
            /* Add delay to wait packet received. */
            udelay(10000);
        
            while ( g_lbTestStatus != 1 && rx_retry_count++ < 20 )
            {
                udelay(10000); /* delay 10ms */
            }
        
            if( g_lbTestStatus != 1 )
            {
                continue;
            }
        
            g_lbTestStatus = 0;
            if ( rx_retry_count > 1 )
            {
                rx_retry_count = 0;
                continue;
            }
            
            break;
        }
        
        if (lbTxCounter == 10)
        {
           log_dbgPrintf("Loopback test2 TX retry 10 times fail\r\n");
           ret = E_TYPE_PORT_TX_ERROR;
           goto __FUN_RET;
        }
    }

    /* Clear counter before test */
    if(switch_halClearPortCounter(txLPort) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to clear %d counter(Before)\r\n", txLPort);
        goto __FUN_RET;
    }

    /* Remain counter unclear on read. */
    if(switch_halPortCounterClearFlagSet(txLPort, FALSE) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set %d counter flag\r\n", txLPort);
        goto __FUN_RET;
    }

    if(rxLPort != txLPort)
    {
        if(switch_halClearPortCounter(rxLPort) != E_TYPE_SUCCESS)
        {
            log_printf("Failed to clear %d counter(Before)\r\n", rxLPort);
            goto __FUN_RET;
        }

        /* Remain counter unclear on read. */
        if(switch_halPortCounterClearFlagSet(rxLPort, FALSE) != E_TYPE_SUCCESS)
        {
            log_printf("Failed to set %d counter flag\r\n", rxLPort);
            goto __FUN_RET;
        }
    }


    g_lbRxCounter = 0;
    lbTxCounter = 0;
        
    for(g_lbPktId=0; g_lbPktId < numPkt ;g_lbPktId++)
    {
        /* Build test packet */
        switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

        /* Send packet to logical port */
        if( (ret=switch_halEthTx(txLPort, pktBuf, size)) < 0 )
        {
            log_dbgPrintf("Loopback test1 TX fail on packet%d\r\n", g_lbPktId);
            ret = E_TYPE_PORT_TX_ERROR;
            goto __FUN_RET;
        }

        lbTxCounter++;

        /* Add delay to wait packet received. */
        udelay(10000);

        while ( g_lbTestStatus != 1 && rx_retry_count++ < 20 )
        {
            udelay(10000); /* delay 10ms */
        }

        if( g_lbTestStatus != 1 )
        {
            log_dbgPrintf("Loopback test1 RX fail on packet%d\r\n", g_lbPktId);
            ret = E_TYPE_PORT_RX_ERROR;
            goto __FUN_RET;
        }

        g_lbTestStatus = 0;
        if ( rx_retry_count > 1 )
        {
            log_dbgPrintf("RX retry1 timeout\r\n");
            ret = E_TYPE_PORT_RX_TIMEOUT;
            rx_retry_count = 0;
        }
    }

    if( lbTestType == E_LB_TEST_TYPE_EXT_P )
    {
        g_lbRxCounter = 0;
        lbTxCounter = 0;
        srcMac[5] += rxLPort;

        for(g_lbPktId=0; g_lbPktId < numPkt ;g_lbPktId++)
        {
            /* Build test packet */
            switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

            /* Send packet to logical port */
            if( (ret=switch_halEthTx(rxLPort, pktBuf, size)) < 0 )
            {
                log_dbgPrintf("Loopback test2 TX fail on packet%d\r\n", g_lbPktId);
                ret = E_TYPE_PORT_TX_ERROR;
                goto __FUN_RET;
            }

            lbTxCounter++;

            /* Add delay to wait packet received. */
            udelay(10000);

            while ( g_lbTestStatus != 1 && rx_retry_count++ < 20 )
            {
                udelay(10000); /* delay 10ms */
            }

            if( g_lbTestStatus != 1 )
            {
                log_dbgPrintf("Loopback test2 RX fail on packet%d\r\n", g_lbPktId);
                ret = E_TYPE_PORT_RX_ERROR;
                goto __FUN_RET;
            }

            g_lbTestStatus = 0;
            if ( rx_retry_count > 1 )
            {
                log_dbgPrintf("RX retry2 timeout\r\n");
                ret = E_TYPE_PORT_RX_TIMEOUT;
                rx_retry_count = 0;
            }
        }
    }

    if( (ret != E_TYPE_SUCCESS) || (g_lbRxCounter != lbTxCounter) || (numPkt != lbTxCounter) || (g_lbRxCounter == 0) || (lbTxCounter == 0) )
    {        
        retValue = E_TYPE_DATA_MISMATCH;
    }
    else
    {
        retValue = E_TYPE_SUCCESS;
    }

    if(txPortInfo->devId != rxPortInfo->devId)
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, FALSE);
        udelay(10000);
        switch_halTrapStatusSet(rxPortInfo->devId, FALSE);
    }
    else
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, FALSE);
        udelay(10000);
    }

    /* delay 100ms, or the packet can't count correctly due to too fast. */
    udelay(1000000);
    
    /* Get txLPort and rxLPort statistic */
    if( switch_halPortCntGet(txLPort, txCounter) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halPortCntGet tx port fail.");
    }
    
    if( switch_halPortCntGet(rxLPort, rxCounter) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halPortCntGet rx port fail.");
    }

    if( switch_halLbReInit(txLPort, rxLPort) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halLbReInit fail.");
    }

    return retValue;

__FUN_RET:
    if(txPortInfo->devId != rxPortInfo->devId)
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, FALSE);
        udelay(10000);
        switch_halTrapStatusSet(rxPortInfo->devId, FALSE);
    }
    else
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, FALSE);
    }

    /* delay 100ms, or the packet can't count correctly due to too fast. */
    udelay(1000000);
    
    /* Get txLPort and rxLPort statistic */
    if( switch_halPortCntGet(txLPort, txCounter) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halPortCntGet tx port fail.");
    }
    
    if( switch_halPortCntGet(rxLPort, rxCounter) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halPortCntGet rx port fail.");
    }
    
    if( switch_halLbReInit(txLPort, rxLPort) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halLbReInit fail.");
    }
    
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_pktgenTest
 *
 *  DESCRIPTION :
 *      switch loopback test
 *
 *  INPUT :
 *      txLPort      - logical port number of transmit
 *      rxLPort      - logical port number of receive
 *      lbTestType   - loopback test type
 *      pattern      - test pattern
 *      size         - test packet size
 *      numPkt       - test packet amount
 *      speed        - test speed
 *      issfp        - Flag to indicate port if SFP or not
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
INT32 switch_pktgenTest
(
    IN UINT32 lPort,
    IN UINT32 pattern,
    IN UINT32 size,
    IN UINT32 numPkt,
    IN UINT32 speed
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT8 *pktBuf = g_pktBuf;
    UINT8 srcMac[MAC_ADDRESS_SIZE]={0x00, 0x02, 0x03, 0x04, 0x05, 0x06};
    UINT8 l2TestMac[MAC_ADDRESS_SIZE]={0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
    UINT32 i, lbTxCounter = 0, rx_retry_count = 0;

    /* Prepare to build packet */
    switch_halMacAddrGet(srcMac);
    if(srcMac[0] & 0x1)
    {
        memcpy(&srcMac[0], l2TestMac, 6);
    }

    srcMac[5] += lPort;
    
    g_lbRxCounter = 0;

    /* Must add delay here to wait phy port stable. */
    udelay(500000);

    if(pattern == E_SW_LB_PATTERN_CJPAT)
    {
        log_dbgPrintf("pattern-8 CJPAT is used\n");
    }
    else if(pattern == E_SW_LB_PATTERN_RANDOM)
    {
        log_dbgPrintf("True random pattern is used\n");
    }

    for(g_lbPktId=0; g_lbPktId < numPkt ;g_lbPktId++)
    {
        /* Build test packet */
        switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

        /* Send packet to logical port */
        if( (ret=switch_halEthTx(lPort, pktBuf, size)) < 0 )
        {
            log_dbgPrintf("Pktgen test TX fail on packet%d\r\n", g_lbPktId);
            ret = E_TYPE_PORT_TX_ERROR;
            goto __FUN_RET;
        }

        /* Add delay to wait packet received. */
        udelay(10000);
    }

__FUN_RET:
    return ret;
}

INT32 switch_linespeedTestStart
(
    IN  void
)
{
    INT32       ret = E_TYPE_SUCCESS;
    UINT32      devNum=0;

    for(devNum=0; devNum<port_utilsTotalDevGet(); devNum++)
    {
        ret = switch_halTrapStatusSet(devNum, FALSE);
    }
    return ret;
}

INT32 switch_linespeedTestStop
(
    IN  UINT32          txLPort,
    IN  UINT32          rxLPort,
    IN  E_LB_TEST_TYPE  lbTestType,
    IN  UINT32          numPkt
)
{
    INT32       ret, retValue = E_TYPE_SUCCESS;
    UINT32      devNum=0;

    /* Enable packet tarp to CPU */
    for(devNum=0; devNum<port_utilsTotalDevGet(); devNum++)
    {
        ret = switch_halTrapStatusSet(devNum, TRUE);
    }

    udelay(1000000); /* delay 1000ms, Eric Hsu 20110917 */

    for ( g_lbPktId = 0; g_lbPktId < numPkt; g_lbPktId ++ )
    {
        if ( g_lbRxCounter >= numPkt )
        {
            break;
        }

        g_lbTestStatus = 0;
    }

    udelay(200000); /* delay 200ms */

    if((lbTestType != E_LB_TEST_TYPE_EXT_P) && (lbTestType != E_LB_TEST_SNAKE_MODE_CPU))
    {
        log_dbgPrintf("Compare RX packet ... ");

        for ( g_lbPktId = 0; g_lbPktId < numPkt; g_lbPktId ++ )
        {
            if (txLPort == 0 && rxLPort == 0)
            {
                /* Only check if g_lbRxCounter == numPkt */
                ret = E_TYPE_SUCCESS;
            }
            else
            {
                if ( g_lineSpeedRxPkt[g_lbPktId] == FALSE )
                {
                    if ( ret == E_TYPE_SUCCESS )
                    {
                        log_dbgPrintf("fail\n");
                    }

                    ret = E_TYPE_DATA_MISMATCH;
                    log_dbgPrintf("Can't RX packet %d\n", g_lbPktId);
                }
            }
        }
    }
    else
    {
        ret = E_TYPE_SUCCESS;
    }

    if((lbTestType != E_LB_TEST_TYPE_EXT_P) && (lbTestType != E_LB_TEST_SNAKE_MODE_CPU))
    {
        if ( ret == E_TYPE_SUCCESS )
        {
            log_dbgPrintf("pass\n");
        }
    }
#if 0
    log_printf("Send %ld packets, receive %ld packets.\r\n", numPkt, g_lbRxCounter);
#endif
    if ( ret != E_TYPE_SUCCESS || g_lbRxCounter != numPkt || g_lbRxCounter == 0  )
    {
        retValue = E_TYPE_DATA_MISMATCH;
    }
    else
    {
        retValue = E_TYPE_SUCCESS;
    }

    for(devNum=0; devNum<port_utilsTotalDevGet(); devNum++)
    {
        ret = switch_halTrapStatusSet(devNum, FALSE);
    }

    /* Temporary workaround for SFP LED in case of traffic test */
    mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, 0x1);
    mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, 0x2);
    mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, 0x3);
    mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, 0x4);

    return retValue;
}

INT32 switch_linespeedTestInit
(
    IN  UINT32          txLPort,
    IN  UINT32          rxLPort,
    IN  E_LB_TEST_TYPE  lbTestType,
    IN  UINT32          pattern,
    IN  UINT32          size,
    IN  UINT32          numPkt,
    IN  UINT32          speed,
    IN  BOOL            issfp
)
{
    INT32       ret = E_TYPE_SUCCESS, retValue = E_TYPE_SUCCESS;
    UINT8 *     pktBuf = g_pktBuf, srcMac[6];
    UINT16      regValue=0;
    UINT32      lbTxCounter = 0, data=0, sfp_port_index=0;
    S_PORT_CNT  Counter;
    UINT32      i, dummyPktNum = 5;
    UINT32 txPortPHYLink, txPortMACLink;
    S_PORT_INFO *txPortInfo, *rxPortInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    txPortInfo=port_utilsLPortInfoGet(txLPort);
    if( txPortInfo == NULL )
    {
        log_dbgPrintf("%s get port %d map data fail\n", __FUNCTION__, txLPort);
        return E_TYPE_DATA_GET;
    }

    rxPortInfo=port_utilsLPortInfoGet(rxLPort);
    if( rxPortInfo == NULL )
    {
        log_dbgPrintf("%s get port %d map data fail\n", __FUNCTION__, rxLPort);
        return E_TYPE_DATA_GET;
    }

    g_pattern = pattern;

    if ( (ret = switch_halLbInit(txLPort, rxLPort, lbTestType, speed, issfp, portLinespeedTestCB)) < 0)
    {
        log_dbgPrintf("switch_halLbInit fail.\n");
        goto __FUN_RET;
    }

    /* check link status */
    for(i=0; i < LINK_CHECK_RETRY_COUNT ;i++)
    {
        udelay(1000000); /* delay 1000ms in extp case */
        if( (ret=switch_lbLinkCheck(txLPort, rxLPort, lbTestType)) == E_TYPE_SUCCESS )
        {
            break;
        }
    }

    if( i >= LINK_CHECK_RETRY_COUNT )
    {
        log_dbgPrintf("Port link retry timeout. Still link down.\r\n");
        ret = E_TYPE_PORT_LINK_DOWN;
        goto __FUN_RET;
    }

    /* check the link of the copper interface to fix the bug unplug loopback cable but copper interface
     * still pass , Eric Hsu
     */
    if ( (lbTestType == E_LB_TEST_TYPE_EXT_S) && ( !issfp ) )
    {
        switch_halSMIRegSet(txLPort, 0x16, 0);    /* change to page 0 */
        switch_halSMIRegGet(txLPort, 0x1, &regValue);
        udelay(1000);
        switch_halSMIRegGet(txLPort, 0x1, &regValue);
        if ( (regValue & 0x4) == 0 ) /* Copper Status Register - copper link status */
        {
            log_dbgPrintf("copper not link, regValue=0x%x.\n", regValue);
            ret = E_TYPE_DATA_MISMATCH;
            goto __FUN_RET;
        }
    }

    if(txLPort >= boardInfo.firstfiberNum)
    {
        sfp_port_index = txLPort - boardInfo.copperMaxNum;

        if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
        {
	        sfp_port_index = sfp_port_index+2;
		}

        data |= (SFP_LED_ACTIVITY | SFP_LED_GREEN_ON);
        data += sfp_port_index;
        /* Temporary workaround for SFP LED in case of traffic test */
        mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
    }

    if(txLPort != rxLPort)
    {
        data=0;
        if(rxLPort >= boardInfo.firstfiberNum)
        {
            sfp_port_index = rxLPort - boardInfo.copperMaxNum;

            if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
                ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
                ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
                ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
            {
	            sfp_port_index = sfp_port_index+2;
		    }
            data |= (SFP_LED_ACTIVITY | SFP_LED_GREEN_ON);
            data += sfp_port_index;
            /* Temporary workaround for SFP LED in case of traffic test */
            mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
        }
    }

    switch_halMacAddrGet(srcMac);
    srcMac[5] += txLPort;

    g_lbPktId = 0xffffffff;

    udelay(10000);

    /* enable PVE */
    if ( txLPort == rxLPort )
    {
        switch_halPortPVESet(txLPort, txLPort, TRUE);
    }
    else
    {
        switch_halPortPVESet(txLPort, rxLPort, TRUE);
        switch_halPortPVESet(rxLPort, txLPort, TRUE);
    }

    /* Remain counter unclear on read. */
    if(switch_halPortCounterClearFlagSet(txLPort, FALSE) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set %d counter flag\r\n", txLPort);
        goto __FUN_RET;
    }

    /* Clear counter before test */
    if(switch_halClearPortCounter(txLPort) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to clear %d counter\r\n", txLPort);
        goto __FUN_RET;
    }

    if(lbTestType==E_LB_TEST_TYPE_EXT_P)
    {
        if(switch_halPortCounterClearFlagSet(rxLPort, FALSE) != E_TYPE_SUCCESS)
        {
            log_printf("Failed to set %d counter flag\r\n", rxLPort);
            goto __FUN_RET;
        }

        if(switch_halClearPortCounter(rxLPort) != E_TYPE_SUCCESS)
        {
            log_printf("Failed to clear %d counter\r\n", rxLPort);
            goto __FUN_RET;
        }
    }

    udelay(200000); /* delay 20ms */

    g_lbRxCounter = 0;
    lbTxCounter = 0;
    memset(g_lineSpeedRxPkt, 0, sizeof(g_lineSpeedRxPkt));

    if(pattern == E_SW_LB_PATTERN_CJPAT)
    {
        log_dbgPrintf("pattern-8 CJPAT is used\n");
    }
    else if(pattern == E_SW_LB_PATTERN_RANDOM)
    {
        log_dbgPrintf("True random pattern is used\n");
    }

    for ( g_lbPktId = 0; g_lbPktId < numPkt; g_lbPktId ++ )
    {
        switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

        if ( (ret = switch_halEthTx(txLPort, pktBuf, size)) < 0 )
        {
            log_dbgPrintf("Linespeed test TX fail on packet%d\n", g_lbPktId);
            goto __FUN_RET;
        }

        if( lbTestType == E_LB_TEST_TYPE_EXT_P )
        {
            switch_halMacAddrGet(srcMac);
            srcMac[5] += rxLPort;

            /* Build test packet */
            switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

             /* Send packet to bcm logical port */
            if( (ret=switch_halEthTx(rxLPort, pktBuf, size)) < 0 )
            {
                log_dbgPrintf("Linespeed test TX fail on packet%d\n", g_lbPktId);
                goto __FUN_RET;
            }
        }

        lbTxCounter++;
    }

    return retValue;

__FUN_RET:
    if( switch_halLbReInit(txLPort, rxLPort) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("Failed to re-init switch port.\r\n");
    }

    return ret;
}

INT32 switch_linespeedTestReinit
(
    IN  UINT32          txLPort,
    IN  UINT32          rxLPort,
    IN  E_LB_TEST_TYPE  lbTestType
)
{
    INT32       ret = E_TYPE_SUCCESS;
    UINT32      i=0, regValue, devNum=0;

    if ( txLPort != rxLPort )
    {
        ret = switch_halPortPVESet(txLPort, rxLPort, FALSE);
        ret = switch_halPortPVESet(rxLPort, txLPort, FALSE);
    }
    else
    {
        ret = switch_halPortPVESet(txLPort, txLPort, FALSE);
    }

    if( (ret = switch_halLbReInit(txLPort, rxLPort)) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halLbReInit fail.\n");
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_lbSnakeTest
 *
 *  DESCRIPTION :
 *      switch snake test
 *
 *  INPUT :
 *      snakeMode    - snake test mode
 *      startPort    - logical port number of transmit
 *      endPort      - logical port number of receive
 *      pattern      - snake test pattern
 *      size         - packet size
 *      speed        - snake test speed
 *      numPkt       - test packet amount
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
INT32 switch_lbSnakeTest
(
    IN E_LB_TEST_SNAKE_MODE snakeMode,
    IN UINT32               startPort,
    IN UINT32               endPort,
    IN UINT32               pattern,
    IN UINT32               size,
    IN UINT32               speed,
    IN UINT32               numPkt
)
{
    INT32   ret = E_TYPE_SUCCESS, retValue = E_TYPE_SUCCESS;
    UINT8 * pktBuf = g_pktBuf+16 /* +16 is workaround */, srcMac[6];
    UINT32  lbTxCounter = 0, txLPort=0, rxLPort=0, port=0, i=0, devNum=0, data=0, sfp_port_index=0;
    UINT32 txPortPHYLink, txPortMACLink;
    BOOL    infiniteTX = FALSE;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    switch (snakeMode)
    {
        case E_LB_TEST_SNAKE_MODE_CPU:

            g_pattern = pattern;

            if ( (ret = switch_halSnakeLbTestInit(snakeMode, startPort, endPort, speed, portlbtestCB)) < E_TYPE_SUCCESS )
            {
                log_dbgPrintf("switch_halSnakeLbTestInit fail.\n");
                goto __FUN_RET;
            }

            udelay(500000); /* delay 500ms */

            for ( port = startPort; port <= endPort; port ++ )
            {
                txLPort = port;

                if(port<=port_utilsTotalFixedPortGet())
                {
                    if( port % 2 )
                    {
                        rxLPort = txLPort+1;
                    }
                    else
                    {
                        rxLPort = txLPort-1;
                    }
                }
                else
                {
                    rxLPort = txLPort;
                }

                /* check link status */
                for(i=0; i < LINK_CHECK_RETRY_COUNT ;i++)
                {
                    if( (ret=switch_lbLinkCheck(txLPort, rxLPort, snakeMode)) == E_TYPE_SUCCESS )
                    {
                        break;
                    }
                    udelay(1000000); /* delay 1000ms in extp case */
                }

                if( i >= LINK_CHECK_RETRY_COUNT )
                {
                    log_dbgPrintf("Port link retry timeout. Still link down.\r\n");
                    ret = E_TYPE_PORT_LINK_DOWN;
                    goto __FUN_RET;
                }   
                else
                {
                    if(snakeMode == E_LB_TEST_SNAKE_MODE_CPU)
                    {
                        if( (txLPort <= boardInfo.copperMaxNum) && (rxLPort <= boardInfo.copperMaxNum))
                        {
                            log_dbgPrintf("Port %d Link: MAC UP, PHY UP\n", txLPort);
                            log_dbgPrintf("Port %d Link: MAC UP, PHY UP\n", rxLPort);
                        }
                        else if( (txLPort >= boardInfo.firstfiberNum) && (rxLPort >= boardInfo.firstfiberNum))
                        {
                            log_dbgPrintf("Port %d Link: MAC UP\n", txLPort);
                            log_dbgPrintf("Port %d Link: MAC UP\n", rxLPort);
                        }
                        else
                        {
                            log_dbgPrintf("Port %d Link: MAC UP, PHY UP\n", txLPort);
                            log_dbgPrintf("Port %d Link: MAC UP, PHY UP\n", rxLPort);
                        }
                        port++;
                    }
                    else
                    {
                        log_dbgPrintf("Port %d Link: MAC UP, PHY UP\n", txLPort);
                    }
                }

                if(txLPort >= boardInfo.firstfiberNum)
                {
                    data=0;
                    sfp_port_index = txLPort - boardInfo.copperMaxNum;
    
                    if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
                        ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
                        ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
                        ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
                    {
   	        	        sfp_port_index = sfp_port_index+2;
    	        	}
                    data |= (SFP_LED_ACTIVITY | SFP_LED_GREEN_ON);
                    data += sfp_port_index;

                    /* Temporary workaround for SFP LED in case of traffic test */
                    mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
                }

                if(txLPort != rxLPort)
                {
                    data=0;
                    if(rxLPort >= boardInfo.firstfiberNum)
                    {
                        sfp_port_index = rxLPort - boardInfo.copperMaxNum;
                
                        if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
                            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
                            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
                            ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
                        {
	                        sfp_port_index = sfp_port_index+2;
		                }
                        data |= (SFP_LED_ACTIVITY | SFP_LED_GREEN_ON);
                        data += sfp_port_index;

                        /* Temporary workaround for SFP LED in case of traffic test */
                        mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
                    }
                }
            }

            if( (boardInfo.boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardInfo.boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
            {
                for(devNum=0;devNum<port_utilsTotalDevGet();devNum++)
                {
                    /* Check cascade port link status */
                    if ( (ret = switch_halCascadePortMACLinkStatusGet(devNum, 24, &txPortMACLink)) != E_TYPE_SUCCESS )
                    {
                        log_dbgPrintf("switch_halPortMACLinkStatusGet: fail, %ld\n", ret);
                        return ret;
                    }

                    if(txPortMACLink == 0)
                    {
                        log_dbgPrintf("Device %ld cascade port %ld link check timeout. MAC link down.\r\n", devNum, 24);
                    }

                    /* Check cascade port link status */
                    if ( (ret = switch_halCascadePortMACLinkStatusGet(devNum, 26, &txPortMACLink)) != E_TYPE_SUCCESS )
                    {
                        log_dbgPrintf("switch_halPortMACLinkStatusGet: fail, %ld\n", ret);
                        return ret;
                    }

                    if(txPortMACLink == 0)
                    {
                        log_dbgPrintf("Device %ld cascade port %ld link check timeout. MAC link down.\r\n", devNum, 26);
                    }
                }
            }

            switch_halMacAddrGet(srcMac);

            if ( numPkt == 0 )
            {
                infiniteTX = TRUE;
            }

            g_lbRxCounter = 0;

            if(pattern == E_SW_LB_PATTERN_CJPAT)
            {
                log_dbgPrintf("pattern-8 CJPAT is used\n");
            }
            else if(pattern == E_SW_LB_PATTERN_RANDOM)
            {
                log_dbgPrintf("True random pattern is used\n");
            }

            for ( g_lbPktId = 0; (g_lbPktId < numPkt) || (infiniteTX == TRUE); g_lbPktId ++ )
            {
                switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

                if ( (ret = switch_halEthTx(startPort, pktBuf, size)) < 0 )
                {
                    log_dbgPrintf("Snake test TX fail on packet%d, ret=%d\n", g_lbPktId, ret);
                    goto __FUN_RET;
                }

                udelay(50000);

                switch_halMacAddrGet(srcMac);
                srcMac[5] += (startPort+1);

                /* Build test packet */
                switchBuildTestPacket(srcMac, pktBuf, pattern, size, g_lbPktId);

                /* Send packet to bcm logical port */
                if( (ret=switch_halEthTx((startPort+1), pktBuf, size)) < 0 )
                {
                    log_dbgPrintf("Snake test TX fail on packet%d\n", g_lbPktId);
                    goto __FUN_RET;
                }
                lbTxCounter ++;
            }

            if ( ret != E_TYPE_SUCCESS )
            {
                retValue = E_TYPE_DATA_MISMATCH;
            }
            else
            {
                retValue = E_TYPE_SUCCESS;
            }

            if ( retValue < E_TYPE_SUCCESS )
            {
                goto __FUN_RET;
            }
            break;

        case E_LB_TEST_SNAKE_MODE_PKT_GEN:
            switch_halSnakeSet(startPort, endPort);
            break;

        default:
            return E_TYPE_INVALID_PARA;
    }

    return retValue;

__FUN_RET:
    if ( retValue < E_TYPE_SUCCESS )
    {
        return retValue;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_lbTcamBist
 *
 *  DESCRIPTION :
 *      switch build-in self test for TCAM
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
INT32 switch_lbTcamBist
(
    IN void
)
{
    INT32   ret = E_TYPE_SUCCESS, retValue = E_TYPE_SUCCESS;
    UINT8   *pktBuf = g_pktBuf;
    UINT32  lbTxCounter = 0, txLPort=0, rxLPort=0, port=0, i=0, devNum=0, speed=0;
    UINT32  rx_retry_count = 0;
    UINT32  txPortPHYLink, txPortMACLink, size=0, pattern=0, numPkt=0;
    UINT8   destMac[MAC_ADDRESS_SIZE]={0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
    BOOL    issfp=FALSE;
    S_BOARD_INFO boardInfo;
    S_PORT_INFO *txPortInfo, *rxPortInfo;
    E_LB_TEST_TYPE lbTestType;

    sys_utilsBoardInfoGet(&boardInfo);

    if( (boardInfo.boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardInfo.boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        txLPort=1;
        rxLPort=25;
    }
    else
    {
        txLPort=1;
        rxLPort=1;        
    }

    /* Lookup 0	PCL-ID mode 
     * port 0 trap ingress frames with DA=destMac
     */
    switch_halBistTcamConfigure(txLPort, destMac);
    if(txLPort != rxLPort)
    {
        switch_halBistTcamConfigure(rxLPort, destMac);
    }
#if 0
    txPortInfo=port_utilsLPortInfoGet(txLPort);
    if( txPortInfo == NULL )
    {
        log_dbgPrintf("%s get port %d map data fail\n", __FUNCTION__, txLPort);
        return E_TYPE_DATA_GET;
    }

    rxPortInfo=port_utilsLPortInfoGet(rxLPort);
    if( rxPortInfo == NULL )
    {
        log_dbgPrintf("%s get port %d map data fail\n", __FUNCTION__, rxLPort);
        return E_TYPE_DATA_GET;
    }

    if(txPortInfo->devId != rxPortInfo->devId)
    {
        /* Disable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, FALSE);
        udelay(100);
        switch_halTrapStatusSet(rxPortInfo->devId, FALSE);
    }
    else
    {
        /* Enable packet tarp to CPU */
        switch_halTrapStatusSet(txPortInfo->devId, FALSE);
        udelay(100);
    }

    lbTestType = E_LB_TEST_TYPE_MAC;
    speed = 1000;
    issfp = 0x0;
    size=64;
    numPkt=1;
    pattern = 0xff;
    g_pattern = pattern;

    if( (ret=switch_halLbInit(txLPort, rxLPort, lbTestType, speed, issfp, portlbtestCB)) != E_TYPE_SUCCESS)
    {
        log_dbgPrintf("switch_halLbInit fail.");
        goto __FUN_RET;
    }

    udelay(50000); /* delay 50ms */

    /* Prepare to build packet */
    g_lbRxCounter = 0;
    g_lbTestStatus = 0;

    /* Must add delay here to wait phy port stable. */
    udelay(500000);

    for(g_lbPktId=0; g_lbPktId < numPkt ;g_lbPktId++)
    {
        /* Build test packet */
        switchBuildTestPacket(destMac, pktBuf, pattern, size, g_lbPktId);

        /* Send packet to logical port */
        if( (ret=switch_halEthTx(txLPort, pktBuf, size)) < 0 )
        {
            log_dbgPrintf("Loopback test TX fail on packet%d\r\n", g_lbPktId);
            ret = E_TYPE_PORT_TX_ERROR;
            goto __FUN_RET;
        }

        lbTxCounter++;

        /* Add delay to wait packet received. */
        udelay(10000);

        while ( g_lbTestStatus != 1 && rx_retry_count++ < 20 )
        {
            udelay(10000); /* delay 10ms */
        }

        if( g_lbTestStatus != 1 )
        {
            log_dbgPrintf("Loopback test RX fail on packet%d\r\n", g_lbPktId);
            ret = E_TYPE_PORT_RX_ERROR;
            goto __FUN_RET;
        }

        g_lbTestStatus = 0;
        if ( rx_retry_count > 1 )
        {
            log_dbgPrintf("RX retry timeout\r\n");
            ret = E_TYPE_PORT_RX_TIMEOUT;
            rx_retry_count = 0;
        }
    }

    if( (ret != E_TYPE_SUCCESS) || (g_lbRxCounter != lbTxCounter) || (numPkt != lbTxCounter) || (g_lbRxCounter == 0) || (lbTxCounter == 0) )
    {
        retValue = E_TYPE_DATA_MISMATCH;
    }
    else
    {
        retValue = E_TYPE_SUCCESS;
    }

    if( switch_halLbReInit(txLPort, rxLPort) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halLbReInit fail.");
    }

    /* Reinit rule entry */
    switch_halBistTcamReinit(txLPort);
    if(txLPort != rxLPort)
    {
        switch_halBistTcamReinit(rxLPort);
    }

    return retValue;

__FUN_RET:
    if( switch_halLbReInit(txLPort, rxLPort) != E_TYPE_SUCCESS )
    {
        log_dbgPrintf("switch_halLbReInit fail.");
    }
#endif
    /* Reinit rule entry */
    switch_halBistTcamReinit(txLPort);
    if(txLPort != rxLPort)
    {
        switch_halBistTcamReinit(rxLPort);
    }

    return ret;
}
