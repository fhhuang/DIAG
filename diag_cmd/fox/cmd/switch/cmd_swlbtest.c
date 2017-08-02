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
***      cmd_swlbtest.c
***
***    DESCRIPTION :
***      for switch port packet path test
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
#include "foxCommand.h"
#include "cmn_type.h"
#include "porting.h"

#include "switch_hal.h"
#include "switch_lb.h"

#include "sys_utils.h"
#include "port_utils.h"

#include "fox_init.h"

#include "err_type.h"
#include "log.h"
/*==========================================================================
 *
 *      Constant
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
 *      Static Variable segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */


/*==========================================================================
 *
 *      Local Function Body segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      portErrCntDump
 *
 *  DESCRIPTION :
 *      Dump error counter of a logical port
 *
 *  INPUT :
 *      lPort        - logical port number
 *      counter      - error counter structure
 *      lPortTypeStr - Port type
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
static void portErrCntDump
(
    IN UINT32       port,
    IN S_PORT_CNT   counter,
    IN INT8         *lPortTypeStr
)
{
    if ( counter.errCnt.txCrcNum != 0 )
        log_dbgPrintf("%s %d TX CRC = %10llu\n", lPortTypeStr, port, counter.errCnt.txCrcNum);

    if ( counter.errCnt.txDeferred != 0 )
        log_dbgPrintf("%s %d TX deferred = %10llu\n", lPortTypeStr, port, counter.errCnt.txDeferred);

    if ( counter.errCnt.txExcessiveCollision != 0 )
        log_dbgPrintf("%s %d TX excessive collision = %10llu\n", lPortTypeStr, port, counter.errCnt.txExcessiveCollision);

    if ( counter.errCnt.txSentMultiple != 0 )
        log_dbgPrintf("%s %d TX multiple = %10llu\n", lPortTypeStr, port, counter.errCnt.txSentMultiple);

    if ( counter.errCnt.txCollisionNum != 0 )
        log_dbgPrintf("%s %d TX collision = %10llu\n", lPortTypeStr, port, counter.errCnt.txCollisionNum);

    if ( counter.errCnt.rxOverRunNum != 0 )
        log_dbgPrintf("%s %d RX overrun = %10llu\n", lPortTypeStr, port, counter.errCnt.rxOverRunNum);

    if ( counter.errCnt.rxUnderSizeNum != 0 )
        log_dbgPrintf("%s %d RX under size = %10llu\n", lPortTypeStr, port, counter.errCnt.rxUnderSizeNum);

    if ( counter.errCnt.rxFragmentsNum != 0 )
        log_dbgPrintf("%s %d RX fragments = %10llu\n", lPortTypeStr, port, counter.errCnt.rxFragmentsNum);

    if ( counter.errCnt.rxOverSizeNum != 0 )
        log_dbgPrintf("%s %d RX oversize = %10llu\n", lPortTypeStr, port, counter.errCnt.rxOverSizeNum);

    if ( counter.errCnt.rxJabbberNum != 0 )
        log_dbgPrintf("%s %d jRX abber = %10llu\n", lPortTypeStr, port, counter.errCnt.rxJabbberNum);

    if ( counter.errCnt.rxErrorNum != 0 )
        log_dbgPrintf("%s %d RX error = %10llu\n", lPortTypeStr, port, counter.errCnt.rxErrorNum);

    if ( counter.errCnt.rxCrcNum != 0 )
        log_dbgPrintf("%s %d RX CRC = %10llu\n", lPortTypeStr, port, counter.errCnt.rxCrcNum);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      portCntDump
 *
 *  DESCRIPTION :
 *      Dump counter of a logical port
 *
 *  INPUT :
 *      lPort        - logical port number
 *      counter      - counter structure
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
static void portCntDump
(
    IN UINT32       lPort,
    OUT S_PORT_CNT  portCounter
)
{
    log_printf("Port %d\n", lPort);

    log_printf("TX UNICAST   : %10llu\r\nRX UNICAST   : %10llu\n" \
               "TX MCAST     : %10llu\r\nRX MCAST     : %10llu\n" \
               "TX BCAST     : %10llu\r\nRX BCAST     : %10llu\n",
               portCounter.txCnt.txUnicastNum,   portCounter.rxCnt.rxUnicastNum,
               portCounter.txCnt.txMulticastNum, portCounter.rxCnt.rxMulticastNum,
               portCounter.txCnt.txBroadcastNum, portCounter.rxCnt.rxBroadcastNum);

    log_printf("TX CRC       : %10llu\r\nRX CRC       : %10llu\n" \
               "RX Overrun   : %10llu\r\nRX Undersize : %10llu\n" \
               "RX Fragments : %10llu\r\nRX Oversize  : %10llu\n",
               portCounter.errCnt.txCrcNum,      portCounter.errCnt.rxCrcNum,
               portCounter.errCnt.rxOverRunNum,  portCounter.errCnt.rxUnderSizeNum,
               portCounter.errCnt.rxFragmentsNum, portCounter.errCnt.rxOverSizeNum);
    log_printf("RX Jabber    : %10llu\r\nRX Error     : %10llu\n" \
               "==============================================================\n",
               portCounter.errCnt.rxJabbberNum, portCounter.errCnt.rxErrorNum);
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      vlanPort_utilsParaPass
 *
 *  DESCRIPTION :
 *      pass user input parameter
 *
 *  INPUT :
 *      argv - parameter
 *
 *  OUTPUT :
 *      *portList[] - get input port list
 *      *portNum  - get input port number
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
 INT32 vlanPort_utilsParaPass
 (
       IN INT8 *ptr,
       OUT UINT32 *portList,
       OUT INT8 *portNum
 )
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 startPort=0;
    UINT32 endPort=0,tempPort;
    UINT8 i=0,j=0,*tempptr;
    tempptr=ptr;
    /*enter input port is valid*/
    if((strstr(ptr, ",,") != NULL) || (strstr(ptr, "--") != NULL)||(strstr(ptr, ",-")!= NULL) || (strstr(ptr, "-," )!= NULL))
    {
        return E_TYPE_INVALID_PARA;
    }
    while(*tempptr!='\0')
    {
        if(!(((*tempptr >='0' ) && (*tempptr <= '9')) ||(*tempptr == '-') || (*tempptr == ',') ) )
        {
            return E_TYPE_INVALID_PARA;
        }
        tempptr++;
    }

    log_dbgPrintf("get Port \n");
    ptr = strtok(ptr, ",");
    do
    {
        if(strstr(ptr,"-")!=NULL)
        {
            startPort = simple_strtoul(ptr, NULL, 10);
            endPort=simple_strtoul(strstr(ptr,"-")+1, NULL, 10);
            if(endPort < startPort)
            {
                tempPort=endPort;
                endPort=startPort;
                startPort=tempPort;
            }
            tempPort=startPort;
            for(i=*portNum; tempPort<=endPort; tempPort++)
            {
                if( (tempPort<1) || (tempPort>port_utilsTotalFixedPortGet()) )
                {
                    log_printf("Port number out of range\n");
                    return E_TYPE_INVALID_PARA;
                }
        /*check port repetitipn*/
                for(j=0;j<i;j++)
                {
                    if(tempPort==portList[j])
                    {
                        break;
                    }
                }
                if(j==i)
                {
                    portList[i]=tempPort;
                    log_dbgPrintf("%d:  \t%d\n",i,portList[i]);
                    i++;
                }
            }
            *portNum=i;
        }
        else
        {
            tempPort=simple_strtoul(ptr, NULL, 10);
            if( (tempPort<1) || (tempPort>port_utilsTotalFixedPortGet()) )
            {
                log_printf("Port number out of range\n");
                return E_TYPE_INVALID_PARA;
            }
        /*check port repetitipn*/
            for(j=0;j<i;j++)
            {
                if(tempPort==portList[j])
                {
                    break;
                }
            }
            if(j==i)
            {
                portList[i]=tempPort;
                log_dbgPrintf("%d:  \t%d\n",i,portList[i]);
                i++;
            }
            *portNum=i;
        }
        ptr=strtok(NULL, ",");
    }while(ptr != NULL);

    return ret;
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      vlanShow
 *
 *  DESCRIPTION :
 *      Show VLAN status
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
static INT32 vlanShow
(
    IN void
)
{
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_BOOL isValid=0;
    E_BOARD_ID boardId = sys_utilsDevBoardIdGet();
    UINT32 lPort =0, totalPort=0, vid=0, devId=0;
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    /* Clear Vlan info */
    memset(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));

    /* Fill ports and tagging members */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    memset(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
    {
        totalPort = 24;
    }
    else if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) || (boardId == E_BOARD_ID_HAYWARDS_16G2G_P) )
    {
        totalPort = 18;
    }
    else if( (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) || (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) )
    {
        totalPort = 10;
    }
    else
    {
        totalPort = 28;
    }

    log_printf("VLAN\tPorts\t\r\n");
    log_printf("==================================================================================\r\n");

    for(devId=0; devId < port_utilsTotalDevGet(); devId++)
    {
        for(vid=1;vid<=4094;vid++)
        {
            /* Check Vlan exist or not first */
            if (ret = cpssDxChBrgVlanEntryRead(devId, vid, &portsMembers, &portsTagging, &cpssVlanInfo, &isValid, &portsTaggingCmd) != E_TYPE_SUCCESS)
            {
                log_printf("Failed to read vlan Entry, ret %ld.\r\n", ret);
                return ret;
            }

            if(isValid == TRUE)
            {
                log_printf("%ld\t", vid);
                for(lPort=1;lPort<=totalPort;lPort++)
                {
                    portInfo=port_utilsLPortInfoGet(lPort);
                    if( portInfo == NULL )
                    {
                        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
                        return E_TYPE_DATA_GET;
                    }

                    if(portsMembers.ports[0] & (1<<portInfo->portId))
                    {
                        if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
                        {
                            if(devId == 0x0)
                            {
                                log_printf("%ld ", lPort+24);
                            }
                            else if(devId == 0x1)
                            {
                                log_printf("%ld ", lPort);
                            }
                        }
                        else
                        {
                            log_printf("%ld ", lPort);
                        }
                    }
                }

                if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
                {
                    if(devId == 0x1)
                    {
                        for(lPort=49;lPort<=50;lPort++)
                        {
                            portInfo=port_utilsLPortInfoGet(lPort);
                            if( portInfo == NULL )
                            {
                                log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
                                return E_TYPE_DATA_GET;
                            }

                            if(portsMembers.ports[0] & (1<<portInfo->portId))
                            {
                                log_printf("%ld ", lPort);
                            }
                        }
                    }
                    else if(devId == 0x0)
                    {
                        for(lPort=51;lPort<=52;lPort++)
                        {
                            portInfo=port_utilsLPortInfoGet(lPort);
                            if( portInfo == NULL )
                            {
                                log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
                                return E_TYPE_DATA_GET;
                            }

                            if(portsMembers.ports[0] & (1<<portInfo->portId))
                            {
                                log_printf("%ld ", lPort);
                            }
                        }
                    }
                }

                log_printf("\r\n");
            }
        }
    }

    return ret;
}
INT32 do_swlbtest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    BOOL isSfp = FALSE, skipLb = FALSE;
    INT8 lPortTypeStr[20]="N/A";
    INT32 ret=E_TYPE_SUCCESS, retValue=E_TYPE_SUCCESS;
    UINT8  sfpTaskFlag=0;
    UINT32 startPort=0, endPort=0, pattern=0xff, size=1518, numPkt=10, port;
    UINT32 txLPort, rxLPort, lPortBase;
    UINT32 paraCnt;
    UINT32 speed=0, delay_sec=0;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LB_TEST_TYPE    lbTestType=E_LB_TEST_TYPE_MAX;
    S_PORT_CNT  txCounter, rxCounter;
    S_PORT_INFO *portInfo;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    /* Set socket non-block mode for ctrlc (getc) without waiting for enter */
    uartAttrSet(0);

    if( strstr(argv[0], "swlbtest") )
    {
        if( argc != 8)
        {
            ERR_PRINT_CMD_USAGE("swlbtest");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if( strcmp("mac", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_MAC;
        }
        else if( strcmp("phy", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_PHY;
        }
        else if( strcmp("exts", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_EXT_S;
        }
        else if( strcmp("extp", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_EXT_P;
        }
        else
        {
            log_printf("Unsupport parameter\r\n");
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt = 2;
        if( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE("swlbtest");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        /* Get pattern */
        if( strcmp("cjpat", argv[paraCnt]) == 0 )
        {
            pattern = E_SW_LB_PATTERN_CJPAT;
        }
        else if( strcmp("random", argv[paraCnt]) == 0 )
        {
            pattern = E_SW_LB_PATTERN_RANDOM;
        }
        else
        {
            pattern = simple_strtoul(argv[paraCnt], NULL, 16);
        }

        if(pattern > 0xffff)
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        /* Get packet length */
        paraCnt++;
        size = simple_strtoul(argv[paraCnt], NULL, 10);
        if( (size<FOXCONN_PKT_MIN_SIZE) || (size>FOXCONN_PKT_MAX_SIZE) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        /* Get packet number */
        paraCnt++;
        numPkt = simple_strtoul(argv[paraCnt], NULL, 10);
        if( numPkt <= 0 || numPkt > PKT_LOOPBACK_MAX_NUM )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        switch (lPortType)
        {
            case E_LINER_PORT_TYPE_FIXED:
                paraCnt++;

                /* For "all" ports, speed is necessary. */
                if( !startPort || startPort <= port_utilsTotalFixedPortGet())
                {
                    if(!argv[paraCnt])
                    {
                        ERR_PRINT_CMD_USAGE("swlbtest");
                        ret = E_TYPE_INVALID_CMD_FORMAT;
                        goto __CMD_ERROR;
                    }

                    speed = simple_strtoul(argv[paraCnt], NULL, 10);

                    switch (speed)
                    {
                        case 10:
                        case 100:
                        case 1000:
                            if( startPort == 0 ) /* all */
                            {
                                if(lbTestType == E_LB_TEST_TYPE_PHY)
                                {
                                    endPort = boardInfo.copperMaxNum;
                                }
                            }
                            if (lbTestType == E_LB_TEST_TYPE_PHY)
                            {
                                if( (startPort > boardInfo.copperMaxNum) || (endPort > boardInfo.copperMaxNum) )
                                {
                                    ret = E_TYPE_INVALID_PARA;
                                    goto __CMD_ERROR;
                                }
                            }
                            break;
                        case 10000:
                            if( startPort == 0 ) /* all */
                            {
                                startPort = boardInfo.firstfiberNum;
                            }

                            if( startPort < boardInfo.firstfiberNum)
                            {
                                ret = E_TYPE_INVALID_PARA;
                                goto __CMD_ERROR;
                            }
                            break;
                        default:
                            ret = E_TYPE_INVALID_PARA;
                            goto __CMD_ERROR;
                    }
                }
                break;
            case E_LINER_PORT_TYPE_INTER_LINK:
            case E_LINER_PORT_TYPE_STACKING:
            case E_LINER_PORT_TYPE_EXPANSION:
            default:
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
        }

        /* Indicate the port is sfp or not */
        paraCnt++;
        if( argv[paraCnt] )
        {
            isSfp = simple_strtoul(argv[paraCnt], NULL, 10);
        }

        switch (lbTestType)
        {
            case E_LB_TEST_TYPE_MAC:
            case E_LB_TEST_TYPE_PHY:
                log_printf("Make sure no cable on all ports\r\n");
                break;
            case E_LB_TEST_TYPE_EXT_S:
                if( lPortType != E_LINER_PORT_TYPE_INTER_LINK )
                {
                    log_printf("Make sure single-port loopback stub are installed on all ports\r\n");
                }
                break;
            case E_LB_TEST_TYPE_EXT_P:
                if( lPortType != E_LINER_PORT_TYPE_INTER_LINK )
                {
                    log_printf("Make sure cables are installed on all port pairs (P1, P2), (P3, P4) ...\r\n");
                }
                break;
            default:
                return E_TYPE_INVALID_PARA;
        }

        memset(&txCounter, 0 , sizeof(S_PORT_CNT));
        memset(&rxCounter, 0 , sizeof(S_PORT_CNT));

        if( startPort == 0 ) /* all */
        {
            port = 1;
            if(lbTestType == E_LB_TEST_TYPE_PHY)
            {
                endPort = boardInfo.copperMaxNum;
            }
            else if( (speed == 10) || (speed == 100) )
            {
                endPort = boardInfo.copperMaxNum;
            }
            else if( speed == 1000 || speed == 10000)
            {
                endPort = boardInfo.lPortMaxNum;
            }
        }
        else
        {
            port = startPort;
        }

        /* Before CPSS provide valid semaphore or mutex, suspend SFP LED task to avoid critical section.
         */
        switch_halSfpLedTaskFlagGet(&sfpTaskFlag);
        if(sfpTaskFlag == TRUE)
        {
            switch_halSfpLedTaskFlagSet(FALSE);
        }

        for (; port<=endPort; port++)
        {
            skipLb = FALSE;
            if( startPort == 0 ) /* all */
            {
                txLPort = lPortBase + port;
            }
            else
            {
                txLPort = port;
            }

            switch (lPortType)
            {
                case E_LINER_PORT_TYPE_FIXED:
                    if( (port<=port_utilsTotalFixedPortGet()) && (lbTestType==E_LB_TEST_TYPE_EXT_P) )
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
                    break;
                case E_LINER_PORT_TYPE_STACKING:
                default:
                    break;
            }

            portInfo = port_utilsLPortInfoGet(txLPort);

            switch (lPortType)
            {
                case E_LINER_PORT_TYPE_FIXED:
                    switch (speed)
                    {
                        case 1000:
                        case 10000:
                            break;
                        case 100:
                            if( (portInfo->capability & (E_PORT_SPEED_CAP_100M)) != E_PORT_SPEED_CAP_100M)
                            {
                                skipLb = TRUE;
                            }
                            break;
                        case 10:
                            if( (portInfo->capability & (E_PORT_SPEED_CAP_10M)) != E_PORT_SPEED_CAP_10M)
                            {
                                skipLb = TRUE;
                            }
                            break;
                    }
                    break;
                default:
                    break;
            }

            if (lbTestType==E_LB_TEST_TYPE_PHY)
            {
                if( (portInfo->lbCap & (E_PORT_LB_CAP_PHY)) != E_PORT_LB_CAP_PHY)
                {
                    skipLb = TRUE;
                }
            }

            if( skipLb == TRUE )
            {
                continue;
            }

            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                log_printf("Start %s %d loopback test in Speed %dM\n", lPortTypeStr, txLPort, speed);
                log_printf("Start %s %d loopback test in Speed %dM\n", lPortTypeStr, rxLPort, speed);
            }
            else
            {
                log_printf("Start %s %d loopback test in Speed %dM\n", lPortTypeStr, port, speed);
            }

            /* Use ARP test packet to perform loopback test */
            switch_lbTest(txLPort, rxLPort, lbTestType, pattern, size, numPkt, speed, isSfp, &txCounter, &rxCounter);

            /* txLPort and rxLPort counter get move to switch_lbTest function 1017-02-10 */

            /* Clear counter after test */
            if(switch_halClearPortCounter(txLPort) != E_TYPE_SUCCESS)
            {
                ret = E_TYPE_IO_ERROR;
                goto __CMD_ERROR;
            }

            /* Counter clear on read. */
            if(switch_halPortCounterClearFlagSet(txLPort, FALSE) != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, txLPort);
                goto __CMD_ERROR;
            }

            if(rxLPort != txLPort)
            {
                if(switch_halClearPortCounter(rxLPort) != E_TYPE_SUCCESS)
                {
                    ret = E_TYPE_PORT_COUNTER_CLEAR_ERROR;
                    goto __CMD_ERROR;
                }

                /* Counter clear on read. */
                if(switch_halPortCounterClearFlagSet(rxLPort, FALSE) != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, rxLPort);
                    goto __CMD_ERROR;
                }
            }

            if ( (ret != E_TYPE_SUCCESS ) || (rxCounter.rxCnt.rxMulticastNum != numPkt) || (txCounter.rxCnt.rxMulticastNum != numPkt))
            {
                portErrCntDump(txLPort, txCounter, lPortTypeStr);
                if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                {
                    portErrCntDump(rxLPort, rxCounter, lPortTypeStr);
                }

                if ( txCounter.txCnt.txMulticastNum == 0 || rxCounter.rxCnt.rxMulticastNum == 0 )
                {
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, port, txCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);
                }
                else if ( txCounter.txCnt.txMulticastNum != rxCounter.rxCnt.rxMulticastNum )
                {
                    log_dbgPrintf("%s %d TX %llu != RX %llu\r\n", lPortTypeStr, port, txCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);
                }

                log_dbgPrintf("%s %d CPU TX %u != RX %llu\r\n", lPortTypeStr, port, numPkt, rxCounter.rxCnt.rxMulticastNum);

                if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                {
                    log_dbgPrintf("%s %d %dM loopback test (%llu)\r\n", lPortTypeStr, txLPort, speed, txCounter.txCnt.txMulticastNum);
                    log_dbgPrintf("%s %d %dM loopback test (%llu)\r\n", lPortTypeStr, rxLPort, speed, rxCounter.rxCnt.rxMulticastNum);
                    log_cmdPrintf(E_LOG_MSG_FAIL, "Loopback test %s (%d, %d) %dM\r\n", lPortTypeStr, txLPort, rxLPort, speed);
                }
                else
                {
                    log_cmdPrintf(E_LOG_MSG_FAIL, "Loopback test %s %d %dM (%llu)\r\n", lPortTypeStr, port, speed, rxCounter.rxCnt.rxMulticastNum);
                }

                if( ret == E_TYPE_CTRL_C )
                {
                    goto __CMD_ERROR;
                }
            }
            else if ( ret < E_TYPE_SUCCESS )
            {
                if( startPort != 0 )  /* single port test */
                {
                    goto __CMD_ERROR;
                }
            }
            else
            {
                if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                {
                    log_dbgPrintf("%s %d %dM loopback test (%llu)\r\n", lPortTypeStr, txLPort, speed, txCounter.txCnt.txMulticastNum);
                    log_dbgPrintf("%s %d %dM loopback test (%llu)\r\n", lPortTypeStr, rxLPort, speed, rxCounter.rxCnt.rxMulticastNum);
                    log_cmdPrintf(E_LOG_MSG_PASS, "Loopback test %s (%d, %d) %dM\r\n", lPortTypeStr, txLPort, rxLPort, speed);
                }
                else
                {
                    log_dbgPrintf("%s %d %dM loopback test (%llu)\r\n", lPortTypeStr, port, speed, rxCounter.rxCnt.rxMulticastNum);
                    log_cmdPrintf(E_LOG_MSG_PASS, "Loopback test %s %d %dM\r\n", lPortTypeStr, port, speed);
                }
            }

            if(lbTestType == E_LB_TEST_TYPE_EXT_P)
            {
                port++;
            }

            if( (startPort==0) &&/* all */
                ((port+1)<=endPort))
            {
                udelay(600000);
            }

        }
        if( retValue != E_TYPE_SUCCESS )
        {
            ret = retValue;
        }
    }

__CMD_ERROR:
    /* Restore block mode */
    switch_halSfpLedTaskFlagSet(sfpTaskFlag);
    uartAttrRestore();
    return ret;
}

INT32 do_linespeedtest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    BOOL isSfp = FALSE, skipLb=FALSE;
    INT8 lPortTypeStr[20]="N/A";
    INT32 ret=E_TYPE_SUCCESS, testResult=E_TEST_PASS;
    UINT8  sfpTaskFlag=0;
    UINT16 LinespeedVlanId, regValue=0;
    UINT32 portList[MAX_LOGIC_PORT_52]={0};
    UINT32 testTime=0;
    UINT32 cntTestTime, cnt1S, prevCntTestTime;
    UINT32 startPortId;
    UINT32 paraCnt;
    UINT32 speed=0, highspeed=0;
    UINT32 startPort=0, endPort=0, pattern=0xff, size=1518, numPkt=10, port;
    UINT32 i, txLPort=0, rxLPort=0, lPortBase;
    UINT32 log_dbg_st = FALSE;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LINER_PORT_TYPE startLPortType;
    E_LB_TEST_TYPE    lbTestType=E_LB_TEST_TYPE_MAX;
    S_PORT_INFO *portInfo;
    S_PORT_CNT txCounter, rxCounter, zeroCounter;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    /* Set socket non-block mode for ctrlc (getc) without waiting for enter */
    uartAttrSet(0);

    if( strstr(argv[0], "linespeed") )
    {
        if( argc != 9 )
        {
            ERR_PRINT_CMD_USAGE("linespeed");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if( strcmp("mac", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_MAC;
        }
        else if( strcmp("phy", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_PHY;
        }
        else if( strcmp("exts", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_EXT_S;
        }
        else if( strcmp("extp", argv[1]) == 0 )
        {
            lbTestType = E_LB_TEST_TYPE_EXT_P;
        }
        else
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt = 2;
        if( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE("linespeed");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        /* Get pattern */
        if( strcmp("cjpat", argv[paraCnt]) == 0 )
        {
            pattern = E_SW_LB_PATTERN_CJPAT;
        }
        else if( strcmp("random", argv[paraCnt]) == 0 )
        {
            pattern = E_SW_LB_PATTERN_RANDOM;
        }
        else
        {
            pattern = simple_strtoul(argv[paraCnt], NULL, 16);
        }

        if(pattern > 0xffff)
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        size = simple_strtoul(argv[paraCnt], NULL, 10);
        if( (size<FOXCONN_PKT_MIN_SIZE) || (size>FOXCONN_PKT_MAX_SIZE) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        numPkt = simple_strtoul(argv[paraCnt], NULL, 10);
        if( numPkt <= 0 || numPkt > PKT_LINESPEED_MAX_NUM )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        switch (lPortType)
        {
            case E_LINER_PORT_TYPE_FIXED:
                paraCnt++;

                /* For "all" speed is necessary. */
                if( !startPort || (startPort <= port_utilsTotalFixedPortGet()))
                {
                    if(!argv[paraCnt])
                    {
                        ERR_PRINT_CMD_USAGE("linespeed");
                        ret = E_TYPE_INVALID_CMD_FORMAT;
                        goto __CMD_ERROR;
                    }

                    speed = simple_strtoul(argv[paraCnt], NULL, 10);

                    switch (speed)
                    {
                        case 0:
                            /* 03212017, check port speed 0 is going to highest speed */
                            if( startPort == 0 ) /* all */
                            {
                                endPort = boardInfo.lPortMaxNum;
                            }

                            if( endPort > boardInfo.lPortMaxNum)
                            {
                                ret = E_TYPE_INVALID_PARA;
                                goto __CMD_ERROR;
                            }
                            break;
                           case 10:
                        case 100:
                        case 1000:
                            if (lbTestType == E_LB_TEST_TYPE_PHY)
                            {
                                if( startPort == 0 ) /* all */
                                {
                                    if(lbTestType == E_LB_TEST_TYPE_PHY)
                                    {
                                        endPort = boardInfo.copperMaxNum;
                                    }
                                }
                                if( (startPort > boardInfo.copperMaxNum) || (endPort > boardInfo.copperMaxNum) )
                                {
                                    ret = E_TYPE_INVALID_PARA;
                                    goto __CMD_ERROR;
                                }
                            }
                            break;
                        case 10000:
                            if( startPort == 0 ) /* all */
                            {
                                startPort = boardInfo.firstfiberNum;
                            }

                            if( startPort < boardInfo.firstfiberNum)
                            {
                                ret = E_TYPE_INVALID_PARA;
                                goto __CMD_ERROR;
                            }
                            break;
                        default:
                            ret = E_TYPE_INVALID_PARA;
                            goto __CMD_ERROR;
                    }
                }
                break;
            case E_LINER_PORT_TYPE_STACKING:
            case E_LINER_PORT_TYPE_INTER_LINK:
            case E_LINER_PORT_TYPE_EXPANSION:
            default:
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
        }

        /* Indicate the port is sfp or not */
        paraCnt++;
        isSfp = simple_strtoul(argv[paraCnt], NULL, 10);

        /* Get testTime */
        paraCnt++;
        testTime = simple_strtoul(argv[paraCnt], NULL, 10);

        switch (lbTestType)
        {
            case E_LB_TEST_TYPE_PHY:
            case E_LB_TEST_TYPE_MAC:
                log_printf("Make sure no cable on all ports\r\n");
                break;
            case E_LB_TEST_TYPE_EXT_S:
                if( lPortType != E_LINER_PORT_TYPE_INTER_LINK )
                {
                    log_printf("Make sure single-port loopback stub are installed on all ports\r\n");
                }
                break;
            case E_LB_TEST_TYPE_EXT_P:
                if( lPortType != E_LINER_PORT_TYPE_INTER_LINK )
                {
                    log_printf("Make sure cables are installed on all port pairs (P1, P2), (P3, P4) ...\r\n");
                }
                break;
            default:
                goto __CMD_ERROR;
        }

        memset(&txCounter, 0 , sizeof(txCounter));
        memset(&rxCounter, 0 , sizeof(rxCounter));
        memset(&zeroCounter, 0 , sizeof(zeroCounter));

        if( startPort == 0 ) /* all */
        {
            port = 1;
            if(lbTestType == E_LB_TEST_TYPE_PHY)
            {
                endPort = boardInfo.copperMaxNum;
            }
            else if( (speed == 10) || (speed == 100) )
            {
                endPort = boardInfo.copperMaxNum;
            }
            else if( speed == 1000 || speed == 10000 || speed == 0 ) /* check port highest speed = 0 */
            {
                endPort = boardInfo.lPortMaxNum;
            }
        }
        else
        {
            port = startPort;
        }

        /* Before CPSS provide valid semaphore or mutex, suspend SFP LED task to avoid critical section.
         */
        switch_halSfpLedTaskFlagGet(&sfpTaskFlag);
        if(sfpTaskFlag == TRUE)
        {
            switch_halSfpLedTaskFlagSet(FALSE);
        }

        /* Disable trap, prepare for packet flooding */
        switch_linespeedTestStart();

        for (; port<=endPort; port++)
        {
            txLPort = ( startPort == 0 ) ? lPortBase + port : port;

            switch (lPortType)
            {
                case E_LINER_PORT_TYPE_FIXED:
                    if( (port<=port_utilsTotalFixedPortGet()) && (lbTestType==E_LB_TEST_TYPE_EXT_P) )
                    {
                        rxLPort = ( port % 2 ) ? txLPort+1 : txLPort-1;
                    }
                    else
                    {
                        rxLPort = txLPort;
                    }
                    break;
                case E_LINER_PORT_TYPE_STACKING:
                default:
                    break;
            }

            skipLb = FALSE;

            portInfo = port_utilsLPortInfoGet(txLPort);

            switch (lPortType)
            {
                case E_LINER_PORT_TYPE_FIXED:
                    switch (speed)
                    {
                        case 0: /* add port speed is highspeed */
                        case 1000:
                        case 10000:
                            break;
                        case 100:
                            if( (portInfo->capability & (E_PORT_SPEED_CAP_100M)) != E_PORT_SPEED_CAP_100M)
                            {
                                skipLb = TRUE;
                            }
                            break;
                        case 10:
                            if( (portInfo->capability & (E_PORT_SPEED_CAP_10M)) != E_PORT_SPEED_CAP_10M)
                            {
                                skipLb = TRUE;
                            }
                            break;
                    }
                    break;
                default:
                    break;
            }

            if (lbTestType==E_LB_TEST_TYPE_PHY)
            {
                if( (portInfo->lbCap & (E_PORT_LB_CAP_PHY)) != E_PORT_LB_CAP_PHY)
                {
                    skipLb = TRUE;
                }
            }

            /* all port, extp, port-pair port */
            if( (lbTestType==E_LB_TEST_TYPE_EXT_P) && (startPort==0) && (txLPort>rxLPort) )
                skipLb = TRUE;

            if( skipLb == TRUE )
                continue;

            /* all port, extp mode */
            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                switch (speed)
                {
                    case 0:
                        /* set port to highest speed 10G */
                        if ((txLPort > boardInfo.copperMaxNum) && (portInfo->portSpeed == PORT_SPEED_CAP_10G))
                        {
                            highspeed = 10000;
                        }
                        else
                        {
                            highspeed = 1000;
                        }
                        break;
                    default:
                        highspeed = 0;
                        break;
                }
            }

            /* set port highest speed to testing */
            if(highspeed != 0)
                speed = highspeed;

            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                log_printf("Start %s %d linespeed test in Speed %dM\n", lPortTypeStr, txLPort, speed);
                log_printf("Start %s %d linespeed test in Speed %dM\n", lPortTypeStr, rxLPort, speed);
            }
            else
            {
                log_printf("Start %s %d linespeed test in Speed %dM\n", lPortTypeStr, port, speed);
            }

            ret=switch_linespeedTestInit(txLPort, rxLPort, lbTestType, pattern, size, numPkt, speed, isSfp);
            if( ret == E_TYPE_SUCCESS )
            {
                portList[port] = 1; /* Init OK, add to test list */
            }
            else
            {
                portList[port] = 0; /* Init NG, do not add to test list */

                if (ret == E_TYPE_PORT_LINK_DOWN)
                {
                    if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                    {
                        log_cmdPrintf(E_LOG_MSG_FAIL, "Linespeed test %s (%d, %d) %dM\r\n", lPortTypeStr, txLPort, rxLPort, speed);
                    }
                    else
                    {
                        log_cmdPrintf(E_LOG_MSG_FAIL, "Linespeed test %s %d %dM\r\n", lPortTypeStr, txLPort, speed);
                    }
                }
            }

            if(lbTestType == E_LB_TEST_TYPE_EXT_P)
            {
                port++;
            }

            /* rollback speed to user input cmd */
            if(highspeed != 0)
                speed = 0;

        } /* for (; port<=endPort; port++) */

        cntTestTime = 1;
        prevCntTestTime = 0;
        cnt1S = 0;

        if( testTime == 0 )
        {
            log_printf("Ctrl+x to break ... \r\n");
        }
        else
        {
            /* 07272016, The line speed test ... may cause the c2800 terminal console buffer queue stop
             */
            log_dbg_st =log_dbgPrintFlagGet();

            if(log_dbg_st == TRUE)
                log_printf("%d %% ...\r", (cntTestTime*100)/testTime);
            else
                log_printf("Linespeed test is running ...");
        }

        do
        {
            cpssOsTimerWkAfter(90); /* delay 50ms, for to check link status */

            if( testTime == 0 )
            {
                if( ctrlc() )
                    goto __BREAK_TEST;
            }
            else
            {
                cnt1S++;
                if( cnt1S >= 10 )
                {
                    cntTestTime++;
                    cnt1S = 0;
                }
                if( cntTestTime > testTime )
                {
                    log_printf("100 %%");
                    log_printf("\r\n");
                    break;
                }
                else
                {
                    if( prevCntTestTime != cntTestTime )
                    {
                        if(log_dbg_st == TRUE)
                            log_printf("%d %% ...\r", (cntTestTime*100)/testTime);

                        prevCntTestTime = cntTestTime;
                    }
                }
            }
/* Add for Debug CRC issue */
#if 1
            for (; port<=endPort; port++)
            {
                txLPort = ( startPort == 0 ) ? lPortBase + port : port;

                switch (lPortType)
                {
                    case E_LINER_PORT_TYPE_FIXED:
                        if( (port<=port_utilsTotalFixedPortGet()) && (lbTestType==E_LB_TEST_TYPE_EXT_P) )
                        {
                            rxLPort = ( port % 2 ) ? txLPort+1 : txLPort-1;
                        }
                        else
                        {
                            rxLPort = txLPort;
                        }
                        break;
                    case E_LINER_PORT_TYPE_STACKING:
                    default:
                        break;
                }

                portInfo = port_utilsLPortInfoGet(txLPort);

                skipLb = FALSE;

                switch (lPortType)
                {
                    case E_LINER_PORT_TYPE_FIXED:
                        switch (speed)
                        {
                            case 1000:
                                break;
                            case 100:
                                if( (portInfo->capability & (E_PORT_SPEED_CAP_100M)) != E_PORT_SPEED_CAP_100M)
                                {
                                    skipLb = TRUE;
                                }
                                break;
                            case 10:
                                if( (portInfo->capability & (E_PORT_SPEED_CAP_10M)) != E_PORT_SPEED_CAP_10M)
                                {
                                    skipLb = TRUE;
                                }
                                break;
                        }
                        break;
                    default:
                        break;
                }

                if (lbTestType==E_LB_TEST_TYPE_PHY)
                {
                    if( (portInfo->lbCap & (E_PORT_LB_CAP_PHY)) != E_PORT_LB_CAP_PHY)
                    {
                        skipLb = TRUE;
                    }
                }

                /* all port, extp, port-pair port */
                if( (lbTestType==E_LB_TEST_TYPE_EXT_P) && (startPort==0) && (txLPort>rxLPort) )
                {
                    skipLb = TRUE;
                }

                if( skipLb == TRUE )
                {
                    continue;
                }

                /* Get txLPort and rxLPort statistic */
                if( ret = switch_halPortCntGet(txLPort, &txCounter) != E_TYPE_SUCCESS )
                {
                    log_printf("Failed to get statistics of port %d\r\n", txLPort);
                }

                if( ret = switch_halPortCntGet(rxLPort, &rxCounter) != E_TYPE_SUCCESS )
                {
                    log_printf("Failed to get statistics of port %d\r\n", rxLPort);
                }

                if ( (memcmp(&zeroCounter.errCnt, &txCounter.errCnt, sizeof(txCounter.errCnt)) != 0) ||
                     (memcmp(&zeroCounter.errCnt, &rxCounter.errCnt, sizeof(rxCounter.errCnt)) != 0))
                {
                    testResult = E_TEST_FAIL;
                    log_printf("running\n");
                    log_dbgPrintf("running: TX Port %d\r\n", txLPort);

                    log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", txCounter.errCnt.txDeferred,      txCounter.errCnt.txExcessiveCollision);
                    log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", txCounter.errCnt.txSentMultiple,  txCounter.errCnt.txCollisionNum);
                    log_dbgPrintf("TX CRC error           : %llu\r\n", txCounter.errCnt.txCrcNum);
                    log_dbgPrintf("RX CRC error           : %llu\r\n", txCounter.errCnt.rxCrcNum);
                    log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", txCounter.errCnt.rxJabbberNum, txCounter.errCnt.rxErrorNum);
                    log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", txCounter.errCnt.rxOverRunNum, txCounter.errCnt.rxUnderSizeNum);
                    log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", txCounter.errCnt.rxFragmentsNum, txCounter.errCnt.rxOverSizeNum);
                    log_dbgPrintf("==============================================================\r\n");

                    if (lbTestType==E_LB_TEST_TYPE_EXT_P)
                    {
                        log_dbgPrintf("running: RX Port %d\r\n", rxLPort);
                        
                        log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", rxCounter.errCnt.txDeferred, rxCounter.errCnt.txExcessiveCollision);
                        log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", rxCounter.errCnt.txSentMultiple, rxCounter.errCnt.txCollisionNum);
                        log_dbgPrintf("TX CRC error           : %llu\r\n", rxCounter.errCnt.txCrcNum);
                        log_dbgPrintf("RX CRC error           : %llu\r\n", rxCounter.errCnt.rxCrcNum);
                        log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", rxCounter.errCnt.rxJabbberNum, rxCounter.errCnt.rxErrorNum);
                        log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", rxCounter.errCnt.rxOverRunNum, rxCounter.errCnt.rxUnderSizeNum);
                        log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", rxCounter.errCnt.rxFragmentsNum, rxCounter.errCnt.rxOverSizeNum);
                        log_dbgPrintf("==============================================================\r\n");
                    }

                    /* Add debug messages*/
                    /* Change to page 2 */
                    switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 2);

                    /* Read FIFO register */
                    switch_halSMIRegGet(txLPort, 0x13, &regValue);
                    log_printf("MAC specific Status register value %lx\r\n", regValue);

                    /* Change to page 4 */
                    switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 4);

                    /* Read FIFO register */
                    switch_halSMIRegGet(txLPort, 0x13, &regValue);
                    log_printf("QSGMII interrupt Status register value %lx\r\n", regValue);

                    /* Change to page 0 */
                    switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 0);

                    /* Change to page 2 */
                    switch_halSMIRegSet(rxLPort, PHY_PAGE_REG_OFFSET, 2);

                    /* Read FIFO register */
                    switch_halSMIRegGet(rxLPort, 0x13, &regValue);
                    log_printf("MAC specific Status register value %lx\r\n", regValue);

                    /* Change to page 4 */
                    switch_halSMIRegSet(rxLPort, PHY_PAGE_REG_OFFSET, 4);

                    /* Read FIFO register */
                    switch_halSMIRegGet(rxLPort, 0x13, &regValue);
                    log_printf("QSGMII interrupt Status register value %lx\r\n", regValue);

                    /* Change to page 0 */
                    switch_halSMIRegSet(rxLPort, PHY_PAGE_REG_OFFSET, 0);
                    goto __BREAK_TEST;
                }
            }
#endif
        } while(1);

__BREAK_TEST:
        if( startPort == 0 ) /* all */
        {
            port = 1;
            txLPort = startPort;
            rxLPort = startPort;

            if(lbTestType == E_LB_TEST_TYPE_MAC)
            {
                numPkt *= port_utilsTotalFixedPortGet();
            }
            else if(lbTestType == E_LB_TEST_TYPE_PHY)
            {
                numPkt *= boardInfo.copperMaxNum;
            }
            else if(lbTestType == E_LB_TEST_TYPE_EXT_P)
            {
                numPkt *= 2;
            }
        }
        else
        {
            port = startPort;

            if(lbTestType == E_LB_TEST_TYPE_EXT_P)
            {
                numPkt *= 2;
            }
        }

        /* Trap all packet to CPU */
        switch_linespeedTestStop(txLPort, rxLPort, lbTestType, numPkt);

        for (; port<=endPort; port++)
        {
            txLPort = ( startPort == 0 ) ? lPortBase + port : port;

            switch (lPortType)
            {
                case E_LINER_PORT_TYPE_FIXED:
                    if( (port<=port_utilsTotalFixedPortGet()) && (lbTestType==E_LB_TEST_TYPE_EXT_P) )
                    {
                        rxLPort = ( port % 2 ) ? txLPort+1 : txLPort-1;
                    }
                    else
                    {
                        rxLPort = txLPort;
                    }
                    break;
                case E_LINER_PORT_TYPE_STACKING:
                default:
                    break;
            }

            portInfo = port_utilsLPortInfoGet(txLPort);

            skipLb = FALSE;

            switch (lPortType)
            {
                case E_LINER_PORT_TYPE_FIXED:
                    switch (speed)
                    {
                        case 1000:
                            break;
                        case 100:
                            if( (portInfo->capability & (E_PORT_SPEED_CAP_100M)) != E_PORT_SPEED_CAP_100M)
                            {
                                skipLb = TRUE;
                            }
                            break;
                        case 10:
                            if( (portInfo->capability & (E_PORT_SPEED_CAP_10M)) != E_PORT_SPEED_CAP_10M)
                            {
                                skipLb = TRUE;
                            }
                            break;
                    }
                    break;
                default:
                    break;
            }

            if (lbTestType==E_LB_TEST_TYPE_PHY)
            {
                if( (portInfo->lbCap & (E_PORT_LB_CAP_PHY)) != E_PORT_LB_CAP_PHY)
                {
                    skipLb = TRUE;
                }
            }

            /* all port, extp, port-pair port */
            if( (lbTestType==E_LB_TEST_TYPE_EXT_P) && (startPort==0) && (txLPort>rxLPort) )
            {
                skipLb = TRUE;
            }

            if( skipLb == TRUE )
            {
                continue;
            }

            /* Get txLPort and rxLPort statistic */
            if( ret = switch_halPortCntGet(txLPort, &txCounter) != E_TYPE_SUCCESS )
            {
                log_printf("Failed to get statistics of port %d\r\n", txLPort);
            }

            if( ret = switch_halPortCntGet(rxLPort, &rxCounter) != E_TYPE_SUCCESS )
            {
                log_printf("Failed to get statistics of port %d\r\n", rxLPort);
            }

            /* Clear port counter after read */
            if(switch_halPortCounterClearFlagSet(txLPort, FALSE) != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, txLPort);
                goto __CMD_ERROR;
            }

            /* Clear counter after test */
            if(switch_halClearPortCounter(txLPort) != E_TYPE_SUCCESS)
            {
                log_printf("Failed to clear %s %d counters after test\r\n", lPortTypeStr, txLPort);
            }

            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                if(switch_halPortCounterClearFlagSet(rxLPort, FALSE) != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, rxLPort);
                    goto __CMD_ERROR;
                }

                if(switch_halClearPortCounter(rxLPort) != E_TYPE_SUCCESS)
                {
                    log_printf("Failed to clear %s %d counters after test\r\n", lPortTypeStr, rxLPort);
                }
            }

            switch_linespeedTestReinit(txLPort, rxLPort, lbTestType);

            testResult = E_TEST_PASS;

            if ( (memcmp(&zeroCounter.errCnt, &txCounter.errCnt, sizeof(txCounter.errCnt)) != 0) ||
                 (memcmp(&zeroCounter.errCnt, &rxCounter.errCnt, sizeof(rxCounter.errCnt)) != 0))
            {
                /* Fix false alarm 'tx==rx , rx error appear 1 count' issue */
                if ( (txCounter.txCnt.txMulticastNum != rxCounter.rxCnt.rxMulticastNum ) || (rxCounter.txCnt.txMulticastNum != txCounter.rxCnt.rxMulticastNum) )
                {
                    if( (testResult == E_TEST_PASS) && ( (txCounter.errCnt.rxCrcNum == 1) || (txCounter.errCnt.rxErrorNum == 1) || (rxCounter.errCnt.rxCrcNum == 1) || (rxCounter.errCnt.rxErrorNum == 1) )
                        && ( (( txCounter.txCnt.txMulticastNum - rxCounter.rxCnt.rxMulticastNum )==1) || (( rxCounter.txCnt.txMulticastNum - txCounter.rxCnt.rxMulticastNum )==1) ))
                    {
                        /* Skip false alarm 'tx!=rx, rx error and crc appear 1 count' issue due to it related to CPU trap packet but not HW error */
                    }
                    else
                    {
                        testResult = E_TEST_FAIL;
                        log_dbgPrintf("TX Port %d\r\n", txLPort);

                        log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", txCounter.errCnt.txDeferred,      txCounter.errCnt.txExcessiveCollision);
                        log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", txCounter.errCnt.txSentMultiple,  txCounter.errCnt.txCollisionNum);
                        log_dbgPrintf("TX CRC error           : %llu\r\n", txCounter.errCnt.txCrcNum);
                        log_dbgPrintf("RX CRC error           : %llu\r\n", txCounter.errCnt.rxCrcNum);
                        log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", txCounter.errCnt.rxJabbberNum, txCounter.errCnt.rxErrorNum);
                        log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", txCounter.errCnt.rxOverRunNum, txCounter.errCnt.rxUnderSizeNum);
                        log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", txCounter.errCnt.rxFragmentsNum, txCounter.errCnt.rxOverSizeNum);
                        log_dbgPrintf("==============================================================\r\n");

                        if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                        {
                            log_dbgPrintf("RX Port %d\r\n", rxLPort);

                            log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", rxCounter.errCnt.txDeferred, rxCounter.errCnt.txExcessiveCollision);
                            log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", rxCounter.errCnt.txSentMultiple, rxCounter.errCnt.txCollisionNum);
                            log_dbgPrintf("TX CRC error           : %llu\r\n", rxCounter.errCnt.txCrcNum);
                            log_dbgPrintf("RX CRC error           : %llu\r\n", rxCounter.errCnt.rxCrcNum);
                            log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", rxCounter.errCnt.rxJabbberNum, rxCounter.errCnt.rxErrorNum);
                            log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", rxCounter.errCnt.rxOverRunNum, rxCounter.errCnt.rxUnderSizeNum);
                            log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", rxCounter.errCnt.rxFragmentsNum, rxCounter.errCnt.rxOverSizeNum);
                            log_dbgPrintf("==============================================================\r\n");
                        }

                        /* Add debug messages*/
                        /* Change to page 2 */
                        switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 2);

                        /* Read FIFO register */
                        switch_halSMIRegGet(txLPort, 0x13, &regValue);
                        log_printf("MAC specific Status register value %lx\r\n", regValue);

                        /* Change to page 4 */
                        switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 4);

                        /* Read FIFO register */
                        switch_halSMIRegGet(txLPort, 0x13, &regValue);
                        log_printf("QSGMII interrupt Status register value %lx\r\n", regValue);

                        /* Change to page 0 */
                        switch_halSMIRegSet(txLPort, PHY_PAGE_REG_OFFSET, 0);

                        /* Change to page 2 */
                        switch_halSMIRegSet(rxLPort, PHY_PAGE_REG_OFFSET, 2);

                        /* Read FIFO register */
                        switch_halSMIRegGet(rxLPort, 0x13, &regValue);
                        log_printf("MAC specific Status register value %lx\r\n", regValue);

                        /* Change to page 4 */
                        switch_halSMIRegSet(rxLPort, PHY_PAGE_REG_OFFSET, 4);

                        /* Read FIFO register */
                        switch_halSMIRegGet(rxLPort, 0x13, &regValue);
                        log_printf("QSGMII interrupt Status register value %lx\r\n", regValue);

                        /* Change to page 0 */
                        switch_halSMIRegSet(rxLPort, PHY_PAGE_REG_OFFSET, 0);
                    }
                }
            }

            log_dbgPrintf("%s %d TX %llu, RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);

            if( lbTestType==E_LB_TEST_TYPE_EXT_P )
            {
                log_dbgPrintf("%s %d TX %llu, RX %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);
                /* Packet loss number less than LINESPEED_DAC_7M_10M_PKT_LOSS_NUM_MAX(3600) would be ignored  2017-05-27*/
                if ( (txCounter.txCnt.txMulticastNum != rxCounter.rxCnt.rxMulticastNum ) || (rxCounter.txCnt.txMulticastNum != txCounter.rxCnt.rxMulticastNum) )
                {
                    log_dbgPrintf("%s %d TX %llu != %s %d RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, lPortTypeStr, rxLPort, rxCounter.rxCnt.rxMulticastNum);
                    log_dbgPrintf("%s %d TX %llu != %s %d RX %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, lPortTypeStr, txLPort, txCounter.rxCnt.rxMulticastNum);
                    if ( abs(txCounter.txCnt.txMulticastNum-rxCounter.rxCnt.rxMulticastNum) <= LINESPEED_DAC_7M_10M_PKT_LOSS_NUM_MAX  \
                        && abs(rxCounter.txCnt.txMulticastNum - txCounter.rxCnt.rxMulticastNum) <= LINESPEED_DAC_7M_10M_PKT_LOSS_NUM_MAX)
                    {
                        log_printf("Packet loss: P%02d -> P%02d: %u, P%02d -> P%02d:%u\n", txLPort, rxLPort, abs(txCounter.txCnt.txMulticastNum-rxCounter.rxCnt.rxMulticastNum), \
                                              rxLPort, txLPort, abs(rxCounter.txCnt.txMulticastNum - txCounter.rxCnt.rxMulticastNum));
                    }
                    else
                    {
                        testResult = E_TEST_FAIL;
                    }
                }
            }
            else
            {
                /* Loopback mode as MAC / PHY /EXTS */
                if ( (txCounter.txCnt.txMulticastNum != txCounter.rxCnt.rxMulticastNum ) || (rxCounter.txCnt.txMulticastNum != rxCounter.rxCnt.rxMulticastNum) )
                {
                    log_dbgPrintf("%s %d TX %llu != RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                    testResult = E_TEST_FAIL;
                }
            }

            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                if ( txCounter.txCnt.txMulticastNum == 0 || txCounter.rxCnt.rxMulticastNum == 0 || rxCounter.txCnt.txMulticastNum == 0 || rxCounter.rxCnt.rxMulticastNum ==0)
                {
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);
                    testResult = E_TEST_FAIL;
                }
            }
            else
            {
                if ( txCounter.txCnt.txMulticastNum == 0 || txCounter.rxCnt.rxMulticastNum == 0)
                {
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                    testResult = E_TEST_FAIL;
                }
            }

            /* Enhancement, check if packet counter extremely less. */
            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                if ( txCounter.txCnt.txMulticastNum < LINESPEED_PACKET_LIMIT || txCounter.rxCnt.rxMulticastNum < LINESPEED_PACKET_LIMIT || rxCounter.txCnt.txMulticastNum < LINESPEED_PACKET_LIMIT || rxCounter.rxCnt.rxMulticastNum < LINESPEED_PACKET_LIMIT)
                {
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);
                    testResult = E_TEST_FAIL;
                }
            }
            else
            {
                if ( txCounter.txCnt.txMulticastNum < LINESPEED_PACKET_LIMIT || txCounter.rxCnt.rxMulticastNum < LINESPEED_PACKET_LIMIT)
                {
                    log_dbgPrintf("%s %d TX = %llu RX = %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                    testResult = E_TEST_FAIL;
                }
            }

            if( ret == E_TYPE_CTRL_C )
            {
                goto __CMD_ERROR;
            }

            /* check all port, extp mode */
            if(lbTestType==E_LB_TEST_TYPE_EXT_P)
            {
                switch (speed)
                {
                    case 0:
                        /* set sfp port to highest speed*/
                        if ((txLPort > boardInfo.copperMaxNum) && (portInfo->portSpeed == PORT_SPEED_CAP_10G))
                        {
                            highspeed = 10000;
                        }
                        else
                        {
                            highspeed = 1000;
                        }
                        break;
                    default:
                        highspeed = 0;
                        break;
                }
            }

            /* set highest speed to test result */
            if(highspeed != 0)
                speed = highspeed;

            if( testResult != E_TYPE_SUCCESS )
            {
                if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                {
                    log_dbgPrintf("%s %d %dM linespeed test (%llu)\r\n", lPortTypeStr, txLPort, speed, txCounter.txCnt.txMulticastNum);
                    log_dbgPrintf("%s %d %dM linespeed test (%llu)\r\n", lPortTypeStr, rxLPort, speed, rxCounter.txCnt.txMulticastNum);
                    log_cmdPrintf(E_LOG_MSG_FAIL, "Linespeed test %s (%d, %d) %dM\r\n", lPortTypeStr, txLPort, rxLPort, speed);
                }
                else
                {
                    log_cmdPrintf(E_LOG_MSG_FAIL, "Linespeed test %s %d %dM (%llu)\r\n", lPortTypeStr, port, speed, txCounter.txCnt.txMulticastNum);
                }
            }
            else
            {
                if(lbTestType==E_LB_TEST_TYPE_EXT_P)
                {
                    log_dbgPrintf("%s %d %dM linespeed test (%llu)\r\n", lPortTypeStr, txLPort, speed, txCounter.txCnt.txMulticastNum);
                    log_dbgPrintf("%s %d %dM linespeed test (%llu)\r\n", lPortTypeStr, rxLPort, speed, rxCounter.txCnt.txMulticastNum);
                    log_cmdPrintf(E_LOG_MSG_PASS, "Linespeed test %s (%d, %d) %dM\r\n", lPortTypeStr, txLPort, rxLPort, speed);
                }
                else
                {
                    log_dbgPrintf("%s %d %dM linespeed test (%llu)\r\n", lPortTypeStr, port, speed, txCounter.txCnt.txMulticastNum);
                    log_cmdPrintf(E_LOG_MSG_PASS, "Linespeed test %s %d %dM\r\n", lPortTypeStr, port, speed);
                }
            }

            /* rollback speed to user input cmd */
            if(highspeed != 0)
                speed = 0;

            if(lbTestType == E_LB_TEST_TYPE_EXT_P)
            {
                port++;
            }

            if( (startPort==0) &&/* all */
                ((port+1)<=endPort))
            {
                udelay(600000);
            }
        } /* End of for (; port<=endPort; port++) */
    }

__CMD_ERROR:

    /* Restore block mode */
    uartAttrRestore();
    switch_halSfpLedTaskFlagSet(sfpTaskFlag);

    /* 20160822 - add 500ms delay after linespeed finish and uart
    retstored */
    udelay(500000);
    return ret;
}

