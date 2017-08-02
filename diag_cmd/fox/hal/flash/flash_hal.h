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
***      flash_hal.h
***
***    DESCRIPTION :
***      for flash hal 
***
***    HISTORY :
***       - 2008/12/09, 16:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __FLASH_HAL_H_
#define __FLASH_HAL_H_

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
#define FLASH_TEST_MTD_NUM        3

#define CFG_UBOOT_BASE_ADDRESS 0xf8000000 /* the same as DEVICE_SPI_BASE in mv_kw.h */
#define RESERVE_UBOOT_SIZE     0x100000   /* reserve size for uboot used */
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
 *      flash_halOpen
 *
 *  DESCRIPTION :
 *      open flash
 *
 *  INPUT :
 *      mtdNum - mtd number
 *
 *  OUTPUT :
 *      flashHandle - io device handle id
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
INT32 flash_halOpen
(
    IN UINT32 mtdNum,
    OUT INT32 *flashHandle
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halClose
 *
 *  DESCRIPTION :
 *      close flash handler
 *
 *  INPUT :
 *      flashHandle - io device handle id
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
INT32 flash_halClose
(
    IN INT32 flashHandle
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halErase
 *
 *  DESCRIPTION :
 *      erase flash memory
 *
 *  INPUT :
 *      bankNum - bank number
 *      firstBlock - first block number
 *      blockCount - how many block number
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
INT32 flash_halErase
(
	IN UINT32 bankNum,
	IN UINT32 firstBlock,
	IN UINT32 blockCount
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halBlockLenGet
 *
 *  DESCRIPTION :
 *      get block length which bank
 *
 *  INPUT :
 *      bankNum - bank number
 *      block - block number
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
UINT32 flash_halBlockLenGet
(
	IN UINT32 bankNum,
	IN UINT32 block
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halBlockAddrGet
 *
 *  DESCRIPTION :
 *      get block base address which bank
 *
 *  INPUT :
 *      bankNum - bank number
 *      block - block number
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
UINT32 flash_halBlockAddrGet
(
	IN UINT32 bankNum,
	IN UINT32 block
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halWrite
 *
 *  DESCRIPTION :
 *      write data to flash memory
 *
 *  INPUT :
 *      flashHandle - io device handle id
 *      writeAddr - write to which flash address
 *      inBuf - write data address
 *      len - write data length
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
INT32 flash_halWrite
(
    IN INT32 flashHandle,
	IN UINT32 writeAddr,
	IN UINT8 *inBuf,
	IN UINT32 len
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halRead
 *
 *  DESCRIPTION :
 *      read data to flash memory
 *
 *  INPUT :
 *      flashHandle - io device handle id
 *      readOffsetAddr - read to which flash address
 *      inBuf - write data address
 *      len - write data length
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
INT32 flash_halRead
(
    IN INT32 flashHandle,
    IN UINT32 readOffsetAddr,
    IN UINT8 **inBuf,
    IN UINT32 len
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halIsBootSector
 *
 *  DESCRIPTION :
 *      check block whether is boot sector
 *
 *  INPUT :
 *      bankNum - bank number
 *      firstBlock - first block number
 *      blockCount - how many block number
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
BOOL flash_halIsBootSector
(
    IN UINT32 bankNum,
    IN UINT32 firstBlock,
    IN UINT32 blockCount
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halTotalBlockGet
 *
 *  DESCRIPTION :
 *      get total sector number of mtd
 *
 *  INPUT :
 *      mtdNum - mtd number
 *
 *  OUTPUT :
 *      totalBLockNum - total sector number
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
INT32 flash_halTotalBlockNumGet
(
    IN UINT32 mtdNum,
    OUT UINT32 *totalBLockNum
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      flash_halCheckAddressValid
 *
 *  DESCRIPTION :
 *      check if the address is in the boot sector
 *
 *  INPUT :
 *      blockAddr
 *
 *  OUTPUT :
 *      none
 *
 *  RETURN :
 *      E_TYPE_SUCCESS  - valid
 *      Other - invalid
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 flash_halCheckAddressValid
(
	IN UINT32 blockAddr
);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_HAL_H_ */
