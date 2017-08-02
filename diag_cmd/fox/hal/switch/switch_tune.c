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
***      switch_tune.c
***
***    DESCRIPTION :
***      for switch tune
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
#include "switch_hal.h"
#include "switch_port.h"
#include "switch_tune.h"
#include "port_defs.h"
#include "port_utils.h"
#include "sys_utils.h"
#include "err_type.h"
#include "porting.h"

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
 *      switch_tuneResultGet
 *
 *  DESCRIPTION :
 *      switch serdes tune result get
 *
 *  INPUT :
 *      laneNum - serdes lane number
 *
 *  OUTPUT :
 *      tuneValues - tune result ptr
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
INT32 switch_tuneResultGet
(
    IN UINT32 laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr,
    OUT UINT16 *serdesRxVal
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    UINT32 lPort = 0;
    E_BOARD_ID  boardId = sys_utilsDevBoardIdGet();

    lPort = (laneNum*4)+2;

    /* 02162017 - check serdes SFP/SFP+ ports serdes lane num to lPort */
    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) ||
        (boardId == E_BOARD_ID_HAYWARDS_48G4G_P)){
        if ((laneNum>=SERDES_LANE_12) && (laneNum<=SERDES_LANE_15))
        {
            lPort = ((laneNum-(laneNum%12))*4)+(laneNum-12+1);
        }
    } else if( (boardId == E_BOARD_ID_HAYWARDS_24G4G_T) ||
                   (boardId == E_BOARD_ID_HAYWARDS_24G4G_P)) {
         if ((laneNum>=SERDES_LANE_6) && (laneNum<=SERDES_LANE_9))
         {
            lPort = ((laneNum-(laneNum%6))*4)+(laneNum-6+1);
         }
   } else if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) ||
                   (boardId == E_BOARD_ID_HAYWARDS_16G2G_P)) {
         if (laneNum==SERDES_LANE_6)
         {
            lPort = 17; /* lPort = 17 */
         }else if (laneNum==SERDES_LANE_8){
            lPort = 18; /* lPort - 18 */
         }
    } else if( (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) ||
                   (boardId == E_BOARD_ID_HAYWARDS_8G2G_P)) {
         if (laneNum==SERDES_LANE_6)
         {
            lPort = 9; /* lPort = 9 */
         }else if (laneNum==SERDES_LANE_8){
            lPort = 10; /* lPort - 10 */
         }
    }

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    log_dbgPrintf("lPort=%d, laneNum=%d\n",lPort, laneNum);

    /*Get the RX tune result */
    ret = switch_qsgmiiRegRead(lPort, 0xD, serdesRxVal);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("[ERR]: failed to get RX tune value.(ret=%d)\n", ret);
        return ret;
    }
    log_dbgPrintf("RX tune value data is 0x%04X\n", *serdesRxVal);

    /*Get the TX tune result */
    ret = cpssDxChPortSerdesManualTxConfigGet(portInfo->devId, portInfo->portId, QSGMII_LANE_NUM, serdesTxCfgPtr);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to get TX tune value.(ret=%d)\n", ret);
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_qsgmiiRegRead
 *
 *  DESCRIPTION :
 *      switch_qsgmiiRegRead
 *
 *  INPUT :
 *      lPort - logic port number
 *      regAddr - register address
 *
 *  OUTPUT :
 *      None
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
INT32 switch_qsgmiiRegRead
(
    IN UINT32 lPort,
    IN UINT32 regAddr,
    OUT UINT16 *regVal
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 reg =0, temp = 0;

    /* Set to page 0x00FD */
    switch_halPhyPageNumSet(lPort, 0x00FD);

    reg = (1<<12) |  (regAddr & 0xF);
    /* Write the data to data register */
    ret = switch_halSMIRegSet(lPort, 0x7, reg);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to enable read operation.\n");
        return ret;
    }

    /* Read the data back from data register */
    ret = switch_halSMIRegGet(lPort, 0x9, &temp);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to read back data.\n");
        return ret;
    }

    /* Read the data to check read valid? */
    ret = switch_halSMIRegGet(lPort, 0x7, &reg);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to read back check valid data.\n");
        return ret;
    }

    /* Set to page 0x00 */
    switch_halPhyPageNumSet(lPort, 0);

    if (reg&0x8000)
        *regVal = temp;
    else
        return E_TYPE_DATA_GET;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_qsgmiiRegWrite
 *
 *  DESCRIPTION :
 *      switch_qsgmiiRegWrite
 *
 *  INPUT :
 *      lPort - logic port number
 *      regAddr - register address
 *
 *  OUTPUT :
 *      None
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
INT32 switch_qsgmiiRegWrite
(
    IN UINT32 lPort,
    IN UINT32 regAddr,
    OUT UINT16 regVal
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 reg =0;

    /* Set to page 0x00FD */
    switch_halPhyPageNumSet(lPort, 0x00FD);

    /* Write the data to data register */
    ret = switch_halSMIRegSet(lPort, 0x8, regVal);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to set the data.\n");
        return ret;
    }

    reg = (1<<13) |  (regAddr & 0xF);
    /* Enable the write operation */
    ret = switch_halSMIRegSet(lPort, 0x7, reg);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to enable the write operation.\n");
        return ret;
    }

    /* Set to page 0x00 */
    switch_halPhyPageNumSet(lPort, 0);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneRxTuneSet
 *
 *  DESCRIPTION :
 *      switch Rx tune
 *
 *  INPUT :
 *      laneNum - serdes lane number
 *      AmpAdjEn - Transmitter Amplitude Adjust
 *      emphAdjEn - Transmitter Emphasis Amplitude
 *      emphMode - Transmitter Emphasis Amplitude X1 / X2
 *      emph - Transmitter Emphasis Amplitude
 *      Amp - Transmitter Amplitude
 *
 *  OUTPUT :
 *      None
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
INT32 switch_tuneRxTuneSet
(
    IN UINT32 laneNum,
    IN BOOL   AmpAdjEn,
    IN BOOL   emphAdjEn,
    IN UINT32 emphMode,
    IN UINT32 emph,
    IN UINT32 Amp
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT32 lPort = 0;
    UINT16 regVal = 0;

    lPort = (laneNum*4+1);
    log_dbgPrintf("laneNum = %d-%d, AmpAdjEn =%d, emphAdjEn = %d, emphMode=%d, emph=0x%X, Amp=0x%X\n",\
                    laneNum,lPort, AmpAdjEn, emphAdjEn, emphMode, emph, Amp);

    regVal = (AmpAdjEn<<6)|(emphAdjEn<<11)|(emphMode<<10)|(emph<<7)|(Amp<<1);
    log_dbgPrintf("RX tune data = 0x%04X\n",regVal);
    ret = switch_qsgmiiRegWrite(lPort, 0xD, regVal);
    if (ret != E_TYPE_SUCCESS)
        return ret;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneTxManualSet
 *
 *  DESCRIPTION :
 *      switch Tx Manual Set
 *
 *  INPUT :
 *      laneNum - serdes lane number
 *      txAmpAdjEn - Transmitter Amplitude Adjust
 *      txAmpShft - Transmitter Amplitude Shift
 *      txAmp - Tx Driver output amplitude
 *      emph0 - Controls the emphasis amplitude for Gen0 bit rates
 *      emph1 - Controls the emphasis amplitude for Gen1 bit rates
 *
 *  OUTPUT :
 *      None
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
INT32 switch_tuneTxManualSet
(
    IN UINT32 laneNum,
    IN BOOL txAmpAdjEn,
    IN BOOL txAmpShft,
    IN UINT32 txAmp,
    IN UINT32 emph0,
    IN UINT32 emph1
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT32 lPort = 0;
    S_PORT_INFO *portInfo;
    CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  serdesTxCfgPtr;
    E_BOARD_ID              boardId = sys_utilsDevBoardIdGet();

    lPort = (laneNum*4)+2;

    /* 02162017 - check serdes SFP/SFP+ ports serdes lane num to lPort */
    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) ||
        (boardId == E_BOARD_ID_HAYWARDS_48G4G_P)){
        if ((laneNum>=SERDES_LANE_12) && (laneNum<=SERDES_LANE_15))
        {
            lPort = ((laneNum-(laneNum%12))*4)+(laneNum-12+1);
        }
    } else if( (boardId == E_BOARD_ID_HAYWARDS_24G4G_T) ||
                   (boardId == E_BOARD_ID_HAYWARDS_24G4G_P)) {
         if ((laneNum>=SERDES_LANE_6) && (laneNum<=SERDES_LANE_9))
         {
            lPort = ((laneNum-(laneNum%6))*4)+(laneNum-6+1);
         }
   } else if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) ||
                   (boardId == E_BOARD_ID_HAYWARDS_16G2G_P)) {
         if (laneNum==SERDES_LANE_6)
         {
            lPort = 17; /* lPort = 17 */
         }else if (laneNum==SERDES_LANE_8){
            lPort = 18; /* lPort - 18 */
         }
    } else if( (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) ||
                   (boardId == E_BOARD_ID_HAYWARDS_8G2G_P)) {
         if (laneNum==SERDES_LANE_6)
         {
            lPort = 9; /* lPort = 9 */
         }else if (laneNum==SERDES_LANE_8){
            lPort = 10; /* lPort - 10 */
         }
    }

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    log_dbgPrintf("txAmpAdjEn=%d, txAmpShft=%d, exAmp=0x%X, emph0=0x%X, emph1=0x%X.\n", \
                                        txAmpAdjEn, txAmpShft, txAmp, emph0, emph1);
    log_dbgPrintf("lPort=%d, laneNum=%d\n",lPort, laneNum);

    memset(&serdesTxCfgPtr, 0, sizeof(CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC));
    serdesTxCfgPtr.txAmpAdjEn=txAmpAdjEn;
    serdesTxCfgPtr.txAmpShft=txAmpShft;

    serdesTxCfgPtr.emph0=emph0;
    serdesTxCfgPtr.emph1=emph1;
    serdesTxCfgPtr.txAmp=txAmp;

    /* Set the TX manual tune configuration */
    ret = cpssDxChPortSerdesManualTxConfigSet(portInfo->devId, portInfo->portId, \
                            QSGMII_LANE_NUM, &serdesTxCfgPtr);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to set Tx manual configuration.(ret=%d)\n",ret);
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tunePrbsConfig
 *
 *  DESCRIPTION :
 *      switch PRBE test config
 *
 *  INPUT :
 *      lPort - logical port number
 *      prbs - prbs transmit modes
 *      enable - enable/disable the prbs test
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
INT32 switch_tunePrbsConfig
(
    IN UINT32 lPort,
    IN CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT prbs,
    IN BOOL   enable
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    UINT16 regVal = 0, regTemp = 0;
    UINT32 flexLinkPort = 0;

    flexLinkPort = (((lPort-1)/4)*4)+2;
    portInfo=port_utilsLPortInfoGet(flexLinkPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, flexLinkPort);
        return E_TYPE_DATA_GET;
    }

    if ( enable )
    {
        /* Set to page 4 */
        switch_halPhyPageNumSet(lPort, 4);

        regVal = 0x22;
        if(prbs == E_TRANSMIT_MODE_PRBS_7)
        {
            regVal = regVal;
        }
        else if(prbs == E_TRANSMIT_MODE_PRBS_23)
        {
            regVal |= (0x1<<2);
        }
        else if(prbs == E_TRANSMIT_MODE_PRBS_31)
        {
            regVal |= (0x2<<2);
        }

        log_dbgPrintf("TX set prbs(%d) value =0x%04x\n", prbs, regVal);
        /* Enable the PRBS check with PHY */
        ret = switch_halSMIRegSet(lPort, 0x17, regVal);
        if (ret != GT_OK )
        {
            log_printf("[ERR]: fail to enable the PRBS check with PHY.\n");
            return ret;
        }
        /* Set the PRBS test mode with serdes */
        ret = cpssDxChDiagPrbsSerdesTransmitModeSet(portInfo->devId, portInfo->portId, QSGMII_LANE_NUM, prbs);
        if ( ret != GT_OK )
        {
            log_printf("[ERR]: failed to set the prbs transmit mode.(ret=%d)\n",ret);
            return ret;
        }
    }

    /* Enable the PRBS test with serdes*/
    ret = cpssDxChDiagPrbsSerdesTestEnableSet(portInfo->devId, portInfo->portId, QSGMII_LANE_NUM, enable);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to enable the prbs test.(ret=%d)\n",ret);
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneGetPhyPrbsErrCntr
 *
 *  DESCRIPTION :
 *      switch get the PRBS phy error counter
 *
 *  INPUT :
 *      lPort - logical port number
 *
 *  OUTPUT :
 *      errPrbsCntr - error counter
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
INT32 switch_tuneGetPhyPrbsErrCntr
(
    IN UINT32 lPort,
    OUT UINT32 *errPrbsCntr
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 regVal, regTemp;

    /* Set to page 4 */
    switch_halPhyPageNumSet(lPort, 4);

    /* Get error counter with LSB */
    ret = switch_halSMIRegGet(lPort, 0x18, &regTemp);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get error counter with LSB.\n");
        return ret;
    }

    /* Get error counter with MSB */
    ret = switch_halSMIRegGet(lPort, 0x19, &regVal);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get error counter with MSB.\n");
        return ret;
    }

    /* Clear the error counter */
    ret = switch_halSMIRegSet(lPort, 0x17, 0x10);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to get error counter with MSB.\n");
        return ret;
    }

    log_dbgPrintf("LSB=0x%x, MSB=0x%x\n", regTemp, regVal);
    *errPrbsCntr=((regVal<<16)|regTemp)&0xFFFFFFFF;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneBerTXTest
 *
 *  DESCRIPTION :
 *      switch_tuneBerTXTest(MAC -> PHY)
 *
 *  INPUT :
 *      lPort - logical port number
 *      mode - 0 or 1
 *
 *  OUTPUT :
 *      errCntr - error counter
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
INT32 switch_tuneBerTXTest
(
    IN UINT32 lPort,
    IN E_TRANSMIT_MODE_PRBS prbsMode,
    OUT UINT32 *errCntr
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    UINT32 lockedPtr = 0, errorCntrPtr = 0;
    UINT64 patternCntrPtr[2];

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    ret = switch_tunePrbsConfig(lPort, (CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT)prbsMode, TRUE);
    if (ret != E_TYPE_SUCCESS )
        return ret;

    /* MAC-PHY, PHY check the counter */
    ret = switch_tuneGetPhyPrbsErrCntr(lPort, &errorCntrPtr);
    if (ret != E_TYPE_SUCCESS )
        return ret;

    *errCntr = errorCntrPtr;

    /* Disable the PRBS Test */
    ret = switch_tunePrbsConfig(lPort, (CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT)prbsMode, FALSE);
    if (ret != E_TYPE_SUCCESS )
        return ret;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneBerRXTest
 *
 *  DESCRIPTION :
 *      switch_tuneBerRXTest(PHY -> MAC)
 *
 *  INPUT :
 *      lPort - logical port number
 *      mode - 0 or 1
 *
 *  OUTPUT :
 *      errCntr - error counter
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
INT32 switch_tuneBerRXTest
(
    IN UINT32 lPort,
    IN E_TRANSMIT_MODE_PRBS prbsMode,
    OUT UINT32 *errCntr
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo, *flexLinkPortInfo;
    UINT16 regVal = 0, regTemp = 0;
    UINT32 lockedPtr = 0, errorCntrPtr = 0, flexLinkPort = 0;
    UINT64 patternCntrPtr[2];
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesRxCfgPtr;

    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    flexLinkPort=(((lPort-1)/4)*4)+2;
    flexLinkPortInfo=port_utilsLPortInfoGet(flexLinkPort);
    if( flexLinkPortInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, flexLinkPort);
        return E_TYPE_DATA_GET;
    }

    /* Set to page 4 */
    switch_halPhyPageNumSet(lPort, 4);

    regVal = 0x21;
    if(prbsMode == E_TRANSMIT_MODE_PRBS_7)
    {
        regVal = regVal;
    }
    else if(prbsMode == E_TRANSMIT_MODE_PRBS_23)
    {
        regVal |= (0x1<<2);
    }
    else if(prbsMode == E_TRANSMIT_MODE_PRBS_31)
    {
        regVal |= (0x2<<2);
    }

    log_dbgPrintf("RX set prbs(%d) value 0x%04x\n", prbsMode, regVal);
    /* Enable the PRBS test with PHY */
    ret = switch_halSMIRegSet(lPort, 0x17, regVal);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to enable the PRBS.\n");
        return ret;
    }

    /* Enable the PRBS check for each port */
    ret = cpssDxChDiagPrbsPortCheckEnableSet(portInfo->devId, portInfo->portId, QSGMII_LANE_NUM, TRUE);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to enable the port prbs check.(ret=%d)\n",ret);
        return ret;
    }

    /* Set the PRBS test mode */
    ret = cpssDxChDiagPrbsSerdesTransmitModeSet(flexLinkPortInfo->devId, flexLinkPortInfo->portId, \
                            QSGMII_LANE_NUM, prbsMode);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to set the prbs transmit mode.(ret=%d)\n",ret);
        return ret;
    }

    /* Enable the PRBS test */
    ret = cpssDxChDiagPrbsSerdesTestEnableSet(flexLinkPortInfo->devId, flexLinkPortInfo->portId, QSGMII_LANE_NUM, TRUE);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to enable the prbs test.(ret=%d)\n",ret);
        return ret;
    }

    /* Set to page 18 */
    switch_halPhyPageNumSet(lPort, 18);

    /* Generate the packets with PHY */
    ret = switch_halSMIRegSet(lPort, 0x10, 0xffd3);
    if (ret != GT_OK )
    {
        log_printf("[ERR]: fail to generator the PRBS package.\n");
        return ret;
    }

    /* Get the PRBS status */
    ret = cpssDxChDiagPrbsSerdesStatusGet(flexLinkPortInfo->devId, flexLinkPortInfo->portId, \
                            QSGMII_LANE_NUM, &lockedPtr, (GT_U32 *)&errorCntrPtr, (GT_U64 *)&patternCntrPtr);
    if ( ret != GT_OK || lockedPtr != TRUE)
    {
        log_printf("[ERR]: failed to get the prbs status.lockedPtr=%s (ret=%d) \n",lockedPtr? "TRUE":"FALSE", ret);
        if (lockedPtr != TRUE)
            return E_TYPE_DATA_GET;
        return ret;
    }

    log_dbgPrintf("lockedPtr=%d, errorCntrPtr=%d, pattern0=%ld, pattern1=%ld\n", lockedPtr, errorCntrPtr, \
                            patternCntrPtr[0], patternCntrPtr[1]);

    /* Disable the PRBS test */
    ret = cpssDxChDiagPrbsSerdesTestEnableSet(flexLinkPortInfo->devId, flexLinkPortInfo->portId, QSGMII_LANE_NUM, FALSE);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to disable the prbs test.(ret=%d)\n",ret);
        return ret;
    }

    *errCntr=errorCntrPtr;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneBerTest
 *
 *  DESCRIPTION :
 *      switch bit error rate test
 *
 *  INPUT :
 *      lPort - logical port number
 *      mode - 0 or 1
 *
 *  OUTPUT :
 *      errCntr - error counter
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
INT32 switch_tuneBerTest
(
    IN UINT32 lPort,
    IN E_TRANSMIT_MODE_PRBS prbsMode,
    OUT UINT32 *errCntr
)
{
    INT32  ret = E_TYPE_SUCCESS;

    /* MAC -> PHY */
    ret = switch_tuneBerTXTest(lPort, prbsMode, &errCntr[0]);
    if (ret != E_TYPE_SUCCESS)
        return ret;

    /* PHY -> MAC */
    ret = switch_tuneBerRXTest(lPort, prbsMode, &errCntr[1]);
    if (ret != E_TYPE_SUCCESS)
        return ret;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneSFPPrbsSet
 *
 *  DESCRIPTION :
 *      SFP ports PRBS mode set
 *
 *  INPUT :
 *      lPort - logical port number
 *      mode - PRBS9, PRBS31, 8081
 *
 *  OUTPUT :
 *      None
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
INT32 switch_tuneSFPPrbsSet
(
    IN UINT32 lPort,
    IN E_TRANSMIT_MODE_PRBS prbsMode
)
{
    INT32    ret = E_TYPE_SUCCESS;
    UINT32  offset =0, data = 0, laneNum = 0;
    UINT32  devId=0, portId = 0, prbsdata = 0;
    S_PORT_INFO *portInfo;
    E_BOARD_ID  boardId = sys_utilsDevBoardIdGet();

    /* check physical port and device ID */
    portInfo = port_utilsLPortInfoGet(lPort);
    devId = portInfo->devId;
    portId = portInfo->portId;

    /* 03232017 - check serdes SFP/SFP+ ports serdes lane num to lPort
     * 48T/P sch. diagram the SFP ports serdes lane number is 10, 11
     */
    if( (boardId == E_BOARD_ID_HAYWARDS_48G4G_T) ||
        (boardId == E_BOARD_ID_HAYWARDS_48G4G_P)){
        switch(portId){
            case 25:
                laneNum = SERDES_LANE_10;
                break;
            case 27:
                laneNum = SERDES_LANE_11;
                break;
             default:
                laneNum = 0;
                ret  = E_TYPE_INVALID_DATA;
                break;
        }
    } else {
        /* 03232027, for other SKUs 24T/P */
        switch(portId){
            case 24:
                laneNum = SERDES_LANE_6;
                break;
            case 25:
                laneNum = SERDES_LANE_7;
                break;
            case 26:
                laneNum = SERDES_LANE_8;
                break;
            case 27:
                laneNum = SERDES_LANE_9;
                break;
             default:
                laneNum = 0;
                ret  = E_TYPE_INVALID_DATA;
                break;
        }
    }

    log_dbgPrintf("lPort=%d (%d/%d), laneNum=%d\n", lPort, devId, portId, laneNum);

    /* SFP port serdes lane num 6-9, 48T/P SFP serdes lane number 10-11 */
    if ((laneNum == 0) && (ret  == E_TYPE_INVALID_DATA))
        return ret;

    /* User defined 8081 data to registers */
    if(prbsMode == E_TRANSMIT_MODE_PRBS_8081)
    {
        prbsdata = 5;
        data= 0xff00ff;
        offset = 0x13000858|(laneNum<<12);
        switch_halMACRegSet(devId, offset, data);
        log_dbgPrintf("Set the PRBS 8081 (prbsdata=0x%x)\n", prbsdata);
        log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, data);
        offset = 0x1300085C|(laneNum<<12);
        switch_halMACRegSet(devId, offset, data);
        log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, data);
        offset = 0x13000860|(laneNum<<12);
        switch_halMACRegSet(devId, offset, data);
        log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, data);
        offset = 0x13000864|(laneNum<<12);
        switch_halMACRegSet(devId, offset, data);
        log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, data);
        offset = 0x13000868|(laneNum<<12);
        switch_halMACRegSet(devId, offset, data);
        log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, data);

    }
    else if(prbsMode == E_TRANSMIT_MODE_PRBS_9)
    {
        prbsdata = 0x81;
        log_dbgPrintf("Set the PRBS 9 (prbsdata=0x%x)\n", prbsdata);
    }
    else if(prbsMode == E_TRANSMIT_MODE_PRBS_15)
    {
        prbsdata = 0x82;
        log_dbgPrintf("Set the PRBS 15 (prbsdata=0x%x)\n", prbsdata);
    }
    else if(prbsMode == E_TRANSMIT_MODE_PRBS_31)
    {
        prbsdata = 0x84;
        log_dbgPrintf("Set the PRBS 31 (prbsdata=0x%x)\n", prbsdata);
    }

    offset = 0x1300086C|(laneNum<<12);
    switch_halMACRegSet(devId, offset, prbsdata);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, prbsdata);
    offset = 0x13000854|(laneNum<<12);
    switch_halMACRegSet(devId, offset, 0x000000E0);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x000000E0);
    switch_halMACRegSet(devId, offset, 0x000000E0);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x000000E0);
    switch_halMACRegSet(devId, offset, 0x000040E0);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x000040E0);
    switch_halMACRegSet(devId, offset, 0x000000E0);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x000000E0);
    offset = 0x1300088C|(laneNum<<12);
    switch_halMACRegSet(devId, offset, 0x00000872);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x00000872);
    switch_halMACRegSet(devId, offset, 0x00000072);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x00000072);
    offset = 0x1300086C|(laneNum<<12);
    switch_halMACRegSet(devId, offset, prbsdata);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, prbsdata);

    /* bit 15 = 1,	PRBS transmitter enable */
    offset = 0x13000854|(laneNum<<12);
    switch_halMACRegSet(devId, offset, 0x000080E0);
    log_dbgPrintf("Write MAC Register (PRBS) : Device=0x%x, Register 0x%x = 0x%08x\n", devId, offset, 0x000080E0);
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneRxManualSet
 *
 *  DESCRIPTION :
 *      switch Rx Manual Set
 *
 *  INPUT :
 *      laneNum - serdes lane number
 *      txAmpAdjEn - Transmitter Amplitude Adjust
 *      txAmpShft - Transmitter Amplitude Shift
 *      txAmp - Tx Driver output amplitude
 *      emph0 - Controls the emphasis amplitude for Gen0 bit rates
 *      emph1 - Controls the emphasis amplitude for Gen1 bit rates
 *
 *  OUTPUT :
 *      None
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
INT32 switch_tuneRxManualSet
(
    IN UINT32 laneNum,
    IN UINT8 sqlch,
    IN UINT8 ffeRes,
    IN UINT8 ffeCap,
    IN UINT8 align90
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT32 lPort = 0;
    S_PORT_INFO *portInfo;
    CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  serdesRxCfgPtr;
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    lPort = boardInfo.firstfiberNum + laneNum;
    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    log_dbgPrintf("sqlch=0x%X, ffeRes=0x%X, ffeCap=0x%X, align90=0x%X.\n", \
                                        sqlch, ffeRes, ffeCap, align90);
    log_dbgPrintf("lPort=%d, laneNum=%d\n",lPort, laneNum);

    memset(&serdesRxCfgPtr, 0, sizeof(CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC));
    serdesRxCfgPtr.sqlch=sqlch;
    serdesRxCfgPtr.ffeRes=ffeRes;

    serdesRxCfgPtr.ffeCap=ffeCap;
    serdesRxCfgPtr.align90=align90;

    /* Set the RX manual tune configuration */
    ret = cpssDxChPortSerdesManualRxConfigSet(portInfo->devId, portInfo->portId, \
                            QSGMII_LANE_NUM, &serdesRxCfgPtr);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to set Rx manual configuration.(ret=%d)\n",ret);
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneRxGet
 *
 *  DESCRIPTION :
 *      switch serdes tune result get
 *
 *  INPUT :
 *      laneNum - serdes lane number
 *
 *  OUTPUT :
 *      tuneValues - tune result ptr
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
INT32 switch_tuneRxGet
(
    IN UINT32 laneNum,
    OUT CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    UINT32 lPort = 0;
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    lPort = boardInfo.firstfiberNum + laneNum;
    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    log_dbgPrintf("lPort=%d, laneNum=%d\n",lPort, laneNum);

    /*Get the TX tune result */
    ret = cpssDxChPortSerdesManualRxConfigGet(portInfo->devId, portInfo->portId, QSGMII_LANE_NUM, serdesRxCfgPtr);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to get RX tune value.(ret=%d)\n", ret);
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_tuneRxAutoSet
 *
 *  DESCRIPTION :
 *      switch RX serdes auto tune 
 *
 *  INPUT :
 *      laneNum - serdes lane number
 *
 *  OUTPUT :
 *      None
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
INT32 switch_tuneRxAutoSet
(
    IN UINT32 laneNum
)
{
    INT32  ret = E_TYPE_SUCCESS;
    S_PORT_INFO *portInfo;
    UINT32 lPort = 0;
    S_BOARD_INFO            boardInfo;

    sys_utilsBoardInfoGet(&boardInfo);

    lPort = boardInfo.firstfiberNum + laneNum;
    portInfo=port_utilsLPortInfoGet(lPort);
    if( portInfo == NULL )
    {
        log_printf("%s get port %d map data fail\n", __FUNCTION__, lPort);
        return E_TYPE_DATA_GET;
    }

    log_dbgPrintf("lPort=%d, laneNum=%d\n",lPort, laneNum);

    /* Set the RX auto tune */
    ret = cpssDxChPortSerdesAutoTune(portInfo->devId, portInfo->portId, CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
    if ( ret != GT_OK )
    {
        log_printf("[ERR]: failed to set RX auto tune.(lport=%d, ret=%d)\n", lPort, ret);
        return ret;
    }
    
    return ret;
}