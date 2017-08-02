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
***      sys_utils.c
***
***    DESCRIPTION :
***      APIs for board and module info
***
***    HISTORY :
***       - 2010/07/01, Jungle Chen
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
#include <fcntl.h>
#include <stdarg.h>
#include <sys/mman.h>

#include "i2c_hal.h"
#include "i2c_fpga.h"

#include "cmn_type.h"
#include "porting.h"
#include "sys_utils.h"

#include "err_type.h"
#include "log.h"


UINT8           g_hwRevStr[5][10] = { "PROTO", "ALPHA", "BETA", "PVT", "MP" };
/*==========================================================================
 *
 *      Local Constant and Macro Definition Segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Local Type and Structure Definition Segment
 *
 *==========================================================================
 */
static S_BOARD_INFO g_boardInfo;

/*==========================================================================
 *
 *      Static Variable Segment
 *
 *==========================================================================
 */
static BOOL             g_sysUtilsInit  = FALSE;

/* They will be initialized on sys_utilsInit() */
static E_HW_REV         g_hwRev         = E_HW_REV_MAX;
static UINT32           g_modelId       = 0xFFFFFFFF;
static E_POE_MODULE     g_poeModule     = E_POE_MODULE_NOT_SUPPORTED;
#ifdef FOX_AC3_HAYWARDS  /* Added by Foxconn Alex, 2015/10/02 */
static UINT32           g_poePortNum    = XCAT3_24_PoE_PORT_NUM;
#else
static UINT32           g_poePortNum    = XCAT3_8_PoE_PORT_NUM;
#endif

