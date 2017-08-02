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
***      sflashtest.c
***
***    DESCRIPTION :
***      for SPI flash test
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#ifndef FOX_SFLASH_SUPPORT
#define FOX_SFLASH_SUPPORT 1
#endif

#ifdef FOX_SFLASH_SUPPORT
#include "cmn_type.h"
#include "porting.h"
#include "err_type.h"
#include "log.h"
#include "sflashtest.h"
#include "sflash_hal.h"

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

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
INT32 sflashtest
(
    IN UINT32 bankNum,
    IN UINT32 firstBlock,
    IN UINT32 blockCount
)
{
    INT32 ret=E_TYPE_SUCCESS;
    INT32 flashHandle;
    UINT32 totalBlocks;
    INT32 realCnt;
    UINT32 sectorSize;
    UINT8 *pBufAddr,*pReadBuf;
    UINT32 i,j;
    UINT32 startAddr;

    /* Open SFLASH Device and Check bankNum is valid*/
    if( (ret=sflash_halOpen(bankNum, &flashHandle)) < E_TYPE_SUCCESS )
    {
        return ret;
    }

    if( (ret=sflash_halSectorSizeGet(flashHandle,&sectorSize)) < E_TYPE_SUCCESS )
    {
        return E_TYPE_OUT_TEST_RANGE;
    }

    if(!sectorSize)
    {
        return E_TYPE_OUT_TEST_RANGE;
    }

    if( (ret=sflash_halTotalBlockNumGet(flashHandle, &totalBlocks)) < E_TYPE_SUCCESS )
    {
        return ret;
    }

    if(firstBlock >=totalBlocks)
    {
        return E_TYPE_OUT_TEST_RANGE;
    }

    if(!blockCount)
    {
        realCnt=totalBlocks-firstBlock;
    }
    else 
    {
        if( (firstBlock+blockCount)>=totalBlocks)
        {
            realCnt=totalBlocks-firstBlock;
        }
        else
        {
            realCnt=blockCount;
        }
    }

    pBufAddr=sflash_halWriteBufGet(sectorSize);
    pReadBuf=sflash_halReadBufGet(sectorSize);

    if( (!pBufAddr) || (!pReadBuf))
    {
        sflash_halFreeBuf(pBufAddr);
        sflash_halFreeBuf(pReadBuf);
        sflash_halClose(&flashHandle);
        return E_TYPE_ALLOC_MEM_FAIL;
    }

    sflash_halPattern(pBufAddr, sectorSize);
    
    for(i=firstBlock;i<(firstBlock+realCnt);i++)
    {
        startAddr=sectorSize*i;

        if( sflash_halIsReserveSector(flashHandle, i)==TRUE)
        {
            continue;
        }

        if( (ret=sflash_halIsBad(flashHandle, startAddr)) <0)
        {
            continue;
        }

        if( (ret=sflash_halErase(flashHandle, startAddr)) <0)
        {
            continue;
        }

        if( (ret=sflash_halWrite(flashHandle, startAddr,pBufAddr,sectorSize)) <0)
        {
            sflash_halFreeBuf(pBufAddr);
            sflash_halFreeBuf(pReadBuf);
            sflash_halClose(&flashHandle);
            return E_TYPE_OUT_SPEC;
        }

        if( (ret=sflash_halRead(flashHandle, startAddr,pReadBuf,sectorSize)) <0)
        {
            sflash_halFreeBuf(pBufAddr);
            sflash_halFreeBuf(pReadBuf);
            sflash_halClose(&flashHandle);
            return E_TYPE_OUT_SPEC;
        }

        for(j=0;j<sectorSize;j++)
        {
            if( *(pBufAddr+j) != *(pReadBuf+j))
            {
                log_printf("FLASH comapre fail at %x \n",startAddr+j);
                sflash_halFreeBuf(pBufAddr);
                sflash_halFreeBuf(pReadBuf);
                sflash_halClose(&flashHandle);
                return E_TYPE_OUT_SPEC;
            }
        }

        log_printf("FLASH test OK - offset 0x%8.8X , sector %d \n", startAddr, i );
    }

    sflash_halFreeBuf(pBufAddr);
    sflash_halFreeBuf(pReadBuf);
    sflash_halClose(&flashHandle);
    return E_TYPE_SUCCESS;  
}
#endif


