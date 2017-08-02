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
***            sys_utils.h
***
***    DESCRIPTION :
***      APIs for board and module info
***
***    HISTORY :
***       - 2010/07/01, Jungle Chen
***             File Creation
***
***************************************************************************/

#ifndef __SYS_UTILS_H_
#define __SYS_UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "err_type.h"
#include "port_utils.h"
/*==========================================================================
 *
 *      Library Inclusion Segment
 *                                                                          
 *==========================================================================
 */

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

/* 20170626 -- Haywards2 MAC chip id is 98DX3236 */
#define SYS_UTILS_AC3_DEVICE                0
#define SYS_UTILS_XCAT_AC3_98DX3236_DEV_ID  0xF410
#define SYS_UTILS_READ_MAC_ID_REG           0x4C

#define XCAT3_8_PoE_PORT_NUM     	8
#define XCAT3_8_PoE_MAX_DEV      	1

#ifdef FOX_AC3_HAYWARDS /* Added by Foxconn Alex, 2015/10/02 */
#define XCAT3_24_PoE_PORT_NUM      24
#define XCAT3_24_PoE_MAX_DEV     	1
#endif

typedef enum {
    E_BUILD_REV_0,                      /* Build Revision 0 */
    E_BUILD_REV_1                       /* Build Revision 1 */
} E_BUILD_REV;

typedef enum {
    E_FRU_PRESENT_PSU1  = 0x0001,
    E_FRU_PRESENT_PSU2  = 0x0002,
    E_FRU_PRESENT_FAN1  = 0x0004,
    E_FRU_PRESENT_FAN2  = 0x0008,
    E_FRU_PRESENT_FAN3  = 0x0010
} E_FRU_PRESENT;

typedef enum {
    E_BOARD_ID_HAYWARDS_8G2G_T,
    E_BOARD_ID_HAYWARDS_8G2G_P,
    E_BOARD_ID_HAYWARDS_16G2G_T,
    E_BOARD_ID_HAYWARDS_16G2G_P,
    E_BOARD_ID_HAYWARDS_24G4G_T,
    E_BOARD_ID_HAYWARDS_24G4G_P,
    E_BOARD_ID_HAYWARDS_48G4G_T,
    E_BOARD_ID_HAYWARDS_48G4G_P,
    E_BOARD_ID_MAX_BOARID
} E_BOARD_ID;

typedef enum {
    E_EXP_MODULE_NOT_INSTALLED,
    E_EXP_MODULE_MAX
} E_EXP_MODULE;

typedef enum {
    E_HW_REV_PROTO,
    E_HW_REV_ALPHA,
    E_HW_REV_BETA,
    E_HW_REV_PVT,
    E_HW_REV_MP,
    E_HW_REV_MAX,
} E_HW_REV;

typedef enum {
    E_POE_MODULE_MICROSEMI,
    E_POE_MODULE_V2,
    E_POE_MODULE_V3,
    E_POE_MODULE_NOT_INSTALLED,    
    E_POE_MODULE_NOT_SUPPORTED,    
    E_POE_MODULE_MAX
} E_POE_MODULE;


typedef struct {
    UINT32              boardId;        /* Board ID */
    E_HW_REV            hwRev;          /* HW Revision */
    E_BUILD_REV         buildRev;       /* Build Revision */
    UINT32              devMaxNum;      /* MAC Unit Amount */
    E_FRU_PRESENT       fruPresent;     /* Fan/PSU Present Status*/
    UINT32              lPortMaxNum;    /* Logical Port Amount */
    UINT32              stPortMaxNum;   /* Stacking Port Amount */
    UINT32              pPortMaxNum;    /* Physical Port Amount */
    UINT32              firstfiberNum;
    UINT32              copperMaxNum;
} S_BOARD_INFO;

/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsInit
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
INT32 sys_utilsInit
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsPoeModuleGet
 *
 *  DESCRIPTION :
 *      get poe module status
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      g_poeModule - E_POE_MODULE
 *      
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT E_POE_MODULE sys_utilsPoeModuleGet
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsHWRevGet
 *
 *  DESCRIPTION :
 *      to get hw revision
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      g_hwRev - E_HW_REV
 *      
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT E_HW_REV sys_utilsHWRevGet
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsDevMACChipIdGet
 *
 *  DESCRIPTION :
 *      get device MAC chip board Id 
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      UINT32 - Device MAC chip id
 *      
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT UINT32 sys_utilsDevMACChipIdGet
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsDevBoardIdGet
 *
 *  DESCRIPTION :
 *      get device board Id 
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      g_devBoardId - E_BOARD_ID
 *      
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT E_BOARD_ID sys_utilsDevBoardIdGet
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsDevBoardIdSet
 *
 *  DESCRIPTION :
 *      set device board Id 
 *
 *  INPUT :
 *      sys_devBoardId -E_BOARD_ID
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      none
 *      
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 sys_utilsDevBoardIdSet
(
    UINT32 sys_devBoardId
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsPoePortNumGet
 *
 *  DESCRIPTION :
 *      get poe port number
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      g_poePortNum
 *      
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
OUT UINT32 sys_utilsPoePortNumGet
(
    void
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsBoardInfoGet
 *
 *  DESCRIPTION :
 *      Get the information relevant to this board
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      boardInfo
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
OUT E_ERROR_TYPE sys_utilsBoardInfoGet
(
    OUT S_BOARD_INFO                *boardInfo
);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_UTILS_H_ */