/* Add switch board Id, Brian Lu 20151005 */
#define SYS_UTILS_AC3_I2C_BUS_0                 0
#define SYS_UTILS_AC3_EE_I2C_ADD               0x50
static UINT32 g_devBoardId = E_BOARD_ID_MAX_BOARID;

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
 *      sys_utilsInit
 *
 *  DESCRIPTION :
 *      init sys info for this board
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
)
{
    UINT8   u8RegData;
    UINT32  device;
    UINT32  offset;
    UINT32  data = 0;
    UINT32  modelId;
    UINT32  ret;

    if ( g_sysUtilsInit == TRUE )
    {
        return E_TYPE_SUCCESS;
    }

    g_boardInfo.boardId=g_devBoardId;

    switch(g_devBoardId)
    {
        case E_BOARD_ID_HAYWARDS_48G4G_T:
        case E_BOARD_ID_HAYWARDS_48G4G_P:
            g_boardInfo.devMaxNum    = 2;
            g_boardInfo.lPortMaxNum  = MAX_LOGIC_PORT_52;
            g_boardInfo.stPortMaxNum = 0;
            g_boardInfo.pPortMaxNum  = MAX_LOGIC_PORT_52;
            g_boardInfo.firstfiberNum = 49;
            g_boardInfo.copperMaxNum = 48;
            if(g_devBoardId == E_BOARD_ID_HAYWARDS_48G4G_P)
            {
                g_poeModule = E_POE_MODULE_MICROSEMI;
                g_poePortNum = 48;
            }
            else
            {
                g_poeModule = E_POE_MODULE_NOT_SUPPORTED;
                g_poePortNum = 0;
            }
            break;
        case E_BOARD_ID_HAYWARDS_24G4G_T:
        case E_BOARD_ID_HAYWARDS_24G4G_P:
            g_boardInfo.devMaxNum    = 1;
            g_boardInfo.lPortMaxNum  = MAX_LOGIC_PORT_28;
            g_boardInfo.stPortMaxNum = 0;
            g_boardInfo.pPortMaxNum  = MAX_LOGIC_PORT_28;
            g_boardInfo.firstfiberNum = 25;
            g_boardInfo.copperMaxNum = 24;
            if(g_devBoardId == E_BOARD_ID_HAYWARDS_24G4G_P)
            {
                g_poeModule = E_POE_MODULE_MICROSEMI;
                g_poePortNum = 24;
            }
            else
            {
                g_poeModule = E_POE_MODULE_NOT_SUPPORTED;
                g_poePortNum = 0;
            }
            break;
        case E_BOARD_ID_HAYWARDS_16G2G_T:
        case E_BOARD_ID_HAYWARDS_16G2G_P:
            g_boardInfo.devMaxNum    = 1;
            g_boardInfo.lPortMaxNum  = MAX_LOGIC_PORT_18;
            g_boardInfo.stPortMaxNum = 0;
            g_boardInfo.pPortMaxNum  = MAX_LOGIC_PORT_18;
            g_boardInfo.firstfiberNum = 17;
            g_boardInfo.copperMaxNum = 16;
            if(g_devBoardId == E_BOARD_ID_HAYWARDS_16G2G_P)
            {
                g_poeModule = E_POE_MODULE_MICROSEMI;
                g_poePortNum = 16;
            }
            else
            {
                g_poeModule = E_POE_MODULE_NOT_SUPPORTED;
                g_poePortNum = 0;
            }
            break;
        case E_BOARD_ID_HAYWARDS_8G2G_T:
        case E_BOARD_ID_HAYWARDS_8G2G_P:
            g_boardInfo.devMaxNum    = 1;
            g_boardInfo.lPortMaxNum  = MAX_LOGIC_PORT_10;
            g_boardInfo.stPortMaxNum = 0;
            g_boardInfo.pPortMaxNum  = MAX_LOGIC_PORT_10;
            g_boardInfo.firstfiberNum = 9;
            g_boardInfo.copperMaxNum = 8;
            if(g_devBoardId == E_BOARD_ID_HAYWARDS_8G2G_P)
            {
                g_poeModule = E_POE_MODULE_MICROSEMI;
                g_poePortNum = 8;
            }
            else
            {
                g_poeModule = E_POE_MODULE_NOT_SUPPORTED;
                g_poePortNum = 0;
            }
            break;
        default:
            g_boardInfo.devMaxNum    = 1;
            g_boardInfo.lPortMaxNum  = MAX_LOGIC_PORT_28;
            g_boardInfo.stPortMaxNum = 0;
            g_boardInfo.pPortMaxNum  = MAX_LOGIC_PORT_28;
            g_boardInfo.firstfiberNum = 25;
            g_boardInfo.copperMaxNum = 24;
            g_poeModule = E_POE_MODULE_NOT_SUPPORTED;
            log_printf("Unknown device!\n");
            break;
    }

#if 0
    /* read Chip Revision */
    log_printf("HW Rev. - %s build\n", g_hwRevStr[sys_utilsHWRevGet()]);
#endif
    g_sysUtilsInit = TRUE;

    return E_TYPE_SUCCESS;
}

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
)
{
    return g_poeModule;
}

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
)
{
    return g_poePortNum;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sys_utilsDevBoardIdSet
 *
 *  DESCRIPTION :
 *      set device board Id
 *
 *  INPUT :
 *      UINT32 sys_devBoardId
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
)
{
    /* ac3 switch will call back to global board Id, Brian Lu 20151006 */
    switch(sys_devBoardId)
    {
        case 0x0:
            g_devBoardId = E_BOARD_ID_HAYWARDS_8G2G_T;
            break;
        case 0x1:
            g_devBoardId = E_BOARD_ID_HAYWARDS_8G2G_P;
            break;
        case 0x2:
            g_devBoardId = E_BOARD_ID_HAYWARDS_16G2G_T;
            break;
        case 0x3:
            g_devBoardId = E_BOARD_ID_HAYWARDS_16G2G_P;
            break;
        case 0x4:
            g_devBoardId = E_BOARD_ID_HAYWARDS_24G4G_T;
            break;
        case 0x5:
            g_devBoardId = E_BOARD_ID_HAYWARDS_24G4G_P;
            break;
        case 0x6:
            g_devBoardId = E_BOARD_ID_HAYWARDS_48G4G_T;
            break;
        case 0x7:
            g_devBoardId = E_BOARD_ID_HAYWARDS_48G4G_P;
            break;
        default:
            g_devBoardId = E_BOARD_ID_MAX_BOARID;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    /* cpss api will set global board Id on EV board, Brian Lu 20151006 */
    return g_devBoardId;
};

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
)
{
    INT32     ret   = E_TYPE_SUCCESS;
    UINT32   regVal = 0;
    UINT32   chipId = 0;
    
    /* 20170626 -- This MAC AC3 98DX3236 chip is only for Haywards2 */
    ret = cpssDrvPpHwRegBitMaskRead(SYS_UTILS_AC3_DEVICE, 0, SYS_UTILS_READ_MAC_ID_REG, 
    0xffffffff, &regVal);

   if (ret != E_TYPE_SUCCESS){
        log_printf("sys_utilsDevMACChipIdGet: failed MAC chip id register 0x%x \n", SYS_UTILS_READ_MAC_ID_REG); 
        return 0; /* return chip is none */ 
   } 

   /* read cpss reserved register 0x4c for device id */
   chipId = (regVal & 0x0ffff0) >> 4;
   log_dbgPrintf("sys_utilsDevMACChipIdGet: Reading system MAC id is 0x%4X\n", chipId); 

   return chipId;
};


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
)
{
    INT32     ret = E_TYPE_SUCCESS;
    UINT16   regVal = 0;

    if ( g_hwRev == E_HW_REV_MAX ) /* initial value */
    {
        /* Read board id value */
        ret = i2c_halRegGet(FPGA_I2C_ADDR, FPGA_BOARD_ID_REG, FPGA_REG_LEN, (UINT8 *)&regVal, 1);

        if ( ret != E_TYPE_SUCCESS )
        {
            log_printf("Fail to read the default value.\n");
            return ret;
        }
        g_hwRev = (regVal&0x18) >> 3; /* HW_REV 4:3 */
    }

    return g_hwRev;
}

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
)
{
    if (boardInfo == NULL)
    {
        return E_TYPE_INVALID_ADDR;
    }

    memcpy(boardInfo, &g_boardInfo, sizeof(S_BOARD_INFO));
    return E_TYPE_SUCCESS;
}

