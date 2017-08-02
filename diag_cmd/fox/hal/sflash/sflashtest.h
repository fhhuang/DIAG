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
***
***
***    DESCRIPTION :
***
***
***************************************************************************/

#ifndef __SFLASHTEST_H_
#define __SFLASHTEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
INT32 sflashtest
(
    IN UINT32 bankNum,
    IN UINT32 firstBlock,
    IN UINT32 blockCount
);

#ifdef __cplusplus
}
#endif

#endif /* __FLASHTEST_H_ */
