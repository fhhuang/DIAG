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

#ifndef __SFLASH_HAL_H_
#define __SFLASH_HAL_H_

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
#ifndef CONFIG_SYS_MAX_SFLASH_DEVICE
#define CONFIG_SYS_MAX_SFLASH_DEVICE 1
#endif

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


INT32 sflash_halOpen
(
    IN INT32 mtdNum,
    OUT INT32 *flashHandle
);


INT32 sflash_halClose
(
    IN INT32 *flashHandle
);

INT32 sflash_halIsBad
(
    IN UINT32 flashHandle,
    IN UINT32 Offset
);


INT32 sflash_halErase
(
    IN INT32 flashHandle,
    IN UINT32 eraseOffset
);


UINT32 sflash_halSectorSizeGet
(
    IN INT32 flashHandle,
    OUT UINT32 *sectorSize
);


INT32 sflash_halWrite
(
    IN INT32 	flashHandle,
    IN UINT32 	writeOffset,
    IN UINT8 	*inBuf,
    IN UINT32 	len
);

INT32 sflash_halRead
(
    IN INT32 flashHandle,
    IN UINT32 readOffset,
    IN UINT8 *outBuf,
    IN UINT32 len
);

BOOL sflash_halIsReserveSector
(
    IN INT32 flashHandle,
    IN UINT32 Block
);


INT32 sflash_halTotalBlockNumGet
(
    IN INT32 flashHandle,
    OUT UINT32 *totalBLockNum
);



UINT8 *sflash_halWriteBufGet
(
    IN UINT32 len
);

UINT8 *sflash_halReadBufGet
(
    IN UINT32 len
);

void sflash_halFreeBuf
(
    IN UINT8 *freeBuf
);

void sflash_halPattern
(
    IN UINT8 *inBuf,
    IN UINT32 len
);


#ifdef __cplusplus
}
#endif

#endif /* __FLASH_HAL_H_ */














