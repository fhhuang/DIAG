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
***      port_utils.c
***
***    DESCRIPTION :
***      port info of board
***
***    HISTORY :
***       - 2009/12/15, 16:30:52, Eden Weng
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *                                                                          
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */
/* System Library */

/* User-defined Library */
#include "cmn_type.h"
#include "port_utils.h"
#include "porting.h"
#include "sys_utils.h"
#include "gpio_hal.h"
#include "err_type.h"
#include "port_cfg.h"
#include "log.h"

/*==========================================================================
 *
 *      Local Constant and Macro Definition Segment
 *
 *==========================================================================
 */
#define DBG_PRINTF log_dbgPrintf

/*==========================================================================
 *
 *      Local Type and Structure Definition Segment
 *
 *==========================================================================
 */


/*==========================================================================
 *
 *      Static Variable Segment
 *
 *==========================================================================
 */
static BOOL g_portUtilsInit = FALSE;

static S_PORT_INFO                 *g_portInfo;
static UINT32                      g_maxPoeNum=0;
/*==========================================================================
 *                                                                          
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */


/*==========================================================================
 *
 *      Local Function Segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsInit
 *
 *  DESCRIPTION :
 *      init port info for this board
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
int port_utilsInit
(
    void
)
{
    int                             i, count, lp = 0;
    S_BOARD_INFO                    boardInfo;
    UINT32 hwRev;
    UINT32 readDevMACId = 0;
    
    if( g_portUtilsInit == TRUE )
    {
        return E_TYPE_SUCCESS;
    }

    memset(&boardInfo, 0x0, sizeof(boardInfo));

    /* Get board info before all */
    if (sys_utilsBoardInfoGet(&boardInfo) != E_TYPE_SUCCESS)
    {
        log_printf("Get board info fail!\n");
        return E_TYPE_INIT_FAIL;
    }

    hwRev = sys_utilsHWRevGet();

    /*20170626 - Try to read device MAC chip Id information */
    readDevMACId  = sys_utilsDevMACChipIdGet();

    switch (boardInfo.boardId)
    {
        case E_BOARD_ID_HAYWARDS_8G2G_T:
            g_portInfo = (S_PORT_INFO*)&g_Haywards_8_PortInfo[0];
            g_maxPoeNum=0;
            break;

        case E_BOARD_ID_HAYWARDS_8G2G_P:
            g_portInfo = (S_PORT_INFO*)&g_Haywards_8P_PortInfo[0];
            g_maxPoeNum=8;
            break;

        case E_BOARD_ID_HAYWARDS_16G2G_T:
            g_portInfo = (S_PORT_INFO*)&g_Haywards_16_PortInfo[0];
            g_maxPoeNum=0;
            break;

        case E_BOARD_ID_HAYWARDS_16G2G_P:
            g_portInfo = (S_PORT_INFO*)&g_Haywards_16P_PortInfo[0];
            g_maxPoeNum=16;
            break;

        case E_BOARD_ID_HAYWARDS_24G4G_T:
            /* Add Haywards2 MAC chip is 98DX3236 */
            if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID){
                g_portInfo = (S_PORT_INFO*)&g_Haywards2_24_PortInfo[0];
            } else {
                g_portInfo = (S_PORT_INFO*)&g_Haywards_24_PortInfo[0];
            }  
  
            g_maxPoeNum=0;
            break;

        case E_BOARD_ID_HAYWARDS_24G4G_P:
            /* Add Haywards2 MAC chip is 98DX3236 */
            if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
                g_portInfo = (S_PORT_INFO*)&g_Haywards2_24P_PortInfo[0];
            } else {
                g_portInfo = (S_PORT_INFO*)&g_Haywards_24P_PortInfo[0];
            }

            g_maxPoeNum=24;
            break;

        case E_BOARD_ID_HAYWARDS_48G4G_T:
            /* Add Haywards2 MAC chip is 98DX3236 */
            if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
                g_portInfo = (S_PORT_INFO*)&g_Haywards2_48_PortInfo[0];
            } else {
                g_portInfo = (S_PORT_INFO*)&g_Haywards_48_PortInfo[0];
            }

            g_maxPoeNum=0;
            break;

        case E_BOARD_ID_HAYWARDS_48G4G_P:
            /* Add Haywards2 MAC chip is 98DX3236 */
            if (readDevMACId == SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID) {
                g_portInfo = (S_PORT_INFO*)&g_Haywards2_48P_PortInfo[0];
            } else {
                g_portInfo = (S_PORT_INFO*)&g_Haywards_48P_PortInfo[0];
            }

            g_maxPoeNum=48;
            break;

        default: /* switch (boardInfo.boardId) */ 
            log_printf("Unsupported Board ID detected(%X)!\n", boardInfo.boardId);
            return E_TYPE_UNSUPPORT_DEV;
    } /* end of switch (boardInfo.boardId)*/

    g_portUtilsInit = TRUE;
    
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsLPortInfoGet
 *
 *  DESCRIPTION :
 *      information of port
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      devId - device id
 *        groupId - pex id of device
 *      portId - port id of pex        
 *        portType - media type of port
 *        portSpeed - speed of port
 *      smiAddr - address of smi
 *      xsmiAddr - address of xsmi
 *      poe - poe or non-poe
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
S_PORT_INFO * port_utilsLPortInfoGet
(
    IN  UINT32                      lPort
)
{
    S_BOARD_INFO                    boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if (lPort <= boardInfo.lPortMaxNum)
    {
        /* lport is the index of g_portInfo */
        return (S_PORT_INFO *)&g_portInfo[lPort-1];
    }
    return NULL;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsPPortInfoGet
 *
 *  DESCRIPTION :
 *      information of port
 *
 *  INPUT :
 *      unit - unit id
 *      pport - physical port number
 *
 *  OUTPUT :
 *      portInfo - port information
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
INT32 port_utilsPPortInfoGet
(
    IN  UINT32                      devId,
    IN  UINT32                      portId,
    OUT S_PORT_INFO                 *portInfo
)
{
    int                             i;
    BOOL                            found = FALSE;
    
    if (portInfo == NULL)
        return E_TYPE_INVALID_ADDR;

    for (i = 1 ; i < port_utilsTotalFixedPortGet() ; i++)
    {
        if ((g_portInfo[i].devId == devId) && (g_portInfo[i].portId== portId))
        {
            memcpy(portInfo, &g_portInfo[i], sizeof(S_PORT_INFO));
            found = TRUE;    
            break;
        }        
    }

    if (!found)
        return E_TYPE_DATA_GET;
    
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsTotalDevGet
 *
 *  DESCRIPTION :
 *      total device number
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      > 0  - device number
 *      == 0 - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT UINT32 port_utilsTotalDevGet
(
    void
)
{
    S_BOARD_INFO                    boardInfo;
    sys_utilsBoardInfoGet(&boardInfo);
    
    return boardInfo.devMaxNum;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsTotalPoePortGet
 *
 *  DESCRIPTION :
 *      To get the total PoE ports number
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      > 0  - pex number of special device
 *      == 0 - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT INT32 port_utilsTotalPoePortGet
(
    void
)
{
    return g_maxPoeNum;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsTotalPortByTypeGet
 *
 *  DESCRIPTION :
 *      total port number
 *
 *  INPUT :
 *      portType - port type
 *
 *  OUTPUT :
 *        none
 *
 *  RETURN :
 *      > 0  - port number
 *      == 0 - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT UINT32 port_utilsTotalPortByTypeGet
(
    E_LINER_PORT_TYPE portType
)
{
    switch (portType)
    {
        case E_LINER_PORT_TYPE_FIXED:
            return port_utilsTotalFixedPortGet();
        default:
            return E_TYPE_UNSUPPORT;
    }
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsTotalFixedPortGet
 *
 *  DESCRIPTION :
 *      total fixed port number
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *        none
 *
 *  RETURN :
 *      > 0  - port number
 *      == 0 - fail to action
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT UINT32 port_utilsTotalFixedPortGet
(
    void
)
{
    S_BOARD_INFO boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    return boardInfo.lPortMaxNum;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsLportToMacPort
 *
 *  DESCRIPTION :
 *      convert logical port to mac port number
 *
 *  INPUT :
 *      lPort - logiclal port number
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      mac port number
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
UINT32 port_utilsLportToMacPort
(
    IN UINT32                       lPort
)
{
    S_PORT_INFO                     *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    
    return portInfo->portId;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsPortType2LPortGet
 *
 *  DESCRIPTION :
 *      port type/id to liner port
 *
 *  INPUT :
 *      portType - port type
 *      portId - port number of special port type
 *
 *  OUTPUT :
 *      lPort - liner port number
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
INT32 port_utilsPortType2LPortGet
(
    E_LINER_PORT_TYPE portType,
    UINT32 portId,
    UINT32 *lPort
)
{
    switch (portType)
    {
        case E_LINER_PORT_TYPE_FIXED:
            *lPort = FIXED_PORT_TO_LINER_PORT(portId);
            break;
        default:
            return E_TYPE_OUT_RANGE;
    }

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsLPort2PortTypeGet
 *
 *  DESCRIPTION :
 *      liner port to port type/id
 *
 *  INPUT :
 *      lPort - liner port number
 *
 *  OUTPUT :
 *      portType - port type
 *      portId - port number of special port type
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
INT32 port_utilsLPort2PortTypeGet
(
    UINT32                          lport,
    E_LINER_PORT_TYPE               *portType,
    UINT32                          *portId
)
{
    S_PORT_INFO                     *portInfo;

    portInfo=port_utilsLPortInfoGet(lport);

    if(portInfo==NULL)
    {
        return E_TYPE_DATA_GET;
    }

    switch (portInfo->portType)
    {
        case E_PORT_TYPE_STACKING:
            *portType = E_LINER_PORT_TYPE_STACKING;
            break;
        case E_PORT_TYPE_INTER_LINK:
            *portType = E_LINER_PORT_TYPE_INTER_LINK;
            break;
        default:
            *portType = E_LINER_PORT_TYPE_FIXED;
            break;
    }

    *portId = portInfo->portId;

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsLPortGetFromPhy
 *
 *  DESCRIPTION :
 *      Logical port map from PHY
 *
 *  INPUT :
 *      device_id
 *
 *  OUTPUT :
 *      lPort - port index
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
INT32 port_utilsLPortGetFromPhy
(
    IN  int device_id,
    IN  int phyport,
    OUT int *lport
)
{
    S_PORT_INFO *tmportInfo;
    int i;
    int port=0;
    tmportInfo=g_portInfo;

    for(i=0;i<port_utilsTotalFixedPortGet();i++,tmportInfo++)
    {
        if( (tmportInfo->devId==device_id) &&(tmportInfo->portId==phyport)){
            *lport=i;
            return 0;
        }
    }
    *lport=port;
    return -1;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsLportToPoeChNum
 *
 *  DESCRIPTION :
 *      Logical port map from PoE
 *
 *  INPUT :
 *      lPort - port index
 *
 *  OUTPUT :
 *      PoE port index
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
UINT32 port_utilsLportToPoeChNum
(
    IN UINT32 lPort
)
{
    E_LINER_PORT_TYPE portType;
    S_PORT_INFO *portInfo;
    UINT32 portId=0;

    port_utilsLPort2PortTypeGet(lPort, &portType, &portId);

    switch (portType)
    {
        case E_LINER_PORT_TYPE_FIXED:
            portInfo=port_utilsLPortInfoGet(lPort);
            if( portInfo == NULL )
            {
                log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
                return E_TYPE_DATA_GET;
            }
            return portInfo->poePortId;
        case E_LINER_PORT_TYPE_STACKING:
        case E_LINER_PORT_TYPE_EXPANSION:
        case E_LINER_PORT_TYPE_INTER_LINK:
        default:
            return 0;
    }
}


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsSFPPortGet
 *
 *  DESCRIPTION :
 *      Get SFP port ID
 *
 *  INPUT :
 *      lPort
 *
 *  OUTPUT :
 *      sfpid
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
UINT32 port_utilsSFPPortGet
(
    IN UINT32 lPort,
    OUT UINT32 *sfpid
)
{
    S_PORT_INFO *portInfo;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    if(portInfo->sfpPortId == -1)
    {
        *sfpid=0;
        return E_TYPE_DATA_GET;
    }
    *sfpid=portInfo->sfpPortId;
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      port_utilsParaPass
 *
 *  DESCRIPTION :
 *      pass user input parameter
 *
 *  INPUT :
 *      paraCnt - start parameter
 *      argv - parameter
 *      twoPortPara - has two port number parameter
 *
 *  OUTPUT :
 *      startPort - start port on special port range
 *      endPort - end port on special port range
 *      lPortType - port type
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
INT32 port_utilsParaPass
(
    INOUT UINT32 *paraCnt,
    IN INT8 *argv[],
    IN BOOL twoPortPara,
    OUT UINT32 *startPort,
    OUT UINT32 *endPort,
    OUT E_LINER_PORT_TYPE *lPortType,
    OUT UINT32 *lPortBase,
    OUT INT8 *lPortTypeStr
)
{
    UINT8   *ptr;
    S_BOARD_INFO boardinfo;

    sys_utilsBoardInfoGet(&boardinfo);

    if( paraCnt ) ;

    *endPort = 0;

    *startPort = 0; /* all */
    if( strcmp(argv[*paraCnt], "all") == 0 )
    {
        *endPort = port_utilsTotalFixedPortGet();
        strcpy(lPortTypeStr, "Port");
    }
    else if( strcmp(argv[*paraCnt], "copper") == 0 )
    {
        *startPort = 1;
        *endPort = boardinfo.copperMaxNum;
        strcpy(lPortTypeStr, "Copper Port");
    }
    else if( strcmp(argv[*paraCnt], "fiber") == 0 )
    {
        *startPort = boardinfo.firstfiberNum;
        *endPort = boardinfo.lPortMaxNum;
        strcpy(lPortTypeStr, "Fiber Port");
    }
    else
    {
        if( strcmp(argv[*paraCnt], "stk") == 0 )
        {
            *lPortType = E_LINER_PORT_TYPE_STACKING;
            strcpy(lPortTypeStr, "Stacking Port");
        }
        else
        {
            *lPortType = E_LINER_PORT_TYPE_FIXED;
            strcpy(lPortTypeStr, "Port");
        }

        switch (*lPortType)
        {
            case E_LINER_PORT_TYPE_FIXED:
                *startPort = simple_strtoul(argv[*paraCnt], NULL, 10);

                if( (*startPort<1) || (*startPort>port_utilsTotalFixedPortGet()) )
                {
                    log_printf("Port number out of range\n");
                    return E_TYPE_INVALID_PARA;
                }

                ptr=argv[*paraCnt];
                while(*ptr != '\0'){
                    if(*ptr == '-'){
                        *ptr ='\0';
                        *startPort = simple_strtoul(argv[*paraCnt], NULL, 10);
                        ptr++;
                        *endPort=simple_strtoul(ptr, NULL, 10);
                        break;
                    }
                    ptr++;
                }

                if( twoPortPara == TRUE )
                {
                    (*paraCnt)++;
                    *endPort = simple_strtoul(argv[*paraCnt], NULL, 10);

                    /* no end port parameter */
                    if( (*endPort<1) || (*endPort>port_utilsTotalFixedPortGet()) )
                    {
                        *endPort = *startPort;
                    }
                }
                else
                {
                    //*endPort = *startPort;
                    if( (*endPort > boardinfo.lPortMaxNum) || (*endPort==0))
                    *endPort = *startPort;
                    (*startPort > *endPort)?*endPort:*startPort;
                }
                break;
            default:
                return E_TYPE_INVALID_PARA;
        }
    }

    if( *endPort == 0 )
    {
        log_printf("Port number out of range\n");
        return E_TYPE_UNSUPPORT_DEV;
    }

    port_utilsPortType2LPortGet(*lPortType, *startPort, lPortBase);

    (*paraCnt)++;

    return E_TYPE_SUCCESS;
}
