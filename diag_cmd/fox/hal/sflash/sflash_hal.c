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
***      sflash_hal.c
***
***    DESCRIPTION :
***      for sflash HAL
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

#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>

typedef struct mtdTable_s{
    int index;
    int fd;
    struct mtd_info_user *meminfo;
    char *name;
}mtdTable_t;

struct mtd_info_user Gmeminfo[10];

mtdTable_t mtdlookup[]={
    {1,-1, &Gmeminfo[0],"/dev/mtd3"},
    {-1,-1,0,NULL}
};

struct mtd_info_user * searchMtdInfo(IN INT32 flashHandle)
{
    mtdTable_t *pmtdInfo;
    for(pmtdInfo = &mtdlookup[0] ;(pmtdInfo&&pmtdInfo->index !=-1);pmtdInfo++)
    {
        if(pmtdInfo->fd == flashHandle)
            return pmtdInfo->meminfo;
    }   

    return NULL;
}

INT32 sflash_halOpen
(
    IN INT32 mtdNum,
    OUT INT32 *flashHandle
)
{
    mtdTable_t *pmtdInfo;
    struct mtd_ecc_stats oldstats;
    for(pmtdInfo = &mtdlookup[0] ;(pmtdInfo&&pmtdInfo->index !=-1);pmtdInfo++)
    {
        if(pmtdInfo->index == mtdNum)
        {

            /* open device */
            pmtdInfo->fd= open(pmtdInfo->name, O_RDWR);
            if(pmtdInfo->fd<0)
                goto sflash_open_err;

        /* get mtd meminfo */
        if (ioctl(pmtdInfo->fd, MEMGETINFO, pmtdInfo->meminfo)) 
            {
            close(pmtdInfo->fd);
            pmtdInfo->fd=-1;
            goto sflash_open_err;
            }
            ioctl(pmtdInfo->fd, ECCGETSTATS, &oldstats);
            *flashHandle=pmtdInfo->fd;
            return E_TYPE_SUCCESS;

        }
    }


sflash_open_err:
    *flashHandle=-1;
    return E_TYPE_OUT_SPEC;
}

INT32 sflash_halClose
(
    IN INT32 *flashHandle
)
{
    if(*flashHandle != -1)
        close(*flashHandle);
    *flashHandle=-1;
    return E_TYPE_SUCCESS;
}


UINT32 sflash_halSectorSizeGet
(
    IN INT32 flashHandle,
    OUT UINT32 *sectorSize
){
    struct mtd_info_user *pmeminfo;

    if(flashHandle<0)
        return E_TYPE_OUT_SPEC;

    pmeminfo=searchMtdInfo(flashHandle);
    if(pmeminfo)
    {
        *sectorSize=pmeminfo->erasesize;
        return E_TYPE_SUCCESS;
    }

    *sectorSize=0;
    return E_TYPE_OUT_SPEC;
}

INT32 sflash_halTotalBlockNumGet
(
    IN INT32 flashHandle,
    OUT UINT32 *totalBLockNum
)
{
    struct mtd_info_user *pmeminfo;

    if(flashHandle<0)
        return E_TYPE_OUT_SPEC;

    pmeminfo=searchMtdInfo(flashHandle);
    if(pmeminfo)
    {
        *totalBLockNum=pmeminfo->size/pmeminfo->erasesize;
        return E_TYPE_SUCCESS;
    }

    *totalBLockNum=0;
    return E_TYPE_OUT_SPEC;
}


BOOL sflash_halIsReserveSector
(
    IN INT32 flashHandle,
    IN UINT32 Block
)
{
    return FALSE;
}

INT32 sflash_halIsBad
(
    IN UINT32 flashHandle,
    IN UINT32 Offset
)
{
    loff_t test_ofs=Offset;

    if(flashHandle<0)
        return E_TYPE_OUT_SPEC;

    if (ioctl(flashHandle, MEMGETBADBLOCK, &test_ofs)) 
    {
        log_printf("\rSkipping bad block at  "
                "0x%8.8x        "
                "                         \n",Offset);
        return E_TYPE_OUT_SPEC;
    }

    return E_TYPE_SUCCESS;
}


INT32 sflash_halErase
(
    IN INT32 flashHandle,
    IN UINT32 eraseOffset
)
{
    struct erase_info_user er;
    UINT32 sector;
    int ret;

    if(flashHandle<0)
        return E_TYPE_OUT_SPEC;

    sflash_halSectorSizeGet(flashHandle,&sector);

    er.start = eraseOffset;
    er.length = sector;
    
    if (ioctl(flashHandle, MEMERASE, &er)) 
    {
        log_printf("\nMTD Erase failure: %d\n",ret);
        return E_TYPE_OUT_SPEC;
    }
    return E_TYPE_SUCCESS;
}

INT32 sflash_halWrite
(
    IN INT32    flashHandle,
    IN UINT32   writeOffset,
    IN UINT8    *inBuf,
    IN UINT32   len
)
{
    int ret;
    UINT32 writelen;
    
    fflush(stdout);

    writelen = pwrite(flashHandle, inBuf, len, writeOffset);
    if (writelen < 0) 
    {
        log_printf ("SPI write to offset %x failed %d\n",writeOffset, ret);
        return E_TYPE_OUT_SPEC;
    }
    
    if (writelen < len) 
    {
        log_printf ("SPI write to offset %x failed %d\n",writeOffset, ret);
        return E_TYPE_OUT_SPEC;
    }

    return E_TYPE_SUCCESS;
}

INT32 sflash_halRead
(
    IN INT32 flashHandle,
    IN UINT32 readOffset,
    IN UINT8 *outBuf,
    IN UINT32 len
)
{
    int ret;

    UINT32 readlen;

    fflush(stdout);

    readlen = pread(flashHandle, outBuf, len, readOffset);
    if (readlen < len) 
    {
        log_printf ("NAND read to offset %x failed %d\n",readOffset, ret);
        return E_TYPE_OUT_SPEC;
    }

    return E_TYPE_SUCCESS;
}


UINT8 *sflash_halWriteBufGet
(
    IN UINT32 len
)
{
    return (UINT8 *)malloc(len);
}

UINT8 *sflash_halReadBufGet
(
    IN UINT32 len
)
{
    return (UINT8 *)malloc(len);
}


void sflash_halFreeBuf
(
    IN UINT8 *freeBuf
)
{
    if(freeBuf)
        free(freeBuf);
    return;
}

void sflash_halPattern
(
    IN UINT8 *inBuf,
    IN UINT32 len
)
{
    INT32 i;
    INT8 tmp;

    if(inBuf==NULL)
        return;
    
    srand(time(NULL));  

    for(i=0;i<len;i++)
        inBuf[i]=rand();

    return;
}
