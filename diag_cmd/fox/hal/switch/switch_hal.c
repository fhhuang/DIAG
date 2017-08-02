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
***      switch_hal.c
***
***    DESCRIPTION :
***      for switch hal
***
***    HISTORY :
***       - 2009/05/20, 16:30:52, Eden Weng
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
#include "switch_hal.h"
#include "switch_port.h"
#include "port_defs.h"
#include "port_utils.h"
#include "sys_utils.h"
#include "err_type.h"
#include "porting.h"
#include "mcu_hal.h"
#include "i2c_fpga.h"
#include "i2c_hal.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if_arp.h>
#include "log.h"
#include "foxCommand.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define MAX_LINK_RETRY              3
#define PHY_TIMEOUT_MS              10000
#define SFP_COUNTER_NUM             4

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
static S_PORT_CNT gSfpLedCurrentPacketCounter[SFP_COUNTER_NUM]={0}, gSfpLedPreviousPacketCounter[SFP_COUNTER_NUM]={0};
static RX_PACKET_RECEIVE_CB_FUN *g_lbRxCbFun = (RX_PACKET_RECEIVE_CB_FUN *)NULL;
static UINT8  board_mac_addr[MAC_ADDRESS_SIZE] = { 0x00, 0x90, 0xE5, 0x00, 0x00, 0x01 };
static UINT8  *buffList[1];
static UINT8  gSfpLedTaskFlag=FALSE, previousState[4]={0}, currentState[4]={0};
static UINT32 buffLenList[1]={1522};
static UINT32 gPclId=1;
static UINT8 boardName[8][8] = {"8T", "8P", "16T", "16P", "24T", "24P", "48T", "48P"};
/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
static unsigned __TASKCONV sfpPortPolling(GT_VOID * unused);

/*==========================================================================
 *
 *      Local Function Body segment
 *
 *==========================================================================
 */
static unsigned __TASKCONV sfpPortPolling
(
    GT_VOID * param
)
{
    UINT32       lPort=0, macLink=0, data=0, port_index=0;
    UINT16       regVal = 0;
    S_BOARD_INFO boardInfo;
    UINT8        counterStatus=0;

    sys_utilsBoardInfoGet(&boardInfo);

    memset(gSfpLedCurrentPacketCounter, 0, sizeof(gSfpLedCurrentPacketCounter));
    memset(gSfpLedPreviousPacketCounter, 0, sizeof(gSfpLedPreviousPacketCounter));

    while (1)
    {
        if (gSfpLedTaskFlag == TRUE)
        {
            for(lPort=boardInfo.firstfiberNum;lPort<=boardInfo.lPortMaxNum; lPort++)
            {
                data=0;

                /* Get all SFP port link status */
                switch_halPortMACLinkStatusGet(lPort, &macLink);

                port_index = lPort - boardInfo.copperMaxNum;

                currentState[port_index-1] = macLink;

                /* Light LED according to port link status */
                if(macLink == 0x1)
                {
                    data |= SFP_LED_GREEN_ON;

                    switch_halPortCntGet(lPort, &gSfpLedCurrentPacketCounter[port_index-1]);

                    if ( memcmp(&gSfpLedCurrentPacketCounter[port_index-1], &gSfpLedPreviousPacketCounter[port_index-1], sizeof(gSfpLedCurrentPacketCounter[port_index-1]))!=0 )
                    {
                        data |= SFP_LED_ACTIVITY;
                    }
                    else
                    {
                        data &= ~SFP_LED_ACTIVITY;
                    }
                }
                else
                {
                    data &= ~SFP_LED_GREEN_ON;
                    data &= ~SFP_LED_ACTIVITY;
                }

                /*Fix the 8P/T 16P/T sfp number is 3 & 4*/
                if( ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_T ) ||
                    ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_16G2G_P ) ||
                    ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_T ) ||
                    ( boardInfo.boardId == E_BOARD_ID_HAYWARDS_8G2G_P ) )
                {
                    port_index = port_index+2;
                }

                data += port_index;

                /* Configure MCU LED if MCU is not in reset state */
                if(currentState[(lPort - boardInfo.copperMaxNum-1)] != previousState[(lPort - boardInfo.copperMaxNum-1)])
                {
                    fpgaRegRead(FPGA_RESET_REG, &regVal);
                    if((regVal & FPGA_MCU_RESET_BIT) == FPGA_MCU_RESET_BIT)
                    {
                        /* Configure LED via MCU */
                        mcu_halDataSet(MCU_SET_LED, MCU_ADDR_HIGH, data);
                    }
                }
                memcpy(&gSfpLedPreviousPacketCounter[port_index-1], &gSfpLedCurrentPacketCounter[port_index-1], sizeof( S_PORT_CNT));
                previousState[(lPort - boardInfo.copperMaxNum-1)] = currentState[(lPort - boardInfo.copperMaxNum-1)];
            }
        }
        osTimerWkAfter(1000);
    }
}

INT32 halPhy88E1680MacLoopback(UINT8 devNum, UINT8 port, CPSS_PORT_SPEED_ENT speed, UINT8 enable)
{
    INT32  ret;
    UINT32 i;
    UINT16 reg;

    /* disable SMI auto-polling */
    ret = prvCpssDrvHwPpSetRegField(devNum,0x7004034,0,32,0x1C0);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }
    ret = prvCpssDrvHwPpSetRegField(devNum,0x9004034,0,32,0x1C0);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }

    if(speed == CPSS_PORT_SPEED_1000_E)
    {
        if(enable == TRUE)
        {
            /* start phy configuration */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 2);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 21, 0x1046);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 9, 0x1F00);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x9140);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 0, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & 0x8000) == 0 )
                {
                    break;
                }

                udelay(10);

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u reset time out at line %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x00FA);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 1, 0x0418);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 7, 0x20C);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0xA400) ) == 0xA400 )
                {
                    break;
                }

                udelay(10);

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at line %u \n", devNum, port, __LINE__);
                    return E_TYPE_DATA_GET;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x4);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0xA400) ) == 0xA400 )
                {
                    break;
                }

                udelay(10);

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at QSGMII %u \n", devNum, port, __LINE__);
                    return E_TYPE_DATA_GET;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            /* enable loopback, set reg 0_0 bit 14 = 1 */
            ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 0, &reg);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            reg |= (0x4000);
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, reg);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            udelay(10);

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x00FA);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 7, 0x200);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 1, 0x400);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x4);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0xA400) ) == 0xA400 )
                {
                    break;
                }

                udelay(10);

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at QSGMII %u \n", devNum, port, __LINE__);
                    return E_TYPE_DATA_SET;
                }
            }
            udelay(600);
        }
        else    /* disable 1000M loopback */
        {
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 2);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 21, 0x1046);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 9, 0xE00);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x9140);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
        }
    }
    else if(speed == CPSS_PORT_SPEED_100_E)
    {
        if(enable == TRUE)
        {
            /* start phy configuration */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 2);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 21, 0x1045);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0xA100);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            /* force port link good */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 16, 0x3470);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0x6400) ) == 0x6400 )
                {
                    break;
                }

                udelay(10);

                i++;

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at line %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x4);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0x6400) ) == 0x6400 )
                {
                    break;
                }

                udelay(10);

                i++;

                if(i==PHY_TIMEOUT_MS)
                {
                    cpssOsPrintf("MTL dev %u port %u time out at QSGMII %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            /* enable loopback, set reg 0_0 bit 14 = 1 */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x6100);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            udelay(100);

            /* disable force port link good */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 16, 0x3070);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x4);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0x6400) ) == 0x6400 )
                {
                    break;
                }

                udelay(10);

                i++;

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at QSGMII %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            udelay(500);
        }
        else /* disable */
        {
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 2);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 21, 0x1046);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x9140);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
        }
    }
    else if(speed == CPSS_PORT_SPEED_10_E)
    {
        if(enable == TRUE)
        {
            /* start phy configuration */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 2);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 21, 0x1044);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x8100);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            /* force port link good */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 16, 0x3470);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0x2400) ) == 0x2400 )
                {
                    break;
                }

                udelay(10);

                i++;

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at line %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x4);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0x2400) ) == 0x2400 )
                {
                    break;
                }

                udelay(10);

                i++;

                if(i==PHY_TIMEOUT_MS)
                {
                    cpssOsPrintf("MTL dev %u port %u time out at QSGMII %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            /* enable loopback, set reg 0_0 bit 14 = 1 */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x6100);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            udelay(100);

            /* disable force port link good */
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 16, 0x3070);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0x4);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            for(i=0;i<PHY_TIMEOUT_MS;i++)
            {
                ret = cpssDxChPhyPortSmiRegisterRead(devNum, port, 17, &reg);
                if(ret != E_TYPE_SUCCESS)
                {
                    return ret;
                }

                if( (reg & (0x2400) ) == 0x2400 )
                {
                    break;
                }

                udelay(10);

                i++;

                if(i==PHY_TIMEOUT_MS)
                {
                    log_printf("dev %u port %u time out at QSGMII %u \n", devNum, port, __LINE__);
                    return E_TYPE_SUCCESS;
                }
            }
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            udelay(1000);
        }
        else /* disable */
        {
            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 2);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 21, 0x1046);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 22, 0);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }

            ret = cpssDxChPhyPortSmiRegisterWrite(devNum, port, 0, 0x9140);
            if(ret != E_TYPE_SUCCESS)
            {
                return ret;
            }
        }
    }

    /* enable SMI auto-polling */
    ret = prvCpssDrvHwPpSetRegField(devNum,0x7004034,0,32,0x140);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }

    ret = prvCpssDrvHwPpSetRegField(devNum,0x9004034,0,32,0x140);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }

    return E_TYPE_SUCCESS;
}