INT32 do_pktgentest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    BOOL isSfp = FALSE, skipLb=FALSE;
    INT8 lPortTypeStr[20]="N/A";
    INT32 ret=E_TYPE_SUCCESS, testResult=E_TEST_PASS;
    UINT8  sfpTaskFlag=0;
    UINT16 LinespeedVlanId, regValue=0;
    UINT32 portList[MAX_LOGIC_PORT_52]={0};
    UINT32 testTime=0;
    UINT32 cntTestTime, cnt1S, prevCntTestTime;
    UINT32 startPortId;
    UINT32 paraCnt;
    UINT32 speed=0;
    UINT32 startPort=0, endPort=0, pattern=0xff, size=1518, numPkt=10, port;
    UINT32 i, txLPort=0, rxLPort=0, lPortBase;
    UINT32 log_dbg_st = FALSE;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LINER_PORT_TYPE startLPortType;
    E_LB_TEST_TYPE    lbTestType=E_LB_TEST_TYPE_MAX;
    S_PORT_INFO *portInfo;
    S_PORT_CNT txCounter, rxCounter, zeroCounter;
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    /* Set socket non-block mode for ctrlc (getc) without waiting for enter */
    uartAttrSet(0);

    if( strstr(argv[0], "pktgen") )
    {
        if( argc != 6 )
        {
            ERR_PRINT_CMD_USAGE("pktgen");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;
        if( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE("linespeed");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        /* Get pattern */
        if( strcmp("cjpat", argv[paraCnt]) == 0 )
        {
            pattern = E_SW_LB_PATTERN_CJPAT;
        }
        else if( strcmp("random", argv[paraCnt]) == 0 )
        {
            pattern = E_SW_LB_PATTERN_RANDOM;
        }
        else
        {
            pattern = simple_strtoul(argv[paraCnt], NULL, 16);
        }

        if(pattern > 0xffff)
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        size = simple_strtoul(argv[paraCnt], NULL, 10);
        if( (size<FOXCONN_PKT_MIN_SIZE) || (size>FOXCONN_PKT_MAX_SIZE) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        numPkt = simple_strtoul(argv[paraCnt], NULL, 10);
        if( numPkt <= 0 || numPkt > PKT_LINESPEED_MAX_NUM )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        switch (lPortType)
        {
            case E_LINER_PORT_TYPE_FIXED:
                paraCnt++;

                /* For "all" speed is necessary. */
                if( !startPort || (startPort <= port_utilsTotalFixedPortGet()))
                {
                    if(!argv[paraCnt])
                    {
                        ERR_PRINT_CMD_USAGE("linespeed");
                        ret = E_TYPE_INVALID_CMD_FORMAT;
                        goto __CMD_ERROR;
                    }

                    speed = simple_strtoul(argv[paraCnt], NULL, 10);

                    switch (speed)
                    {
                        case 10:
                        case 100:
                        case 1000:
                            if (lbTestType == E_LB_TEST_TYPE_PHY)
                            {
                                if( startPort == 0 ) /* all */
                                {
                                    if(lbTestType == E_LB_TEST_TYPE_PHY)
                                    {
                                        endPort = boardInfo.copperMaxNum;
                                    }
                                }
                                if( (startPort > boardInfo.copperMaxNum) || (endPort > boardInfo.copperMaxNum) )
                                {
                                    ret = E_TYPE_INVALID_PARA;
                                    goto __CMD_ERROR;
                                }
                            }
                            break;
                        case 10000:
                            if( startPort == 0 ) /* all */
                            {
                                startPort = boardInfo.firstfiberNum;
                            }

                            if( startPort < boardInfo.firstfiberNum)
                            {
                                ret = E_TYPE_INVALID_PARA;
                                goto __CMD_ERROR;
                            }
                            break;
                        default:
                            ret = E_TYPE_INVALID_PARA;
                            goto __CMD_ERROR;
                    }
                }
                break;
            case E_LINER_PORT_TYPE_STACKING:
            case E_LINER_PORT_TYPE_INTER_LINK:
            case E_LINER_PORT_TYPE_EXPANSION:
            default:
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
        }

        /* Get testTime */
        //paraCnt++;
        //testTime = simple_strtoul(argv[paraCnt], NULL, 10);

        if( startPort == 0 ) /* all */
        {
            port = 1;
            if(lbTestType == E_LB_TEST_TYPE_PHY)
            {
                endPort = boardInfo.copperMaxNum;
            }
            else if( (speed == 10) || (speed == 100) )
            {
                endPort = boardInfo.copperMaxNum;
            }
            else if( speed == 1000 || speed == 10000)
            {
                endPort = boardInfo.lPortMaxNum;
            }
        }
        else
        {
            port = startPort;
        }


        for (; port<=endPort; port++)
        {
            ret = switch_pktgenTest(port, pattern, size, numPkt, speed);
            if (ret != E_TYPE_SUCCESS)
            {
                log_printf("Failed to send packet to port %d\n", port);
            }
        }

}
__CMD_ERROR:

    /* Restore block mode */
    uartAttrRestore();

    /* 20160822 - add 500ms delay after linespeed finish and uart
    retstored */
    udelay(500000);
    return ret;
}

INT32 do_snaketest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    UINT32 startPort=0, endPort=0;
    INT32 ret=E_TYPE_SUCCESS, retValue=E_TYPE_SUCCESS;
    UINT8                   sfpTaskFlag=0;
    UINT32                  paraCnt;
    UINT32                  pattern=0xff, size=1518, numPkt=10, port;
    UINT32                  i, txLPort=0, rxLPort=0, lPortBase;
    UINT32                  lPortNumber = 0, speed=0;
    UINT32                  testTime, cntTestTime, cnt1S, prevCntTestTime, checkFlag=0;
    E_LINER_PORT_TYPE       startLPortType, endLPortType, errPortType;
    S_PORT_CNT              zeroCounter, txCounter, rxCounter;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LB_TEST_TYPE    snakeMode=E_LB_TEST_TYPE_MAX;
    INT8 lPortTypeStr[20]="N/A";
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if( strstr(argv[0], "snaketest") )
    {
        if( ( argc != 4) && ( argc != 8) )
        {
            ERR_PRINT_CMD_USAGE("snaketest");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if ( (strcmp("goldswitch", argv[1]) == 0) && (argc != 4) )
        {
            snakeMode = E_LB_TEST_SNAKE_MODE_CPU;
        }
        else if ( strcmp("pkt_generator", argv[1]) == 0 )
        {
            snakeMode = E_LB_TEST_SNAKE_MODE_PKT_GEN;
        }
        else
        {
            ERR_PRINT_CMD_USAGE("snaketest");
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt = 2;

        if( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr)<E_TYPE_SUCCESS)
        {
            ERR_PRINT_CMD_USAGE("snaketest");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        switch ( lPortType )
        {
            case E_LINER_PORT_TYPE_FIXED:
                break;
            case E_LINER_PORT_TYPE_STACKING:
            case E_LINER_PORT_TYPE_EXPANSION:
            case E_LINER_PORT_TYPE_INTER_LINK:
            default:
                log_printf("Unsupport this port type\n");
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
        }

        switch ( snakeMode )
        {
            case E_LB_TEST_SNAKE_MODE_CPU:
                /* Get pattern */
                if( strcmp("cjpat", argv[paraCnt]) == 0 )
                {
                    pattern = E_SW_LB_PATTERN_CJPAT;
                }
                else if( strcmp("random", argv[paraCnt]) == 0 )
                {
                    pattern = E_SW_LB_PATTERN_RANDOM;
                }
                else
                {
                    pattern = simple_strtoul(argv[paraCnt], NULL, 16);
                }

                if(pattern > 0xffff)
                {
                    ret = E_TYPE_INVALID_PARA;
                    goto __CMD_ERROR;
                }

                paraCnt ++;
                size = simple_strtoul(argv[paraCnt], NULL, 10);
                if ( size < 64 || size > 1518 )
                {
                    log_printf("Packet length is out of length\n");
                    ret = E_TYPE_INVALID_PARA;
                    goto __CMD_ERROR;
                }

                paraCnt ++;
                numPkt = simple_strtoul(argv[paraCnt], NULL, 10);

                if ( numPkt <= 0 || numPkt > 50 )
                {
                    log_printf("Number of packets is out of range\n");
                    ret = E_TYPE_INVALID_PARA;
                    goto __CMD_ERROR;
                }

                paraCnt++;
                speed = simple_strtoul(argv[paraCnt], NULL, 10);

                switch (speed)
                {
                    case 10:
                    case 100:
                    case 1000:
                        break;
                    default:
                        log_printf("The speed out of range\r\n");
                        ret = E_TYPE_INVALID_PARA;
                        goto __CMD_ERROR;
                }

                /* Get testTime */
                paraCnt++;
                testTime = simple_strtoul(argv[paraCnt], NULL, 10);
                break;
            case E_LB_TEST_SNAKE_MODE_PKT_GEN:
                checkFlag = simple_strtoul(argv[paraCnt], NULL, 10);
                if ( checkFlag > 1 )
                {
                    log_printf("Unsupport mode.\n");
                    ret = E_TYPE_INVALID_PARA;
                    goto __CMD_ERROR;
                }
                break;
            default:
                break;
        }

        if ( lPortType != E_LINER_PORT_TYPE_INTER_LINK )
        {
            switch ( snakeMode )
            {
                case E_LB_TEST_SNAKE_MODE_CPU:
                    log_printf("Please confirm cable connection: GOLDSWITCH P1<->UUT P1, GOLDSWITCH P2<->UUT P2, P3<->P4, ....,P%d<->P%d\n", endPort-1, endPort );
                    break;

                case E_LB_TEST_SNAKE_MODE_PKT_GEN:
                    log_printf("Please confirm cable Connection: PKT_GEN<->P1, PKT_GEN<->P2, P3<->P4, ..., P%d<->P%d\n", endPort-1, endPort);
                    break;

                default:
                    return E_TYPE_INVALID_PARA;
            }
        }

        if( startPort == 0 ) /* all */
        {
            startPort = 1;

            if( (speed == 10) || (speed == 100) )
            {
                endPort = boardInfo.copperMaxNum;
            }
            else
            {
                endPort = boardInfo.lPortMaxNum;
            }
        }
        else
        {
            startPort = lPortBase; /* pase to logical port in port_utilsParaPass */
            port_utilsPortType2LPortGet(lPortType, endPort, &lPortBase);
            endPort = lPortBase;
        }

        memset(&txCounter, 0 , sizeof(txCounter));
        memset(&rxCounter, 0 , sizeof(rxCounter));
        memset(&zeroCounter, 0 , sizeof(zeroCounter));

        for ( port = startPort; port <= endPort; port ++ )
        {
            /* Clear counter before test */
            if(switch_halClearPortCounter(port) != E_TYPE_SUCCESS)
            {
                log_printf("Failed to clear %s %d counter\r\n", lPortTypeStr, port);
                goto __CMD_ERROR;
            }

            if(switch_halPortCounterClearFlagSet(port, FALSE) != E_TYPE_SUCCESS)
            {
                log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, port);
                goto __CMD_ERROR;
            }
        }

        /* Before CPSS provide valid semaphore or mutex, suspend SFP LED task to avoid critical section.
         */
        switch_halSfpLedTaskFlagGet(&sfpTaskFlag);
        if(sfpTaskFlag == TRUE)
        {
            switch_halSfpLedTaskFlagSet(FALSE);
        }

        if(snakeMode == E_LB_TEST_SNAKE_MODE_CPU)
        {
            switch_linespeedTestStart();
        }
        ret = switch_lbSnakeTest(snakeMode, startPort, endPort, pattern, size, speed, numPkt);

        if ( ret < E_TYPE_SUCCESS )
        {
            goto __CMD_ERROR;
        }

        switch ( snakeMode )
        {
            case E_LB_TEST_SNAKE_MODE_CPU:
                cntTestTime = 1;
                prevCntTestTime = 0;
                cnt1S = 0;

                if( testTime == 0 )
                {
                    log_printf("Ctrl+x to break ... \r\n");
                }
                else
                {
                    log_printf("%d %% ...\r", (cntTestTime*100)/testTime);
                }

                do
                {
                    udelay(50000); /* delay 50ms, for to check link status */

                    if( testTime == 0 )
                    {
                        if( ctrlc() )
                        {
                            break;
                        }

                        goto __BREAK_TEST;
                    }
                    else
                    {
                        cnt1S++;
                        if( cnt1S >= 20 )
                        {
                            cntTestTime++;
                            cnt1S = 0;
                        }

                        if( cntTestTime > testTime )
                        {
                            log_printf("100 %%");
                            log_printf("\r\n");
                            break;
                        }
                        else
                        {
                            if( prevCntTestTime != cntTestTime )
                            {
                                log_printf("%d %% ...\r", (cntTestTime*100)/testTime);
                                prevCntTestTime = cntTestTime;
                            }
                        }
                    }
                } while(1);
__BREAK_TEST:
                if( startPort == 0 ) /* all */
                {
                    if(speed == 1000)
                    {
                        numPkt *= (port_utilsTotalFixedPortGet()/2);
                    }
                    else
                    {
                        numPkt *= boardInfo.copperMaxNum/2;
                    }
                    if(snakeMode == E_LB_TEST_SNAKE_MODE_CPU)
                    {
                        numPkt *= 2;
                    }
                }
                else
                {
                    if(snakeMode == E_LB_TEST_SNAKE_MODE_CPU)
                    {
                        numPkt *= 2;
                    }
                }

                /* Trap all packet to CPU */
                switch_linespeedTestStop(startPort, endPort, snakeMode, numPkt);

                for ( port = startPort; port <= endPort; port ++ )
                {
                    if( startPort == 0 ) /* all */
                    {
                        txLPort = lPortBase + port;
                    }
                    else
                    {
                        txLPort = port;
                    }

                    switch (lPortType)
                    {
                        case E_LINER_PORT_TYPE_FIXED:
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
                            break;
                        case E_LINER_PORT_TYPE_STACKING:
                        default:
                            break;
                    }

                    switch_halPortCntGet(txLPort, &txCounter);
                    switch_halPortCntGet(rxLPort, &rxCounter);

                    /* Clear counter after test */
                    if(switch_halClearPortCounter(txLPort) != E_TYPE_SUCCESS)
                    {
                        log_printf("Failed to clear %s %d counter after test\r\n", lPortTypeStr, txLPort);
                    }

                    if(switch_halPortCounterClearFlagSet(txLPort, FALSE) != E_TYPE_SUCCESS)
                    {
                        log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, txLPort);
                    }

                    if(switch_halClearPortCounter(rxLPort) != E_TYPE_SUCCESS)
                    {
                        log_printf("Failed to clear %s %d counter after test\r\n", lPortTypeStr, rxLPort);
                    }

                    if(switch_halPortCounterClearFlagSet(rxLPort, FALSE) != E_TYPE_SUCCESS)
                    {
                        log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, rxLPort);
                    }

                    if ( (memcmp(&zeroCounter.errCnt, &txCounter.errCnt, sizeof(txCounter.errCnt)) != 0) ||
                         (memcmp(&zeroCounter.errCnt, &rxCounter.errCnt, sizeof(rxCounter.errCnt)) != 0) )
                    {
                        log_dbgPrintf("TX Port %d\r\n", txLPort);

                        log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", txCounter.errCnt.txDeferred,      txCounter.errCnt.txExcessiveCollision);
                        log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", txCounter.errCnt.txSentMultiple,  txCounter.errCnt.txCollisionNum);
                        log_dbgPrintf("TX CRC error           : %llu\r\n", txCounter.errCnt.txCrcNum);
                        log_dbgPrintf("RX CRC error           : %llu\r\n", txCounter.errCnt.rxCrcNum);
                        log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", txCounter.errCnt.rxJabbberNum, txCounter.errCnt.rxErrorNum);
                        log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", txCounter.errCnt.rxOverRunNum, txCounter.errCnt.rxUnderSizeNum);
                        log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", txCounter.errCnt.rxFragmentsNum, txCounter.errCnt.rxOverSizeNum);
                        log_dbgPrintf("==============================================================\r\n");

                        log_dbgPrintf("RX Port %d\r\n", rxLPort);

                        log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", rxCounter.errCnt.txDeferred, rxCounter.errCnt.txExcessiveCollision);
                        log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", rxCounter.errCnt.txSentMultiple, rxCounter.errCnt.txCollisionNum);
                        log_dbgPrintf("TX CRC error           : %llu\r\n", rxCounter.errCnt.txCrcNum);
                        log_dbgPrintf("RX CRC error           : %llu\r\n", rxCounter.errCnt.rxCrcNum);
                        log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", rxCounter.errCnt.rxJabbberNum, rxCounter.errCnt.rxErrorNum);
                        log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", rxCounter.errCnt.rxOverRunNum, rxCounter.errCnt.rxUnderSizeNum);
                        log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", rxCounter.errCnt.rxFragmentsNum, rxCounter.errCnt.rxOverSizeNum);
                        log_dbgPrintf("==============================================================\r\n");
                        ret = E_TYPE_SNAKE_PACKET_ERROR;
                    }

                    log_dbgPrintf("%s %d TX %llu, RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                    log_dbgPrintf("%s %d TX %llu, RX %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);

                    if ( (txCounter.txCnt.txMulticastNum != rxCounter.rxCnt.rxMulticastNum ) || (rxCounter.txCnt.txMulticastNum != txCounter.rxCnt.rxMulticastNum) )
                    {
                        log_dbgPrintf("%s %d TX %llu != %s %d RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, lPortTypeStr, rxLPort, rxCounter.rxCnt.rxMulticastNum);
                        log_dbgPrintf("%s %d TX %llu != %s %d RX %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, lPortTypeStr, txLPort, txCounter.rxCnt.rxMulticastNum);
                        ret = E_TYPE_SNAKE_COUNTER_MISMATCH;
                    }

                    if ( txCounter.txCnt.txMulticastNum == 0 || txCounter.rxCnt.rxMulticastNum == 0 || rxCounter.txCnt.txMulticastNum == 0 || rxCounter.rxCnt.rxMulticastNum ==0)
                    {
                        log_dbgPrintf("%s %d TX = %llu RX = %llu\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum);
                        log_dbgPrintf("%s %d TX = %llu RX = %llu\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum, rxCounter.rxCnt.rxMulticastNum);
                        ret = E_TYPE_SNAKE_COUNTER_MISMATCH;
                    }

                    if((txCounter.txCnt.txMulticastNum == rxCounter.txCnt.txMulticastNum) && (txCounter.txCnt.txMulticastNum == numPkt))
                    {
                        log_dbgPrintf("%s %d TX = %llu RX = %llu = %llu\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum, txCounter.rxCnt.rxMulticastNum, numPkt);
                    }

                    if( ret == E_TYPE_CTRL_C )
                    {
                        goto __CMD_ERROR;
                    }
                    else if( ret<E_TYPE_SUCCESS )
                    {
                        retValue = ret;
                        if((txCounter.txCnt.txMulticastNum == rxCounter.txCnt.txMulticastNum) && (txCounter.txCnt.txMulticastNum == numPkt))
                        {
                            log_dbgPrintf("%s %d gold switch test (%llu) Loop storm not generated\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum);
                            log_dbgPrintf("%s %d gold switch test (%llu) Loop storm not generated\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum);
                        }
                        else
                        {
                            log_dbgPrintf("%s %d gold switch test (%llu)\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum);
                            log_dbgPrintf("%s %d gold switch test (%llu)\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum);
                        }
                        log_cmdPrintf(E_LOG_MSG_FAIL, "Gold switch\r\n");

                    }
                    else
                    {
                        log_dbgPrintf("%s %d gold switch test (%llu)\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txMulticastNum);
                        log_dbgPrintf("%s %d gold switch test (%llu)\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txMulticastNum);
                        log_cmdPrintf(E_LOG_MSG_PASS, "Gold switch\r\n");
                    }
                    port++;
                }
                break;
            case E_LB_TEST_SNAKE_MODE_PKT_GEN:
                if( checkFlag == 1 )
                {
                    log_printf("Ctrl+x to break ... \r\n");

                    do
                    {
                        udelay(500000); /* delay 500ms, for receive ctrlc signal */

                        if( ctrlc() )
                        {
                            break;
                        }
                    }while(1);


                    for ( port = startPort; port <= endPort; port ++ )
                    {
                        if( startPort == 0 ) /* all */
                        {
                            txLPort = lPortBase + port;
                        }
                        else
                        {
                            txLPort = port;
                        }

                        switch (lPortType)
                        {
                            case E_LINER_PORT_TYPE_FIXED:
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
                                break;
                            case E_LINER_PORT_TYPE_STACKING:
                            default:
                                break;
                        }

                        switch_halPortCntGet(txLPort, &txCounter);
                        switch_halPortCntGet(rxLPort, &rxCounter);

                        /* Clear counter after test */
                        if(switch_halClearPortCounter(txLPort) != E_TYPE_SUCCESS)
                        {
                            log_printf("Failed to clear %s %d counter after test\r\n", lPortTypeStr, txLPort);
                        }

                        if(switch_halPortCounterClearFlagSet(txLPort, FALSE) != E_TYPE_SUCCESS)
                        {
                            log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, txLPort);
                        }

                        if(switch_halClearPortCounter(rxLPort) != E_TYPE_SUCCESS)
                        {
                            log_printf("Failed to clear %s %d counter after test\r\n", lPortTypeStr, rxLPort);
                        }

                        if(switch_halPortCounterClearFlagSet(rxLPort, FALSE) != E_TYPE_SUCCESS)
                        {
                            log_printf("Failed to set %s %d counter flag\r\n", lPortTypeStr, rxLPort);
                        }

                        if ( (memcmp(&zeroCounter.errCnt, &txCounter.errCnt, sizeof(txCounter.errCnt)) != 0) ||
                             (memcmp(&zeroCounter.errCnt, &rxCounter.errCnt, sizeof(rxCounter.errCnt)) != 0) )
                        {
                            retValue = ret = E_TYPE_DATA_MISMATCH;
                            log_dbgPrintf("TX Port %d\r\n", txLPort);

                            log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", txCounter.errCnt.txDeferred,      txCounter.errCnt.txExcessiveCollision);
                            log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", txCounter.errCnt.txSentMultiple,  txCounter.errCnt.txCollisionNum);
                            log_dbgPrintf("TX CRC error           : %llu\r\n", txCounter.errCnt.txCrcNum);
                            log_dbgPrintf("RX CRC error           : %llu\r\n", txCounter.errCnt.rxCrcNum);
                            log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", txCounter.errCnt.rxJabbberNum, txCounter.errCnt.rxErrorNum);
                            log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", txCounter.errCnt.rxOverRunNum, txCounter.errCnt.rxUnderSizeNum);
                            log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", txCounter.errCnt.rxFragmentsNum, txCounter.errCnt.rxOverSizeNum);
                            log_dbgPrintf("==============================================================\r\n");

                            log_dbgPrintf("RX Port %d\r\n", rxLPort);

                            log_dbgPrintf("TX Deferred            : %llu\tTX Excessive Collision : %llu\r\n", rxCounter.errCnt.txDeferred, rxCounter.errCnt.txExcessiveCollision);
                            log_dbgPrintf("TX Sent Multiple       : %llu\tTX Collision           : %llu\r\n", rxCounter.errCnt.txSentMultiple, rxCounter.errCnt.txCollisionNum);
                            log_dbgPrintf("TX CRC error           : %llu\r\n", rxCounter.errCnt.txCrcNum);
                            log_dbgPrintf("RX CRC error           : %llu\r\n", rxCounter.errCnt.rxCrcNum);
                            log_dbgPrintf("RX Jabber              : %llu\tRX Error               : %llu\r\n", rxCounter.errCnt.rxJabbberNum, rxCounter.errCnt.rxErrorNum);
                            log_dbgPrintf("RX Overrun             : %llu\tRX Undersize           : %llu\r\n", rxCounter.errCnt.rxOverRunNum, rxCounter.errCnt.rxUnderSizeNum);
                            log_dbgPrintf("RX Fragments           : %llu\tRX Oversize            : %llu\r\n", rxCounter.errCnt.rxFragmentsNum, rxCounter.errCnt.rxOverSizeNum);
                            log_dbgPrintf("==============================================================\r\n");
                        }

                        log_dbgPrintf("%s %d TX %llu, RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txUnicastNum, txCounter.rxCnt.rxUnicastNum);
                        log_dbgPrintf("%s %d TX %llu, RX %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txUnicastNum, rxCounter.rxCnt.rxUnicastNum);

                        if ( (txCounter.txCnt.txUnicastNum != rxCounter.rxCnt.rxUnicastNum ) || (rxCounter.txCnt.txUnicastNum != txCounter.rxCnt.rxUnicastNum) )
                        {
                            log_dbgPrintf("%s %d TX %llu != %s %d RX %llu\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txUnicastNum, lPortTypeStr, rxLPort, rxCounter.rxCnt.rxUnicastNum);
                            log_dbgPrintf("%s %d TX %llu != %s %d RX %llu\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txUnicastNum, lPortTypeStr, txLPort, txCounter.rxCnt.rxUnicastNum);
                            retValue = ret = E_TYPE_DATA_MISMATCH;
                        }

                        if ( txCounter.txCnt.txUnicastNum == 0 || txCounter.rxCnt.rxUnicastNum == 0 || rxCounter.txCnt.txUnicastNum == 0 || rxCounter.rxCnt.rxUnicastNum ==0)
                        {
                            log_dbgPrintf("%s %d TX = %llu RX = %llu\n", lPortTypeStr, txLPort, txCounter.txCnt.txUnicastNum, txCounter.rxCnt.rxUnicastNum);
                            log_dbgPrintf("%s %d TX = %llu RX = %llu\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txUnicastNum, rxCounter.rxCnt.rxUnicastNum);
                            retValue = ret = E_TYPE_DATA_MISMATCH;
                        }
                        if( ret == E_TYPE_CTRL_C )
                        {
                            goto __CMD_ERROR;
                        }
                        else if( ret<E_TYPE_SUCCESS )
                        {
                            retValue = ret;
                            log_dbgPrintf("%s %d snake test (%llu)\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txUnicastNum);
                            log_dbgPrintf("%s %d snake test (%llu)\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txUnicastNum);
                            log_cmdPrintf(E_LOG_MSG_FAIL, "Snake test\r\n");
                        }
                        else
                        {
                            log_dbgPrintf("%s %d snake test (%llu)\r\n", lPortTypeStr, txLPort, txCounter.txCnt.txUnicastNum);
                            log_dbgPrintf("%s %d snake test (%llu)\r\n", lPortTypeStr, rxLPort, rxCounter.txCnt.txUnicastNum);
                            log_cmdPrintf(E_LOG_MSG_PASS, "Snake test\r\n");
                        }
                        port++;
                    }
                }
                else
                {
                    if ( ret < E_TYPE_SUCCESS )
                    {
                        goto __CMD_ERROR;
                    }
                }
                break;
            default:
                break;
        }
    }

__CMD_ERROR:

    switch_halSfpLedTaskFlagSet(sfpTaskFlag);

    if(snakeMode == E_LB_TEST_SNAKE_MODE_CPU)
    {
        switch_halSnakeLbTestReinit(snakeMode);
    }
    else if(snakeMode == E_LB_TEST_SNAKE_MODE_PKT_GEN)
    {
        if(checkFlag==1)
        {
            switch_halVlanReInit();
        }
    }
    return ret;
}
INT32 do_vlan
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 portList[MAX_LOGIC_PORT_52]={0};
    UINT32 pvid,vid,devId;
    UINT8 portNum=0,i=0,*tempvid;
    BOOL isTagged=1;

    if( strstr(argv[0], "vlan") )
    {
        if((argc != 2)&&(argc!=4))
        {
            ERR_PRINT_CMD_USAGE("vlan");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
       if(argc ==2)
       {
            if ( strcmp("status", argv[1]) == 0 )
            {
                ret = vlanShow();
            }
            else if ( strcmp("reinit", argv[1]) == 0 )
            {
                ret = switch_halVlanReInit();
            }
            else
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }
        }
        else if (argc ==4 )
        {
            vid=simple_strtoul(argv[3],NULL,10);
            /*get user input ports*/
            if( vlanPort_utilsParaPass(argv[2],portList,&portNum) < E_TYPE_SUCCESS )
            {
                ERR_PRINT_CMD_USAGE("vlan");
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }
            if(strcmp("add", argv[1]) == 0)
            {
              /*remove port from previous vlan and add this port to new vlan*/
                for(i=0;i<portNum;i++)
                {
                    ret=switch_halPortPvidGet(portList[i],&pvid);
                    if(pvid!=vid)
                    {
                        if( (ret = switch_halRemovePortFromVlan(portList[i], pvid)) != E_TYPE_SUCCESS)
                        {
                            return E_TYPE_REG_WRITE;
                        }
                        if( (ret = switch_halAddPortToVlan(portList[i], vid, isTagged)) != E_TYPE_SUCCESS)
                        {
                            return E_TYPE_REG_WRITE;
                        }
                        ret=switch_halPortPvidSet(portList[i], vid);
                    }
                }
            }
            else if(strcmp("del", argv[1]) == 0)
            {
            	/*remove port and add this port to 1 vlan*/
                for(i=0;i<portNum;i++)
                {
                    ret=switch_halPortPvidGet(portList[i],&pvid);
                    if(pvid == vid)
                    {
                        if( (ret = switch_halRemovePortFromVlan(portList[i], vid)) != E_TYPE_SUCCESS)
                        {
                            return E_TYPE_REG_WRITE;
                        }
                        if( (ret = switch_halAddPortToVlan(portList[i], 1, isTagged)) != E_TYPE_SUCCESS)
                        {
                            return E_TYPE_REG_WRITE;
                        }
                        ret=switch_halPortPvidSet(portList[i], 1);
                    }
                }
				/*remove vlan if it is empty*/
                for(devId=0; devId < port_utilsTotalDevGet(); devId++)
                {
                    for(i=1;i<=port_utilsTotalFixedPortGet();i++)
                    {
                        ret=switch_halPortPvidGet(i,&pvid);
                        if(pvid==vid)
                        {
                            break;
                        }
                    }
                    if(i==(port_utilsTotalFixedPortGet()+1))
                    {
                        ret=switch_halVlanInvalid(devId, vid);
                    }
                }
            }
            else
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }
        }
    }
    else
    {
        ERR_PRINT_CMD_USAGE("vlan");
        ret = E_TYPE_INVALID_PARA;
        goto __CMD_ERROR;
    }

    log_cmdPrintf(ret, "Vlan\r\n");
__CMD_ERROR:
    return ret;
}
INT32 do_casvlan
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret=E_TYPE_SUCCESS;
    BOOL isTagged=1;
    UINT32 vid=0,devNum;

    if( strstr(argv[0], "casvlan") )
    {
        if(argc!=4)
        {
            ERR_PRINT_CMD_USAGE("casvlan");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        if(argc ==4)
        {
            devNum=simple_strtoul(argv[2],NULL,10);
            vid=simple_strtoul(argv[3],NULL,10);
            if (strcmp("add", argv[1]) ==0 )
            {
                ret=switch_halAddCascadePortToVlan(devNum,24,vid,isTagged);
                ret=switch_halCascadePortPvidSet(devNum,24,vid);
                ret=switch_halAddCascadePortToVlan(devNum,26,vid,isTagged);
                ret=switch_halCascadePortPvidSet(devNum,26,vid);
            }
            else if(strcmp("del", argv[1]) ==0)
            {
                ret=switch_halRemoveCascadePortFromVlan(devNum,24,vid);
                ret=switch_halRemoveCascadePortFromVlan(devNum,26,vid);
            }
            else
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }
        }
    }
    else
    {
        ERR_PRINT_CMD_USAGE("casvlan");
        ret = E_TYPE_INVALID_PARA;
        goto __CMD_ERROR;
    }

    log_cmdPrintf(ret, "casvlan\r\n");
__CMD_ERROR:
    return ret;
}

INT32 do_tcamtest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret=E_TYPE_SUCCESS;

    if( strstr(argv[0], "tcamtest") )
    {
        if(argc != 1)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        ret = switch_lbTcamBist();
    }

    log_cmdPrintf(ret, "TCAM test\r\n");
__CMD_ERROR:
    return ret;
}

INT32 do_sfpledtask
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT8 taskFlag=TRUE;

    if( strstr(argv[0], "sfpledtask") )
    {
        if(argc != 2)
        {
            ERR_PRINT_CMD_USAGE("sfpledtask");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if ( strcmp("on", argv[1]) == 0 )
        {
            taskFlag=TRUE;
        }
        else if ( strcmp("off", argv[1]) == 0 )
        {
            taskFlag=FALSE;
        }
        else
        {
            ERR_PRINT_CMD_USAGE("sfpledtask");
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        switch_halSfpLedTaskFlagSet(taskFlag);
    }

__CMD_ERROR:
    return ret;
}


U_BOOT_CMD(
     swlbtest,    CONFIG_SYS_MAXARGS,    1,    do_swlbtest,
     "swlbtest \t- Simple loopback test. CPU tx/rx packets one by one.\n",
     "<mode> <port> <pattern> <size> <numpkt> <speed> <isSfp>\n"
     "  - mode: Specify the loopback mode. Valid values are <mac|phy|exts|extp>.\n"
     "  - port: Specify the port to test. Valid values are 1 to max ports.\n"
     "  - pattern: Specify the pattern to test. Valid values are <0~0xffff, 'cjpat' for CJ pattern, 'random' for random pattern>\n"
     "  - size: Specify the packet length ot test. Valid values are <64~1518>\n"
     "  - numpkt: Specify the number of packets to send. Valid values are <1~" MK_STR(PKT_LOOPBACK_MAX_NUM) ">\n"
     "  - speed: Specify the speed during test. Valid values are <10|100|1000|10000(fiber port only)>. \n"
     "  - isSfp: Specify the port type. Valid values are <0|1>.\n"
     "      where 0:copper, 1:fiber, respectively.\n"
);

U_BOOT_CMD(
     linespeed,    CONFIG_SYS_MAXARGS,    1,    do_linespeedtest,
     "linespeed \t- Loop storm traffic test.\n",
     "<mode> <port> <pattern> <size> <numpkt> <speed> <isSfp> <duration> \n"
     "  - mode: Specify the loopback mode. Valid values are <mac|phy|exts|extp>.\n"
     "  - port: Specify the port to test. Valid values are 1 to max ports.\n"
     "  - pattern: Specify the pattern to test. Valid values are <0~0xffff, 'cjpat' for CJ pattern, 'random' for random pattern>\n"
     "  - size: Specify the packet length ot test. Valid values are <64~1518>\n"
     "  - numpkt: Specify the number of packets to send. Valid values are <1~" MK_STR(PKT_LINESPEED_MAX_NUM) ">\n"
     "  - speed: Specify the speed during test. Valid values are <10|100|1000|10000(fiber port only)|0(Highest speed for extp mode)>. \n"
     "  - isSfp: Specify the port type. Valid values are <0|1>.\n"
     "      where 0:copper, 1:fiber, respectively.\n"
     "  - duration: Specify the test duration in seconds. Valid values are <0~4294967295>0 is infinite loop\n"
);

U_BOOT_CMD(
     pktgen,    CONFIG_SYS_MAXARGS,    1,    do_pktgentest,
     "pktgen \t\t- generator packet test.\n",
     "<port> <pattern> <size> <numpkt> <speed> \n"
     "  - port: Specify the port to test. Valid values are 1 to max ports.\n"
     "  - pattern: Specify the pattern to test. Valid values are <0~0xffff, 'cjpat' for CJ pattern, 'random' for random pattern>\n"
     "  - size: Specify the packet length ot test. Valid values are <64~1518>\n"
     "  - numpkt: Specify the number of packets to send. Valid values are <1~" MK_STR(PKT_LINESPEED_MAX_NUM) ">\n"
     "  - speed: Specify the speed during test. Valid values are <10|100|1000|10000(fiber port only)>. \n"
     "  - duration: Specify the test duration in seconds. Valid values are <0~4294967295>0 is infinite loop\n"
);

U_BOOT_CMD(
     snaketest,    CONFIG_SYS_MAXARGS,    1,    do_snaketest,
     "snaketest \t- Traffic test with snake path.\n",
     "<pkt_generator|goldswitch> <port> [<continueFlag>]|[<pattern> <size> <numpkt> <speed> <duration>]\n"
     "  - mode: Specify the test mode. Valid values are <pkt_generator|goldswitch>.\n"
     "  - port: Specify the start port and end port. Valid values are <1-port max|all>.\n"
     "  - continueFlag: Specify the snake pkt_generator keep testing or not. Valid values are <0|1>\n"
     "  - pattern: Specify the pattern to test. Valid values are <0~0xffff, 'cjpat' for CJ pattern, 'random' for random pattern>\n"
     "  - size: Specify the packet length of test. Valid values are <64~1518>\n"
     "  - numpkt: Specify the number of packets to send. Valid values are <1~" MK_STR(PKT_GOLDSWITCH_MAX_NUM) ">\n"
     "  - speed: Specify the speed during test. Valid values are <10|100|1000|10000(fiber port only)>. \n"
     "  - duration: Specify the test duration in seconds. Valid values are <0~4294967295>0 is infinite loop\n"
);

U_BOOT_CMD(
    vlan,    CONFIG_SYS_MAXARGS,    1,    do_vlan,
    "vlan \t\t- vlan operation.\n",
    "<reinit|status>|[<add|del><port><vid>]\n"
    "  -reinit | status This command re-init or display vlan settings.\n"
    "  -add |del  add or remove port to/from vlan.\n"
    "  -port:  Specify the port to add or remove.Valid values are 1 to max ports.\n"
    "  -vid:  Specify the vlan ID. Valid values are <1-4094>"
    );

U_BOOT_CMD(
    casvlan,    CONFIG_SYS_MAXARGS,    1,    do_casvlan,
    "casvlan \t\t- cascade vlan operation.\n",
    "<add|del><devid><vid>\n"
    "  -add |del  add or remove cascade port to/from vlan.\n"
    "  -devid:  Mac ID. Valid values are<0|1>\n"
    "  -vid:  Specify the vlan ID. Valid values are <1-4094>\n"
    );

U_BOOT_CMD(
     tcamtest,    CONFIG_SYS_MAXARGS,    1,    do_tcamtest,
     "tcamtest \t- TCAM test.\n",
     "<tcam>\n"
     "  - This command executes TCAM test for switch.\n"
);

U_BOOT_CMD(
     sfpledtask,    CONFIG_SYS_MAXARGS,    1,    do_sfpledtask,
     "sfpledtask \t- Enable/Disable SFP LED task\n",
     "<on|off>\n"
     "  - This command enable/disable SFP LED task.\n"
);
