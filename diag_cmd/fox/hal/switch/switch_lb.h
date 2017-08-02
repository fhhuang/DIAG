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
***			switch_lb.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/13, 11:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __SWITCH_LB_H_
#define __SWITCH_LB_H_

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
#include "switch_hal.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define LINK_CHECK_RETRY_COUNT          10

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef enum {
    E_SW_LB_PATTERN_CJPAT  = 0x7e7e,
    E_SW_LB_PATTERN_RANDOM = 0x5b5b,
    E_SW_LB_PATTERN_MAX
} E_SW_LB_PATTERN;

/*==========================================================================
 *                                                                          
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_lbTest
 *
 *  DESCRIPTION :
 *      switch loopback test
 *
 *  INPUT :
 *      txLPort      - logical port number of transmit
 *      rxLPort      - logical port number of receive
 *      lbTestType   - loopback test type
 *      pattern      - test pattern
 *      size         - test packet size
 *      numPkt       - test packet amount
 *      speed        - test speed
 *      issfp        - Flag to indicate port if SFP or not
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
INT32 switch_lbTest
(
    IN UINT32 txLPort,
    IN UINT32 rxLPort,
    IN E_LB_TEST_TYPE lbTestType,
    IN UINT32 pattern,
    IN UINT32 size,
    IN UINT32 numPkt,
    IN UINT32 speed,
    IN BOOL issfp,
    OUT S_PORT_CNT *txCounter,
    OUT S_PORT_CNT *rxCounter
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_lbSnakeTest
 *
 *  DESCRIPTION :
 *      switch snake test
 *
 *  INPUT :
 *      snakeMode    - snake test mode
 *      startPort    - logical port number of transmit
 *      endPort      - logical port number of receive
 *      pattern      - snake test pattern
 *      size         - packet size
 *      speed        - snake test speed
 *      numPkt       - test packet amount
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
INT32 switch_lbSnakeTest
(
    IN E_LB_TEST_SNAKE_MODE snakeMode,
    IN UINT32               startPort,
    IN UINT32               endPort,
    IN UINT32               pattern,
    IN UINT32               size,
    IN UINT32               speed,
    IN UINT32               numPkt
);

INT32 switch_linespeedTestStart
(
    IN  void
);

INT32 switch_linespeedTestStop
(
    IN  UINT32          txLPort,
    IN  UINT32          rxLPort,
    IN  E_LB_TEST_TYPE  lbTestType,
    IN  UINT32          numPkt
);

INT32 switch_linespeedTestInit
(
    IN  UINT32          txLPort,
    IN  UINT32          rxLPort,
    IN  E_LB_TEST_TYPE  lbTestType,
    IN  UINT32          pattern,
    IN  UINT32          size,
    IN  UINT32          numPkt,
    IN  UINT32          speed,
    IN  BOOL            issfp
);

INT32 switch_linespeedTestReinit
(
    IN  UINT32          txLPort,
    IN  UINT32          rxLPort,
    IN  E_LB_TEST_TYPE  lbTestType
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      switch_lbTcamBist
 *
 *  DESCRIPTION :
 *      switch build-in self test for TCAM
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
INT32 switch_lbTcamBist
(
    IN void
);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_LB_H_ */

