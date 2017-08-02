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
***      cmd_tune.c
***
***    DESCRIPTION :
***      for switch port
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
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
#include "switch_tune.h"
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

INT32 do_txtune
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0, laneNum = 0;
    UINT32                  txAmpAdjEn = 0, txAmpShft = 0, txAmp = 0, emph0 = 0, emph1 = 0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    S_BOARD_INFO                    boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("txtune", argv[0]) == 0 )
    {
        if ( argc != 7 )
        {
            ERR_PRINT_CMD_USAGE("txtune");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;
        laneNum = simple_strtoul(argv[paraCnt], NULL, 10);

        /* 02162017 - 8/16 (SFP) 24/48 SFP/SFP+ ports serdes lane */
        switch(boardId){
            case E_BOARD_ID_HAYWARDS_48G4G_T:
            case E_BOARD_ID_HAYWARDS_48G4G_P:
                 /* sfp/sfp+ ports serdes ports 0/25, 0/27, 1/25, 1/27
                   However, check laneNum 0-11 is GE ports,
                   SFP ports laneNum 0/25=12, 0/27=13, 1/25=14, 1/27=15 */
                 if(laneNum> SERDES_LANE_15) {
                  ret = E_TYPE_INVALID_PARA;
                 }
                 break;
            case E_BOARD_ID_HAYWARDS_24G4G_T:
            case E_BOARD_ID_HAYWARDS_24G4G_P:
                 /* sfp/sfp+ ports serdes ports 0/24-0/27, lane 6-9*/
                 if(laneNum> SERDES_LANE_9) {
                  ret = E_TYPE_INVALID_PARA;
                 }
                 break;
            case E_BOARD_ID_HAYWARDS_8G2G_T:
            case E_BOARD_ID_HAYWARDS_8G2G_P:
            case E_BOARD_ID_HAYWARDS_16G2G_T:
            case E_BOARD_ID_HAYWARDS_16G2G_P:
                 /* SFP port serdes ports 0/24=lane 6, 0/26=lane 8 */
                 if ((laneNum != SERDES_LANE_6) && (laneNum != SERDES_LANE_8))
                 {
                    if((laneNum+1)*4 > boardInfo.copperMaxNum){
                        ret = E_TYPE_INVALID_PARA;
                    }
                 }
                 break;
            default:
                if((laneNum+1)*4 > boardInfo.copperMaxNum) {
                  ret = E_TYPE_INVALID_PARA;
                 }
                break;
        }

        /* check laneNum parameter invalid */
        if (ret == E_TYPE_INVALID_PARA)
            goto __CMD_ERROR;

        paraCnt++;
        txAmpAdjEn = simple_strtoul(argv[paraCnt], NULL, 10);

        paraCnt++;
        txAmpShft = simple_strtoul(argv[paraCnt], NULL, 10);

        paraCnt++;
        txAmp = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (txAmp<TUNE_PARA_MIN) || (txAmp>TX_AMP_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        emph0 = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (emph0<TUNE_PARA_MIN) || (emph0>TX_EMPH0_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        emph1 = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (emph1<TUNE_PARA_MIN) || (emph1>TX_EMPH1_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        ret = switch_tuneTxManualSet(laneNum, txAmpAdjEn, txAmpShft, txAmp, emph0, emph1);
        if (ret != E_TYPE_SUCCESS )
        {
            ret = E_TYPE_DATA_SET;
            goto __CMD_ERROR;
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_tuneparaGet
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0, maxLanePhy = 0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0, laneNum = 0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  txPara;
    UINT16                  rxPara = 0;
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("tunepara", argv[0]) == 0 )
    {
        if ( argc < 2 )
        {
            ERR_PRINT_CMD_USAGE("tunepara");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;

        if ( strcmp("all", argv[paraCnt]) == 0 )
        {
            endPort = ((boardInfo.copperMaxNum/4)-1);

            /* 02162017 - 8/16 (SFP) 24/48 SFP/SFP+ ports serdes lane  */
            switch(boardId){
                case E_BOARD_ID_HAYWARDS_48G4G_T:
                case E_BOARD_ID_HAYWARDS_48G4G_P:
                     endPort = SERDES_LANE_15;
                     break;
                case E_BOARD_ID_HAYWARDS_24G4G_T:
                case E_BOARD_ID_HAYWARDS_24G4G_P:
                     endPort = SERDES_LANE_9;
                     break;
                case E_BOARD_ID_HAYWARDS_8G2G_T:
                case E_BOARD_ID_HAYWARDS_8G2G_P:
                case E_BOARD_ID_HAYWARDS_16G2G_T:
                case E_BOARD_ID_HAYWARDS_16G2G_P:
                     endPort = SERDES_LANE_8;
                     break;
                default:
                    laneNum =  port = endPort; /* defaul endPort */
                    break;
            }
        }
        else
        {
            laneNum = endPort = port = simple_strtoul(argv[paraCnt], NULL, 10);
        }

        /* 02162017 - 8/16 (SFP) 24/48 SFP/SFP+ ports serdes lane */
        switch(boardId){
            case E_BOARD_ID_HAYWARDS_48G4G_T:
            case E_BOARD_ID_HAYWARDS_48G4G_P:
                 /* sfp/sfp+ ports serdes ports 0/25, 0/27, 1/25, 1/27
                   However, check laneNum 0-11 is GE ports,
                   SFP ports laneNum 0/25=12, 0/27=13, 1/25=14, 1/27=15 */
                 maxLanePhy = SERDES_LANE_12;
                 if(laneNum> SERDES_LANE_15) {
                  ret = E_TYPE_INVALID_PARA;
                 }
                 break;
            case E_BOARD_ID_HAYWARDS_24G4G_T:
            case E_BOARD_ID_HAYWARDS_24G4G_P:
                 /* sfp/sfp+ ports serdes ports 0/24-0/27, lane 6-9*/
                 maxLanePhy = SERDES_LANE_6;
                 if(laneNum> SERDES_LANE_9) {
                  ret = E_TYPE_INVALID_PARA;
                 }
                 break;
            case E_BOARD_ID_HAYWARDS_8G2G_T:
            case E_BOARD_ID_HAYWARDS_8G2G_P:
            case E_BOARD_ID_HAYWARDS_16G2G_T:
            case E_BOARD_ID_HAYWARDS_16G2G_P:
                 maxLanePhy = SERDES_LANE_6;
                 /* SFP port serdes ports 0/24=lane 6, 0/26=lane 8 */
                 if ((laneNum != SERDES_LANE_6) && (laneNum != SERDES_LANE_8))
                 {
                    if((laneNum+1)*4 > boardInfo.copperMaxNum){
                        ret = E_TYPE_INVALID_PARA;
                    }
                 }
                 break;
            default:
                if((laneNum+1)*4 > boardInfo.copperMaxNum) {
                  ret = E_TYPE_INVALID_PARA;
                 }
                break;
        }

        /* check laneNum parameter invalid */
        if (ret == E_TYPE_INVALID_PARA)
            goto __CMD_ERROR;

        log_printf("Lane txAmpAdj txAmpShft txAmp txEmp0 txEmp1 rxAmpAdj rxEmpAdj rxEmp2X rxEmp rxAmp\n");
        log_printf("---- -------- --------- ----- ------ ------ -------- -------- ------- ----- -----\n");

        memset(&txPara, 0, sizeof(CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC ));
        for ( ; port <=endPort; port ++ )
        {
            /* 03152017 - 8/16 (SFP) */
            if ((boardId == E_BOARD_ID_HAYWARDS_8G2G_T) ||
                 (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) ||
                 (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) ||
                 (boardId == E_BOARD_ID_HAYWARDS_16G2G_P))
            {
                /* check max lane num if present */
                 if( ((port+1)*4 > boardInfo.copperMaxNum) &&
                      ((port != SERDES_LANE_6) && (port != SERDES_LANE_8)))
                  continue;
            }

            ret = switch_tuneResultGet(port, &txPara, &rxPara);
            if (ret != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_DATA_GET;
                goto __CMD_ERROR;
            }

            if (port >= maxLanePhy)
            {
                /*02182017, show parameters to hex value */
                log_printf(" %-4d  %-7s  %-8s 0x%-3X  0x%-3X  0x%-3X  NONE     NONE    NONE   NONE  NONE\n", port, \
                    txPara.txAmpAdjEn? "TRUE":"FALSE",
                    txPara.txAmpShft? "TRUE":"FALSE",
                    txPara.txAmp,
                    txPara.emph0,
                    txPara.emph1);
            }
            else
            {
                /*02182017, show parameters to hex value */
                log_printf(" %-4d  %-7s  %-8s 0x%-3X  0x%-3X  0x%-3X  %-7s  %-7s  X %-4d 0x%-3X 0x%X\n", port, \
                    txPara.txAmpAdjEn? "TRUE":"FALSE",
                    txPara.txAmpShft? "TRUE":"FALSE",
                    txPara.txAmp,
                    txPara.emph0,
                    txPara.emph1,
                    (rxPara>>6 & 0x1)? "TRUE":"FALSE",
                    (rxPara>>11 & 0x1)? "TRUE":"FALSE",
                    (rxPara>>10 & 0x1)+1,
                    (rxPara>>7 & 0x7),
                    (rxPara>>1 & 0x1F));
            }
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_rxtune
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
    UINT32                  emphMode = 0, emph = 0, Amp = 0, laneNum = 0;
    BOOL                    AmpAdjEn = FALSE, emphAdjEn = FALSE;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("rxtune", argv[0]) == 0 )
    {
        if ( argc != 7 )
        {
            ERR_PRINT_CMD_USAGE("rxtune");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;
        laneNum = simple_strtoul(argv[paraCnt], NULL, 10);

        /* RX tune for copper ports */
        if((laneNum+1)*4 > boardInfo.copperMaxNum)
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        AmpAdjEn = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (AmpAdjEn<FALSE) || (AmpAdjEn>TRUE) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        emphAdjEn = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (emphAdjEn<FALSE) || (emphAdjEn>TRUE) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        emphMode = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (emphMode<TUNE_PARA_MIN) || (emphMode>TRUE) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        emph = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (emph<TUNE_PARA_MIN) || (emph>RX_EMPH_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        Amp = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (Amp<TUNE_PARA_MIN) || (Amp>RX_AMP_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            ret = switch_tuneRxTuneSet(laneNum, AmpAdjEn, emphAdjEn, emphMode, emph, Amp);
            if (ret != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_DATA_SET;
                goto __CMD_ERROR;
            }
        }
        return ret;
    }
__CMD_ERROR:
    return ret;
}

INT32 do_berttest
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0, fail = 0;
    UINT32                  port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0;
    UINT32                  mode = 0, errCntr[2] = {0};
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_TRANSMIT_MODE_PRBS    prbs=E_TRANSMIT_MODE_PRBS_MAX;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("berttest", argv[0]) == 0 )
    {
        if ( argc != 3 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;

        if ( port_utilsParaPass(&paraCnt, argv, FALSE, &startPort, &endPort, &lPortType, &lPortBase, lPortTypeStr) < E_TYPE_SUCCESS )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        lPortBase=0;
        port = ( startPort == 0 ) ?  1 : startPort;

        /*PHY only support prbs7, prbs23, prbs31*/
        paraCnt=2;
        if( strcmp(argv[paraCnt], "prbs7") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_7;
        }
        else if( strcmp(argv[paraCnt], "prbs23") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_23;
        }
        else if( strcmp(argv[paraCnt], "prbs31") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_31;
        }
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        log_printf("Port Lane Result errCntr(mac->phy) errCntr(phy->mac)\n");
        log_printf("---- ---- ------ ----------------- -----------------\n");
        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            if (lPort >= boardInfo.firstfiberNum)
                continue;
            ret = switch_tuneBerTest(lPort, prbs, (UINT32 *)&errCntr);
            if(ret == E_TYPE_SUCCESS)
            {
                log_printf(" %2d   %-3d  %-14s%-18u%u\n",lPort, (lPort-1)/4, \
                (errCntr[0] || errCntr[1])? "FAIL":"PASS",errCntr[0], errCntr[1]);
            }

            /* one port fail, the BER test would show fail */
            if( (ret != E_TYPE_SUCCESS) || (errCntr[0]) || (errCntr[1]))
                fail = 1;
            usleep(100000);
        }
        if( fail )
        {
            log_cmdPrintf(E_LOG_MSG_FAIL, "BER Test\r\n");
        }
        else
        {
            log_cmdPrintf(E_LOG_MSG_PASS, "BER Test\r\n");
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_qsgmii_init
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    UINT32 ret = 0;
    UINT32 readDevMACId = 0;
    E_BOARD_ID  boardId;

    boardId = sys_utilsDevBoardIdGet();

    /*20170626 - Try to read device MAC chip Id information */
    readDevMACId  = sys_utilsDevMACChipIdGet();
                    
    if ( strcmp("qsgmii_init", argv[0]) == 0 )
    {
        if ( argc > 1 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        switch(boardId)
        {
            case E_BOARD_ID_HAYWARDS_8G2G_T:
            case E_BOARD_ID_HAYWARDS_8G2G_P:
                 switch_tuneTxManualSet(1, 1, 1, 5, 0, 9);
                 switch_tuneTxManualSet(0, 1, 1, 5, 0, 9);
                 log_printf("Init Haywards_8G2G config \n");
                 break;
             case E_BOARD_ID_HAYWARDS_16G2G_T:
             case E_BOARD_ID_HAYWARDS_16G2G_P:
                 switch_tuneTxManualSet(3, 1, 1, 5, 0, 7);
                 switch_tuneTxManualSet(2, 1, 1, 5, 0, 7);
                 switch_tuneTxManualSet(1, 1, 1, 5, 0, 9);
                 switch_tuneTxManualSet(0, 1, 1, 5, 0, 9);
                 log_printf("Init Haywards_16G2G config \n");
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
                     /* 03282017 -- updated 24T/P SFP+ ports tx serdes init */
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
                     log_printf("Init Haywards2_24G4XG config \n");
                 } else {
                     log_printf("Init Haywards_24G4G config \n");
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
                     log_printf("Init Haywards2_48G4XG config \n");
                 } else {
                     log_printf("Init Haywards_48G4G config \n"); 
                 } 
                 break;
        }
    }
    else
    {
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;
}

INT32 do_sfp_prbsmode
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT32                   ret = 0;
    UINT32                  port = 0, paraCnt = 0, lPort = 0;
    E_TRANSMIT_MODE_PRBS    prbs=E_TRANSMIT_MODE_PRBS_MAX;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("sfpprbs", argv[0]) == 0 )
    {
        if ( argc != 3 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;

        port = simple_strtoul(argv[paraCnt], NULL, 10);

        /* check if SFP fiber ports only */
        if((port < boardInfo.firstfiberNum) || (port > boardInfo.lPortMaxNum))
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        /* SFP+ Port support prbs9, prbs31, customer 8081, add prbs 9, 04122017 */
        paraCnt = 2;
        if( strcmp(argv[paraCnt], "9") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_9;
        }
        else if( strcmp(argv[paraCnt], "15") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_15;
        }
        else if( strcmp(argv[paraCnt], "31") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_31;
        }
        else if( strcmp(argv[paraCnt], "8081") == 0)
        {
            prbs = E_TRANSMIT_MODE_PRBS_8081;
        }
        else
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        /*  check lport to serdes lane num in SFPPrbsSet function */
        lPort = port;

        /* Set SFP port PRBS mode is 9, 15, 31, 8081 */
        ret = switch_tuneSFPPrbsSet(lPort, prbs);

        if( ret )
        {
            log_cmdPrintf(E_LOG_MSG_FAIL, "Set SFP Port=%d PRBS=%s\r\n", port, argv[paraCnt] );
        }
        else
        {
            log_cmdPrintf(E_LOG_MSG_PASS, "Set SFP Port=%d PRBS=%s\r\n", port, argv[paraCnt] );
        }
    }

__CMD_ERROR:
    return ret;
}

INT32 do_sfp_rxtune
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT32                  laneNum = 0, port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0;
    UINT8                   sqlch = 0, ffeRes = 0, ffeCap = 0, align90 = 0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesRxCfgPtr;
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( strcmp("sfprxtune", argv[0]) == 0 )
    {
        if ( argc != 6 )
        {
            ERR_PRINT_CMD_USAGE("sfprxtune");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;
        if ( strcmp("all", argv[paraCnt]) == 0 )
        {
            endPort = (boardInfo.lPortMaxNum - boardInfo.firstfiberNum);
        }
        else
        {
            laneNum = simple_strtoul(argv[paraCnt], NULL, 10);
            laneNum = laneNum - 1;
            endPort = port = laneNum;

             /* RX tune for sfp ports */
            if((boardInfo.firstfiberNum + laneNum) > boardInfo.lPortMaxNum)
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }
        }

        paraCnt++;
        sqlch = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (sqlch<TUNE_PARA_MIN) || (sqlch>RX_SQLCH_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        ffeRes = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (ffeRes<TUNE_PARA_MIN) || (ffeRes>RX_FFERES_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        ffeCap = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (ffeCap<TUNE_PARA_MIN) || (ffeCap>RX_FFECAP_MAX ) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        paraCnt++;
        align90 = simple_strtoul(argv[paraCnt], NULL, 16);
        if( (align90<TUNE_PARA_MIN) || (align90>RX_ALIGN90_MAX) )
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }

        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            ret = switch_tuneRxManualSet(port, sqlch, ffeRes, ffeCap, align90);
            if (ret != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_DATA_SET;
                goto __CMD_ERROR;
            }
        }
        return ret;
    }
    else if ( strcmp("sfprxpara", argv[0]) == 0 )
    {
        if ( argc != 2 )
        {
            ERR_PRINT_CMD_USAGE("sfprxpara");
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        paraCnt = 1;
        if ( strcmp("all", argv[paraCnt]) == 0 )
        {
            endPort = (boardInfo.lPortMaxNum - boardInfo.firstfiberNum);
        }
        else
        {
            laneNum = simple_strtoul(argv[paraCnt], NULL, 10);
            laneNum = laneNum - 1;
            endPort = port = laneNum;

             /* RX tune for sfp ports */
            if((boardInfo.firstfiberNum + laneNum) > boardInfo.lPortMaxNum)
            {
                ret = E_TYPE_INVALID_PARA;
                goto __CMD_ERROR;
            }
        }

        log_printf("Lane sqlch ffeRes ffeCap align90\n");
        log_printf("---- ----- ------ ------ -------\n");

        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            ret = switch_tuneRxGet(port, &serdesRxCfgPtr);
            if (ret != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_DATA_SET;
                goto __CMD_ERROR;
            }
            log_printf(" %-4d 0x%X   0x%X    0x%-3X 0x%-3X\n", port+1,
                        serdesRxCfgPtr.sqlch, serdesRxCfgPtr.ffeRes,
                        serdesRxCfgPtr.ffeCap, serdesRxCfgPtr.align90);
        }

    }
__CMD_ERROR:
    return ret;
}

/* Function: do_sfp_rxauto
  * do RX serdex auto tune parameters
  */
INT32 do_sfp_rxauto
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    INT8                    lPortTypeStr[20] = "UNKNOWN";
    INT32                   ret = 0, lPortBase = 0;
    UINT32                  laneNum = 0, port = 0, paraCnt = 0, startPort = 0, endPort = 0, lPort = 0;
    UINT8                   sampleTime = 0, startTime  = 0;
    S_PORT_INFO             *portInfo;
    E_LINER_PORT_TYPE       lPortType = E_LINER_PORT_TYPE_FIXED;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr;
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    if ( argc != 3 )
    {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
    }
    
    paraCnt = 1;
    if ( strcmp("all", argv[paraCnt]) == 0 )
    {
        endPort = (boardInfo.lPortMaxNum - boardInfo.firstfiberNum);
    }
    else
    {
        laneNum = simple_strtoul(argv[paraCnt], NULL, 10);
        laneNum = laneNum - 1;
        endPort = port = laneNum;

         /* RX tune for sfp ports */
        if((boardInfo.firstfiberNum + laneNum) > boardInfo.lPortMaxNum)
        {
            ret = E_TYPE_INVALID_PARA;
            goto __CMD_ERROR;
        }
    }
    
    paraCnt++;
    sampleTime = simple_strtoul(argv[paraCnt], NULL, 16);
    if( (sampleTime<RX_AUTOTUNE_MIN) || (sampleTime>RX_AUTOTUNE_MAX) )
    {
        ret = E_TYPE_INVALID_PARA;
        goto __CMD_ERROR;
    }

    /* malloc the RX serdes config */
    serdesRxCfgPtr = malloc(sizeof(CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC)*sampleTime);
    if(serdesRxCfgPtr == NULL){
        ret = E_TYPE_ALLOC_MEM_FAIL;
        goto __CMD_ERROR;
    }    

    memset(serdesRxCfgPtr, 0x0, sizeof(serdesRxCfgPtr)*sampleTime);
    
    startPort = port;
    /* test sample time with start port */
    for(startTime=0; startTime < sampleTime; startTime++)
    {
        /* reset start port */
        port = startPort;
        for ( ; port <= endPort; port ++ )
        {
            lPort=lPortBase+port;
            ret = switch_tuneRxAutoSet(port);
            if (ret != E_TYPE_SUCCESS )
            {
                log_printf("Failed to set switch serdes RX auto tune (port=%d)\n", port);
                ret = E_TYPE_DATA_SET;
                free(serdesRxCfgPtr);
                goto __CMD_ERROR;
            }

            ret = switch_tuneRxGet(port, &serdesRxCfgPtr[startTime]);
            if (ret != E_TYPE_SUCCESS )
            {
                ret = E_TYPE_DATA_SET;
                free(serdesRxCfgPtr);
                goto __CMD_ERROR;
            }
          
            log_printf(" %-4d 0x%X   0x%X    0x%-3X 0x%-3X\n", port+1,
                        serdesRxCfgPtr[startTime].sqlch, 
                        serdesRxCfgPtr[startTime].ffeRes,
                        serdesRxCfgPtr[startTime].ffeCap, serdesRxCfgPtr[startTime].align90);
        }
    }

    free(serdesRxCfgPtr);
__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
     tunepara,    CONFIG_SYS_MAXARGS,    1,    do_tuneparaGet,
     "tunepara \t- This test is used to dump the serdes tune parameters.\n",
     "<lane_num | all>.\r\n"
     "  - lane_num: Specify a range of serdes lane numbers.Valid values <0~phyNum*2|6,8|6~9|12~15>\r\n"
);


U_BOOT_CMD(
     berttest,    CONFIG_SYS_MAXARGS,    1,    do_berttest,
     "berttest  \t- This test is used to do the bit error rate test.\n",
     "<port_num> <prbs_mode>.\r\n"
     "  - port_num: Specify a range of port numbers.\r\n"
     "  - prbs_mode: Serdes transmit modes. Valid values <prbs7 \r\n"
     "  - .......... | prbs23 | prbs31 >\r\n"
);

U_BOOT_CMD(
     txtune,    CONFIG_SYS_MAXARGS,    1,    do_txtune,
     "txtune \t\t- This test is used to tune the tx(for MAC) signal quality of the serdes.\n",
     "<lane_num> <txAmpAdjEn> <txAmpShft> <txAmp> <emph0> <emph1>.\r\n"
     "  - lane_num: Specify a range of serdes lane numbers.Valid values <0~phyNum*2|6,8|6~9|12~15>\r\n"
     "  - txAmpAdjEn: Transmitter Amplitude Adjust. Valid values <0|1>\r\n"
     "  - txAmpShft: Transmitter Amplitude Shift. Valid values <0|1>\r\n"
     "  - txAmp: Tx Driver output amplitude. Valid values <0x0~0x1F>\r\n"
     "  - emph0: emphasis amplitude for Gen0 bit rates. Valid values <0x0~0xF>\r\n"
     "  - emph1: emphasis amplitude for Gen0 bit rates. Valid values <0x0~0xF>\r\n"
);

U_BOOT_CMD(
     rxtune,    CONFIG_SYS_MAXARGS,    1,    do_rxtune,
     "rxtune \t\t- This test is used to tune the rx(for MAC) signal quality of the serdes.\n",
     "<lane_num> <AmpAdjEn> <emphAdjEn> <emphMode> <emph> <Amp>.\r\n"
     "  - lane_num: Specify a range of serdes lane numbers.Valid values <0~phyNum*2>\r\n"
     "  - AmpAdjEn: Transmitter Amplitude Adjust. Valid values <0|1>\r\n"
     "  - emphAdjEn: Transmitter Emphasis Amplitude. Valid values <0|1>\r\n"
     "  - emphMode: Transmitter Emphasis Amplitude X1 / X2. Valid values <0|1>\r\n"
     "  - emph: Transmitter Emphasis Amplitude. Valid values <0x0~0x7>\r\n"
     "  - Amp: Transmitter Amplitude. Valid values <0x0~0x1F>\r\n"
);

U_BOOT_CMD(
     qsgmii_init,    CONFIG_SYS_MAXARGS,    1,    do_qsgmii_init,
     "qsgmii_init \t- Init the tx/rx tune parameters for each unit.\n",
     "Init each unit tx/rx tune parameters \n"
);

U_BOOT_CMD(
     sfpprbs,    CONFIG_SYS_MAXARGS,    1,    do_sfp_prbsmode,
     "sfpprbs  \t- Set PRBS mode to SFP ports\n",
     "<port_num> <prbs_mode>.\r\n"
     "  - port_num: Specify a range of SFP port numbers.\r\n"
     "  - prbs_mode: Serdes transmit modes. Valid values <9|15|31|8081>\r\n"
);

U_BOOT_CMD(
     sfprxtune,    CONFIG_SYS_MAXARGS,    1,    do_sfp_rxtune,
     "sfprxtune  \t- This command use to set rx tune configuration with SFP ports\n",
     "<lane_num> <sqlch> <ffeRes> <ffeCap> <align90>.\r\n"
     "  - lane_num: Specify a range of serdes lane numbers.Valid values <1~4>\r\n"
     "  - sqlch: Threshold that trips the Squelch detector peak-to-peak. Valid values <0x0~0x1F>\r\n"
     "  - ffeRes: mainly controls the low frequency gain. Valid values <0x0~0x7>\r\n"
     "  - ffeCap: mainly controls the high frequency gain <0x0~0xF>\r\n"
     "  - align90: Align 90 Calibration Phase Offset. Valid values <0x0~0x7F>\r\n"
);

U_BOOT_CMD(
     sfprxpara,    CONFIG_SYS_MAXARGS,    1,    do_sfp_rxtune,
     "sfprxpara \t- This test is used to dump the serdes RX tune parameters with SFP ports.\n",
     "<lane_num | all>.\r\n"
     "  - lane_num: Specify a range of serdes lane numbers.Valid values <1~4>\r\n"
);


U_BOOT_CMD(
     sfprxauto,    CONFIG_SYS_MAXARGS,    1,    do_sfp_rxauto,
     "sfprxauto \t- This test is used to the RX serdes auto tune parameters with SFP ports.\n",
     "<lane_num | all> <time>.\r\n"
     "  - lane_num: Specify a range of serdes lane numbers.Valid values <1~4>\r\n"
     "  - time: Specify a auto tune sample times.Valid values <1~100>\r\n"
);

