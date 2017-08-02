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
***            intrtest.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/20, 10:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __INTRTEST_H_
#define __INTRTEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

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
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */
INT32 intrtest
(
    E_EXT_INTR intrSource,
    INT32         timeout
);


#ifdef __cplusplus
}
#endif

#endif /* __INTRTEST_H_ */
