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
***    HISTORY :
***       - 2008/12/09, 16:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __MEM_HAL_H_
#define __MEM_HAL_H_

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

/*==========================================================================
 *
 *      Constant Definition Segment
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
 *      mem_halRead
 *
 *  DESCRIPTION :
 *      Read memory
 *
 *  INPUT :
 *      addr - memory address
 *      size - bus width
 *		phy - is physical address
 *
 *  OUTPUT :
 *      data - address data
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
INT32 mem_halRead
(
    IN UINT32 addr,
    IN UINT32 dataSize,
    IN BOOL isPhy,
    OUT UINT32 *data
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mem_halWrite
 *
 *  DESCRIPTION :
 *      Write memory
 *
 *  INPUT :
 *      addr - memory address
 *      size - bus width
 *		phy - is physical address
 *      data - addr*ess data
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
INT32 mem_halWrite
(
    IN UINT32 addr,
    IN UINT32 dataSize,
    IN BOOL isPhy,
    IN UINT32 data
);


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mem_halDramSizeGet
 *
 *  DESCRIPTION :
 *      Get memory size
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      dramSize - memory size
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
INT32 mem_halDramSizeGet
(
    UINT32 *dramSize
);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_HAL_H_ */
