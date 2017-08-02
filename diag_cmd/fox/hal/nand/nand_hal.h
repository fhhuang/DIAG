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
***      nand_hal.h
***
***    DESCRIPTION :
***      for nand hal 
***
***************************************************************************/

#ifndef __NAND_HAL_H_
#define __NAND_HAL_H_

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
INT32 nand_halOpen
(
    IN INT32 mtdNum,
    OUT INT32 *flashHandle
);

INT32 nand_halClose
(
    IN INT32 *flashHandle
);

INT32 nand_halIsBad
(
    IN UINT32 flashHandle,
    IN UINT32 Offset
);

INT32 nand_halErase
(
    IN INT32 flashHandle,
    IN UINT32 eraseOffset
);

UINT32 nand_halSectorSizeGet
(
    IN INT32 flashHandle,
    OUT UINT32 *sectorSize
);

INT32 nand_halWrite
(
    IN INT32 	flashHandle,
    IN UINT32 	writeOffset,
    IN UINT8 	*inBuf,
    IN UINT32 	len
);

INT32 nand_halRead
(
    IN INT32 flashHandle,
    IN UINT32 readOffset,
    IN UINT8 *outBuf,
    IN UINT32 len
);

BOOL nand_halIsReserveSector
(
    IN INT32 flashHandle,
    IN UINT32 Block
);

INT32 nand_halTotalBlockNumGet
(
    IN INT32 flashHandle,
    OUT UINT32 *totalBLockNum
);

UINT8 *nand_halWriteBufGet
(
    IN UINT32 len
);

UINT8 *nand_halReadBufGet
(
    IN UINT32 len
);

void nand_halFreeBuf
(
    IN UINT8 *freeBuf
);

void nand_halPattern
(
    IN UINT8 *inBuf,
    IN UINT32 len
);

/* Function Name: nand_halFindNANDTest 
 * find nand_test mtd partition  
 * Return: mtdName 
 */
INT32 nand_halFindNANDTest
(
    OUT UINT8 *mtdName
);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_HAL_H_ */