/*==========================================================================
 *
 *      Static Funtion Body segment
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
)
{
    *flagStatus = gSfpLedTaskFlag;
    return E_TYPE_SUCCESS;
}

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
)
{
    gSfpLedTaskFlag = flagStatus;
    return E_TYPE_SUCCESS;
}

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
)
{
    INT32 ret;

    ret = cpssDrvPpHwRegBitMaskRead(device,0, regNum, 0xffffffff, (MV_U32 *)regValue);

    return ret;
}

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
    IN  UINT32      device,
    IN  UINT32      regNum,
    IN  UINT32      regValue
)
{
    INT32   ret;

    ret = cpssDrvPpHwRegBitMaskWrite(device,0, regNum, 0xffffffff, regValue);

    return ret;
}

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
    IN UINT32 lPort,
    IN UINT32 regNum,
    OUT UINT16 *regValue
)
{
    CPSS_PHY_SMI_INTERFACE_ENT smiInterface;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if(portInfo->portId > 15)
    {
        /* SMI interface #1 control 2nd phy address 0x8~0xF */
        smiInterface = CPSS_PHY_SMI_INTERFACE_1_E;
    }
    else
    {
        /* SMI interface #0 control first phy address 0x0~0x7, 0x8~0xF */
        smiInterface = CPSS_PHY_SMI_INTERFACE_0_E;
    }
    return cpssSmiRegisterReadShort(portInfo->devId, 0, smiInterface, portInfo->phyAddr, regNum, (MV_U16 *)regValue);
}

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
    IN UINT32 lPort,
    IN UINT32 regNum,
    IN UINT16 regValue
)
{
    CPSS_PHY_SMI_INTERFACE_ENT smiInterface;
    S_PORT_INFO *portInfo;
    INT32 ret = E_TYPE_SUCCESS;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* disable SMI auto-polling */
    ret = prvCpssDrvHwPpSetRegField(portInfo->devId,0x7004034,0,32,0x1C0);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }
    ret = prvCpssDrvHwPpSetRegField(portInfo->devId,0x9004034,0,32,0x1C0);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }

    if(portInfo->portId > 15)
    {
        /* SMI interface #1 control 2nd phy address 0x8~0xF */
        smiInterface = CPSS_PHY_SMI_INTERFACE_1_E;
    }
    else
    {
        /* SMI interface #0 control first phy address 0x0~0x7, 0x8~0xF */
        smiInterface = CPSS_PHY_SMI_INTERFACE_0_E;
    }

    ret = cpssSmiRegisterWriteShort(portInfo->devId, 0, smiInterface, portInfo->phyAddr, regNum, regValue);

    /* enable SMI auto-polling */
    ret = prvCpssDrvHwPpSetRegField(portInfo->devId,0x7004034,0,32,0x140);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }

    ret = prvCpssDrvHwPpSetRegField(portInfo->devId,0x9004034,0,32,0x140);
    if(ret != E_TYPE_SUCCESS)
    {
        return ret;
    }

    return ret;
}

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
    OUT UINT16      *regValue
)
{
    /* No XSMI interface used in Haywards */
    return E_TYPE_SUCCESS;
}

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
    IN UINT32   lPort,
    IN UINT32   devAddr,
    IN UINT32   regNum,
    IN UINT16   regValue
)
{
    /* No XSMI interface used in Haywards */
    return E_TYPE_SUCCESS;
}

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
)
{
    UINT16 regData;

    switch_halSMIRegGet(lPort, PHY_PAGE_REG_OFFSET, &regData);
    regData &= (~0xff);
    regData |= pageNum;
    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, regData);

    return E_TYPE_SUCCESS;
}

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
    IN UINT32 lPort,
    OUT UINT32 *speed
)
{
    S_PORT_INFO *portInfo;
    CPSS_PORT_SPEED_ENT mvSpeed;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    cpssDxChPortSpeedGet(portInfo->devId, portInfo->portId, &mvSpeed);

    if(mvSpeed == CPSS_PORT_SPEED_10_E)
    {
        *speed = 10;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_100_E)
    {
        *speed = 100;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_1000_E)
    {
        *speed = 1000;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_2500_E)
    {
        *speed = 2500;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_10000_E)
    {
        *speed = 10000;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_20000_E)
    {
        *speed = 20000;
    }
    else
    {
        log_printf("%s get port %d speed fail, mvSpeed %ld\n", __FUNCTION__, lPort, mvSpeed);
        return E_TYPE_DATA_GET;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    CPSS_PORTS_BMP_STC           initPortsBmp;
    CPSS_PORT_INTERFACE_MODE_ENT interface;
    CPSS_PORT_SPEED_ENT          mvSpeed;
    S_PORT_INFO                  *portInfo;
    UINT32                       dev=0;
    UINT16                       regValue;
    INT32                        ret = E_TYPE_SUCCESS;
    S_BOARD_INFO boardInfo;
    E_BOARD_ID  boardId;
    S_SWITCH_PORT_STATUS    portStatus;

    boardId = sys_utilsDevBoardIdGet();

    sys_utilsBoardInfoGet(&boardInfo);

    portInfo = port_utilsLPortInfoGet(lPort);

    if(speed == 10)
    {
        mvSpeed = CPSS_PORT_SPEED_10_E;
    }
    else if(speed == 100)
    {
        mvSpeed = CPSS_PORT_SPEED_100_E;
    }
    else if(speed == 1000)
    {
        mvSpeed = CPSS_PORT_SPEED_1000_E;
    }
    /* Add 10G speed support 2017-01-12 Lowell-Li */
    else if(speed == 10000)
    {
        mvSpeed = CPSS_PORT_SPEED_10000_E;
    }
    else
    {
        log_printf("Unsupport speed %ld.\r\n");
        return E_TYPE_UNSUPPORT;
    }

    if(lPort > boardInfo.copperMaxNum)
    {
        /* Add 10G speed support 2017-01-12 Lowell-Li */
        if(speed == 1000)
        {
            interface = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            mvSpeed = CPSS_PORT_SPEED_1000_E;
        }
        else
        {
            interface = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            mvSpeed = CPSS_PORT_SPEED_10000_E;
        }

        /* Configure port speed */
        memset(&initPortsBmp,0,sizeof(CPSS_PORTS_BMP_STC));

        CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp, portInfo->portId);

        ret = cpssDxChPortModeSpeedSet(portInfo->devId, initPortsBmp, TRUE, interface, mvSpeed);
        if (ret != GT_OK )
        {
            log_printf("Failed to set the speed to %dM\n", mvSpeed);
        }

        /* IMPORTANT!!!, can't set low than 300ms, the manual mode need more time to link up */
        udelay(300000);

        /* 04202017 -- modified the TX and ppm value for 10G speed port only*/
        if (speed == 10000)
        {
            /* Add by miscroft patch 2017-01-16 Lowell-Li*/
            cpssDxChPortSerdesPpmSet(portInfo->devId, portInfo->portId, 100);

            udelay(3000);
            /* 04122017 - For Haywards2 to fix tx serdes init parameter by 10G SFP ports */
            switch(boardId)
            {
                case E_BOARD_ID_HAYWARDS_24G4G_T:
                case E_BOARD_ID_HAYWARDS_24G4G_P:
                    /* 03282017 -- updated 24T/P  SFP+ ports tx serdes init
                     * update TX serdes with start laneNum 6 for only test lport
                     */
                    ret = switch_tuneTxManualSet((lPort - boardInfo.firstfiberNum)+6, 1, 0, 0x11, 0, 6);

                    /* Read SFP port lengh - DAC 10m/7m, and changed new RX serdes parameters with special vendor */
                    ret |= switch_portSFPSetRXSerdesToDAC(lPort, &portStatus);
                    break;
                case E_BOARD_ID_HAYWARDS_48G4G_T:
                case E_BOARD_ID_HAYWARDS_48G4G_P:
                    /* 03282017 -- updated 48T/P SFP+ ports tx serdes init
                      * update TX serdes with start laneNum 12 for only test lport
                      */
                    ret = switch_tuneTxManualSet((lPort - boardInfo.firstfiberNum)+12, 1, 0, 0x10, 0, 7);

                    /* Read SFP port lengh - DAC 10m/7m, and changed new RX serdes parameters with special vendor  */
                    ret |= switch_portSFPSetRXSerdesToDAC(lPort, &portStatus);
                    break;
                default:
                /* do nothing for other SKUs is 1G SFP ports */
                break;
            }
            udelay(3000);

            if( ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld TX/RX serdes, ret %ld.\r\n", lPort, ret);
                return ret;
            }
        }
    }
    else
    {
        if( (lbTestType == E_LB_TEST_TYPE_MAC) || (lbTestType == E_LB_TEST_TYPE_PHY) )
        {
            ret = cpssDxChPortDuplexAutoNegEnableSet(portInfo->devId, portInfo->portId, FALSE);
            if( ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld duplex and autoneg enable, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            ret = cpssDxChPortSpeedAutoNegEnableSet(portInfo->devId, portInfo->portId, FALSE);
            if(ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld speed and autoneg enable, ret %ld.\r\n", lPort, ret);
            }

            ret = cpssDxChPortDuplexModeSet(portInfo->devId, portInfo->portId, CPSS_PORT_FULL_DUPLEX_E);
            if(ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld duplex mode, ret %ld.\r\n", lPort, ret);
            }

            ret = cpssDxChPortSpeedSet(portInfo->devId, portInfo->portId, mvSpeed);
            if(ret != GT_OK)
            {
                log_printf("Failed to set port %ld speed, ret %ld.\r\n", lPort, ret);
            }

            ret = cpssDxChPortForceLinkPassEnableSet(portInfo->devId, portInfo->portId, TRUE);
            if( ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld force link pass enable, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            if(lbTestType == E_LB_TEST_TYPE_PHY)
            {
                ret = halPhy88E1680MacLoopback(portInfo->devId, portInfo->portId, mvSpeed, TRUE);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set port %ld phy MAC loopback, ret %ld.\r\n", lPort, ret);
                    return ret;
                }
            }

            switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
            switch_halSMIRegGet(lPort, 0x10, &regValue);
            regValue |= 0x8;
            switch_halSMIRegSet(lPort, 0x10, regValue);
        }
        else
        {
            ret = cpssDxChPortForceLinkPassEnableSet(portInfo->devId, portInfo->portId, FALSE);
            if( ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld force link pass enable, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            ret = cpssDxChPortDuplexAutoNegEnableSet(portInfo->devId, portInfo->portId, TRUE);
            if( ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld duplex and autoneg enable, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            ret = cpssDxChPortSpeedAutoNegEnableSet(portInfo->devId, portInfo->portId, TRUE);
            if( ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld speed and autoneg enable, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            ret = halPhy88E1680MacLoopback(portInfo->devId, portInfo->portId, mvSpeed, FALSE);
            if(ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld phy MAC loopback, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            udelay(1000);

            /* mac reset */
            ret = cpssDxChPortMacResetStateSet(portInfo->devId, portInfo->portId, TRUE);
            if(ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set MAC port %ld reset state, ret %ld.\r\n", lPort, ret);
                return ret;
            }

            udelay(1000);

            ret = cpssDxChPortMacResetStateSet(portInfo->devId, portInfo->portId, FALSE);
            if(ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set MAC port %ld reset state, ret %ld.\r\n", lPort, ret);
                return ret;
            }
            udelay(1000);

            switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
            regValue = PHY_RESET;
            switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, regValue);

            if(speed == 10)
            {
                switch_halSMIRegSet(lPort, 0x9, 0xc00);
                switch_halSMIRegSet(lPort, 0x4, 0x41);
                switch_halSMIRegSet(lPort, 0x0, 0x1300);               /* Advertise 10M-TX */

                /* wait the force speed to link */
                udelay(1000);
            }
            else if(speed == 100)
            {
                switch_halSMIRegSet(lPort, 0x9, 0xc00);/* 1000BASE T */
                switch_halSMIRegSet(lPort, 0x4, 0x101);
                switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x1340);

                /* 100M need more time to link up in force mode */
                udelay(50000);
            }
            else if(speed == 1000)
            {
                switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 2);    /* change to page 2 */
                switch_halSMIRegSet(lPort, 0x15, 0x1046);
                switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
                switch_halSMIRegSet(lPort, 0x9, 0xE00);
#if 0
                switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x9140);
#else
                switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x3300);
#endif

                if(lbTestType == E_LB_TEST_TYPE_EXT_S)
                {
                    ret = cpssDxChPortDuplexAutoNegEnableSet(portInfo->devId, portInfo->portId, FALSE);
                    if( ret != E_TYPE_SUCCESS)
                    {
                        log_printf("Failed to set port %ld duplex and autoneg enable, ret %ld.\r\n", lPort, ret);
                        return ret;
                    }

                    ret = cpssDxChPortSpeedAutoNegEnableSet(portInfo->devId, portInfo->portId, FALSE);
                    if( ret != E_TYPE_SUCCESS)
                    {
                        log_printf("Failed to set port %ld speed and autoneg enable, ret %ld.\r\n", lPort, ret);
                        return ret;
                    }

                    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 1);    /* change to page 1 */
                    switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x8140);
                    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 4);    /* change to page 4 */
                    switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x8140);
                    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
                    switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x8140);

                    /* Need to turn on stub test mode in PHY. */
                    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 6);    /* change to page 6 */
                    switch_halSMIRegGet(lPort, 0x12, &regValue);
                    regValue |= 0x08;
                    switch_halSMIRegSet(lPort, 0x12, regValue);
                }
                else
                {
                    /* Make sure stub test mode is disable in PHY. */
                    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 6);    /* change to page 6 */
                    switch_halSMIRegGet(lPort, 0x12, &regValue);
                    regValue &= ~(0x08);
                    switch_halSMIRegSet(lPort, 0x12, regValue);
                }
                /* IMPORTANT!!!, can't set low than 500ms, the manual mode need more time to link up */
                udelay(500000);
            }
            else
            {
                log_printf("Unsupport speed %ld.\r\n");
                return E_TYPE_UNSUPPORT;
            }

            switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
            switch_halSMIRegGet(lPort, 0x10, &regValue);
            regValue &= (~0x08);
            switch_halSMIRegSet(lPort, 0x10, regValue);

            switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
        }
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    S_PORT_INFO *portInfo;
    CPSS_PORT_DUPLEX_ENT mvDuplex;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    cpssDxChPortDuplexModeGet(portInfo->devId, portInfo->portId, &mvDuplex);
    if(mvDuplex == CPSS_PORT_FULL_DUPLEX_E)
    {
        *duplex = E_SWITCH_PORT_DUPLEX_FULL;
    }
    else if(mvDuplex == CPSS_PORT_HALF_DUPLEX_E)
    {
        *duplex = E_SWITCH_PORT_DUPLEX_HALF;
    }
    else
    {
        log_printf("%s get port %d duplex fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    CPSS_PORTS_BMP_STC           initPortsBmp;
    CPSS_PORT_INTERFACE_MODE_ENT interface;
    CPSS_PORT_SPEED_ENT          mvDuplex;
    S_PORT_INFO *portInfo;
    UINT32 dev=0;
    INT32 ret = E_TYPE_SUCCESS;

    portInfo = port_utilsLPortInfoGet(lPort);

    if(duplex == E_SWITCH_PORT_DUPLEX_HALF)
    {
        mvDuplex = CPSS_PORT_HALF_DUPLEX_E;
    }
    else if (duplex == E_SWITCH_PORT_DUPLEX_FULL)
    {
        mvDuplex = CPSS_PORT_FULL_DUPLEX_E;
    }

    cpssDxChPortDuplexModeSet(portInfo->devId, portInfo->portId, mvDuplex);

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortAllCntGet
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
INT32 switch_halPortAllCntGet
(
    IN  UINT32          lPort,
    OUT S_PORT_TX_CNT * txCnt,
    OUT S_PORT_RX_CNT * rxCnt,
    OUT S_PORT_ERR_CNT * errCnt
)
{
    INT32       ret;
    UINT32      tempCnt = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    cpssDxChPortMacCountersOnPortGet(portInfo->devId, portInfo->portId, &portMacCounterSetArray);

    if(portMacCounterSetArray.ucPktsSent.l[1] == 0)
    {
        txCnt->txUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsSent.l[0]);
    }
    else
    {
        txCnt->txUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsSent.l[0]) + (((UINT64)portMacCounterSetArray.ucPktsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.mcPktsSent.l[1] == 0)
    {
        txCnt->txMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsSent.l[0]);
    }
    else
    {
        txCnt->txMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsSent.l[0]) + (((UINT64)portMacCounterSetArray.mcPktsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.brdcPktsSent.l[1] == 0)
    {
        txCnt->txBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsSent.l[0]);
    }
    else
    {
        txCnt->txBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsSent.l[0]) + (((UINT64)portMacCounterSetArray.brdcPktsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.goodOctetsSent.l[1] == 0)
    {
        txCnt->txTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsSent.l[0]);
    }
    else
    {
        txCnt->txTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsSent.l[0]) + (((UINT64)portMacCounterSetArray.goodOctetsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.ucPktsRcv.l[1] == 0)
    {
        rxCnt->rxUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsRcv.l[0]);
    }
    else
    {
        rxCnt->rxUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.ucPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.mcPktsRcv.l[1] == 0)
    {
        rxCnt->rxMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsRcv.l[0]);
    }
    else
    {
        rxCnt->rxMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.mcPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.brdcPktsRcv.l[1] == 0)
    {
        rxCnt->rxBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsRcv.l[0]);
    }
    else
    {
        rxCnt->rxBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.brdcPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.goodOctetsRcv.l[1] == 0)
    {
        rxCnt->rxTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsRcv.l[0]);
    }
    else
    {
        rxCnt->rxTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsRcv.l[0]) + (((UINT64)portMacCounterSetArray.goodOctetsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.badOctetsRcv.l[1] == 0)
    {
        errCnt->rxBadBytes = ((UINT32)portMacCounterSetArray.badOctetsRcv.l[0]);
    }
    else
    {
        errCnt->rxBadBytes = ((UINT32)portMacCounterSetArray.badOctetsRcv.l[0]) + (((UINT64)portMacCounterSetArray.badOctetsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.macTransmitErr.l[1] == 0)
    {
        errCnt->txCrcNum = ((UINT32)portMacCounterSetArray.macTransmitErr.l[0]);
    }
    else
    {
        errCnt->txCrcNum = ((UINT32)portMacCounterSetArray.macTransmitErr.l[0]) + (((UINT64)portMacCounterSetArray.macTransmitErr.l[1]) << 32);
    }

    if(portMacCounterSetArray.badPktsRcv.l[1] == 0)
    {
        errCnt->rxOverRunNum = ((UINT32)portMacCounterSetArray.badPktsRcv.l[0]);
    }
    else
    {
        errCnt->rxOverRunNum = ((UINT32)portMacCounterSetArray.badPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.badPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.undersizePkts.l[1] == 0)
    {
        errCnt->rxUnderSizeNum = ((UINT32)portMacCounterSetArray.undersizePkts.l[0]);
    }
    else
    {
        errCnt->rxUnderSizeNum = ((UINT32)portMacCounterSetArray.undersizePkts.l[0]) + (((UINT64)portMacCounterSetArray.undersizePkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.fragmentsPkts.l[1] == 0)
    {
        errCnt->rxFragmentsNum = ((UINT32)portMacCounterSetArray.fragmentsPkts.l[0]);
    }
    else
    {
        errCnt->rxFragmentsNum = ((UINT32)portMacCounterSetArray.fragmentsPkts.l[0]) + (((UINT64)portMacCounterSetArray.fragmentsPkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.oversizePkts.l[1] == 0)
    {
        errCnt->rxOverSizeNum = ((UINT32)portMacCounterSetArray.oversizePkts.l[0]);
    }
    else
    {
        errCnt->rxOverSizeNum = ((UINT32)portMacCounterSetArray.oversizePkts.l[0]) + (((UINT64)portMacCounterSetArray.oversizePkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.jabberPkts.l[1] == 0)
    {
        errCnt->rxJabbberNum = ((UINT32)portMacCounterSetArray.jabberPkts.l[0]);
    }
    else
    {
        errCnt->rxJabbberNum = ((UINT32)portMacCounterSetArray.jabberPkts.l[0]) + (((UINT64)portMacCounterSetArray.jabberPkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.macRcvError.l[1] == 0)
    {
        errCnt->rxErrorNum = ((UINT32)portMacCounterSetArray.macRcvError.l[0]);
    }
    else
    {
        errCnt->rxErrorNum = ((UINT32)portMacCounterSetArray.macRcvError.l[0]) + (((UINT64)portMacCounterSetArray.macRcvError.l[1]) << 32);
    }

    if(portMacCounterSetArray.badCrc.l[1] == 0)
    {
        errCnt->rxCrcNum = ((UINT32)portMacCounterSetArray.badCrc.l[0]);
    }
    else
    {
        errCnt->rxCrcNum = ((UINT32)portMacCounterSetArray.badCrc.l[0]) + (((UINT64)portMacCounterSetArray.badCrc.l[1]) << 32);
    }

    return 0;
}

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
)
{
    INT32       ret;
    UINT32      tempCnt = 0, devId=0, phyPort=0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
    S_PORT_INFO *portInfo;

    if(lPort == 1)
    {
        devId = 1;
        phyPort = 24;
    }
    else if(lPort == 2)
    {
        devId = 1;
        phyPort = 26;
    }
    else if(lPort == 3)
    {
        devId = 0;
        phyPort = 24;
    }
    else if(lPort == 4)
    {
        devId = 0;
        phyPort = 26;
    }
    else
    {
        return E_TYPE_DATA_GET;
    }

    cpssDxChPortMacCountersOnPortGet(devId, phyPort, &portMacCounterSetArray);

    if(portMacCounterSetArray.ucPktsSent.l[1] == 0)
    {
        txCnt->txUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsSent.l[0]);
    }
    else
    {
        txCnt->txUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsSent.l[0]) + (((UINT64)portMacCounterSetArray.ucPktsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.mcPktsSent.l[1] == 0)
    {
        txCnt->txMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsSent.l[0]);
    }
    else
    {
        txCnt->txMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsSent.l[0]) + (((UINT64)portMacCounterSetArray.mcPktsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.brdcPktsSent.l[1] == 0)
    {
        txCnt->txBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsSent.l[0]);
    }
    else
    {
        txCnt->txBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsSent.l[0]) + (((UINT64)portMacCounterSetArray.brdcPktsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.goodOctetsSent.l[1] == 0)
    {
        txCnt->txTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsSent.l[0]);
    }
    else
    {
        txCnt->txTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsSent.l[0]) + (((UINT64)portMacCounterSetArray.goodOctetsSent.l[1]) << 32);
    }

    if(portMacCounterSetArray.ucPktsRcv.l[1] == 0)
    {
        rxCnt->rxUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsRcv.l[0]);
    }
    else
    {
        rxCnt->rxUnicastNum = ((UINT32)portMacCounterSetArray.ucPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.ucPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.mcPktsRcv.l[1] == 0)
    {
        rxCnt->rxMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsRcv.l[0]);
    }
    else
    {
        rxCnt->rxMulticastNum = ((UINT32)portMacCounterSetArray.mcPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.mcPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.brdcPktsRcv.l[1] == 0)
    {
        rxCnt->rxBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsRcv.l[0]);
    }
    else
    {
        rxCnt->rxBroadcastNum = ((UINT32)portMacCounterSetArray.brdcPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.brdcPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.goodOctetsRcv.l[1] == 0)
    {
        rxCnt->rxTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsRcv.l[0]);
    }
    else
    {
        rxCnt->rxTotalBytes = ((UINT32)portMacCounterSetArray.goodOctetsRcv.l[0]) + (((UINT64)portMacCounterSetArray.goodOctetsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.badOctetsRcv.l[1] == 0)
    {
        errCnt->rxBadBytes = ((UINT32)portMacCounterSetArray.badOctetsRcv.l[0]);
    }
    else
    {
        errCnt->rxBadBytes = ((UINT32)portMacCounterSetArray.badOctetsRcv.l[0]) + (((UINT64)portMacCounterSetArray.badOctetsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.macTransmitErr.l[1] == 0)
    {
        errCnt->txCrcNum = ((UINT32)portMacCounterSetArray.macTransmitErr.l[0]);
    }
    else
    {
        errCnt->txCrcNum = ((UINT32)portMacCounterSetArray.macTransmitErr.l[0]) + (((UINT64)portMacCounterSetArray.macTransmitErr.l[1]) << 32);
    }

    if(portMacCounterSetArray.badPktsRcv.l[1] == 0)
    {
        errCnt->rxOverRunNum = ((UINT32)portMacCounterSetArray.badPktsRcv.l[0]);
    }
    else
    {
        errCnt->rxOverRunNum = ((UINT32)portMacCounterSetArray.badPktsRcv.l[0]) + (((UINT64)portMacCounterSetArray.badPktsRcv.l[1]) << 32);
    }

    if(portMacCounterSetArray.undersizePkts.l[1] == 0)
    {
        errCnt->rxUnderSizeNum = ((UINT32)portMacCounterSetArray.undersizePkts.l[0]);
    }
    else
    {
        errCnt->rxUnderSizeNum = ((UINT32)portMacCounterSetArray.undersizePkts.l[0]) + (((UINT64)portMacCounterSetArray.undersizePkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.fragmentsPkts.l[1] == 0)
    {
        errCnt->rxFragmentsNum = ((UINT32)portMacCounterSetArray.fragmentsPkts.l[0]);
    }
    else
    {
        errCnt->rxFragmentsNum = ((UINT32)portMacCounterSetArray.fragmentsPkts.l[0]) + (((UINT64)portMacCounterSetArray.fragmentsPkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.oversizePkts.l[1] == 0)
    {
        errCnt->rxOverSizeNum = ((UINT32)portMacCounterSetArray.oversizePkts.l[0]);
    }
    else
    {
        errCnt->rxOverSizeNum = ((UINT32)portMacCounterSetArray.oversizePkts.l[0]) + (((UINT64)portMacCounterSetArray.oversizePkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.jabberPkts.l[1] == 0)
    {
        errCnt->rxJabbberNum = ((UINT32)portMacCounterSetArray.jabberPkts.l[0]);
    }
    else
    {
        errCnt->rxJabbberNum = ((UINT32)portMacCounterSetArray.jabberPkts.l[0]) + (((UINT64)portMacCounterSetArray.jabberPkts.l[1]) << 32);
    }

    if(portMacCounterSetArray.macRcvError.l[1] == 0)
    {
        errCnt->rxErrorNum = ((UINT32)portMacCounterSetArray.macRcvError.l[0]);
    }
    else
    {
        errCnt->rxErrorNum = ((UINT32)portMacCounterSetArray.macRcvError.l[0]) + (((UINT64)portMacCounterSetArray.macRcvError.l[1]) << 32);
    }

    if(portMacCounterSetArray.badCrc.l[1] == 0)
    {
        errCnt->rxCrcNum = ((UINT32)portMacCounterSetArray.badCrc.l[0]);
    }
    else
    {
        errCnt->rxCrcNum = ((UINT32)portMacCounterSetArray.badCrc.l[0]) + (((UINT64)portMacCounterSetArray.badCrc.l[1]) << 32);
    }

    return 0;
}

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
    IN UINT32 lPort,
    OUT S_PORT_CNT *portCnt
)
{
    switch_halPortAllCntGet(lPort, &portCnt->txCnt, &portCnt->rxCnt, &portCnt->errCnt);
    return 0;
}

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
)
{
    switch_halCascadePortAllCntGet(lPort, &portCnt->txCnt, &portCnt->rxCnt, &portCnt->errCnt);
    return 0;
}

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
)
{
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    cpssDxChBrgGenArpTrapEnable(portInfo->devId, portInfo->portId, status);

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortEnableGet
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
)
{
    E_LINER_PORT_TYPE portType;
    INT32  rc=E_TYPE_SUCCESS;
    UINT32 temp;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* CPSS GT_BOOL is UINT32 */
    rc = cpssDxChPortForceLinkDownEnableGet(portInfo->devId, portInfo->portId, &temp);
    *status = temp;

    return rc;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halPortEnableSet
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
)
{
    S_PORT_INFO *portInfo;
    UINT8       forceLinkDownStatus=0;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if(status == E_SWITCH_PORT_ADMIN_ON)
    {
        forceLinkDownStatus = 0;
    }
    else
    {
        forceLinkDownStatus = 1;
    }

    cpssDxChPortForceLinkDownEnableSet(portInfo->devId, portInfo->portId, forceLinkDownStatus);

    return E_TYPE_SUCCESS;
}

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
)
{
    INT32 ret;
    UINT32 portId, loopback_mode;
    S_PORT_INFO *portInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    switch (loopback)
    {
        case E_LB_TEST_TYPE_MAC:
            if(lPort > boardInfo.copperMaxNum)
            {
                ret = cpssDxChPortInternalLoopbackEnableSet(portInfo->devId, portInfo->portId, FALSE);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set internal loopback, ret %ld.\r\n", ret);
                }
                ret = cpssDxChPortSerdesLoopbackModeSet(portInfo->devId, portInfo->portId, 0xffffffff, CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set serdes loopback mode, ret %ld.\r\n", ret);
                }
                ret = cpssDxChPortSerdesTxEnableSet(portInfo->devId, portInfo->portId, TRUE);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set internal loopback, ret %ld.\r\n", ret);
                }
            }
            else
            {
                ret = cpssDxChPortInternalLoopbackEnableSet(portInfo->devId, portInfo->portId, TRUE);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set port %ld internal loopback, ret %ld.\r\n", lPort, ret);
                }
            }
            break;
        case E_LB_TEST_TYPE_PHY:
            /* Move to halPhy88E1680MacLoopback of switch_halPortSpeedSet */
            break;
        case E_LB_TEST_TYPE_NONE:
        default:
            ret = cpssDxChPortInternalLoopbackEnableSet(portInfo->devId, portInfo->portId, FALSE);
            if(ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set port %ld internal loopback, ret %ld.\r\n", lPort, ret);
            }
            if(lPort > boardInfo.copperMaxNum)
            {
                ret = cpssDxChPortSerdesLoopbackModeSet(portInfo->devId, portInfo->portId, 0xffffffff, CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set serdes loopback mode, ret %ld.\r\n", ret);
                }
                ret = cpssDxChPortSerdesTxEnableSet(portInfo->devId, portInfo->portId, FALSE);
                if(ret != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set internal loopback, ret %ld.\r\n", ret);
                }
            }
            break;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    E_LINER_PORT_TYPE portType;
    INT32  rc=E_TYPE_SUCCESS;
    UINT32 status;
    UINT16 regValue=0;
    S_PORT_INFO *portInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    rc = cpssDxChPortInternalLoopbackEnableGet(portInfo->devId, portInfo->portId, &status);

    if(status==TRUE)
    {
        *loopback = E_SWITCH_PORT_LOOPBACK_MAC;
    }
    else
    {
        /* Check phy loopback bit here */
        if(lPort < boardInfo.copperMaxNum)
        {
            switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
            switch_halSMIRegGet(lPort, PHY_CNTL_REG_OFFSET, &regValue);
            if((regValue&0x4000) == 0x4000)
            {
                *loopback = E_SWITCH_PORT_LOOPBACK_PHY;
            }
            else
            {
                *loopback = E_SWITCH_PORT_LOOPBACK_NONE;
            }
        }
        else
        {
            *loopback = E_SWITCH_PORT_LOOPBACK_NONE;
        }
    }

    return E_TYPE_SUCCESS;
}

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
    IN UINT32 lPort,
    OUT UINT32* status
)
{
    E_LINER_PORT_TYPE portType;
    INT32  rc=E_TYPE_SUCCESS;
    UINT16 portStatus;
    S_PORT_INFO *portInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    rc = cpssDxChPortDuplexAutoNegEnableGet(portInfo->devId, portInfo->portId, status);
    if( rc != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld duplex and autoneg enable, ret %ld.\r\n", lPort, rc);
        return rc;
    }

    if(*status == TRUE)
    {
        rc = cpssDxChPortSpeedAutoNegEnableGet(portInfo->devId, portInfo->portId, status);
        if( rc != E_TYPE_SUCCESS)
        {
            log_printf("Failed to set port %ld speed and autoneg enable, ret %ld.\r\n", lPort, rc);
            return rc;
        }

        if(lPort < boardInfo.copperMaxNum)
        {
            /* Need to check phy autoneg here. */
            switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
            switch_halSMIRegGet(lPort, PHY_CNTL_REG_OFFSET, &portStatus);
            if ( portStatus & 0x1000 )
            {
                *status = TRUE;
            }
        }
    }

    return rc;
}

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
)
{
    S_PORT_INFO *portInfo;
    INT32 ret=E_TYPE_SUCCESS;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    ret = cpssDxChPortDuplexAutoNegEnableSet(portInfo->devId, portInfo->portId, status);
    if( ret != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld duplex and autoneg enable, ret %ld.\r\n", lPort, ret);
        return ret;
    }

    ret = cpssDxChPortSpeedAutoNegEnableSet(portInfo->devId, portInfo->portId, status);
    if( ret != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld speed and autoneg enable, ret %ld.\r\n", lPort, ret);
        return ret;
    }

    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 1);    /* change to page 1 */
    switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x8140);
    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 4);    /* change to page 4 */
    switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x8140);
    switch_halSMIRegSet(lPort, PHY_PAGE_REG_OFFSET, 0);    /* change to page 0 */
    switch_halSMIRegSet(lPort, PHY_CNTL_REG_OFFSET, 0x8140);

    return E_TYPE_SUCCESS;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;

    /* Register callback function. */
    ret = appDemoDxChNetRxPacketCbRegister(rcv_fun);
    if(ret != E_TYPE_SUCCESS)
    {
        log_printf("Failed to register callback function, ret %ld.\r\n", ret);
    }
    return ret;
}

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
    IN UINT32                   txPort,
    IN UINT32                   rxPort,
    IN E_LB_TEST_TYPE           lbTestType,
    IN UINT32                   speed,
    IN BOOL                     issfp,
    IN GT_STATUS (*rcv_fun)(UINT8,  UINT8,  UINT32,  UINT8 **, UINT32 *, void *)
)
{
    INT32                   ret = E_TYPE_SUCCESS;

    if(lbTestType != E_LB_TEST_TYPE_SNAKE)
    {
        /* Set speed and loopback mode */
        switch_halPortSpeedSet(txPort, speed, lbTestType);
        udelay(10000);
        if(txPort != rxPort)
        {
            switch_halPortSpeedSet(rxPort, speed, lbTestType);
            udelay(10000);
        }

        switch_halPortLoopbackSet(txPort, lbTestType);
        udelay(10000);
        if(txPort != rxPort)
        {
            switch_halPortLoopbackSet(rxPort, lbTestType);
            udelay(10000);
        }
    }

    /* Register callback function. */
    ret = appDemoDxChNetRxPacketCbRegister(rcv_fun);
    if(ret != E_TYPE_SUCCESS)
    {
        log_printf("Failed to register callback function, ret %ld.\r\n", ret);
    }
    return ret;
}

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
)
{
    UINT32  speed=0;
    INT32   ret = E_TYPE_SUCCESS;
    S_BOARD_INFO boardInfo;
    UINT32 readDevMACId = 0;

    /* 20170626 - Try to read device MAC chip Id information */
    readDevMACId  = sys_utilsDevMACChipIdGet();

    sys_utilsBoardInfoGet(&boardInfo);

    /* Disable loopback mode and retrieve speed */
    if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
        /* Set Fiber to 10G speed default 2017-1-13 Lowell-Li */
        speed = (txPort > boardInfo.copperMaxNum)? 10000 : 1000;
    } else {
        speed = 1000; 
    }

    switch_halPortSpeedSet(txPort, speed, E_LB_TEST_TYPE_NONE);
    if(txPort != rxPort)
    {
        if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
            /* Set Fiber to 10G speed default 2017-1-13 Lowell-Li */
            speed = (rxPort > boardInfo.copperMaxNum)? 10000 : 1000;
        } else {
            speed = 1000;
        }

        switch_halPortSpeedSet(rxPort, speed, E_LB_TEST_TYPE_NONE);
    }

    udelay(1000);
    switch_halPortLoopbackSet(txPort, E_LB_TEST_TYPE_NONE);
    if(txPort != rxPort)
    {
        switch_halPortLoopbackSet(rxPort, E_LB_TEST_TYPE_NONE);
    }

    return ret;
}

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
)
{
    INT32   ret = E_TYPE_SUCCESS;
    GT_U8   devNum = 0;
    GT_U8   blockPort = 0;
    GT_U8   ruleIndex = 0;
    BOOL    ruleValidFlag=FALSE;

    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg, newLookupCfg;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT   mask, pattern, newMask, newPattern;
    CPSS_DXCH_PCL_ACTION_STC        action, newAction;
    GT_PORT_GROUPS_BMP              portGroupsBmp=0;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* initial all struct */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
    cpssOsMemSet(&lookupCfg, 0, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC));
    cpssOsMemSet(&newLookupCfg, 0, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC));
    cpssOsMemSet(&mask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&pattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&action, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
    cpssOsMemSet(&newMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&newPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&newAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    /* Set mask, pattern and action of the PCL rule */
    mask.ruleStdNotIp.common.pclId = 0x3FF;
    mask.ruleIngrExtUdb.sipBits127_80orMacDa[0] = 0xFF;
    mask.ruleIngrExtUdb.sipBits127_80orMacDa[1] = 0xFF;
    mask.ruleIngrExtUdb.sipBits127_80orMacDa[2] = 0xFF;
    mask.ruleIngrExtUdb.sipBits127_80orMacDa[3] = 0xFF;
    mask.ruleIngrExtUdb.sipBits127_80orMacDa[4] = 0xFF;
    mask.ruleIngrExtUdb.sipBits127_80orMacDa[5] = 0xFF;

    pattern.ruleStdNotIp.common.pclId = gPclId;
    pattern.ruleIngrExtUdb.sipBits127_80orMacDa[0] = srcMac[0];
    pattern.ruleIngrExtUdb.sipBits127_80orMacDa[1] = srcMac[1];
    pattern.ruleIngrExtUdb.sipBits127_80orMacDa[2] = srcMac[2];
    pattern.ruleIngrExtUdb.sipBits127_80orMacDa[3] = srcMac[3];
    pattern.ruleIngrExtUdb.sipBits127_80orMacDa[4] = srcMac[4];
    pattern.ruleIngrExtUdb.sipBits127_80orMacDa[5] = srcMac[5];   /* SA */

    action.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode = 1219;
    ret = cpssDxChPclRuleSet(portInfo->devId, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E, 1 /*ruleIndex*/, 0, &mask, &pattern, &action);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    /* I-PCL */
    ret = cpssDxChPclIngressPolicyEnable(portInfo->devId, GT_TRUE);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    ret = cpssDxChPclPortIngressPolicyEnable(portInfo->devId, portInfo->portId, GT_TRUE);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    /* Set parameters for I-PCL configuration table */
    portGroupsBmp = 0xffffffff;
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(portInfo->devId);
    interfaceInfo.devPort.portNum = portInfo->portId;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = gPclId;
    lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
    lookupCfg.groupKeyTypes.ipv4Key = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
    lookupCfg.groupKeyTypes.ipv6Key = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;

    ret = cpssDxChPclPortGroupCfgTblSet(portInfo->devId, portGroupsBmp, &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    /* Read entry and compare, compare PCL configure table */
    ret = cpssDxChPclPortGroupCfgTblGet(portInfo->devId, portGroupsBmp, &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_E, &newLookupCfg);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    if( memcmp(&lookupCfg, &newLookupCfg, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC)) != 0)
    {
        ret = E_TYPE_DATA_GET;
    }

    ret = cpssDxChPclRuleParsedGet(portInfo->devId, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E, gPclId /*ruleIndex*/, 0, &ruleValidFlag, &newMask, &newPattern, &newAction);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    if( (memcmp(&mask, &newMask, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT)) != 0) ||
        (memcmp(&pattern, &newPattern, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT)) != 0) ||
        (memcmp(&action, &newAction, sizeof(CPSS_DXCH_PCL_ACTION_STC)) != 0) )
    {
        ret = E_TYPE_DATA_GET;
    }

    log_printf("Complete PCL rule configuration.\r\n");

    return ret;
}

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
)
{
    INT32   ret = E_TYPE_SUCCESS;
    GT_U8   devNum = 0;
    GT_U8   blockPort = 0;
    GT_U8   ruleIndex = 0;
    BOOL    ruleValidFlag=FALSE;

    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT   mask, pattern;
    CPSS_DXCH_PCL_ACTION_STC        action;
    GT_PORT_GROUPS_BMP              portGroupsBmp=0;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* initial all struct */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
    cpssOsMemSet(&lookupCfg, 0, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC));
    cpssOsMemSet(&mask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&pattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&action, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    /* Set mask, pattern and action of the PCL rule */
    ret = cpssDxChPclRuleSet(portInfo->devId, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E, gPclId /*ruleIndex*/, 0, &mask, &pattern, &action);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    /* I-PCL */
    ret = cpssDxChPclIngressPolicyEnable(portInfo->devId, GT_FALSE);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    ret = cpssDxChPclPortIngressPolicyEnable(portInfo->devId, portInfo->portId, GT_FALSE);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }

    /* Set parameters for I-PCL configuration table */
    portGroupsBmp = 0xffffffff;
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(portInfo->devId);
    interfaceInfo.devPort.portNum = portInfo->portId;

    lookupCfg.enableLookup = GT_FALSE;
    lookupCfg.pclId = gPclId;
    cpssDxChPclPortGroupCfgTblSet(portInfo->devId, portGroupsBmp, &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    /* Invalidate rule */
    ret = cpssDxChPclPortGroupRuleInvalidate (portInfo->devId, portGroupsBmp, 1, gPclId);

    ret = cpssDxChPclRuleInvalidate(portInfo->devId, 1, gPclId);
    if (ret != GT_OK)
    {
        log_dbgPrintf("Error!! ret = %u   at  %s, %u\n", ret, __FILE__, __LINE__);
    }
    return ret;
}

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
)
{
    memcpy(srcMac,board_mac_addr,6);
    return E_TYPE_SUCCESS;
}

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
)
{
    GT_U32 rc;
    UINT8 devNum=0;
    CPSS_MAC_ENTRY_EXT_STC      macEntry;
    E_BOARD_ID boardId = sys_utilsDevBoardIdGet();

    cpssOsMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));

    /* Set CPU MAC into FDB */
    macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.macAddr.arEther[0] = srcMac[0];
    macEntry.key.key.macVlan.macAddr.arEther[1] = srcMac[1];
    macEntry.key.key.macVlan.macAddr.arEther[2] = srcMac[2];
    macEntry.key.key.macVlan.macAddr.arEther[3] = srcMac[3];
    macEntry.key.key.macVlan.macAddr.arEther[4] = srcMac[4];
    macEntry.key.key.macVlan.macAddr.arEther[5] = srcMac[5];

    macEntry.key.key.macVlan.vlanId = 0x1;

    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = devNum;
    macEntry.dstInterface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;
    macEntry.isStatic = GT_TRUE;
    macEntry.daCommand = CPSS_MAC_TABLE_CNTL_E;
    macEntry.saCommand = CPSS_MAC_TABLE_FRWRD_E;

    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if (rc != GT_OK)
    {
        log_printf("Error of set MAC entry : %d\n", rc);
    }

    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        macEntry.dstInterface.devPort.hwDevNum = 0x1;
        devNum=0x1;
        rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
        if (rc != GT_OK)
        {
            log_printf("Error of set MAC entry : %d\n", rc);
        }

    }

    return E_TYPE_SUCCESS;
}

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
)
{
    GT_U32                  rc, index=0;
    UINT8                   devNum=0;
    GT_BOOL                 validGet    = GT_FALSE;
    GT_BOOL                 skipGet     = GT_FALSE;
    GT_BOOL                 agedGet     = GT_FALSE;
    GT_HW_DEV_NUM           HwDevNumGet   = 0;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);
    cpssOsMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));

    for(devNum=0;devNum<boardInfo.devMaxNum;devNum++)
    {
        for(index=0;index<(16*1024);index++)
        {
            /* Get status of FDB entry */
            cpssDxChBrgFdbMacEntryRead(devNum, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &macEntry);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* If the entry is valid need to invalidate
               If the entry is invalid - do nothing */
            if(validGet)
            {
                if(macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
                {
                    if(memcmp(&macEntry.key.key.macVlan.macAddr, srcMac, sizeof(GT_ETHERADDR)) == 0)
                    {
                        rc = cpssDxChBrgFdbMacEntryInvalidate(devNum,index);
                        if(rc!=GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    S_PORT_CNT          mibCounter, zeroCounter;
    UINT8               counterStatus=0;
    UINT32              retry, status, portId;
    INT32               ret=E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    memset(&zeroCounter, 0, sizeof(zeroCounter));
    memset(&mibCounter, 0, sizeof(mibCounter));

    if( (ret = cpssDxChPortMacCountersClearOnReadGet(portInfo->devId, portInfo->portId, &counterStatus)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port %ld mac counter clear on read flag. ret %ld\r\n", lPort, ret);
    }
    udelay(10000);
    /* Clear all counters before test */
    status = TRUE;
    if( (ret = cpssDxChPortMacCountersClearOnReadSet(portInfo->devId, portInfo->portId, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld mac counter clear on read flag. ret %ld\r\n", lPort, ret);
    }
    udelay(10000);
    for (retry=0; retry<10; retry++)
    {
        switch_halPortCntGet(lPort, &mibCounter);
        if ( !memcmp(&mibCounter, &zeroCounter, sizeof(mibCounter)) )
        {
            break;
        }
    }
    udelay(10000);
    if ( retry >= 10 )
    {
        log_printf("Cannot clear port %ld counter\n", lPort);
    }
    return E_TYPE_SUCCESS;
}

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
)
{
    S_PORT_CNT          mibCounter, zeroCounter;
    UINT8               counterStatus=0, devId=0, phyPort=0;
    UINT32              retry, status, portId;
    INT32               ret=E_TYPE_SUCCESS;

    if(lPort == 1)
    {
        devId = 1;
        phyPort = 24;
    }
    else if(lPort == 2)
    {
        devId = 1;
        phyPort = 26;
    }
    else if(lPort == 3)
    {
        devId = 0;
        phyPort = 24;
    }
    else if(lPort == 4)
    {
        devId = 0;
        phyPort = 26;
    }
    else
    {
        return E_TYPE_DATA_GET;
    }

    memset(&zeroCounter, 0, sizeof(zeroCounter));
    memset(&mibCounter, 0, sizeof(mibCounter));

    if( (ret = cpssDxChPortMacCountersClearOnReadGet(devId, phyPort, &counterStatus)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port %ld mac counter clear on read flag. ret %ld\r\n", lPort, ret);
    }

    /* Clear all counters before test */
    status = TRUE;
    if( (ret = cpssDxChPortMacCountersClearOnReadSet(devId, phyPort, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld mac counter clear on read flag. ret %ld\r\n", lPort, ret);
    }

    for (retry=0; retry<10; retry++)
    {
        switch_halCascadePortCntGet(lPort, &mibCounter);
        if ( !memcmp(&mibCounter, &zeroCounter, sizeof(mibCounter)) )
        {
            break;
        }
    }

    if ( retry >= 10 )
    {
        log_printf("Cannot clear port %ld counter\n", lPort);
    }
    return E_TYPE_SUCCESS;
}

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
 *      used IO Expander for SFP+ rate selection.
 *      shift[4]={2,0,6,4} - IO mapping of SFP+ ports
 *      2 - Port 25 (24 SKUs), Port 49 (48 SKUs)
 *      0 - Port 26 (24 SKUs), Port 50 (48 SKUs)
 *      6 - Port 27 (24 SKUs), Port 51 (48 SKUs)
 *      4 - Port 28 (24 SKUs), Port 52 (48 SKUs)
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_halPortSfpRateSet
(
    IN UINT32 lPort,
    IN UINT32 rateLevel
)
{
    S_BOARD_INFO boardInfo;
    UINT32 sPort = 0;
    /* define sfp port to ioexp offset address  */
    UINT32 shift[4]={E_SFP0_IOEXP2, E_SFP1_IOEXP0, E_SFP2_IOEXP6, E_SFP3_IOEXP4};
    INT32 ret=E_TYPE_SUCCESS;
    UINT8 outputVal = 0, value = 0, data = 0;

    /* check sfp first fiber port number  */
    sys_utilsBoardInfoGet(&boardInfo);
    sPort = lPort - boardInfo.firstfiberNum;

    if (sPort > 3)
        return E_TYPE_INVALID_DATA;

    /* set output pin to ioexp cfg 0x6 regs */
    if( i2c_halRegSet(IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR, 1, &outputVal, 1) < E_TYPE_SUCCESS )
    {
        log_printf("Write device id %x addr %x fail\n", IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR);
        return E_TYPE_REG_WRITE;
    }

    /* read output register value before set rate */
    if( i2c_halRegGet(IO_EXP_I2C_ADDR, IOEXP_OUT_ADDR, 1, &data, 1) < E_TYPE_SUCCESS )
    {
        log_printf("Read device id %x addr %x fail\n", IO_EXP_I2C_ADDR, IOEXP_OUT_ADDR);
        return E_TYPE_REG_READ;
    }

    /* prepare the ioexp output data for sfp rate select pins */
    if(rateLevel == E_SWITCH_SFP_RATE_HI)
        value = data | (0x3 << shift[sPort]);
    else if(rateLevel == E_SWITCH_SFP_RATE_LO)
        value = data & ~(0x3 << shift[sPort]);


    log_dbgPrintf("The value write to output register is 0x%x\n", value);

    if( i2c_halRegSet(IO_EXP_I2C_ADDR, IOEXP_OUT_ADDR, 1,&value, 1) < E_TYPE_SUCCESS )
    {
        log_printf("Write device id %x addr %x fail\n", IO_EXP_I2C_ADDR, IOEXP_OUT_ADDR);
        return E_TYPE_REG_WRITE;
    }

    return E_TYPE_SUCCESS;
}

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
 *      used IO Expander for SFP+ rate selection.
 *      shift[4]={2,0,6,4} - IO mapping of SFP+ ports
 *      2 - Port 25 (24 SKUs), Port 49 (48 SKUs)
 *      0 - Port 26 (24 SKUs), Port 50 (48 SKUs)
 *      6 - Port 27 (24 SKUs), Port 51 (48 SKUs)
 *      4 - Port 28 (24 SKUs), Port 52 (48 SKUs)
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_halPortSfpRateTest
(
    IN UINT32 lPort
)
{
    S_BOARD_INFO boardInfo;
    UINT32 sPort = 0;
    /* define sfp port to ioexp offset address  */
    UINT32 shift[4]={E_SFP0_IOEXP2, E_SFP1_IOEXP0, E_SFP2_IOEXP6, E_SFP3_IOEXP4};
    INT32 ret=E_TYPE_SUCCESS;
    UINT8 inputValue = 0, data = 0, getValue = 0, outputValue = 0;

    /* check sfp first fiber port number  */
    sys_utilsBoardInfoGet(&boardInfo);
    sPort = lPort - boardInfo.firstfiberNum;

    if (sPort > 3)
        return E_TYPE_INVALID_DATA;

    /* set sfp test port as input pins */
    inputValue = 0x3 << shift[sPort];

    /* set input pin to ioexp cfg 0x6 regs */
    if( i2c_halRegSet(IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR, 1,&inputValue, 1) < E_TYPE_SUCCESS )
    {
        log_printf("Write device id %x addr %x fail\n", IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR);
        return E_TYPE_REG_WRITE;
    }

    /* thru config register 0x6 to set output pin to input pin */
    log_dbgPrintf("The value write to reg 0x6 is 0x%X \n", inputValue);

    /* read the status of input register 0x0 */
    if( i2c_halRegGet(IO_EXP_I2C_ADDR, IOEXP_IN_ADDR, 1, &getValue, 1) < E_TYPE_SUCCESS )
    {
        log_printf("Read device id %x addr %x fail\n", IO_EXP_I2C_ADDR, IOEXP_IN_ADDR);
        return E_TYPE_REG_READ;
    }

    /* rollback all sfp port to output pin */
    if( i2c_halRegSet(IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR, 1, &outputValue, 1) < E_TYPE_SUCCESS )
    {
        log_printf("Write device id %x addr %x fail\n", IO_EXP_I2C_ADDR, IOEXP_CFG_ADDR);
        return E_TYPE_REG_WRITE;
    }

    /* check if getValue is input data high */
    log_dbgPrintf("Check SFP Test value: 0x%X, Read back value: 0x%X \n",
    inputValue, getValue);

    /* check the input data value by sfp port */
    getValue &= (0x3 << shift[sPort]);

    if(memcmp(&inputValue, &getValue, 1) != 0)
    {
        return E_TYPE_DATA_MISMATCH;
    }

    return E_TYPE_SUCCESS;
}


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
)
{
    INT32       ret=E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if( (ret = cpssDxChPortMacCountersClearOnReadSet(portInfo->devId, portInfo->portId, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld mac counter clear on read flag. ret %ld\r\n", lPort, ret);
    }

    return ret;
}

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
)
{
    UINT8 enable, dstTrunk, dstHwDev0, dstHwDev1;
    INT32  ret=E_TYPE_SUCCESS;
    S_PORT_INFO *sPortInfo, *dPortInfo;

    sPortInfo=port_utilsLPortInfoGet(src_lPort);
    if( sPortInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, src_lPort);
        return E_TYPE_DATA_GET;
    }

    dPortInfo=port_utilsLPortInfoGet(dest_lPort);
    if( dPortInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, dest_lPort);
        return E_TYPE_DATA_GET;
    }

    if(dPortInfo->devId != sPortInfo->devId)
    {
        /* Configure PVE global status */
        if ( ret = cpssDxChBrgPrvEdgeVlanEnable(sPortInfo->devId, status) != E_TYPE_SUCCESS )
        {
            log_printf("Failed to enable PVE, ret %ld.\r\n", ret);
            return ret;
        }

        /* Configure PVE global status */
        if ( ret = cpssDxChBrgPrvEdgeVlanEnable(dPortInfo->devId, status) != E_TYPE_SUCCESS )
        {
            log_printf("Failed to enable PVE, ret %ld.\r\n", ret);
            return ret;
        }
    }
    else
    {
        /* Configure PVE global status */
        if ( ret = cpssDxChBrgPrvEdgeVlanEnable(sPortInfo->devId, status) != E_TYPE_SUCCESS )
        {
            log_printf("Failed to enable PVE, ret %ld.\r\n", ret);
            return ret;
        }
    }

    /* Configure interface PVE status */
    enable = status;
    dstTrunk = FALSE;
    /* Fixed value, get from Lua CLI.
     * For 48 Sku, it might have different value.
     */
    if(dPortInfo->devId != sPortInfo->devId)
    {
        if (sPortInfo->devId == 0x0)
        {
            dstHwDev0 = 0x10;
        }
        else if (sPortInfo->devId == 0x1)
        {
            dstHwDev0 = 0x11;
        }

        if (dPortInfo->devId == 0x0)
        {
            dstHwDev1 = 0x10;
        }
        else if (dPortInfo->devId == 0x1)
        {
            dstHwDev1 = 0x11;
        }
    }
    else
    {
        dstHwDev0 = 0x10 + sPortInfo->devId;
    }

    if(dPortInfo->devId != sPortInfo->devId)
    {
        ret = cpssDxChBrgPrvEdgeVlanPortEnable(sPortInfo->devId, sPortInfo->portId, enable, dPortInfo->portId, dstHwDev0, dstTrunk);
        if(ret != E_TYPE_SUCCESS)
        {
            log_printf("Failed to configure PVE, ret %ld.\r\n", ret);
        }

        ret = cpssDxChBrgPrvEdgeVlanPortEnable(dPortInfo->devId, dPortInfo->portId, enable, sPortInfo->portId, dstHwDev1, dstTrunk);
        if(ret != E_TYPE_SUCCESS)
        {
            log_printf("Failed to configure PVE, ret %ld.\r\n", ret);
        }
    }
    else
    {
        ret = cpssDxChBrgPrvEdgeVlanPortEnable(sPortInfo->devId, sPortInfo->portId, enable, dPortInfo->portId, dstHwDev0, dstTrunk);
        if(ret != E_TYPE_SUCCESS)
        {
            log_printf("Failed to configure PVE, ret %ld.\r\n", ret);
        }
    }

    return ret;
}

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
)
{
    INT32                    ret=E_TYPE_SUCCESS;
    UINT8                    bpduStatus;
    UINT32                   profileIndex=0, lPort=0, totalPort=0;
    UINT32                   protocol;
    CPSS_PACKET_CMD_ENT      cmd;
    E_BOARD_ID boardId = sys_utilsDevBoardIdGet();

    if(status == TRUE)
    {
        bpduStatus = FALSE;
    }
    else
    {
        bpduStatus = TRUE;
    }

    /* Configure BPDU trap status */
    if ( ret = cpssDxChBrgGenBpduTrapEnableSet(devNum, bpduStatus) != E_TYPE_SUCCESS )
    {
        log_printf("Failed to set BPDU trap, ret %ld.\r\n", ret);
        return ret;
    }
    udelay(10000);
    if ( ret = cpssDxChBrgGenIeeeReservedMcastTrapEnable(devNum, status) != E_TYPE_SUCCESS )
    {
        log_printf("Failed to set mcast trap, ret %ld.\r\n", ret);
        return ret;
    }

    udelay(10000);
    if(status == TRUE)
    {
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }
    else
    {
        cmd = CPSS_PACKET_CMD_FORWARD_E;
    }

    for(protocol=0;protocol<=0xff;protocol++)
    {
        if ( ret = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(devNum, profileIndex, protocol, cmd) != E_TYPE_SUCCESS )
        {
            log_printf("Failed to set mcast protocol command, ret %ld. protocol %ld\r\n", ret, protocol);
            return ret;
        }
    }

    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        totalPort = 28;
    }
    else if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) || (boardId == E_BOARD_ID_HAYWARDS_16G2G_P) )
    {
        totalPort = 16;
    }
    else if( (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) || (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) )
    {
        totalPort = 8;
    }
    else
    {
        totalPort = 28;
    }

    /* Fix value 28 for each MAC */
    for(lPort=0; lPort<totalPort; lPort++)
    {
        if ( ret = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(devNum, lPort, profileIndex) != E_TYPE_SUCCESS )
        {
            log_printf("Failed to set dev %ld port %ld mcast profile index %ld, ret %ld.\r\n", devNum, lPort, profileIndex, ret);
            return ret;
        }
    }

    return ret;
}

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
)
{
    buffList[0] = cpssOsCacheDmaMalloc(FOXCONN_PKT_MAX_SIZE*sizeof(GT_U8));

    return E_TYPE_SUCCESS;
}

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
)
{
    GT_TASK         SfpPortPollingTid;

    /* Create Task to polling SFP LED */
    osTaskCreate("LedSFPTask",                /* Task Name      */
                 SWITCH_HANDLER_LOW_PRIORITY, /* Task Priority  */
                 _8KB,                        /* Stack Size     */
                 sfpPortPolling,              /* Starting Point */
                 (GT_VOID*)0,                 /* Arguments list */
                 &SfpPortPollingTid);         /* task ID        */

    return E_TYPE_SUCCESS;
}

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
    IN  UINT8   *pktBuf,
    IN  UINT32  pktLen
)
{
    INT32                       rc;
    UINT32                      portId,i;
    E_LINER_PORT_TYPE           portType;
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    UINT32                      numOfBufs;
    CPSS_UNI_EV_CAUSE_ENT       uniEventArr[1];
    UINT64                      txEventHndl;
    UINT32                      evBitmapArr[8];
    CPSS_UNI_EV_CAUSE_ENT       evCause[1] = { CPSS_PP_TX_BUFFER_QUEUE_E };
    GT_U32                      evReqHndl;
    S_PORT_INFO                 *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    port_utilsLPort2PortTypeGet(lPort, &portType, &portId);

    switch ( portType )
    {
        case E_LINER_PORT_TYPE_FIXED:
#if 0
            uniEventArr[0] = CPSS_PP_TX_BUFFER_QUEUE_E;
            cpssEventBind(uniEventArr, 1, &txEventHndl);
#endif
            osMemSet(&pcktParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
            pcktParams.sdmaInfo.txQueue = 0;
            pcktParams.sdmaInfo.recalcCrc = 1;
            pcktParams.sdmaInfo.invokeTxBufferQueueEvent = FALSE;
            pcktParams.sdmaInfo.evReqHndl = txEventHndl;
            pcktParams.packetIsTagged = FALSE;
            pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            /* use here HW device number! */
            pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = PRV_CPSS_HW_DEV_NUM_MAC(portInfo->devId);
            pcktParams.dsaParam.dsaInfo.fromCpu.cascadeControl = FALSE;
            pcktParams.dsaParam.dsaInfo.fromCpu.tc = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.dp = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterEn = FALSE;
            pcktParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered = FALSE;
            pcktParams.dsaParam.dsaInfo.fromCpu.srcId = 0;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(portInfo->devId);
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = portId;
            pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 1;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = FALSE;
            pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = FALSE;
            pcktParams.dsaParam.commonParams.cfiBit = 0;
            pcktParams.dsaParam.commonParams.vid = 1;
            pcktParams.dsaParam.commonParams.vpt = 0;
            pcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E;
            pcktParams.dsaParam.commonParams.dropOnSource = FALSE;
            pcktParams.dsaParam.commonParams.packetIsLooped = FALSE;
            numOfBufs = 1;
            buffLenList[0] = pktLen;
            osMemCpy(buffList[0], pktBuf, buffLenList[0]);
            rc = cpssDxChNetIfSdmaTxQueueEnable(portInfo->devId, 0, TRUE);
            if (rc != GT_OK)
            {
                log_printf("TxQueueEnable failed, rc %ld\r\n", rc);
                return rc;
            }

            rc = cpssDxChNetIfSdmaSyncTxPacketSend(portInfo->devId, &pcktParams, buffList, buffLenList, numOfBufs);
            if (rc != GT_OK)
            {
                log_printf("cpssDxChNetIfSdmaSyncTxPacketSend failed, rc %ld\r\n", rc);
                return rc;
            }
            break;

        case E_LINER_PORT_TYPE_STACKING:
        case E_LINER_PORT_TYPE_EXPANSION:
        case E_LINER_PORT_TYPE_INTER_LINK:
        default:
            return E_TYPE_UNSUPPORT;
    }
    return 0;
}

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
)
{
    UINT8   mac;
    UINT8 * pStrMac = macStr;
    UINT32  i;
    UINT32  j;

    /* Remove ':' from MAC affress */
    UINT8 enet_addr[6 * 2];

    for ( i = 0, j = 0; i < sizeof(enet_addr); j ++ )
    {
        if ( (pStrMac[j] != ':') && (pStrMac[j] != '-') )
        {
            enet_addr[i] = pStrMac[j];
            i++;
        }
    }

    memset(macAddr, 0, 6);
    for ( i = 0, j = 0; j < sizeof(enet_addr); j ++ )
    {
        mac = enet_addr[j];

        if ( (mac >= '0') && (mac <= '9') )
            mac = mac - '0';
        else if ( (mac >= 'a') && (mac <= 'f') )
            mac = ( mac - 'a' + 10 );
        else if ( (mac >= 'A') && (mac <= 'F') )
            mac = ( mac - 'A' + 10 );
        else
        {
            macAddr = NULL;
            return E_TYPE_INVALID_DATA;
        }

        macAddr[i] += mac;

        if ( (j + 1) % 2 )
            macAddr[i] <<= 4;
        else
            i ++;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    E_LINER_PORT_TYPE portType;
    INT32  rc=E_TYPE_SUCCESS;
    UINT32 temp;

    /* CPSS GT_BOOL is UINT32 */
    rc = cpssDxChPortForceLinkDownEnableGet(devNum, phyPort, &temp);
    *status = temp;

    return rc;
}

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
)
{
    INT32  rc=E_TYPE_SUCCESS;

    rc = cpssDxChPortDuplexAutoNegEnableGet(devNum, phyPort, status);
    if( rc != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get cascade port duplex and autoneg enable, ret %ld.\r\n", rc);
        return rc;
    }

    if(*status == TRUE)
    {
        rc = cpssDxChPortSpeedAutoNegEnableGet(devNum, phyPort, status);
        if( rc != E_TYPE_SUCCESS)
        {
            log_printf("Failed to get cascade port speed and autoneg enable, ret %ld.\r\n", rc);
            return rc;
        }
    }

    return rc;
}

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
)
{
    CPSS_PORT_DUPLEX_ENT mvDuplex;

    cpssDxChPortDuplexModeGet(devNum, phyPort, &mvDuplex);
    if(mvDuplex == CPSS_PORT_FULL_DUPLEX_E)
    {
        *duplex = E_SWITCH_PORT_DUPLEX_FULL;
    }
    else if(mvDuplex == CPSS_PORT_HALF_DUPLEX_E)
    {
        *duplex = E_SWITCH_PORT_DUPLEX_HALF;
    }
    else
    {
        log_printf("%s get cascade port duplex fail\n", __FUNCTION__);
        return E_TYPE_DATA_GET;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    CPSS_PORT_SPEED_ENT mvSpeed;

    cpssDxChPortSpeedGet(devNum, phyPort, &mvSpeed);

    if(mvSpeed == CPSS_PORT_SPEED_10_E)
    {
        *speed = 10;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_100_E)
    {
        *speed = 100;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_1000_E)
    {
        *speed = 1000;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_2500_E)
    {
        *speed = 2500;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_10000_E)
    {
        *speed = 10000;
    }
    else if(mvSpeed == CPSS_PORT_SPEED_20000_E)
    {
        *speed = 20000;
    }
    else
    {
        log_printf("%s get cascade port speed fail, mvSpeed %ld\n", __FUNCTION__, mvSpeed);
        return E_TYPE_DATA_GET;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;

    if (ret = cpssDxChPortLinkStatusGet(devNum, phyPort, link) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port link status, ret %ld\r\n", ret);
    }

    return ret;
}

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
)
{
    S_PORT_INFO *portInfo;
    INT32 ret = E_TYPE_SUCCESS;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if (ret = cpssDxChPortLinkStatusGet(portInfo->devId, portInfo->portId, link) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port link status, ret %ld\r\n", ret);
    }
    return ret;
}

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
)
{
    UINT32          macLinkStatus=0;
    UINT16          portStatus = 0;
    S_PORT_INFO     *portInfo;

    *link = 0;

    portInfo = port_utilsLPortInfoGet(lPort);

    switch ( portInfo->portType )
    {
        case E_PORT_TYPE_1000BASEX:
            /* Since in this sku, there is no PHY on this type of port type. So check mac link */
            switch_halPortMACLinkStatusGet(lPort, &macLinkStatus);
            if(macLinkStatus == 0x1)
            {
                *link = 1;
            }
            break;

        case E_PORT_TYPE_1000BASET:
        default:
            switch_halPhyPageNumSet(lPort, 0);
            switch_halSMIRegGet(lPort, 0x1, &portStatus);
            if ( portStatus & 0x4 )
            {
                *link = 1;
            }
            break;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    S_PORT_INFO *portInfo;
    INT32 ret = E_TYPE_SUCCESS;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if (ret = cpssDxChPortMruGet(portInfo->devId, portInfo->portId, frameSize) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port max length, ret %ld\r\n", ret);
    }

    return ret;
}

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
)
{
    S_PORT_INFO *portInfo;
    INT32 ret = E_TYPE_SUCCESS;
    UINT16 pvid=0;
    UINT32 mruIndex=0;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if((ret = cpssDxChPortMruSet(portInfo->devId, portInfo->portId, frameSize)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port max length, ret %ld\r\n", ret);
    }

    /* Get Port PVID first */
    if((ret =switch_halPortPvidGet(lPort, &pvid)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port PVID, ret %ld\r\n", ret);
    }

    if((ret =cpssDxChBrgVlanMruProfileIdxSet(portInfo->devId, pvid, mruIndex)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set vlan MRU profile index %ld, ret %ld\r\n", mruIndex, ret);
    }

    if((ret =cpssDxChBrgVlanMruProfileValueSet(portInfo->devId, mruIndex, frameSize)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set vlan MRU profile value %ld, ret %ld\r\n", frameSize, ret);
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;

    if (ret = cpssDxChPortMruGet(devNum, phyPort, frameSize) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port max length, ret %ld\r\n", ret);
    }

    return ret;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT16 pvid=0;
    UINT32 mruIndex=0;

    if((ret = cpssDxChPortMruSet(devNum, phyPort, frameSize)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port max length, ret %ld\r\n", ret);
    }

    /* Get Port PVID first */
    pvid=1;

    if((ret =cpssDxChBrgVlanMruProfileIdxSet(devNum, pvid, mruIndex)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set vlan MRU profile index %ld, ret %ld\r\n", mruIndex, ret);
    }

    if((ret =cpssDxChBrgVlanMruProfileValueSet(devNum, mruIndex, frameSize)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set vlan MRU profile value %ld, ret %ld\r\n", frameSize, ret);
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    S_PORT_INFO *portInfo;
    UINT8 advertiseStatus=0, flowControlAutoNeg=0;
    INT32 ret = E_TYPE_SUCCESS;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if (ret = cpssDxChPortFlowControlEnableGet(portInfo->devId, portInfo->portId, status) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port flow control mode, ret %ld\r\n", ret);
    }

    if (ret = cpssDxChPortFlowCntrlAutoNegEnableGet(portInfo->devId, portInfo->portId, &flowControlAutoNeg, &advertiseStatus) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port flow control advertise mode, ret %ld\r\n", ret);
    }

    return ret;
}

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
)
{
    S_PORT_INFO *portInfo;
    INT32 ret = E_TYPE_SUCCESS;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if((ret = cpssDxChPortFlowCntrlAutoNegEnableSet(portInfo->devId, portInfo->portId, FALSE, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port flow control AN, ret %ld\r\n", ret);
    }

    if((ret =cpssDxChPortFlowControlEnableSet(portInfo->devId, portInfo->portId, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set port %ld flow control %ld, ret %ld\r\n", lPort, ret);
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    UINT8 advertiseStatus=0, flowControlAutoNeg=0, cascadeFcStatus=0;
    INT32 ret = E_TYPE_SUCCESS;

    if (ret = cpssDxChPortFlowControlEnableGet(devNum, phyPort, &cascadeFcStatus) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port flow control mode, ret %ld\r\n", ret);
    }

    if(cascadeFcStatus >=CPSS_PORT_FLOW_CONTROL_RX_TX_E)
    {
        *status = TRUE;
    }
    else
    {
        *status = FALSE;
    }

    if (ret = cpssDxChPortFlowCntrlAutoNegEnableGet(devNum, phyPort, &flowControlAutoNeg, &advertiseStatus) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get port flow control advertise mode, ret %ld\r\n", ret);
    }

    return ret;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;
#if 0
    if((ret = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, phyPort, FALSE, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set cascade port flow control AN, ret %ld\r\n", ret);
    }
#endif
    if((ret =cpssDxChPortFlowControlEnableSet(devNum, phyPort, status)) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set cascade port flowcontrol, ret %ld\r\n", ret);
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_BOOL isValid=0;
    INT32  ret = E_TYPE_SUCCESS;

    /* Check Vlan exist or not first */
    if (ret = cpssDxChBrgVlanEntryRead(devNum, vid, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to read vlan Entry, ret %ld.\r\n", ret);
        return ret;
    }

    if(isValid == TRUE)
    {
        return E_TYPE_REG_READ;
    }

    /* Fill Vlan info */
    memset(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0xFFF;
    cpssVlanInfo.naMsgToCpuEn  = TRUE;
    cpssVlanInfo.floodVidx     = 0xFFF;
    cpssVlanInfo.fidValue      = vid;
    cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;

    /* Fill ports and tagging members */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    memset(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* Write default VLAN entry */
    ret = cpssDxChBrgVlanEntryWrite(devNum, vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);

    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;

    /* Invalidate vlan */
    if (ret = cpssDxChBrgVlanEntryInvalidate(devId, vid) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to invalid vlan Entry %ld, ret %ld.\r\n", vid, ret);
        return ret;
    }

    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_BOOL isValid=0;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* Fill ports and tagging members */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    memset(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    memset(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

    /* Get vlan entry */
    if (ret = cpssDxChBrgVlanEntryRead(portInfo->devId, vid, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to read vlan Entry, ret %ld.\r\n", ret);
        return ret;
    }

    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0;
    cpssVlanInfo.naMsgToCpuEn  = TRUE;
    cpssVlanInfo.floodVidx     = 0xFFF;
    cpssVlanInfo.fidValue      = 0;
    cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
    cpssVlanInfo.autoLearnDisable = FALSE;

    portsMembers.ports[0] |= 1 << portInfo->portId;

    /* Write default VLAN entry */
    ret = cpssDxChBrgVlanEntryWrite(portInfo->devId, vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);
    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_BOOL isValid=0;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    memset(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    memset(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

    /* Get vlan entry */
    if (ret = cpssDxChBrgVlanEntryRead(portInfo->devId, vid, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to read vlan Entry, ret %ld.\r\n", ret);
        return ret;
    }

    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0;
    cpssVlanInfo.naMsgToCpuEn  = TRUE;
    cpssVlanInfo.floodVidx     = 0xFFF;
    cpssVlanInfo.fidValue      = 0;
    cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    cpssVlanInfo.autoLearnDisable = TRUE;

    /* Fill ports and tagging members */
    portsMembers.ports[0] &= ~(1 << portInfo->portId);

    /* Write default VLAN entry */
    ret = cpssDxChBrgVlanEntryWrite(portInfo->devId, vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);
    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_BOOL isValid=0;

    /* Fill ports and tagging members */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    memset(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    memset(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

    /* Get vlan entry */
    if (ret = cpssDxChBrgVlanEntryRead(devNum, vid, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to read vlan Entry, ret %ld.\r\n", ret);
        return ret;
    }

    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0;
    cpssVlanInfo.naMsgToCpuEn  = TRUE;
    cpssVlanInfo.floodVidx     = 0xFFF;
    cpssVlanInfo.fidValue      = 0;
    cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
    cpssVlanInfo.autoLearnDisable = FALSE;

    portsMembers.ports[0] |= 1 << phyPort;

    /* Write default VLAN entry */
    ret = cpssDxChBrgVlanEntryWrite(devNum, vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);

    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_BOOL isValid=0;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    memset(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    memset(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

    /* Get vlan entry */
    if (ret = cpssDxChBrgVlanEntryRead(devNum, vid, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to read vlan Entry, ret %ld.\r\n", ret);
        return ret;
    }

    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0;
    cpssVlanInfo.naMsgToCpuEn  = TRUE;
    cpssVlanInfo.floodVidx     = 0xFFF;
    cpssVlanInfo.fidValue      = 0;
    cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    cpssVlanInfo.autoLearnDisable = TRUE;

    /* Fill ports and tagging members */
    portsMembers.ports[0] &= ~(1 << phyPort);

    /* Write default VLAN entry */
    ret = cpssDxChBrgVlanEntryWrite(devNum, vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &cpssVlanInfo,
                                   &portsTaggingCmd);
    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;

    if (ret = cpssDxChBrgVlanPortVidSet(devNum, phyPort, CPSS_DIRECTION_INGRESS_E,pvid) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set PVID %ld, ret %ld.\r\n", pvid, ret);
    }
    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if (ret = cpssDxChBrgVlanPortVidGet(portInfo->devId, portInfo->portId, CPSS_DIRECTION_INGRESS_E,pvid) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to get PVID %ld, ret %ld.\r\n", pvid, ret);
    }
    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if (ret = cpssDxChBrgVlanPortVidSet(portInfo->devId, portInfo->portId, CPSS_DIRECTION_INGRESS_E,pvid) != E_TYPE_SUCCESS)
    {
        log_printf("Failed to set PVID %ld, ret %ld.\r\n", pvid, ret);
    }
    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    /* Temp add, need to revise to support 48 sku. */
    UINT32 devId=0, vid=0, i,pvid;

    for(devId=0; devId < port_utilsTotalDevGet(); devId++)
    {
        vid =1;
        /* Restore all ports in default VLAN 1.and remove ports from previous VLAN */
        for(i=1;i<=port_utilsTotalFixedPortGet();i++)
        {
            ret=switch_halPortPvidGet(i,&pvid);
            if(pvid != vid)
            {
                ret=switch_halRemovePortFromVlan(i,pvid);
                if( (ret = switch_halAddPortToVlan(i, vid, 1)) != E_TYPE_SUCCESS)
                {
                    return E_TYPE_REG_WRITE;
                }
                switch_halPortPvidSet(i, vid);
            }
        }

        if( (ret = switch_halAddCascadePortToVlan(devId, 24, vid, 1)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        if( (ret = switch_halAddCascadePortToVlan(devId, 24, vid, 1)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        if( (ret = switch_halAddCascadePortToVlan(devId, 26, vid, 1)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        if( (ret = switch_halAddCascadePortToVlan(devId, 26, vid, 1)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        switch_halCascadePortPvidSet(devId, 24, vid);
        switch_halCascadePortPvidSet(devId, 24, vid);
        switch_halCascadePortPvidSet(devId, 26, vid);
        switch_halCascadePortPvidSet(devId, 26, vid);
        for(vid=2;vid<=4094;vid++)
        {
            switch_halVlanInvalid(devId, vid);
        }
    }
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halBoardIDInit
 *
 *  DESCRIPTION :
 *      set xcat3 switch board id from cpss init
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
 *      The uboot will save board to reserved mac register 0x7C
 *
 *--------------------------------------------------------------------------
 */
INT32 switch_halBoardIDInit
(
    IN  void
)
{
    UINT32 data;

    /* read cpss reserved register 0x7c for EV board id, change to get by FPGA  */
    /* switch_halMACRegGet(SWITCH_HAL_AC3_DEVICE, SWITCH_HAL_AC3_CPSS_BOARDID_REG, &data) */
    if ( fpga_boardIdGet(&data) == 0 )
    {
        data &=0xFF;
        printf("DBG: switch_halBoardIDInit-- get board id value 0x%0x\n", data);
        sys_utilsDevBoardIdSet(data);

        /* ac3 switch will call back to global board Id, Brian Lu 20151006 */
        printf("DBG: (sys_utilsDevBoardIdGet) the switch board is ");

        switch(sys_utilsDevBoardIdGet())
        {
            case E_BOARD_ID_HAYWARDS_48G4G_T:
                printf("HAYWARDS_48G4G_T\n");
                break;
            case E_BOARD_ID_HAYWARDS_48G4G_P:
                printf("HAYWARDS_48G4G_P\n");
                break;
            case E_BOARD_ID_HAYWARDS_24G4G_T:
                printf("HAYWARDS_24G4G_T\n");
                break;
            case E_BOARD_ID_HAYWARDS_24G4G_P:
                printf("HAYWARDS_24G4G_P\n");
                break;
            case E_BOARD_ID_HAYWARDS_16G2G_T:
                printf("HAYWARDS_16G2G_T\n");
                break;
            case E_BOARD_ID_HAYWARDS_16G2G_P:
                printf("HAYWARDS_16G2G_P\n");
                break;
            case E_BOARD_ID_HAYWARDS_8G2G_T:
                printf("HAYWARDS_8G2G_T\n");
                break;
            case E_BOARD_ID_HAYWARDS_8G2G_P:
                printf("HAYWARDS_8G2G_P\n");
                break;
            default:
                printf("E_BOARD_ID_MAX_BOARID or undefined!!\n");
        }
    }

    /* Added by Foxconn Alex, 2015/10/14 */
    fox_init();

    return E_TYPE_SUCCESS;
}

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
)
{
  UINT32 data;
  /* default disable the usb until we test mini usb console */
  //fpgaRegWrite(FPGA_USB_CTRL_REG, 0x0);
  //#udelay(10);

  /* 20160715 - Fix FPGA TX_DISABLE for Cisco Pilot BL requirement */
  log_printf("FPGA set TX_DISABLE, 0x7=0x0\r\n");
  fpgaRegWrite(FPGA_SFP_DISABLE_REG, 0x0);
  udelay(100);

  /* 20160908, Set FPGA TX_DISABLE again before USB reset and cntl enabled.
   * This protect the write ioctl error when USB disk present after 48P power on
   */
  fpgaRegWrite(FPGA_SFP_DISABLE_REG, 0x0);
  udelay(1000);

  /* 20151223 - Fix FPGA Reset Control Register, PoE, USB, PHYs */
  log_printf("FPGA Reset Control, Reset USB, All PHYs 0xB=0x01FF\r\n");
  fpgaRegWrite(FPGA_RESET_REG, 0x01FF);
  udelay(1000);

  /* 20160506 - Fix USB ctrl can't disabled when use Cisco BL on BE mode */
  fpgaRegWrite(FPGA_USB_CTRL_REG, 0x1);
  udelay(1000);

  /* default disable the usb until we test mini usb console */
  fpgaRegWrite(FPGA_USB_CTRL_REG, 0x0);
  udelay(1000);

  return E_TYPE_SUCCESS;
}

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
)
{
    UINT32              portId;
    E_LINER_PORT_TYPE   portType;

    port_utilsLPort2PortTypeGet(startPort, &portType, &portId);

    switch ( portType )
    {
        case E_LINER_PORT_TYPE_FIXED:

            switch ( snakeMode )
            {
                case E_LB_TEST_SNAKE_MODE_CPU:
                    switch_halCallbackRegister(rcv_fun);
                    switch_halSnakeSet(startPort, endPort);
                    break;

                case E_LB_TEST_SNAKE_MODE_PKT_GEN:
                    break;

                default:
                    return E_TYPE_INVALID_PARA;
            }
            break;

        case E_LINER_PORT_TYPE_STACKING:
        case E_LINER_PORT_TYPE_EXPANSION:
        case E_LINER_PORT_TYPE_INTER_LINK:
        default:
            return E_TYPE_UNSUPPORT;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    if ( (snakeMode == E_LB_TEST_SNAKE_MODE_CPU) || (snakeMode == E_LB_TEST_SNAKE_MODE_PKT_GEN) )
    {
        switch_halVlanReInit();
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    INT8                portTypeStr[20] = "";
    INT32               ret = E_TYPE_SUCCESS;
    UINT32              i;
    UINT32              startPort, endPort, lPort;
    UINT32              startPortId, endPortId;
    UINT32              port1Id, port2Id;
    S_PORT_CNT          currCounter, prevCounter;
    S_PORT_TX_CNT       txCnt[MAX_LOGIC_PORT_52];
    S_PORT_RX_CNT       rxCnt[MAX_LOGIC_PORT_52];
    S_PORT_ERR_CNT      errCnt[MAX_LOGIC_PORT_52];
    E_LINER_PORT_TYPE   port1Type, port2Type;
    E_LINER_PORT_TYPE   startPortType, endPortType;

    startPort   = 0;
    endPort     = 0;

    port_utilsLPort2PortTypeGet(startPort, &startPortType, &startPortId);
    port_utilsLPort2PortTypeGet(endPort, &endPortType, &endPortId);

    switch ( startPortType )
    {
        case E_LINER_PORT_TYPE_FIXED:
            strcpy(portTypeStr, "Fixed Port");
            break;

        case E_LINER_PORT_TYPE_STACKING:
        case E_LINER_PORT_TYPE_EXPANSION:
        case E_LINER_PORT_TYPE_INTER_LINK:
        default:
            break;
    }

    switch ( snakeMode )
    {
        case E_LB_TEST_SNAKE_MODE_CPU:
            break;

        case E_LB_TEST_SNAKE_MODE_PKT_GEN:
            break;

        default:
            return E_TYPE_INVALID_PARA;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSnakeCascadeSet
 *
 *  DESCRIPTION :
 *      set snake cascade trunk
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
INT32 switch_halSnakeCascadeSet
(
    IN void
)
{
    INT32 ret;
    UINT32 dev;

    for (dev = 0; dev <= 1; dev++) {
        cpssDxChTrunkHashGlobalModeSet(dev,CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E);

        if( (ret=switch_halMACRegSet(dev, 0x02E4002C, 0xfeffffff)) < 0 ) {
            return E_TYPE_IO_ERROR;
        }
        if( (ret=switch_halMACRegSet(dev, 0x02E4003C, 0xfbffffff)) < 0 ) {
            return E_TYPE_IO_ERROR;
        }

        cpssDxChPortPreambleLengthSet(dev, 24, CPSS_PORT_DIRECTION_BOTH_E, 4);
        cpssDxChPortPreambleLengthSet(dev, 26, CPSS_PORT_DIRECTION_BOTH_E, 4);
        cpssDxChPortIpgSet(dev, 24, 8);
        cpssDxChPortIpgSet(dev, 26, 8);
        cpssDxChPortSerdesPpmSet(dev, 24, 60);
        cpssDxChPortSerdesPpmSet(dev, 26, 60);
        cpssDxChPortFlowControlEnableSet(dev, 24, CPSS_PORT_FLOW_CONTROL_DISABLE_E);
        cpssDxChPortFlowControlEnableSet(dev, 26, CPSS_PORT_FLOW_CONTROL_DISABLE_E);
    }

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halSnakeSet
 *
 *  DESCRIPTION :
 *      set snake topology
 *
 *  INPUT :
 *      startPort - logical port number
 *      endPort   - logical port number
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
INT32 switch_halSnakeSet
(
    IN UINT32 startPort,
    IN UINT32 endPort
)
{
    UINT32     i;
    int        devNum=0, vlanId,vlanIDbase=100;
    UINT32     cPort=startPort, devIndex=0, tempPort=0;
    INT32      ret = E_TYPE_SUCCESS;
    E_BOARD_ID boardId = sys_utilsDevBoardIdGet();
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if((endPort-startPort)<3)
    {
        log_printf("Snake minimum port must large than 4\n");
        return E_TYPE_REG_WRITE;
    }

    /* Remove all logic port from vlan 1 */
    vlanId=1;

    for(i=startPort;i<=endPort;i++)
    {
        if( (ret = switch_halRemovePortFromVlan(i, vlanId)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }
    }

    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        if( (ret = switch_halRemoveCascadePortFromVlan(0, 24, vlanId)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        if( (ret = switch_halRemoveCascadePortFromVlan(1, 24, vlanId)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        if( (ret = switch_halRemoveCascadePortFromVlan(0, 26, vlanId)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }

        if( (ret = switch_halRemoveCascadePortFromVlan(1, 26, vlanId)) != E_TYPE_SUCCESS)
        {
            return E_TYPE_REG_WRITE;
        }
    }

    log_printf("Snake Setup : P%d - P%d \n",startPort,endPort);

    vlanId=vlanIDbase+startPort;
    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        devNum = (startPort <= 24) ? 1 : 0;
    }

    tempPort = (endPort > boardInfo.copperMaxNum) ? boardInfo.copperMaxNum : endPort;

    if ((endPort - startPort + 1) == 48 ) {
        vlanId += vlanIDbase;

        if (switch_halSnakeCascadeSet()!= E_TYPE_SUCCESS)
            return E_TYPE_IO_ERROR;

        /* VLAN port 1 and 3, 2 and 48 --> changed to VLAN P2--P3, P1--P48 for cascade balance */
        switch_halVlanCreate(1, vlanId);
        switch_halAddPortToVlan(cPort+1,vlanId,1);
        switch_halPortPvidSet(cPort+1, vlanId);
        switch_halAddPortToVlan(cPort+2,vlanId,1);
        switch_halPortPvidSet(cPort+2, vlanId);

        vlanId++;
        switch_halVlanCreate(1, vlanId);
        switch_halAddPortToVlan(cPort,vlanId,1);
        switch_halPortPvidSet(cPort, vlanId);
        switch_halVlanCreate(0, vlanId);
        switch_halAddPortToVlan(endPort,vlanId,1);
        switch_halPortPvidSet(endPort, vlanId);
        switch_halAddCascadePortToVlan(0, 24,vlanId,1);
        switch_halAddCascadePortToVlan(1, 26,vlanId,1);
        switch_halCascadePortPvidSet(0, 24, vlanId);
        switch_halCascadePortPvidSet(1, 26, vlanId);


        /* VLAN port 4 and 5, 44 and 45, 46 and 47 */
        vlanId++;
        switch_halVlanCreate(1, vlanId);
        switch_halAddPortToVlan(cPort+3,vlanId,1);
        switch_halPortPvidSet(cPort+3, vlanId);
        switch_halAddPortToVlan(cPort+4,vlanId,1);
        switch_halPortPvidSet(cPort+4, vlanId);

        vlanId++;
        switch_halVlanCreate(0, vlanId);
        switch_halAddPortToVlan(cPort+43,vlanId,1);
        switch_halPortPvidSet(cPort+43, vlanId);
        switch_halAddPortToVlan(cPort+44,vlanId,1);
        switch_halPortPvidSet(cPort+44, vlanId);

        vlanId++;
        switch_halVlanCreate(0, vlanId);
        switch_halAddPortToVlan(cPort+45,vlanId,1);
        switch_halPortPvidSet(cPort+45, vlanId);
        switch_halAddPortToVlan(cPort+46,vlanId,1);
        switch_halPortPvidSet(cPort+46, vlanId);

        /* VLAN port 6-43 */

        for (i=6;i<25;i++) {
            vlanId++;

            switch_halVlanCreate(0, vlanId);
            switch_halAddPortToVlan(i,vlanId,1);
            switch_halPortPvidSet(i, vlanId);

            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);

            switch_halAddPortToVlan(i+19,vlanId,1);
            switch_halPortPvidSet(i+19, vlanId);
        }
        return ret;
    } else if ((endPort - startPort) > 4) {
        switch_halVlanCreate(devNum, vlanId);
        switch_halAddPortToVlan(cPort,vlanId,1);
        switch_halPortPvidSet(cPort, vlanId);

        cPort +=2;
        switch_halAddPortToVlan(cPort,vlanId,1);
        switch_halPortPvidSet(cPort, vlanId);

        cPort +=2;
    } else {
        vlanId += vlanIDbase;
        if ((endPort < 25) || (endPort >=28 && endPort < 49)) {
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(cPort,vlanId,1);
            switch_halPortPvidSet(cPort, vlanId);

            cPort +=2;
            switch_halAddPortToVlan(cPort,vlanId,1);
            switch_halPortPvidSet(cPort, vlanId);

            vlanId += 1;
            switch_halAddPortToVlan(cPort-1,vlanId,1);
            switch_halPortPvidSet(cPort-1, vlanId);
            switch_halAddPortToVlan(cPort+1,vlanId,1);
            switch_halPortPvidSet(cPort+1, vlanId);

            return ret;
        }

        for (i=cPort;i<=endPort;i++) {
            devNum = ((i < 25)||(i == 49)||(i == 50)) ? 1 : 0;
            if ((cPort == 23) || (cPort == 49)) {
                switch_halAddCascadePortToVlan(0, 26,vlanId,1);
                switch_halAddCascadePortToVlan(1, 26,vlanId,1);
                switch_halCascadePortPvidSet(0, 26, vlanId);
                switch_halCascadePortPvidSet(1, 26, vlanId);

                switch_halAddCascadePortToVlan(0, 24,vlanId,1);
                switch_halAddCascadePortToVlan(1, 24,vlanId,1);
                switch_halCascadePortPvidSet(0, 24, vlanId);
                switch_halCascadePortPvidSet(1, 24, vlanId);

            }
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(i,vlanId,1);
            switch_halPortPvidSet(i, vlanId);

            if (i == (cPort + 1))
                vlanId -= 1;
            else
                vlanId += 1;
        }

        return ret;
    }

    for(i=cPort;i<(tempPort);i+=2)
    {
        vlanId=vlanIDbase+i;
        if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
        {
            if(i<=24)
            {
                devNum=1;
            }
            else
            {
                devNum=0;
            }

            if(i==25)
            {
                switch_halVlanCreate(0, vlanId);
                switch_halVlanCreate(1, vlanId);
            }
            else
            {
                switch_halVlanCreate(devNum, vlanId);
            }
        }
        else
        {
            switch_halVlanCreate(devNum, vlanId);
        }

        switch_halAddPortToVlan(i-1,vlanId,1);
        switch_halPortPvidSet(i-1, vlanId);

        switch_halAddPortToVlan(i,vlanId,1);
        switch_halPortPvidSet(i, vlanId);

        if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
        {
            if(i==25)
            {
                /* Add cascade port for 48 sku */
                switch_halAddCascadePortToVlan(0, 24,vlanId,1);
                switch_halAddCascadePortToVlan(1, 24,vlanId,1);
                switch_halCascadePortPvidSet(0, 24, vlanId);
                switch_halCascadePortPvidSet(1, 24, vlanId);

                switch_halAddCascadePortToVlan(0, 26,vlanId,1);
                switch_halAddCascadePortToVlan(1, 26,vlanId,1);
                switch_halCascadePortPvidSet(0, 26, vlanId);
                switch_halCascadePortPvidSet(1, 26, vlanId);
            }
        }
    }

    vlanId=vlanIDbase+tempPort+1;

    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        if((endPort>24) && (endPort<=48))
        {
            switch_halVlanCreate(0, vlanId);
            switch_halVlanCreate(1, vlanId);

            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);

            /* Add cascade port for 48 sku */
            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);
        }
        else if(endPort == 50)
        {
            /* 48 */
            switch_halVlanCreate(0, vlanId);
            switch_halVlanCreate(1, vlanId);

            switch_halAddPortToVlan(tempPort,vlanId,1);
            switch_halPortPvidSet(tempPort, vlanId);

            tempPort+=1;

            switch_halAddPortToVlan(tempPort,vlanId,1);
            switch_halPortPvidSet(tempPort, vlanId);

            /* Add cascade port for 48 sku */
            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);

            /* 50 */
            vlanId+=2;

            switch_halVlanCreate(0, vlanId);
            switch_halVlanCreate(1, vlanId);

            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);

            /* Add cascade port for 48 sku */
            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);
        }
        else if(endPort == 52)
        {
            /* 48 */
            switch_halVlanCreate(0, vlanId);
            switch_halVlanCreate(1, vlanId);

            switch_halAddPortToVlan(tempPort,vlanId,1);
            switch_halPortPvidSet(tempPort, vlanId);

            tempPort+=1;

            switch_halAddPortToVlan(tempPort,vlanId,1);
            switch_halPortPvidSet(tempPort, vlanId);

            /* Add cascade port for 48 sku */
            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);

            /* 50 */
            vlanId+=2;
            switch_halVlanCreate(0, vlanId);
            switch_halVlanCreate(1, vlanId);

            tempPort+=1;

            switch_halAddPortToVlan(tempPort,vlanId,1);
            switch_halPortPvidSet(tempPort, vlanId);

            tempPort+=1;

            switch_halAddPortToVlan(tempPort,vlanId,1);
            switch_halPortPvidSet(tempPort, vlanId);

            /* Add cascade port for 48 sku */
            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);

            /* 52 */
            vlanId+=2;

            switch_halVlanCreate(0, vlanId);
            switch_halVlanCreate(1, vlanId);

            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);

            /* Add cascade port for 48 sku */
            switch_halAddCascadePortToVlan(0, 24,vlanId,1);
            switch_halAddCascadePortToVlan(1, 24,vlanId,1);
            switch_halCascadePortPvidSet(0, 24, vlanId);
            switch_halCascadePortPvidSet(1, 24, vlanId);

            switch_halAddCascadePortToVlan(0, 26,vlanId,1);
            switch_halAddCascadePortToVlan(1, 26,vlanId,1);
            switch_halCascadePortPvidSet(0, 26, vlanId);
            switch_halCascadePortPvidSet(1, 26, vlanId);
        }
        else
        {
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);
        }
    }
    else if( (boardId == E_BOARD_ID_HAYWARDS_24G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_24G4G_P) )
    {
        if(endPort==26)
        {
            /* 24 */
            switch_halVlanCreate(devNum, vlanId);

            switch_halAddPortToVlan(endPort-2,vlanId,1);
            switch_halPortPvidSet(endPort-2, vlanId);

            switch_halAddPortToVlan(endPort-1,vlanId,1);
            switch_halPortPvidSet(endPort-1, vlanId);

            vlanId+=2;
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);
        }
        else if(endPort==28)
        {
            /* 24 */
            switch_halVlanCreate(devNum, vlanId);

            switch_halAddPortToVlan(endPort-4,vlanId,1);
            switch_halPortPvidSet(endPort-4, vlanId);

            switch_halAddPortToVlan(endPort-3,vlanId,1);
            switch_halPortPvidSet(endPort-3, vlanId);

            vlanId+=2;
            switch_halVlanCreate(devNum, vlanId);

            switch_halAddPortToVlan(endPort-2,vlanId,1);
            switch_halPortPvidSet(endPort-2, vlanId);

            switch_halAddPortToVlan(endPort-1,vlanId,1);
            switch_halPortPvidSet(endPort-1, vlanId);

            vlanId+=2;
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);
        }
        else
        {
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);
        }
    }
    else if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) || (boardId == E_BOARD_ID_HAYWARDS_16G2G_P) ||
             (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) || (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) )
    {
        if(endPort>boardInfo.copperMaxNum)
        {
            /* 16 */
            switch_halVlanCreate(devNum, vlanId);

            switch_halAddPortToVlan(endPort-2,vlanId,1);
            switch_halPortPvidSet(endPort-2, vlanId);

            switch_halAddPortToVlan(endPort-1,vlanId,1);
            switch_halPortPvidSet(endPort-1, vlanId);

            vlanId+=2;
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);
        }
        else
        {
            switch_halVlanCreate(devNum, vlanId);
            switch_halAddPortToVlan(endPort,vlanId,1);
            switch_halPortPvidSet(endPort, vlanId);

            switch_halAddPortToVlan(startPort+1,vlanId,1);
            switch_halPortPvidSet(startPort+1, vlanId);
        }
    }
    else
    {
        switch_halVlanCreate(devNum, vlanId);
        switch_halAddPortToVlan(endPort,vlanId,1);
        switch_halPortPvidSet(endPort, vlanId);

        switch_halAddPortToVlan(startPort+1,vlanId,1);
        switch_halPortPvidSet(startPort+1, vlanId);
    }

    return ret;
}

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
INT32 switch_halCableDiag
(
    IN UINT32 lPort,
    OUT PHY_CABLE_INFO *cableStatus
)
{
    INT32      ret = E_TYPE_SUCCESS;
    UINT32 linkStatus=0, speed=0, temp=0, retry=0;
    UINT16 regValue=0, tempReg=0;
    CPSS_VCT_CABLE_EXTENDED_STATUS_STC extendedCableStatusPtr;
    CPSS_VCT_CABLE_STATUS_STC cableStatusPtr;
    CPSS_VCT_ACTION_ENT vctAction;
    S_PORT_INFO *portInfo;
    char phyType[7][30]={"PHY_100M", "PHY_1000M", "PHY_10000M", "PHY_1000M_B", "PHY_1000M_MP",
        "PHY_1000M_MP_NO_FIBER", "PHY_1000M_MP_NO_FIBER_NG"};
    char cableLen[6][20]={"<50M", "50M~80M", "80M~110M", "110M~140M", ">140M", "UNKNOWN LEN"};
    char cableSwap[3][20]={"Cable Straight", "Cable Crossover", "Cable Not Applicable"};

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* Fiber port not support cable diagnostic, skiped */
    if ((portInfo->sfpPortId) > 0)
    {
        log_printf("Fiber port %d not support cable diagnostic.\n", lPort);
        return ret;
    }

    memset(&extendedCableStatusPtr, 0, sizeof(CPSS_VCT_CABLE_EXTENDED_STATUS_STC));
    memset(&cableStatusPtr, 0, sizeof(CPSS_VCT_CABLE_STATUS_STC));

    /* Set to page 0 */
    switch_halPhyPageNumSet(lPort, 0);

    /* Check the PHY status */
    ret = switch_halSMIRegGet(lPort, 0x0, &regValue);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get the PHY status value.\n");
        return ret;
    }

    if ( regValue & 0x800 )
    {
        log_printf("PHY power down.\n");
        return ret;
    }

    /* Check the Energy detect status, vct test should disabled it */
    ret = switch_halSMIRegGet(lPort, 0x10, &tempReg);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get energy detect value.\n");
        return ret;
    }

    log_dbgPrintf(" Energy detect value = 0x%x\n", tempReg);

    /* Disabled the energy detect bit9:8=00 */
    ret = switch_halSMIRegSet(lPort, 0x10, (tempReg & 0xFCFF));
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to disable the energy detect.\n");
        return ret;
    }

    /* Get the link status */
    ret = switch_halPortLinkStatusGet(lPort, &linkStatus);
    if (linkStatus )
    {
        ret = switch_halPortSpeedGet(lPort, &speed);
        if ( ret != E_TYPE_SUCCESS )
        {
            log_printf("Fail to get the speed.\n");
            return ret;
        }

        ret = cpssVctCableExtendedStatusGet(portInfo->devId, portInfo->portId, &extendedCableStatusPtr);
        if ( ret != GT_OK)
        {
            log_printf("[ERR]: cpssVctCableExtendedStatusGet (ret = %d).\n", ret);
            return ret;
        }

        if ( extendedCableStatusPtr.vctExtendedCableStatus.isValid == GT_TRUE \
            && extendedCableStatusPtr.accurateCableLen.isValid[0] == GT_TRUE)
        {
            cableStatus->linkStatus = TRUE;
            cableStatus->linkSpeed = speed;
            for ( retry=0; retry<4; retry++ )
                cableStatus->cableLen[retry] = extendedCableStatusPtr.accurateCableLen.cableLen[retry];
            log_printf("Pair A/B: %s\n", ((temp=(extendedCableStatusPtr.vctExtendedCableStatus.pairSwap[0]))<3)? \
                cableSwap[temp] : "Unknown");
            log_printf("Pair C/D: %s\n", ((temp=(extendedCableStatusPtr.vctExtendedCableStatus.pairSwap[1]))<3)? \
                cableSwap[temp] : "Unknown");
        }
        else
        {
            log_printf("[ERR]: failed to get port %d status.\n", lPort);
        }
    }
    else
    {
        /* Config the VCT offset offset=16.1m */
        ret = cpssVctLengthOffsetSet(portInfo->devId, portInfo->portId, 161*1000);
        if ( ret != GT_OK )
        {
            log_printf("[ERR]: failed to set the offset.\n");
            return ret;
        }

        /* Firstly call the API with vctAction = CPSS_VCT_START_E */
        vctAction = CPSS_VCT_START_E;
        ret = cpssVctCableStatusGet(portInfo->devId, portInfo->portId, vctAction, &cableStatusPtr);
        if ( ret != GT_OK)
        {
            log_printf("[ERR]: cpssVctCableStatusGet VCT_START (ret = %d).\n", ret);
            return ret;
        }

        /* Set to page 5 */
        switch_halPhyPageNumSet(lPort, 5);

        /* Get the Break link prior */
        ret = switch_halSMIRegGet(lPort, 0x1C, &regValue);
        if (ret != GT_OK )
        {
            log_printf("[ERR]: fail to get the PHY break link wait value.\n");
            return ret;
        }

        log_dbgPrintf("Break link prior value = 0x%x\n", regValue);

        /* Break link prior(bit 12)to 1 would wait 1.5s, so delay 2s */
        if ( !(regValue & 0x1000) )
        {
            sleep(2);
        }

        /* Secondly call the API with vctAction = CPSS_VCT_GET_RES_E */
        for (retry=0; retry<PHY_LPI_RETRY_TIME; retry++)
        {
            vctAction = CPSS_VCT_GET_RES_E;
            ret = cpssVctCableStatusGet(portInfo->devId, portInfo->portId, vctAction, &cableStatusPtr);
            //log_printf("CPSS_VCT_GET_RES-cpssVctCableStatusGet ret=%d\n",ret);
            /* if get the resource successful, break */
            if ( ret == GT_OK)
                break;

            /* Delay 500 ms */
            udelay(500000);
        }

        /* Check the VCT_GET_RES result */
        if ( retry == PHY_LPI_RETRY_TIME )
            log_printf("[ERR]: Retry %d times with cpssVctCableStatusGet VCT_GET_RES (port %d, ret = %d).\n",PHY_LPI_RETRY_TIME,lPort, ret);

        cableStatus->linkStatus = FALSE;
    for ( retry=0; retry<4; retry++ )
            cableStatus->cableStatus[retry] = (UINT32)(cableStatusPtr.cableStatus[retry].testStatus);
        for ( retry=0; retry<4; retry++ )
            cableStatus->cableLen[retry] = cableStatusPtr.cableStatus[retry].errCableLen;

        log_dbgPrintf("Phy Type: %s\n", (temp=(UINT32)(cableStatusPtr.phyType)) < 7? phyType[temp]:"Unknown");
        log_dbgPrintf("Normal Cable Len: %s\n", (temp=(UINT32)(cableStatusPtr.normalCableLen))<6? cableLen[temp]:"Unknown");

        /* Finally call the API with vctAction = CPSS_VCT_ABORT_E */
        vctAction = CPSS_VCT_ABORT_E;
        ret = cpssVctCableStatusGet(portInfo->devId, portInfo->portId, vctAction, &cableStatusPtr);
        if ( ret != GT_OK )
        {
            log_printf("[ERR]: cpssVctCableStatusGet VCT_ABORT (ret = %d).\n", ret);
            return ret;
        }
    }

    /* Set to page 0 */
    switch_halPhyPageNumSet(lPort, 0);

    /* Restore the energy detect */
    ret = switch_halSMIRegSet(lPort, 0x10, tempReg);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to restore the energy detect.\n");
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_halMMDAccess
 *
 *  DESCRIPTION :
 *      switch MMD Access
 *
 *  INPUT :
 *      lPort - logical port number
 *      devId   - device id address
 *      regAddr   - register offset address
 *      rw_flag   - read/write flag
 *      regValue   - read/write value
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
INT32 switch_halMMDAccess
(
    IN UINT32 lPort,
    IN UINT32 devId,
    IN UINT32 regAddr,
    IN UINT32 rw_flag,
    IN UINT16 *regValue
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 reg=0;


    /* 1. Set the device ID */
    ret = switch_halSMIRegSet(lPort, 0xd, devId);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to set device Id %d.\n", devId);
        return ret;
    }

    /* 2. Set the register address */
    ret = switch_halSMIRegSet(lPort, 0xe, regAddr);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to set register address 0x%x.\n", regAddr);
        return ret;
    }

    /* 3. Set the operation mode */
    ret = switch_halSMIRegSet(lPort, 0xd, (devId | 0x4000));
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to set operation mode.\n");
        return ret;
    }

    /* write */
    if ( rw_flag == 0)
    {
        reg = *regValue;
        ret = switch_halSMIRegSet(lPort, 0xe, reg);
    }
    /* read */
    else
    {
        ret = switch_halSMIRegGet(lPort, 0xe, &reg);
        *regValue = reg;
    }
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to read or write value.\n");
        return ret;
    }

    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 regValue=0;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* Fiber port not support LPI, skip */
    if ((portInfo->sfpPortId) > 0)
    {
        log_dbgPrintf("Fiber port %d not support LPI mode.\n", lPort);
        return ret;
    }

    /* Set to global write register */
    switch_halPhyPageNumSet(lPort, 0xC000);

    /* Set advertisement register*/
    if ( lpiMode == E_PHY_LPI_ENABLED)
        regValue = 0x6;
    else
        regValue = 0x0;

    ret = switch_halMMDAccess(lPort, 0x7, 0x3C, 0, &regValue);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to enable the EEE advertisement register.\n");
        return ret;
    }

    /* Reset the PHY */
    ret = switch_halSMIRegSet(lPort, 0x0, 0x9140);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: reset the PHY failed.\n");
        return ret;
    }

    /* Restore the page to 0 */
    ret = switch_halSMIRegSet(lPort, 0x16, 0x0);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: restore the PHY page failed.\n");
        return ret;
    }

    /* Set to page 18 */
    switch_halPhyPageNumSet(lPort, 18);

    /* Get the LPI mode */
    ret = switch_halSMIRegGet(lPort, 0x0, &regValue);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get the PHY LPI mode value.\n");
        return ret;
    }

    log_dbgPrintf("Port %2d current LPI mode:    %s", lPort, (regValue&0x1)? "Enabled":"Disabled");

    /* Set the LPI mode */
    if ( lpiMode == E_PHY_LPI_ENABLED)
        regValue |= 0x1;
    else
        regValue &= 0xFFFE;

    ret = switch_halSMIRegSet(lPort, 0x0, regValue);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to %s the PHY LPI mode.\n", (lpiMode==E_PHY_LPI_ENABLED)? "Enabled":"Disabled");
        return ret;
    }

    log_dbgPrintf("  >>  %s\n",(lpiMode==E_PHY_LPI_ENABLED)? "Enabled":"Disable");

    return ret;
}

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
 *      eeeStatus - EEE status
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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 regValue=0;
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    /* Fiber port not support LPI, skip */
    if ((portInfo->sfpPortId) > 0)
    {
        log_printf("Port %2d LPI mode:    Unsupport\n", lPort);
    *lpiMode = E_PHY_LPI_SHOW;
        return ret;
    }

    /* Set to page 18 */
    switch_halPhyPageNumSet(lPort, 18);

    /* Get the LPI mode */
    ret = switch_halSMIRegGet(lPort, 0x0, &regValue);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get the PHY LPI mode value.\n");
        return ret;
    }

    *lpiMode = (regValue&0x1)? E_PHY_LPI_ENABLED : E_PHY_LPI_DISABLED;

    return ret;
}

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
)
{
    INT32  ret = E_TYPE_DEV_MISMATCH;
    UINT32 regValue=0, chipId=0;
    E_BOARD_ID boardId = sys_utilsDevBoardIdGet();

    /* read cpss reserved register 0x4c for device id   */
    if ( switch_halMACRegGet(SWITCH_HAL_AC3_DEVICE, SWITCH_HAL_AC3_MAC_DEVICE_ID_REG, &regValue) == E_TYPE_SUCCESS )
    {
        chipId = (regValue & 0x0ffff0) >> 4;

        if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) ||
            (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) ||
            (boardId == E_BOARD_ID_HAYWARDS_24G4G_T) ||
            (boardId == E_BOARD_ID_HAYWARDS_24G4G_P) )
        {
            /* Merged Haywards MAC ID 98XC323 to boardid test command */           
            if (chipId == SWITCH_XCAT_AC3_C323_DEV_ID)
            {
                log_printf("Device ID matches 98DXC323, boardID %ld belongs to %s (Haywards)\r\n", boardId, boardName[boardId]);
                ret = E_TYPE_SUCCESS;
            } 
            else if (chipId == SWITCH_XCAT_AC3_3236_DEV_ID) /*Haywards2 onboard with MAC 98DX3236 2017-01-14 Lowell*/
            {
                log_printf("Device ID matches 98DX3236, boardID %ld belongs to %s (Haywards2)\r\n", boardId, boardName[boardId]);
                ret = E_TYPE_SUCCESS;
            }
            else
            {
                log_printf("Get wrong device ID %lx, boardID %ld belongs to %s\r\n", chipId, boardId, boardName[boardId]);
            }
        }
        else if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) ||
                 (boardId == E_BOARD_ID_HAYWARDS_16G2G_P))
        {
            if (chipId == SWITCH_XCAT_AC3_3234_DEV_ID)
            {
                log_printf("Device ID matches 98DX3234, boardID %ld belongs to %s\r\n", boardId, boardName[boardId]);
                ret = E_TYPE_SUCCESS;
            }
            else
            {
                log_printf("Get wrong device ID %lx, boardID %ld belongs to %s\r\n", chipId, boardId, boardName[boardId]);
            }
        }
        else if( (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) ||
                 (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) )
        {
            if (chipId == SWITCH_XCAT_AC3_3233_DEV_ID)
            {
                log_printf("Device ID matches 98DX3233, boardID %ld belongs to %s\r\n", boardId, boardName[boardId]);
                ret = E_TYPE_SUCCESS;
            }
            else
            {
                log_printf("Get wrong device ID %lx, boardID %ld belongs to %s\r\n", chipId, boardId, boardName[boardId]);
            }
        }
        else
        {
            ret = E_TYPE_DEV_MISMATCH;
        }
    }
    else
    {
        ret = E_TYPE_DEV_MISMATCH;
    }

    return ret;
}

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
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT32 readDevMACId = 0;
    E_BOARD_ID  boardId;

    /* 20170626 - Try to read device MAC chip Id information */
    readDevMACId  = sys_utilsDevMACChipIdGet();
    
    boardId = sys_utilsDevBoardIdGet();
   
    switch(boardId)
    {
        case E_BOARD_ID_HAYWARDS_8G2G_T:
        case E_BOARD_ID_HAYWARDS_8G2G_P:
            switch_tuneTxManualSet(1, 1, 1, 5, 0, 9);
            switch_tuneTxManualSet(0, 1, 1, 5, 0, 9);
            log_printf("Init Haywards_8G2G qsgmii config \n");
            break;
        case E_BOARD_ID_HAYWARDS_16G2G_T:
        case E_BOARD_ID_HAYWARDS_16G2G_P:
           switch_tuneTxManualSet(3, 1, 1, 5, 0, 7);
           switch_tuneTxManualSet(2, 1, 1, 5, 0, 7);
           switch_tuneTxManualSet(1, 1, 1, 5, 0, 9);
           switch_tuneTxManualSet(0, 1, 1, 5, 0, 9);
           log_printf("Init Haywards_16G2G qsgmii config \n");
           break;
       case E_BOARD_ID_HAYWARDS_24G4G_T:
       case E_BOARD_ID_HAYWARDS_24G4G_P:
           switch_tuneTxManualSet(0, 1, 0, 5, 0, 7);
           switch_tuneTxManualSet(1, 1, 0, 5, 0, 7);
           switch_tuneTxManualSet(2, 1, 0, 8, 0, 0);
           switch_tuneTxManualSet(3, 1, 0, 8, 0, 0);
           switch_tuneTxManualSet(4, 1, 0, 8, 0, 0);
           switch_tuneTxManualSet(5, 1, 0, 8, 0, 0);
           
           /* Add Haywards2 MAC chip is 98DX3236 */
           if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) 
           {
               /* 03282017 -- updated 24T/P  SFP+ ports tx serdes init */
               switch_tuneTxManualSet(6, 1, 0, 0x11, 0, 6);
               switch_tuneTxManualSet(7, 1, 0, 0x11, 0, 6);
               switch_tuneTxManualSet(8, 1, 0, 0x11, 0, 6);
               switch_tuneTxManualSet(9, 1, 0, 0x11, 0, 6);
           }

           /* RX tune */
           switch_tuneRxTuneSet(0, 1, 0, 1, 6, 7);
           switch_tuneRxTuneSet(1, 1, 0, 1, 6, 7);
           switch_tuneRxTuneSet(2, 1, 0, 1, 6, 7);
           switch_tuneRxTuneSet(3, 1, 0, 1, 6, 7);
           switch_tuneRxTuneSet(4, 1, 0, 1, 6, 7);
           switch_tuneRxTuneSet(5, 1, 0, 1, 6, 7);
           
           if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
               log_printf("Init Haywards2_24G4XG qsgmii config \n");
           } else {
               log_printf("Init Haywards_24G4G qsgmii config \n");
           }     
           break;
       case E_BOARD_ID_HAYWARDS_48G4G_T:
       case E_BOARD_ID_HAYWARDS_48G4G_P:
           switch_tuneTxManualSet(0, 1, 1, 0x7, 0, 0xf);
           switch_tuneTxManualSet(1, 1, 1, 0x7, 0, 0xe);
           switch_tuneTxManualSet(2, 1, 1, 0xd, 0, 0xf);
           switch_tuneTxManualSet(3, 1, 1, 0x7, 0, 0xe);
           switch_tuneTxManualSet(4, 1, 1, 0xe, 1, 0xf);
           switch_tuneTxManualSet(5, 1, 1, 0x7, 1, 0xb);
           switch_tuneTxManualSet(6, 1, 1, 0x6, 1, 0xd);
           switch_tuneTxManualSet(7, 1, 1, 0x6, 0, 0xd);
           switch_tuneTxManualSet(8, 1, 1, 0xe, 2, 0xf);
           switch_tuneTxManualSet(9, 1, 1, 0xe, 2, 0xf);
           switch_tuneTxManualSet(10, 1, 1, 0x7, 0, 0xe);
           switch_tuneTxManualSet(11, 1, 1, 0x8, 0, 0xf);

           /* Add Haywards2 MAC chip is 98DX3236 */
           if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) 
           {
               /* 03282017 -- updated 48T/P SFP+ ports tx serdes init */
               switch_tuneTxManualSet(12, 1, 0, 0x10, 0, 7);
               switch_tuneTxManualSet(13, 1, 0, 0x10, 0, 7);
               switch_tuneTxManualSet(14, 1, 0, 0x10, 0, 7);
               switch_tuneTxManualSet(15, 1, 0, 0x10, 0, 7);
           }
            
           /* Rx tune */
           switch_tuneRxTuneSet(0, 1, 1, 1, 5, 0x9);
           switch_tuneRxTuneSet(1, 1, 1, 1, 5, 0x12);
           switch_tuneRxTuneSet(2, 1, 1, 1, 5, 0x12);
           switch_tuneRxTuneSet(3, 1, 1, 1, 4, 0x10);
           switch_tuneRxTuneSet(4, 1, 1, 1, 3, 0x11);
           switch_tuneRxTuneSet(5, 1, 1, 1, 3, 0x10);
           switch_tuneRxTuneSet(6, 1, 1, 1, 3, 0x9);
           switch_tuneRxTuneSet(7, 1, 1, 1, 4, 0xf);
           switch_tuneRxTuneSet(8, 1, 1, 1, 4, 0xf);
           switch_tuneRxTuneSet(9, 1, 1, 1, 4, 0xe);
           switch_tuneRxTuneSet(10, 1, 1, 1, 4, 0x12);
           switch_tuneRxTuneSet(11, 1, 1, 1, 3, 0x12);
           
           if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
               log_printf("Init Haywards2_48G4XG qsgmii config \n");
           } else {
               log_printf("Init Haywards_48G4G qsgmii config \n");
           }
           break;
       default:
           log_printf("Unsupported unit ! \n");
           ret = E_TYPE_INVALID_PARA;
           break;
    }

    return ret;
}
