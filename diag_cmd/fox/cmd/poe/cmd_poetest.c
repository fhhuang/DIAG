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
***      cmd_poetest.c
***
***    DESCRIPTION :
***      commands for PoE testing&debugging
***
***    HISTORY :
***       - 2010/01/05, Jungle Chen
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
#include "poe_hal.h"
#include "port_utils.h"
#include "sys_utils.h"

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

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
INT32 do_poetest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_INVALID_PARA;
    UINT32 start_port, end_port;
    UINT32 i, j = 0;
    UINT8 stages = 0;
    E_POE_MODULE poeModuleType;
    E_POE_PD_TYPE pdType=E_POE_PD_STANDARD;
    UINT32 pdClass;
    UINT32 paraCnt;
    UINT32 vop_margin=4000;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    UINT32 lPort, lPortBase, port;
    INT8 lPortTypeStr[20];
    S_PORT_INFO *portInfo;
    UINT32 maxPoePort;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if( (argc < 3) || (argc > 4) )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    paraCnt = 1;
    if( port_utilsParaPass(&paraCnt, argv, FALSE, &start_port, &end_port, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    maxPoePort = sys_utilsPoePortNumGet();

    if( start_port == 0 ) /* all */
    {    
        start_port = 1;
        end_port = maxPoePort;
    }    
    else
    {
        if(start_port > maxPoePort)
        {
            goto __CMD_ERROR;
        }
    }

    if( strcmp("all", argv[2]) == 0 )
    {
        stages |= POE_ALL_STAGES;
    }
    else if( strcmp("detect", argv[2]) == 0 )
    {
        stages |= POE_DETECTION_STAGE;
    }
    else if( strcmp("classify", argv[2]) == 0 ) 
    {
        stages |= POE_CLASSIFICATION_STAGE;
    }
    else if( strcmp("powerup", argv[2]) == 0 ) 
    {
        stages |= POE_POWERUP_STAGE;
    }
    else if( strcmp("disconnect", argv[2]) == 0 ) 
    {
        stages |= POE_DISCONNECT_STAGE;
    }
    else if( strcmp("vopcheck", argv[2]) == 0 ) 
    {
        stages |= POE_VOPCHECK_STAGE;
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if(stages&POE_DETECTION_STAGE)
    {
        if(argc == 4)
        {
            if( strcmp("legacy", argv[3]) == 0 )
            {
                pdType = E_POE_PD_LEGACY;
            }
        }
        else
        {
            pdType = POE_PD_DEFAULT_TYPE;
        }

        log_printf("\nDetection test will start\n");
        log_printf("Please CONNECT PD devices into assigned ports in DUT first!\n");

        POE_DELAY(500000); /* delay 500ms */

        for (lPort = start_port; lPort <= end_port; lPort++)
        {
            if((ret = poe_halDetectTest(lPort, pdType)) == E_TYPE_SUCCESS)
            {
                log_cmdPrintf(E_LOG_MSG_PASS, "PoE detect test\r\n");
            }
            else
            {
                log_cmdPrintf(E_LOG_MSG_FAIL, "PoE detect test\r\n");
            }
        }
    }

    if(stages&POE_CLASSIFICATION_STAGE)
    {
        if(argc == 4)
        {
            pdClass = simple_strtoul(argv[3], NULL, 10);
            if(pdClass < 0 || pdClass > 4)
            {
                ERR_PRINT_CMD_USAGE(argv[0]);
                ret = E_TYPE_INVALID_CMD_FORMAT;
                goto __CMD_ERROR;
            }
        }
        else
        {
            pdClass = POE_PD_DEFAULT_CLASS;
        }

        log_printf("\nClassification test will start\n");
        log_printf("Please CONNECT PD devices into assigned ports in DUT first!\n");

        POE_DELAY(500000); /* delay 500ms */

        for (lPort = start_port; lPort <= end_port; lPort++)
        {
            if((ret = poe_halClassifyTest(lPort, pdClass)) == E_TYPE_SUCCESS)
            {
                log_cmdPrintf(E_LOG_MSG_PASS, "PoE classify test\r\n");
            }
            else
            {
                log_cmdPrintf(E_LOG_MSG_FAIL, "PoE classify test\r\n");
            }
        }
    }

    if(stages&POE_POWERUP_STAGE)
    {
        log_printf("\nPowerup test will start\n");    
        log_printf("Please CONNECT PD devices into assigned ports in DUT first!\n");

        POE_DELAY(500000); /* delay 500ms */    

        for (lPort = start_port; lPort <= end_port; lPort++)
        {
            if((ret = poe_halPowerUpTest(lPort)) == E_TYPE_SUCCESS)
            {
                log_cmdPrintf(E_LOG_MSG_PASS, "PoE powerup test\r\n");
            }
            else
            {
                log_cmdPrintf(E_LOG_MSG_FAIL, "PoE powerup test\r\n");
            }
        }
    }

    if(stages&POE_DISCONNECT_STAGE)
    {
        log_printf("\nDisconnection test will start\n");    
        log_printf("Please DISCONNECT PD devices from assigned ports in DUT first!\n");

        POE_DELAY(500000); /* delay 500ms */

        for (lPort = start_port; lPort <= end_port; lPort++)
        {
            if((ret = poe_halDisconnetTest(lPort)) == E_TYPE_SUCCESS)
            {
                log_cmdPrintf(E_LOG_MSG_PASS, "PoE disconnect test\r\n");
            }
            else
            {
                log_cmdPrintf(E_LOG_MSG_FAIL, "PoE disconnect test\r\n");
            }
        }
    }

    if(stages&POE_VOPCHECK_STAGE)
    {
        if(argc == 4)
        {
            vop_margin = simple_strtoul(argv[3], NULL, 10);
        }
        else
        {
            /*default 4000mv*/
            vop_margin = 4000;
        }
        
        log_printf("\nvopcheck test will start\n");    
        log_printf("Please CONNECT PD devices into assigned ports in DUT first!\n");

        POE_DELAY(500000); /* delay 500ms */

        for (lPort = start_port; lPort <= end_port; lPort++)
        {
            if((ret = poe_halVopCheckTest(lPort, vop_margin)) == E_TYPE_SUCCESS)
            {
                log_cmdPrintf(E_LOG_MSG_PASS, "PoE vopcheck test\r\n");
            }
            else
            {
                log_cmdPrintf(E_LOG_MSG_FAIL, "PoE vopcheck test\r\n");
            }
        }
    }

__CMD_ERROR:
    
    return ret;
}

INT32 do_poeforcepw
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    BOOL forcePowerFlag=FALSE;
    INT32 ret = E_TYPE_INVALID_PARA;
    UINT32 start_port, end_port;
    UINT32 maxPoePort=0;
    UINT32 lPort, lPortBase, port;
    INT8 lPortTypeStr[20];
    E_POE_MODULE poeModuleType;
    E_POE_PD_TYPE pdType=E_POE_PD_STANDARD;
    UINT32 paraCnt;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    S_PORT_INFO *portInfo;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if( argc != 3 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    paraCnt = 1;
    if( port_utilsParaPass(&paraCnt, argv, FALSE, &start_port, &end_port, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    maxPoePort = sys_utilsPoePortNumGet();

    if( start_port == 0 ) /* all */
    {    
        port = 1;
        end_port = maxPoePort;
    }    
    else
    {
        if(start_port > maxPoePort)
        {
            goto __CMD_ERROR;
        }

        port = start_port;
    }

    if( strcmp("on", argv[2]) == 0 )
    {
        forcePowerFlag = TRUE;
    }
    else
    {
        forcePowerFlag = FALSE;
    }

    log_printf("\nForce power %s\n", forcePowerFlag?"On":"Off");

    poe_halSetIndividualMask(0x2b, (forcePowerFlag==FALSE)?TRUE:FALSE);

    udelay(3000000);

    for (; port<=end_port; port++)
    {
        if( start_port == 0 )
        {
            lPort = lPortBase + port;
        }
        else
        {
            lPort = port;
        }

        portInfo = port_utilsLPortInfoGet(port);

        if( portInfo->isPoE == FALSE )
        {
            continue;
        }

        log_printf("%s force power port %d\n", (forcePowerFlag==TRUE)?"Enable":"Disable", port);
        ret = poe_halSetPortForcePowerOn(port, forcePowerFlag);
    }

__CMD_ERROR:
    
    return ret;
}

INT32 do_poemode
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    BOOL af_mode=FALSE;
    INT32 ret = E_TYPE_INVALID_PARA;
    UINT8 admin=0;
    UINT32 start_port, end_port;
    UINT32 txLPort, rxLPort;
    UINT32 maxPoePort=0;
    UINT32 lPort, lPortBase, port;
    INT8 lPortTypeStr[20];
    E_POE_MODULE poeModuleType;
    E_POE_PD_TYPE pdType=E_POE_PD_STANDARD;
    UINT32 paraCnt;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    S_PORT_INFO *portInfo;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if( argc != 4 )
    {
        ERR_PRINT_CMD_USAGE("poemode");
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    paraCnt = 1;
    if( port_utilsParaPass(&paraCnt, argv, FALSE, &start_port, &end_port, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( strcmp("at", argv[2]) == 0 )
    {
        af_mode = FALSE;
    }
    else if( strcmp("af", argv[2]) == 0 )
    {
        af_mode = TRUE;
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if( strcmp("on", argv[3]) == 0 )
    {
        admin = 0x1;
    }
    else if( strcmp("off", argv[3]) == 0 )
    {
        admin = 0x0;
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    maxPoePort = sys_utilsPoePortNumGet();

    if( start_port == 0 ) /* all */
    {
        port = 1;
        end_port = maxPoePort;
    }
    else
    {
        if(start_port > maxPoePort)
        {
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        port = start_port;
    }

    for (; port<=end_port; port++)
    {
        if( start_port == 0 ) /* all */
        {
            lPort = lPortBase + port;
        }
        else
        {
            lPort = port;
        }

        portInfo = port_utilsLPortInfoGet(lPort);
        if( portInfo->isPoE == FALSE )
        {
            continue;
        }

        ret = poe_halSetPortMode(lPort, af_mode, admin);
    }

__CMD_ERROR:
    
    return ret;
}


INT32 do_poestatus
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_INVALID_PARA;
    UINT32 start_port, end_port;
    UINT32 max_port, txLPort, rxLPort;
    S_SCP_PKT pkt;
    UINT8 *ch;
    E_SCP_RET_CODE scp_ret;
    UINT8 key;
    E_POE_MODULE poeModuleType;
    E_POE_PD_TYPE pdType=E_POE_PD_STANDARD;
    UINT32 pdClass;
    UINT32 paraCnt;
    E_LINER_PORT_TYPE lPortType = E_LINER_PORT_TYPE_FIXED;
    UINT32 lPort, lPortBase, port, maxPoePort=0;
    INT8 lPortTypeStr[20];
    S_PORT_INFO *portInfo;
    POE_POWERINFO_T portPowerInfo;
    UINT8 pdclass;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if ( argv[1] == NULL )
    {
        log_printf("parameter wrong \n");
        goto __CMD_ERROR;
    }

    if( argc != 2 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    paraCnt = 1;
    if( port_utilsParaPass(&paraCnt, argv, FALSE, &start_port, &end_port, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    maxPoePort = sys_utilsPoePortNumGet();

    if( start_port == 0 ) /* all */
    {
        port = 1;
        end_port = maxPoePort;
    }
    else
    {
        if(start_port > maxPoePort)
        {
            goto __CMD_ERROR;
        }

        port = start_port;
    }

    for (; port<=end_port; port++)
    {
        if( start_port == 0 ) /* all */
        {
            lPort = lPortBase + port;
        }
        else
        {
            lPort = port;
        }

        portInfo = port_utilsLPortInfoGet(lPort);
        if( portInfo->isPoE == FALSE )
        {
            continue;
        }

        ret = poe_halPowerShow(lPort, &portPowerInfo);
        poe_halGetPdClass(lPort, &pdclass);

        log_printf("%d.%dV %dmA %dmW", portPowerInfo.vmainVoltage/10, portPowerInfo.vmainVoltage%10, \
                        portPowerInfo.caculatedCurrent, portPowerInfo.powerConsumption);

        if( pdclass == 255 ) /* illegal value */
            log_printf(" Class N/A ");
        else
            log_printf(" Class %d ", pdclass);

        if(portPowerInfo.vmainVoltage < (POE_PD_POWER_LOWER_LIMIT * 10) || portPowerInfo.vmainVoltage > (POE_PD_POWER_UPPER_LIMIT * 10))
        {
            log_printf(" Out of range %d - %d v,", POE_PD_POWER_LOWER_LIMIT, POE_PD_POWER_UPPER_LIMIT);
        }       
        
        /* define 500mA, and 600mA for min and max, just for test*/
        if((portPowerInfo.caculatedCurrent < POE_DEFINE_TEST_MIN_CURRENT) || (portPowerInfo.caculatedCurrent > POE_DEFINE_TEST_MAX_CURRENT))
        {
            log_printf(" Out of range %d - %d mA,", POE_DEFINE_TEST_MIN_CURRENT, POE_DEFINE_TEST_MAX_CURRENT);
        }

        /* define 27000mW, and 32400mW for min and max, just for test*/
        if((portPowerInfo.powerConsumption < POE_DEFINE_TEST_MIN_CONSUMPTION) || (portPowerInfo.powerConsumption > POE_DEFINE_TEST_MAX_CONSUMPTION))
        {
            log_printf(" Out of range %d - %d mW", POE_DEFINE_TEST_MIN_CONSUMPTION, POE_DEFINE_TEST_MAX_CONSUMPTION);  
        }
        log_printf("\r\n");
    }

__CMD_ERROR:
    
    return ret;
}

INT32 do_poecmd
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_INVALID_PARA;
    S_SCP_PKT pkt;
    UINT8 *ch;
    E_SCP_RET_CODE scp_ret;
    UINT8 key;
    UINT32 i=0;
    E_POE_MODULE poeModuleType;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if(argc < 2 || argc > 12)
    {
        goto __CMD_ERROR;
    }

    memset(&pkt, 0x4e, sizeof(S_SCP_PKT));
    ch = (UINT8 *)&pkt.subject;

    for (i=1;i < argc;i++)
    {
        *ch++ = simple_strtoul(argv[i], NULL, 16);
    }

    pkt.key = SCP_COMMAND;
    scp_ret = poe_halSCPDebugCmd(&pkt);
     
    switch(scp_ret)    
    {            
        case E_SCP_ECHO_ERROR:
            log_dbgPrintf("** Received packet echo number error !! **\r\n");
            break;
        case E_SCP_CHECKSUM_ERROR:
            log_dbgPrintf("** Received packet checksum error !! **\r\n");
            break;
        case E_SCP_STATUS_RECEIVE_ERROR :
            log_dbgPrintf("** Receiving SCP packet error !! **\r\n");
            break;
        case E_SCP_STATUS_TRANSMIT_ERROR :
            log_dbgPrintf("** Transmitting SCP packet error !! **\r\n");
            break;
        default:
            break;
    }
    return E_TYPE_SUCCESS;
    
__CMD_ERROR:
    
    return ret;
}

INT32 do_poerequest
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_INVALID_PARA;
    S_SCP_PKT pkt;
    UINT8 *ch;
    E_SCP_RET_CODE scp_ret;
    UINT8 key;
    UINT32 i=0;
    E_POE_MODULE poeModuleType;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if(argc < 2 || argc > 12)
    {
        goto __CMD_ERROR;
    }

    memset(&pkt, 0x4e, sizeof(S_SCP_PKT));
    ch = (UINT8 *)&pkt.subject;

    for (i=1;i < argc;i++)
    {
        *ch++ = simple_strtoul(argv[i], NULL, 16);
    }

    pkt.key = SCP_REQUEST;
    scp_ret = poe_halSCPDebugCmd(&pkt);
     
    switch(scp_ret)    
    {            
        case E_SCP_ECHO_ERROR:
            log_dbgPrintf("** Received packet echo number error !! **\r\n");
            break;
        case E_SCP_CHECKSUM_ERROR:
            log_dbgPrintf("** Received packet checksum error !! **\r\n");
            break;
        case E_SCP_STATUS_RECEIVE_ERROR :
            log_dbgPrintf("** Receiving SCP packet error !! **\r\n");
            break;
        case E_SCP_STATUS_TRANSMIT_ERROR :
            log_dbgPrintf("** Transmitting SCP packet error !! **\r\n");
            break;
        default:
            break;
    }
    return E_TYPE_SUCCESS;

__CMD_ERROR:
    
    return ret;
}


INT32 do_poeprogram
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_INVALID_PARA;
    S_SCP_PKT pkt;
    UINT8 *ch;
    E_SCP_RET_CODE scp_ret;
    UINT8 key;
    UINT32 i=0;
    E_POE_MODULE poeModuleType;

    poeModuleType = sys_utilsPoeModuleGet();

    /* check if PoE module is supported */
    if(poeModuleType == E_POE_MODULE_NOT_INSTALLED)
    {
        log_printf("PoE Module is not supported!!\n");
        return E_TYPE_UNSUPPORT;
    }

    if(argc < 2 || argc > 12)
    {
        goto __CMD_ERROR;
    }

    memset(&pkt, 0x4e, sizeof(S_SCP_PKT));
    ch = (UINT8 *)&pkt.subject;

    for (i=1;i < argc;i++)
    {
        *ch++ = simple_strtoul(argv[i], NULL, 16);
    }

    pkt.key = SCP_PROGRAM;
    scp_ret = poe_halSCPDebugCmd(&pkt);
     
    switch(scp_ret)    
    {            
        case E_SCP_ECHO_ERROR:
            log_dbgPrintf("** Received packet echo number error !! **\r\n");
            break;
        case E_SCP_CHECKSUM_ERROR:
            log_dbgPrintf("** Received packet checksum error !! **\r\n");
            break;
        case E_SCP_STATUS_RECEIVE_ERROR :
            log_dbgPrintf("** Receiving SCP packet error !! **\r\n");
            break;
        case E_SCP_STATUS_TRANSMIT_ERROR :
            log_dbgPrintf("** Transmitting SCP packet error !! **\r\n");
            break;
        default:
            break;
    }
    return E_TYPE_SUCCESS;

__CMD_ERROR:
    
    return ret;
}

INT32 do_poeshowpower
(
    IN cmd_tbl_t *cmdtp,
    IN INT32 flag,
    IN INT32 argc,
    IN INT8 *argv[]
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 total_power=0;

    if( argc > 1 )
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
    else
    {
        ret = poe_halGetTotalPower(&total_power);

        log_printf("Total Power Consumption: %dW\n", total_power);
    }
    
    return ret;

__CMD_ERROR:
    
    return ret;
}


U_BOOT_CMD(
    poetest,    CONFIG_SYS_MAXARGS,    1,    do_poetest,
    "poetest \t- test if the PoE system is working correctly\n",
    "<port> <detect [legacy|standard]|classify [0-4]|powerup|disconnect|vopcheck [margin(mv)]>. \r\n"
    "  - port: Specify a range of port numbers.\r\n"
    "  - detect: Specify legacy PD or standard PD to detect.\r\n"
    "  - classify: Specify class of PD for classification test.\r\n"
    "  - powerup: Power on PD and check power consumption.\r\n"
    "  - disconnect: Disconnect PD test.\r\n"
    "  - vopcheck: VOP issue check(default margin 4000mv).\r\n"
);

U_BOOT_CMD(
    poeforcepw,    CONFIG_SYS_MAXARGS,    1,    do_poeforcepw,
    "poeforcepw \t- Force output power\r\n",
    "<port> <on|off>\r\n"
    "  - port: Specify a range of port numbers.\r\n"
    "  - output: Output power or not. Valid values are <on|off>.\r\n"
);

U_BOOT_CMD(
    poemode,    CONFIG_SYS_MAXARGS,    1,    do_poemode,
    "poemode \t- PoE port configuration\n",
    "<port_num | all> <mode> <admin>\n"
    "  - port: Specify a range of port numbers.\r\n"
    "  - mode: Specify AT or AF mode. Valid values are <af|at>.\r\n"
    "  - admin: Enable or disable PoE function. Valid values are <on|off>.\r\n"
);

U_BOOT_CMD(
    poestatus,    CONFIG_SYS_MAXARGS,    1,    do_poestatus,
    "poestatus \t- Show power status\n",
    "<port | all>\n"
    "  - port: Specify a range of port numbers.\r\n"
);

U_BOOT_CMD(
    poeshowpower,    CONFIG_SYS_MAXARGS,    1,    do_poeshowpower,
    "poeshowpower \t- Show total power consumption of system\n",
    "- Show total power consumption of system\n"
);

U_BOOT_CMD(
    poecmd,    CONFIG_SYS_MAXARGS,    1,    do_poecmd,
    "poecmd \t\t- transmit a SCP packet with 0x00 as the KEY\n",
    "<f1> <f2> <f3> <f4> <f5> <f6> <f7> <f8> <f9> <f10> <f11>\n"
    "  - 11 bytes of data after ECHO and before CSum H \n"
    "    in the SCP packet. Valid values are hexdecmial numbers, \n"
    "    and 0x4E, which is N in ASCII code, can be omitted. \n"
    "    For example, poecmd 0x1 0x2 0x4E 0x4E 0x4E 0x4E 0x4E \n"
    "    0x4E 0x4E 0x4E 0x4E can be abbreviated as poecmd 0x1 0x2. \n"
);

U_BOOT_CMD(
    poerequest,    CONFIG_SYS_MAXARGS,    1,    do_poerequest,
    "poerequest \t- transmit a SCP packet with 0x02 as the KEY\n",
    "<f1> <f2> <f3> <f4> <f5> <f6> <f7> <f8> <f9> <f10> <f11>\n"
    "  - 11 bytes of data after ECHO and before CSum H \n"
    "    in the SCP packet. Valid values are hexdecmial numbers, \n"
    "    and 0x4E, which is N in ASCII code, can be omitted. \n"
    "    For example, poerequest 0x1 0x2 0x4E 0x4E 0x4E 0x4E 0x4E \n"
    "    0x4E 0x4E 0x4E 0x4E can be abbreviated as poerequest 0x1 0x2. \n"
);

U_BOOT_CMD(
    poeprogram,    CONFIG_SYS_MAXARGS,    1,    do_poeprogram,
    "poeprogram \t- transmit a SCP packet with 0x01 as the KEY\n",
    "<f1> <f2> <f3> <f4> <f5> <f6> <f7> <f8> <f9> <f10> <f11>\n"
    "  - 11 bytes of data after ECHO and before CSum H \n"
    "    in the SCP packet. Valid values are hexdecmial numbers, \n"
    "    and 0x4E, which is N in ASCII code, can be omitted. \n"
    "    For example, poeprogram 0x1 0x2 0x4E 0x4E 0x4E 0x4E 0x4E \n"
    "    0x4E 0x4E 0x4E 0x4E can be abbreviated as poeprogram 0x1 0x2. \n"
);
