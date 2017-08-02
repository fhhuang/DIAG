/***************************************************************************
***
***     Copyright 2009  Foxconn
***     All Rights Reserved
***     No portions of this material may be reproduced in any form
***     without the written permission of:
***
***                 Foxconn CNSBG
***
***     All information contained in this document is Foxconn CNSBG
***     company private, proprietary, and trade secret.
***
****************************************************************************
***
***    FILE NAME :
***            port_utils.h
***
***    DESCRIPTION :
***            port info of board
***
***    HISTORY :
***       - 2009/12/15, 16:30:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __PORT_UTILS_H_
#define __PORT_UTILS_H_

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
/* System Library */

/* User-defined Library */
#include "port_defs.h"
#include "cmn_type.h"
#include "porting.h"
#include "err_type.h"
#include "log.h"


/*==========================================================================
 *
 *      Constant Definition Segment
 *
 *==========================================================================
 */
#define MAX_LOGIC_PORT_10   10
#define MAX_LOGIC_PORT_18  18
#define MAX_LOGIC_PORT_28  28
#define MAX_LOGIC_PORT_52  52

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
);

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
    IN  UINT32                      lport
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
UINT32 port_utilsLportToPoeChNum
(
    IN UINT32 lPort
);

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
);

#ifdef __cplusplus
}
#endif

#endif /* __PORT_UTILS_H_ */
