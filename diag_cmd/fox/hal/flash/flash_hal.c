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
***      flash_hal.c
***
***    DESCRIPTION :
***      for flash hal 
***
***    HISTORY :
***       - 2009/03/04, 13:30:52, Eden Weng
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
#include "err_type.h"
#include "porting.h"
#include "log.h"
#include "flash_hal.h"

#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define MTD_DEV "/dev/mtd%d"
extern UINT32 flashIndex;
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
 *      Local Function segment
 *
 *==========================================================================
 */
static INT32 halMtdBlockInfoGet
(
    IN INT32 handle,
    OUT mtd_info_t *pInfo
)
{
    int ret = E_TYPE_SUCCESS;
    ret = ioctl(handle, MEMGETINFO, pInfo);
    if(ret < E_TYPE_SUCCESS){
        log_printf("MEMGETINFO IOCTL faild");
    }
    return ret;
}

static INT32 flash_halUnprotectAll()
{
    UINT32 i, p, bank;
    flash_info_t *pFlashInfo;
#if defined(CFG_FLASH_PROTECTION)
    UINT32 rcode=0;
#endif
    
    /* unlock protect flash */
    p= 0; 
    for (bank=1; bank<=CFG_MAX_FLASH_BANKS; ++bank) 
    {
        pFlashInfo = &flash_info[bank-1];
        if (pFlashInfo->flash_id == FLASH_UNKNOWN) 
        {
            continue;
        }

        for (i=0; i<pFlashInfo->sector_count; ++i) 
        {
#if defined(CFG_FLASH_PROTECTION)
                if (flash_real_protect(pFlashInfo, i, p))
                    rcode = 1;
#else
                pFlashInfo->protect[i] = p;
#endif    /* CFG_FLASH_PROTECTION */
        }
    }
    
    return E_TYPE_SUCCESS;
}

/*==========================================================================
 *
 *      External Funtion segment
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
)
{
    char devName[20];
    
    sprintf(devName, MTD_DEV, mtdNum);
    
    *flashHandle = open(devName, O_RDWR);
    
    if( *flashHandle == -1 )
    {
        log_printf("Could not open MTD%d", mtdNum);
        return E_TYPE_UNKNOWN_DEV;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    if (close(flashHandle) < 0)
    {
        log_printf("Failed to close flash region\n");
        return E_TYPE_IO_ERROR;
    }

    return E_TYPE_SUCCESS;
}

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
)
{
    mtd_info_t mtdBlockInfo;
    erase_info_t erase_info;
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 totalSector = 0;
		
    ret = halMtdBlockInfoGet((INT32)bankNum, &mtdBlockInfo);
    if(ret < 0)
    {
        log_printf("Get flash sector info failed\n");
        return E_TYPE_UNKNOWN_DEV;
    }
	
    if( !mtdBlockInfo.size || !mtdBlockInfo.erasesize )
    {
        log_printf("Get flash sector info failed\n");
        return E_TYPE_IO_ERROR;
}

    totalSector = mtdBlockInfo.size/mtdBlockInfo.erasesize;

    if( ((firstBlock+blockCount)-1) > totalSector )
        return E_TYPE_INVALID_PARA;

    erase_info.length = mtdBlockInfo.erasesize*blockCount;
    erase_info.start = mtdBlockInfo.erasesize*(firstBlock-1);
    
    if( erase_info.length > mtdBlockInfo.size )
{
        printf("Erase total length > mtd size\n");
        return E_TYPE_INVALID_PARA;
    }
	
    ret = ioctl(bankNum, MEMERASE, &erase_info);
    if(ret < 0)
    {
        log_printf("MEMERASE IOCTL set faild\n");
        return E_TYPE_IO_ERROR;
    }
	
    return E_TYPE_SUCCESS;
}

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
)
{
    mtd_info_t mtdBlockInfo;
    INT32 ret = E_TYPE_SUCCESS;
    
    ret = halMtdBlockInfoGet((INT32)bankNum, &mtdBlockInfo);
    if(ret < 0)
    {
        log_printf("Get flash sector info failed\n");
        return E_TYPE_UNKNOWN_DEV;
    }
    
    return mtdBlockInfo.erasesize;
}

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
)
{
    return 0;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 write_count = 0;
    
    if( flashHandle | writeAddr| write_count ) ;
    
    write_count = write(flashHandle, inBuf, len);
    if(write_count != len)
    {
        ret = E_TYPE_IO_ERROR;
    }

    return E_TYPE_SUCCESS;
}

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
    IN UINT8 **outBuf,
    IN UINT32 len
)
{
    INT32 ret = E_TYPE_SUCCESS;
    
    UINT32 read_count = 0;
    
    if( readOffsetAddr ) ;
    
    read_count = read(flashHandle, *outBuf, len);
    if(read_count != len)
    {
        ret = E_TYPE_IO_ERROR;
    }

    return ret;
}

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
)
{
    if( (bankNum<3) || (bankNum>5) )
    {
        log_printf("Can't test MTD%d\n", bankNum);
        return TRUE;
    }

    return FALSE;
}

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
)
{
    INT32 ret = E_TYPE_SUCCESS;
    
    mtd_info_t mtdBlockInfo;
    INT32 retValue = E_TYPE_SUCCESS;
    UINT32 totalSector = 0;
    INT32 flashHandle;
    
    if( (ret=flash_halOpen(mtdNum, &flashHandle)) < E_TYPE_SUCCESS )
    {
        return ret;
    }
    
    ret = halMtdBlockInfoGet(flashHandle, &mtdBlockInfo);
    if(ret < 0)
    {
        log_printf("Get flash sector info failed\n");
        ret = E_TYPE_UNKNOWN_DEV;
        goto __FUN_RET;
    }
    
    if( !mtdBlockInfo.size || !mtdBlockInfo.erasesize )
    {
        log_printf("Get flash sector info failed\n");
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }
    
    *totalBLockNum = mtdBlockInfo.size/mtdBlockInfo.erasesize;    

__FUN_RET:
    if( (retValue=flash_halClose(flashHandle)) < E_TYPE_SUCCESS )
    {
        return retValue;
    }

    return ret;
}

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
)
{

    if (flashIndex ==1 ) /*skip the check for the 2nd flash boot sector */
    {    
        if ( blockAddr <= ( CFG_UBOOT_BASE_ADDRESS + RESERVE_UBOOT_SIZE ))
            return FALSE;
    }		
	
    return TRUE;
}
