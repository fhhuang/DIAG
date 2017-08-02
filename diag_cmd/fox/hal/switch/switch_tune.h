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
***             switch_tune.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
***             File Creation
***
***************************************************************************/

#ifndef __SWITCH_TUNE_H_
#define __SWITCH_TUNE_H_

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
#include "cmn_type.h"
#include "mvTypes.h"
#include "port_defs.h"
#include "userEventHandler.h"
#include "cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Macro Definition Segment
 *
 *==========================================================================
 */

#define TUNE_PARA_MIN   0x0
#define TX_AMP_MAX      0x1F
#define TX_EMPH0_MAX    0xF
#define TX_EMPH1_MAX    0xF
#define RX_SQLCH_MAX    0x1F
#define RX_FFERES_MAX   0x7
#define RX_FFECAP_MAX   0xF
#define RX_ALIGN90_MAX  0x7F
#define RX_AMP_MAX		0x1F
#define RX_EMPH_MAX	0x7
#define RX_AUTOTUNE_MIN 0x1
#define RX_AUTOTUNE_MAX 0x100
#define QSGMII_LANE_NUM 0
#define SERDES_LANE_6 0x6
#define SERDES_LANE_7 0x7
#define SERDES_LANE_8 0x8
#define SERDES_LANE_9 0x9
#define SERDES_LANE_10 0xA
#define SERDES_LANE_11 0xB
#define SERDES_LANE_12 0xC
#define SERDES_LANE_15 0xF

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef enum {
    E_TRANSMIT_MODE_PRBS_7=5,
    E_TRANSMIT_MODE_PRBS_9,
    E_TRANSMIT_MODE_PRBS_15,
    E_TRANSMIT_MODE_PRBS_23,
    E_TRANSMIT_MODE_PRBS_31,
    E_TRANSMIT_MODE_PRBS_8081,
    E_TRANSMIT_MODE_PRBS_MAX
} E_TRANSMIT_MODE_PRBS;

typedef enum {
    E_BERT_TX_MANUAL=0,
    E_BERT_RX_AUTO,
    E_BERT_RX_MANUAL
} E_BERT_TYPE;

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */

typedef struct {
    GT_BOOL locked;
    UINT32 errCount;
    UINT32 beforePatternCntr[2];
    UINT32 afterPatternCntr[2];
} E_QSGMII_PRBS_INFO;

typedef struct {
    UINT32 dfe;
    UINT32 ffeR;
    UINT32 ffeC;
    GT_BOOL prbsStatus;
    GT_BOOL lockStatus;
    UINT32 prbsErrCntr;
}E_PRBS_DFE_ARRAY;

/*==========================================================================
 *
 *      External Funtion Segment
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
);

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
    IN UINT32 lPort,
    IN BOOL   AmpAdjEn,
    IN BOOL   emphAdjEn,
    IN UINT32 emphMode,
    IN UINT32 emph,
    IN UINT32 Amp
);

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
    IN UINT32 lPort,
    IN BOOL txAmpAdjEn,
    IN BOOL txAmpShft,
    IN UINT32 txAmp,
    IN UINT32 emph0,
    IN UINT32 emph1
);

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
);

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
);

#endif /* __SWITCH_TUNE_H_ */
