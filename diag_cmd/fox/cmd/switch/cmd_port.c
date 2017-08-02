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
***      cmd_port.c
***
***    DESCRIPTION :
***      for switch port
***
***    HISTORY :
***       - 2009/05/21, 14:30:52, Eden Weng
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
#include "switch_hal.h"
#include "switch_port.h"
#include "sys_utils.h"
#include "err_type.h"
#include "log.h"
#include "foxCommand.h"
#include "port_utils.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define DBG_PRINTF log_dbgPrintf

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

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */


INT32 do_portstatus
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT8                    vendor_name[17]="fox", duplexStr[10] = "UNKNOWN", speedstr[10] = "UNKNOWN", autonegStr[10] = "UNKNOWN", lbStr[10] = "UNKNOWN", adminStr[10] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT8                   admin=0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0, rateLevel = 0;
    UINT32                  speed = 0;
    UINT32                  readDevMACId = 0;
    UINT32                  PortPHYLink=0, PortMACLink=0;
    UINT32                  val;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    S_SWITCH_PORT_STATUS    portStatus;
    S_PORT_CNT              portCounter;
    E_SWITCH_PORT_AUTONEG   autoneg;
    E_SWITCH_PORT_DUPLEX    duplex;
    E_SWITCH_PORT_LOOPBACK  loopback;
    E_LB_TEST_TYPE          lbTestType;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();

    if ( strcmp("port", argv[0]) == 0 )
    {
        if ( argc < 3 || argc > 5 )
        {
            ERR_PRINT_CMD_USAGE("port");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if ( strcmp("cascade", argv[1]) == 0 )
        {
            if( strcmp("clear", argv[2]) == 0 )
            {
                if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
                {
                    for (port=1 ; port <= 4; port ++ )
                    {
                        if ( (ret = switch_cascadePortClearCounter(port)) != E_TYPE_SUCCESS )
                        {
                            ret = E_TYPE_PORT_CASCADE_COUNTER_CLEAR_ERROR;
                            goto __CMD_ERROR;
                        }
                    }
                }
                else
                {
                    ret = E_TYPE_UNSUPPORT_DEV;
                    goto __CMD_ERROR;
                }
            }
            else if ( strcmp("counter", argv[2]) == 0 )
            {
                if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
                {
                    for (port=1 ; port <= 4; port ++ )
                    {
                        if ( (ret = switch_cascadePortCounterDump(port, &portCounter)) != E_TYPE_SUCCESS )
                        {
                            ret = E_TYPE_PORT_CASCADE_COUNTER_DUMP_ERROR;
                            goto __CMD_ERROR;
                        }
                    }
                }
                else
                {
                    ret = E_TYPE_UNSUPPORT_DEV;
                    goto __CMD_ERROR;
                }
            }
	            else if ( strcmp("status", argv[2]) == 0 )
	            {
	                if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
	                {
	                    log_printf("Port\tLink\tSpeed\tDuplex\tAN\tAdmin\t \n");
	                    log_printf("====================================================================\n");

	                    for (port=1 ; port <= 4; port ++ )
	                    {
	                        if ( (ret = switch_cascadePortStatusDump(port, &portStatus)) != E_TYPE_SUCCESS )
	                        {
	                            ret = E_TYPE_PORT_CASCADE_STATUS_DUMP_ERROR;
	                            goto __CMD_ERROR;
	                        }

	                        /* Check duplex */
	                        if ( portStatus.link == E_SWITCH_PORT_LINK_UP )
	                        {
	                            if ( portStatus.duplex == E_SWITCH_PORT_DUPLEX_HALF )
	                            {
	                                strcpy(duplexStr, "HALF");
	                            }
                            else
                            {
                                strcpy(duplexStr, "FULL");
                            }
                        }
                        else
                        {
                            strcpy(duplexStr, "N/A");
                        }

                        /* Check speed */
                        if ( portStatus.link == E_SWITCH_PORT_LINK_UP )
                        {
                            switch ( portStatus.speed )
                            {
                                case E_SWITCH_PORT_SPEED_10M:
                                    strcpy(speedstr, "10M");
                                    break;
                                case E_SWITCH_PORT_SPEED_100M:
                                    strcpy(speedstr, "100M");
                                    break;
                                case E_SWITCH_PORT_SPEED_1G:
                                    strcpy(speedstr, "1G");
                                    break;
                                case E_SWITCH_PORT_SPEED_2G5:
                                    strcpy(speedstr, "2.5G");
                                    break;
                                case E_SWITCH_PORT_SPEED_10G:
                                    strcpy(speedstr, "10G");
                                    break;
                                case E_SWITCH_PORT_SPEED_20G:
                                    strcpy(speedstr, "20G");
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            strcpy(speedstr, "N/A");
                        }

                        /* Check auto-nego status */
                        if ( portStatus.link == E_SWITCH_PORT_LINK_UP )
                        {
                            if ( portStatus.autoneg == E_SWITCH_PORT_AUTONEG_ON )
                            {
                                strcpy(autonegStr, "ON");
                            }
                            else
                            {
                                strcpy(autonegStr, "OFF");
                            }
                        }
                        else
                        {
                            strcpy(autonegStr, "N/A");
                        }

                        if ( portStatus.admin == E_SWITCH_PORT_ADMIN_ON )
                        {
                            strcpy(adminStr, "ON");
                        }
                        else if ( portStatus.admin == E_SWITCH_PORT_ADMIN_OFF )
                        {
                            strcpy(adminStr, "OFF");
                        }
                        else
                        {
                            strcpy(adminStr, "N/A");
                        }

                        log_printf("P%2d:\t%s\t%s\t%s\t%s\t%s\t\n",
                                port,
                                ( portStatus.link == E_SWITCH_PORT_LINK_UP ) ? "UP" : "DOWN",
                                speedstr,
                                duplexStr,
                                autonegStr,
                                adminStr);
                    }
                }
                else
                {
                    ret = E_TYPE_UNSUPPORT_DEV;
                    goto __CMD_ERROR;
                }
            }
        }
        else
        {
            paraCnt = 1;

            if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
            {
                ERR_PRINT_CMD_USAGE("port");
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }

            lPortBase=0;
            port = ( startPort == 0 ) ?  1 : startPort;

            if ( strcmp("speed", argv[paraCnt]) == 0 )
            {
                if ( !argv[++paraCnt] )
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                if ( strcmp("10", argv[paraCnt]) == 0 )
                {
                    speed = 10;
                }
                else if ( strcmp("100", argv[paraCnt]) == 0 )
                {
                    speed = 100;
                }
                else if ( strcmp("1000", argv[paraCnt]) == 0 )
                {
                    speed = 1000;
                }
                else if ( strcmp("1g", argv[paraCnt]) == 0 )
                {
                    speed = 1000;
                }
                else if ( strcmp("10000", argv[paraCnt]) == 0 )
                {
                    speed = 10000;
                }
                else if ( strcmp("10g", argv[paraCnt]) == 0 )
                {
                    speed = 10000;
                }
                else
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                if ( argv[++paraCnt] )
                {
                    if ( strcmp("loopback", argv[paraCnt]) == 0 )
                    {
                        lbTestType = E_LB_TEST_TYPE_MAC;
                    }
                }
                else
                {
                    lbTestType = E_LB_TEST_TYPE_NONE;
                }

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if( (ret = switch_halPortSpeedSet(lPort, speed, lbTestType)) != E_TYPE_SUCCESS)
                    {
                        ret = E_TYPE_PORT_SPEED_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else if ( strcmp("autoneg", argv[paraCnt]) == 0 )
            {
                if ( !argv[++paraCnt] )
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                if ( strcmp("on", argv[paraCnt]) == 0 )
                {
                    autoneg = E_SWITCH_PORT_AUTONEG_ON;
                }
                else if ( strcmp("off", argv[paraCnt]) == 0 )
                {
                    autoneg = E_SWITCH_PORT_AUTONEG_OFF;
                }
                else
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if ( (ret = switch_portAutoNegSet(lPort, autoneg)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_AUTONEG_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else if ( strcmp("admin", argv[paraCnt]) == 0 )
            {
                if ( !argv[++paraCnt] )
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                if ( strcmp("on", argv[paraCnt]) == 0 )
                {
                    admin = E_SWITCH_PORT_ADMIN_ON;
                }
                else if ( strcmp("off", argv[paraCnt]) == 0 )
                {
                    admin = E_SWITCH_PORT_ADMIN_OFF;
                }
                else
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if( (ret = switch_portForceLinkDownEnableSet(lPort, admin)) != E_TYPE_SUCCESS)
                    {
                        ret = E_TYPE_PORT_ADMIN_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else if ( strcmp("duplex", argv[paraCnt]) == 0 )
            {
                if ( !argv[++paraCnt] )
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                if ( strcmp("half", argv[paraCnt]) == 0 )
                {
                    duplex = E_SWITCH_PORT_DUPLEX_HALF;
                }
                else if ( strcmp("full", argv[paraCnt]) == 0 )
                {
                    duplex = E_SWITCH_PORT_DUPLEX_FULL;
                }
                else
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if( (ret = switch_portDuplexSet(lPort, duplex)) != E_TYPE_SUCCESS)
                    {
                        ret = E_TYPE_PORT_DUPLEX_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else if ( strcmp("status", argv[paraCnt]) == 0 )
            {
                log_printf("Port\tLink\tSpeed\tDuplex\tAN\tLB\tAdmin\tSFP Vendor\t\n");
                log_printf("==================================================================================\n");

                for ( ; port <= endPort; port++ )
                {
                    lPort=lPortBase+port;

                    if ( (ret = switch_portStatusDump(lPort, &portStatus)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_STATUS_DUMP_ERROR;
                        goto __CMD_ERROR;
                    }

                    /* Check duplex */
                    if ( portStatus.link == E_SWITCH_PORT_LINK_UP )
                    {
                        if ( portStatus.duplex == E_SWITCH_PORT_DUPLEX_HALF )
                        {
                            strcpy(duplexStr, "HALF");
                        }
                        else
                        {
                            strcpy(duplexStr, "FULL");
                        }
                    }
                    else
                    {
                        strcpy(duplexStr, "N/A");
                    }

                    /* Check speed */
                    if ( portStatus.link == E_SWITCH_PORT_LINK_UP )
                    {
                        switch ( portStatus.speed )
                        {
                            case E_SWITCH_PORT_SPEED_10M:
                                strcpy(speedstr, "10M");
                                break;
                            case E_SWITCH_PORT_SPEED_100M:
                                strcpy(speedstr, "100M");
                                break;
                            case E_SWITCH_PORT_SPEED_1G:
                                strcpy(speedstr, "1G");
                                break;
                            case E_SWITCH_PORT_SPEED_2G5:
                                strcpy(speedstr, "2.5G");
                                break;
                            case E_SWITCH_PORT_SPEED_10G:
                                strcpy(speedstr, "10G");
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        strcpy(speedstr, "N/A");
                    }

                    /* Check auto-nego status */
                    if ( portStatus.link == E_SWITCH_PORT_LINK_UP )
                    {
                        if ( portStatus.autoneg == E_SWITCH_PORT_AUTONEG_ON )
                        {
                            strcpy(autonegStr, "ON");
                        }
                        else
                        {
                            strcpy(autonegStr, "OFF");
                        }
                    }
                    else
                    {
                        strcpy(autonegStr, "N/A");
                    }

                    /* Check loopback status */
                    if ( portStatus.loopback == E_SWITCH_PORT_LOOPBACK_MAC )
                    {
                        strcpy(lbStr, "MAC");
                    }
                    else if ( portStatus.loopback == E_SWITCH_PORT_LOOPBACK_PHY )
                    {
                        strcpy(lbStr, "PHY");
                    }
                    else
                    {
                        strcpy(lbStr, "NONE");
                    }

                    if ( portStatus.admin == E_SWITCH_PORT_ADMIN_ON )
                    {
                        strcpy(adminStr, "ON");
                    }
                    else if ( portStatus.admin == E_SWITCH_PORT_ADMIN_OFF )
                    {
                        strcpy(adminStr, "OFF");
                    }
                    else
                    {
                        strcpy(adminStr, "N/A");
                    }

                    /* Check sfp vendor */
                    memset(vendor_name, 0, 17);

                    portInfo=port_utilsLPortInfoGet(lPort);
                    if( portInfo == NULL )
                    {
                        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
                        return E_TYPE_DATA_GET;
                    }

                    if ( (portStatus.link == E_SWITCH_PORT_LINK_UP) &&
                         (portInfo->portType == E_PORT_TYPE_1000BASEX ||
                          portInfo->portType == E_PORT_TYPE_10GXFI) )
                    {
                        strncpy(vendor_name, (INT8 *)portStatus.sfpInfo.parsed_data.vendor_name, 16);
                    }
                    else
                    {
                        strncpy(vendor_name, "N/A", 3);
                    }

                    log_printf("P%2d:\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n",
                            port,
                            ( portStatus.link == E_SWITCH_PORT_LINK_UP ) ? "UP" : "DOWN",
                            speedstr,
                            duplexStr,
                            autonegStr,
                            lbStr,
                            adminStr,
                            vendor_name);
                }
            }
            else if ( strcmp("link", argv[paraCnt]) == 0 )
            {
                log_printf("Fixed Port\tMAC\tExternal\n");
                log_printf("===============================================\n");

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if ( ((ret = switch_halPortLinkStatusGet(lPort, &PortPHYLink)) != E_TYPE_SUCCESS ) ||
                         ((ret = switch_halPortMACLinkStatusGet(lPort, &PortMACLink)) != E_TYPE_SUCCESS ))
                    {
                        ret = E_TYPE_PORT_LINK_STATUS_GET_ERROR;
                        goto __CMD_ERROR;
                    }

                    log_printf("P%2d:\t\t%s\t%s\n",
                                port,
                                (PortMACLink == E_SWITCH_PORT_LINK_UP) ? "UP" : "DOWN",
                                (PortPHYLink == E_SWITCH_PORT_LINK_DOWN) ? "DOWN" : "UP");
                }
            }
            else if ( strcmp("counter", argv[paraCnt]) == 0 )
            {
                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if ( (ret = switch_portCounterDump(lPort, &portCounter)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_COUNTER_DUMP_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else if ( strcmp("loopback", argv[paraCnt]) == 0 )
            {
                if ( !argv[++paraCnt] )
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                if ( strcmp("mac", argv[paraCnt]) == 0 )
                {
                    loopback = E_LB_TEST_TYPE_MAC;
                }
                else if ( strcmp("phy", argv[paraCnt]) == 0 )
                {
                    loopback = E_LB_TEST_TYPE_PHY;
                }
                else if ( strcmp("disable", argv[paraCnt]) == 0 )
                {
                    loopback = E_LB_TEST_TYPE_NONE;
                }
                else
                {
                    ERR_PRINT_CMD_USAGE("port");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if( (ret = switch_portLoopbackSet(lPort, loopback)) != E_TYPE_SUCCESS)
                    {
                        ret = E_TYPE_PORT_LOOPBACK_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else if ( strcmp("sfp", argv[paraCnt]) == 0 )
            {
                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    portInfo=port_utilsLPortInfoGet(lPort);
                    if( portInfo == NULL )
                    {
                        ret = E_TYPE_DATA_GET;
                        goto __CMD_ERROR;
                    }

                    if ( (portInfo->portType != E_PORT_TYPE_1000BASEX) && (portInfo->portType != E_PORT_TYPE_10GSFI) && (portInfo->portType != E_PORT_TYPE_40GQSFP)
                        && (portInfo->portType != E_PORT_TYPE_10GBASEX) && (portInfo->portType != E_PORT_TYPE_10GXFI))
                    {
                        continue;
                    }

                    log_printf("%s %2d\n", lPortTypeStr, port);
                    if ( (ret = switch_portSFPInfoDump(lPort, &portStatus)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_SFP_INFO_DUMP_ERROR;
                        goto __CMD_ERROR;
                    }

                    if ( startPort == 0 ) /* all */
                    {
                        log_printf("\n"); /* add new line for all sfp ports */
                    }
                }
            }
            else if ( strcmp("clear", argv[paraCnt]) == 0 )
            {
                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    if (ret = switch_portClearCounter(lPort) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_COUNTER_CLEAR_ERROR;
                        goto __CMD_ERROR;
                    }
                }
                if ( startPort == 0 ) /* all */
                {
                    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
                    {
                        for (port=1 ; port <= 4; port ++ )
                        {
                            lPort=lPortBase+port;
                            if ( (ret = switch_cascadePortClearCounter(lPort)) != E_TYPE_SUCCESS )
                            {
                                ret = E_TYPE_PORT_CASCADE_COUNTER_CLEAR_ERROR;
                                goto __CMD_ERROR;
                            }
                        }
                    }
                }
            }
            else if ( strcmp("sfprate", argv[paraCnt]) == 0 )
            {
                /* check sfprate select is high/low/test */
                if ( !argv[++paraCnt] )
                {
                    ERR_PRINT_CMD_USAGE("sfprate");
                    ret = E_TYPE_INVALID_CMD_FORMAT;
                    goto __CMD_ERROR;
                }
                
                /*20170626 - Try to read device MAC chip Id information */
                readDevMACId  = sys_utilsDevMACChipIdGet();
            
                /* Add Haywards2 MAC chip is 98DX3236 */
                if (readDevMACId != SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) 
                {
                    ret = E_TYPE_UNSUPPORT_DEV;
                    goto __CMD_ERROR;
                }
                
                if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) || 
                     (boardId == E_BOARD_ID_HAYWARDS_24G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_24G4G_P) )
                {
                    for ( ; port <= endPort; port ++ )
                    {
                        lPort=lPortBase+port;
                        portInfo=port_utilsLPortInfoGet(lPort);
                        if( portInfo == NULL )
                        {
                            ret = E_TYPE_DATA_GET;
                            goto __CMD_ERROR;
                        }

                        if ( (portInfo->portType != E_PORT_TYPE_1000BASEX) && (portInfo->portType != E_PORT_TYPE_10GSFI) && (portInfo->portType != E_PORT_TYPE_40GQSFP)
                            && (portInfo->portType != E_PORT_TYPE_10GBASEX) && (portInfo->portType != E_PORT_TYPE_10GXFI))
                        {
                            continue;
                        }

                        if ( startPort == 0 ) /* all */
                        {
                            log_printf("\n");
                        }

                        if ( strcmp("high", argv[paraCnt]) == 0 )
                        {
                            rateLevel = E_SWITCH_SFP_RATE_HI;

                            log_dbgPrintf("port: %2d sfp rate select is high\n", lPort);
                            ret = switch_portSfpRateSet(lPort, rateLevel);

                            if (ret != E_TYPE_SUCCESS )
                            {
                                log_cmdPrintf(E_LOG_MSG_FAIL, "Port %2d rate select is HIGH\r\n", lPort);
                                ret = E_TYPE_PORT_SFP_RATE_SET_ERROR;
                                goto __CMD_ERROR;
                            }
                            else
                            {
                                log_cmdPrintf(E_LOG_MSG_PASS, "Port %2d rate select is HIGH\r\n", lPort);
                            }

                        }
                        else if ( strcmp("low", argv[paraCnt]) == 0 )
                        {
                            rateLevel = E_SWITCH_SFP_RATE_LO;
                            log_dbgPrintf("port: %2d SFP rate select is low\n", lPort);
                            ret = switch_portSfpRateSet(lPort, rateLevel);

                            if (ret != E_TYPE_SUCCESS )
                            {
                                log_cmdPrintf(E_LOG_MSG_FAIL, "Port %2d rate select is LOW\r\n", lPort);
                                ret = E_TYPE_PORT_SFP_RATE_SET_ERROR;
                                goto __CMD_ERROR;
                            }
                            else
                            {
                                log_cmdPrintf(E_LOG_MSG_PASS, "Port %2d rate select is LOW\r\n", lPort);
                            }
                        }
                        else if ( strcmp("test", argv[paraCnt]) == 0 )
                        {
                            /* Test SFP port rate select pin from ioexp */
                            ret = switch_portSfpRateTest(lPort);

                            if (ret != E_TYPE_SUCCESS )
                            {
                                log_cmdPrintf(E_LOG_MSG_FAIL, "Port %2d rate select tested\r\n", lPort);
                                ret = E_TYPE_PORT_SFP_RATE_TEST_ERROR;
                                goto __CMD_ERROR;
                            }
                            else
                            {
                                log_cmdPrintf(E_LOG_MSG_PASS, "Port %2d rate select tested\r\n", lPort);
                            }
                        }
                        else
                        {
                            ERR_PRINT_CMD_USAGE("sfprate");
                            ret = E_TYPE_INVALID_CMD_FORMAT;
                            goto __CMD_ERROR;
                        }
                    } /* End of for loop end port */
                }
                else
                {
                    ret = E_TYPE_UNSUPPORT_DEV;
                    goto __CMD_ERROR;
                }
            }
            else if ( strcmp("sfptemp", argv[paraCnt]) == 0 )
            {
                log_printf("Port\t  Temperature (Celsius)\n");
                log_printf("-------------------------------\n");

                for ( ; port <= endPort; port ++ )
                {
                    lPort=lPortBase+port;
                    portInfo=port_utilsLPortInfoGet(lPort);
                    if( portInfo == NULL )
                    {
                        ret = E_TYPE_DATA_GET;
                        goto __CMD_ERROR;
                    }

                    if ( (portInfo->portType != E_PORT_TYPE_1000BASEX) && (portInfo->portType != E_PORT_TYPE_10GSFI) && (portInfo->portType != E_PORT_TYPE_40GQSFP)
                        && (portInfo->portType != E_PORT_TYPE_10GBASEX) && (portInfo->portType != E_PORT_TYPE_10GXFI))
                    {
                        continue;
                    }

                    /* Read SFP optical diagnostic monitor data type is available and real-time temperature */
                    if ( (ret = switch_portSFPDiagMonitorTemp(lPort,&portStatus)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_SFP_DIAG_MON_TEMP_ERROR;
                        goto __CMD_ERROR;
                    }

                    if ( startPort == 0 ) /* all */
                    {
                        log_printf("\n"); /* add new line for all sfp ports */
                    }
                }
            }
            else
            {
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_eeestatus
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LB_TEST_TYPE          lbTestType;
    E_PHY_LPI_MODE          lpiMode,tempLpiMode;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();

    if ( strcmp("eee", argv[0]) == 0 )
    {
        if ( argc < 3 || argc > 4 )
        {
            ERR_PRINT_CMD_USAGE("eee");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;

        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE("port");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        lPortBase=0;
        port = ( startPort == 0 ) ?  1 : startPort;

        if ( strcmp("mode", argv[paraCnt]) == 0 )
        {
            paraCnt++;
            if ( strcmp("off", argv[paraCnt]) == 0 )
            {
                lpiMode = E_PHY_LPI_DISABLED;
            }
            else if ( strcmp("on", argv[paraCnt]) == 0 )
            {
                lpiMode = E_PHY_LPI_ENABLED;
            }
        }
        else if ( strcmp("status", argv[paraCnt]) == 0 )
        {
            lpiMode = E_PHY_LPI_SHOW;
        }
        else
        {
            ERR_PRINT_CMD_USAGE("eee");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            if ( lpiMode == E_PHY_LPI_SHOW)
            {
                ret = switch_portEEEGet(lPort,&tempLpiMode);
		if ( tempLpiMode < E_PHY_LPI_SHOW )
                    log_printf("Port %2d LPI mode:    %s\n", lPort, \
                        (tempLpiMode==E_PHY_LPI_ENABLED)? "Enabled" : "Disabled");
            }
            else
                ret = switch_portEEESet(lPort, lpiMode);

            if (ret != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_PHY_LPI_ERROR;
                goto __CMD_ERROR;
            }
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_cablediag
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LB_TEST_TYPE          lbTestType;
    E_PHY_LPI_MODE          lpiMode;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    PHY_CABLE_INFO          cableStatus;
    INT8 testStatus[9][20]={"Fail", "Normal", "Open", "Shorted", "ImpedanceMis", "SHORT_WITH_PAIR0",
        "SHORT_WITH_PAIR1", "SHORT_WITH_PAIR2", "SHORT_WITH_PAIR3"};
    S_BOARD_INFO    boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("cablediag", argv[0]) == 0 )
    {
        if ( argc != 2 )
        {
            ERR_PRINT_CMD_USAGE("cablediag");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;

        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE("cablediag");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        lPortBase=0;
        port = ( startPort == 0 ) ?  1 : startPort;

        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            memset(&cableStatus, 0, sizeof(PHY_CABLE_INFO));
	    /* Skip fiber port */
	    if ( lPort > boardInfo.copperMaxNum )
		continue;

            if ( (ret = switch_portCableDiag(lPort, &cableStatus)) != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_CABLE_DIAG_ERROR;
                goto __CMD_ERROR;
            }
            if (cableStatus.linkStatus)
            {
                log_printf("Interface Speed Pair Cable length Channel Pair status\n");
                log_printf("--------- ----- ---- ------------ ------- -----------\n");
                log_printf("port %2d   %4dM 1-2    +/- %d m    Pair A   Link OK \n", \
                    lPort, cableStatus.linkSpeed, cableStatus.cableLen[0]);
                log_printf("                3-6    +/- %d m    Pair B   Link OK \n", \
                    cableStatus.cableLen[1]);
                log_printf("                4-5    +/- %d m    Pair C   Link OK \n", \
                    cableStatus.cableLen[2]);
                log_printf("                7-8    +/- %d m    Pair D   Link OK \n", \
                    cableStatus.cableLen[3]);
            }
            else
            {
                log_printf("Interface   Pair     Err cable Len     Pair Status\n");
                log_printf("---------  ------    -------------     -----------\n");
                log_printf("port %2d    Pair A       +/- %d m        %s\n", \
                    lPort, cableStatus.cableLen[0], testStatus[cableStatus.cableStatus[0]]);
                log_printf("           Pair B       +/- %d m        %s\n", \
                    cableStatus.cableLen[1], testStatus[cableStatus.cableStatus[1]]);
                log_printf("           Pair C       +/- %d m        %s\n", \
                    cableStatus.cableLen[2], testStatus[cableStatus.cableStatus[2]]);
                log_printf("           Pair D       +/- %d m        %s\n", \
                    cableStatus.cableLen[3], testStatus[cableStatus.cableStatus[3]]);
            }
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_maxmtusize
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0, max_frame_size=0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LB_TEST_TYPE          lbTestType;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();

    if ( argc < 2 || argc > 3 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if ( strcmp("cascade", argv[1]) == 0 )
    {
        if(argc == 3)
        {
            max_frame_size = simple_strtoul(argv[paraCnt], NULL, 10);

            if( (1518 > max_frame_size) || (max_frame_size > 16360) )
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }

            if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
            {
                for (port=1 ; port <= 4; port ++ )
                {
                    if ( (ret = switch_cascadePortFrameSizeSet(port, max_frame_size)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_CASCADE_FRAME_SIZE_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else
            {
                ret = E_TYPE_UNSUPPORT_DEV;
                goto __CMD_ERROR;
            }
        }
        else
        {
            if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
            {
                for (port=1 ; port <= 4; port ++ )
                {
                    if ( (ret = switch_cascadePortFrameSizeGet(port, &max_frame_size)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_CASCADE_FRAME_SIZE_GET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
                log_printf("Cascade port %2d max frame size %ld\r\n", port, max_frame_size);
            }
            else
            {
                ret = E_TYPE_UNSUPPORT_DEV;
                goto __CMD_ERROR;
            }
        }
    }
    else
    {
        paraCnt = 1;

        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        lPortBase=0;
        port = ( startPort == 0 ) ?  1 : startPort;

        if(argc == 3)
        {
            max_frame_size = simple_strtoul(argv[paraCnt], NULL, 10);

            if( (1518 > max_frame_size) || (max_frame_size > 16360) )
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }

            for ( ; port <= endPort; port ++ )
            {
                lPort=lPortBase+port;
                if ( (ret = switch_portFrameSizeSet(lPort, max_frame_size)) != E_TYPE_SUCCESS)
                {
                    ret = E_TYPE_PORT_FRAME_SIZE_SET_ERROR;
                    goto __CMD_ERROR;
                }
            }
        }
        else
        {
            for ( ; port <= endPort; port ++ )
            {
                lPort=lPortBase+port;

                /* Check port max frame size */
                if ( (ret = switch_portFrameSizeGet(lPort, &max_frame_size)) != E_TYPE_SUCCESS )
                {
                    ret = E_TYPE_PORT_FRAME_SIZE_GET_ERROR;
                    goto __CMD_ERROR;
                }
                log_printf("Port %2d max frame size %ld\r\n", lPort, max_frame_size);
            }
        }
    }


__CMD_ERROR:
    return ret;
}

INT32 do_flowcontrol
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT8                   flowControlStatus=E_SWITCH_PORT_FLOWCONTROL_ON;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0, testPort=0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_LB_TEST_TYPE          lbTestType;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    UINT32  portSpeed;
    UINT32  readDevMACId = 0;

    if ( argc < 2 || argc > 3 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if ( strcmp("cascade", argv[1]) == 0 )
    {
        if(argc == 3)
        {
            paraCnt = 2;
            if ( strcmp("on", argv[paraCnt]) == 0 )
            {
                flowControlStatus = E_SWITCH_PORT_FLOWCONTROL_ON;
            }
            else if ( strcmp("off", argv[paraCnt]) == 0 )
            {
                flowControlStatus = E_SWITCH_PORT_FLOWCONTROL_OFF;
            }
            else
            {
                ERR_PRINT_CMD_USAGE("port");
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }

            if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
            {
                for (port=1 ; port <= 4; port ++ )
                {
                    if ( (ret = switch_cascadePortFlowControlSet(port, flowControlStatus)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_CASCADE_FLOW_CONTROL_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
            else
            {
                ret = E_TYPE_UNSUPPORT_DEV;
                goto __CMD_ERROR;
            }
        }
        else
        {
            if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) )
            {
                for (port=1 ; port <= 4; port ++ )
                {
                    if ( (ret = switch_cascadePortFlowControlGet(port, &flowControlStatus)) != E_TYPE_SUCCESS )
                    {
                        ret = E_TYPE_PORT_CASCADE_FLOW_CONTROL_GET_ERROR;
                        goto __CMD_ERROR;
                    }
                    if(flowControlStatus == E_SWITCH_PORT_FLOWCONTROL_ON)
                    {
                        log_printf("Cascade port %ld flow control status enabled\r\n", port);
                    }
                    else
                    {
                        log_printf("cascade port %ld flow control status disabled\r\n", port);
                    }
                }
            }
            else
            {
                ret = E_TYPE_UNSUPPORT_DEV;
                goto __CMD_ERROR;
            }
        }
    }
    else
    {
        paraCnt = 1;

        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        lPortBase=0;
        port = ( startPort == 0 ) ?  1 : startPort;

        if(argc == 3)
        {
            paraCnt = 2;
            if ( strcmp("on", argv[paraCnt]) == 0 )
            {
                flowControlStatus = E_SWITCH_PORT_FLOWCONTROL_ON;
            }
            else if ( strcmp("off", argv[paraCnt]) == 0 )
            {
                flowControlStatus = E_SWITCH_PORT_FLOWCONTROL_OFF;
            }
            else
            {
                ERR_PRINT_CMD_USAGE("port");
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }

            /*20170626 - Try to read device MAC chip Id information */
            readDevMACId  = sys_utilsDevMACChipIdGet();
            
            /* Add Haywards2 MAC chip is 98DX3236 */
            if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) 
            { 
                for ( ;port<=endPort; port++ )
                {
                    lPort=lPortBase+port;
                    switch_portSpeedGet(lPort, &portSpeed);
                  
                    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_48G4G_P) || (boardId == E_BOARD_ID_HAYWARDS_24G4G_T) || (boardId == E_BOARD_ID_HAYWARDS_24G4G_P) )
                    {
                        if ( portSpeed == 10000 )
                        {
                          log_dbgPrintf("Port %ld: 10G port not support to set port flow control\r\n",lPort);
                        }
                    else
                    {
                        if ( (ret = switch_portFlowControlSet(lPort, flowControlStatus)) != E_TYPE_SUCCESS )
                        {
                          ret = E_TYPE_PORT_CASCADE_FLOW_CONTROL_SET_ERROR;
                          goto __CMD_ERROR;
                        }
                        else
                        {
                          log_dbgPrintf("Port %ld: Succeed in setting port flow control\r\n",lPort);
                        }
                    }
                    }
                    else
                    {
                        ret = E_TYPE_UNSUPPORT_DEV;
                        goto __CMD_ERROR;
                    }
                } /* End for loop */   
            } else {
                /* Haywards port is supported 1G Speed, all ports can be set enable flowcontrol */
                for ( ;port<=endPort; port++ )
                {
                    lPort=lPortBase+port;
                    if ( (ret = switch_portFlowControlSet(lPort, flowControlStatus)) != E_TYPE_SUCCESS)
                    {
                        ret = E_TYPE_PORT_FLOW_CONTROL_SET_ERROR;
                        goto __CMD_ERROR;
                    }
                }
            }
        }
        else
        {
            testPort=port;
            for(;testPort<=endPort;testPort++)
            {
                lPort=lPortBase+testPort;

                /* Check port flow control status */
                if ( (ret = switch_portFlowControlGet(lPort, &flowControlStatus)) != E_TYPE_SUCCESS )
                {
                    ret = E_TYPE_PORT_FLOW_CONTROL_GET_ERROR;
                    goto __CMD_ERROR;
                }
                if(flowControlStatus == E_SWITCH_PORT_FLOWCONTROL_ON)
                {
                    log_printf("Port %ld flow control status enabled\r\n", testPort);
                }
                else
                {
                    log_printf("Port %ld flow control status disabled\r\n", testPort);
                }
            }
        }
    }

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    port,   CONFIG_SYS_MAXARGS,    1,  do_portstatus,
        "port \t\t- Port status, counter or configurations.\n",
        "<port_num|cascade> <status|speed <value>|autoneg <value>|counter|sfp|link|loopback|clear|admin <value>|sfprate<value>|sfptemp>\n"
        "  - port_num: Specify a range of port numbers.\n"
        "    cascade : Show cascade port counters. Note: This option only supported <status|counter|clear> in Haywards-48\n"
        "  - status: Show port status\n"
        "  - speed: Change port speed. Valid values are <10 | 100 | 1000 | 10000(fiber port only)>.\n"
        "  - autoneg: Change the autoneg mode. Valid values are <on | off>.\n"
        "  - counter: Show port counters.\n"
        "  - sfp: Dump EEPROM content of sfp module.\n"
        "  - link: Show link status of mac and phy.\n"
        "  - admin : enable or disable ports. Valid values are <on|off>.\n"
        "  - loopback: loopback mode. Valid values are <mac|phy|disable>\n"
        "  - clear: Clear port counters.\n"
        "  - sfprate: Select SFP+ rate and check the SFP+ rate select. Valid values are <high|low|test>.\n"
        "  - sfptemp: Show SFP/SFP+ optical module internal temperature status.\n"
);

U_BOOT_CMD(
    maxmtusize,   CONFIG_SYS_MAXARGS,    1,  do_maxmtusize,
        "maxmtusize \t- Display port max frame size or change port max frame size.\n",
        "<port_num> [<size>].\r\n"
        "  - port_num: Specify a range of port numbers.\r\n"
        "  - size: port max frame size. Valid values are <1518~10304>.\r\n"
);

U_BOOT_CMD(
    flowcontrol,   CONFIG_SYS_MAXARGS,    1,  do_flowcontrol,
        "flowcontrol \t- Display port flow control status or change port flow control status.\n",
        "<port_num> [<status>].\r\n"
        "  - port_num: Specify a range of port numbers.\r\n"
        "  - status: enable or disable port flow control. Valid values are <on|off>.\r\n"
);

U_BOOT_CMD(
    eee,   CONFIG_SYS_MAXARGS,    1,  do_eeestatus,
        "eee \t\t- Display EEE status or configure EEE function.\n",
        "<port_num> <status|mode <value>>.\r\n"
        "  - port_num: Specify a range of port numbers.\r\n"
        "  - status: Show EEE status.\r\n"
        "  - mode : Enable or disable EEE function. Valid values are <on|off>.\r\n"
);

U_BOOT_CMD(
    cablediag,   CONFIG_SYS_MAXARGS,    1,  do_cablediag,
        "cablediag \t- Perform cable diagnostic.\n",
        "<port_num>\r\n"
        "  - port_num: Specify a range of port numbers.\r\n"
);



