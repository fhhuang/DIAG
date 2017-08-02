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
***      nand_hal.c
***
***    DESCRIPTION :
***      for nand HAL
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
#include "log.h"

#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>
#include "porting.h"

typedef struct mtdTable_s{
	int index;
	int fd;
	struct mtd_info_user *meminfo;
	char *name;
}mtdTable_t;

static struct mtd_info_user Gmeminfo[10];

static mtdTable_t mtdlookup[]={
	{1,-1, &Gmeminfo[0],"/dev/mtd5"},  /* SPI boot, NAND flash */ 
	{2,-1, &Gmeminfo[1],"/dev/mtd2"},	 /* NAND boot flash */
	{-1,-1,0,NULL}
};

static struct mtd_info_user * searchMtdInfo(IN INT32 flashHandle)
{
    mtdTable_t *pmtdInfo;
    for(pmtdInfo = &mtdlookup[0] ;(pmtdInfo&&pmtdInfo->index !=-1);pmtdInfo++)
    {
        if(pmtdInfo->fd == flashHandle)
            return pmtdInfo->meminfo;
    }	

    return NULL;
}

/* Function Name: nand_halFindNANDTest
 * find nand_test mtd partition 
 * Return: mtdName
 */
INT32 nand_halFindNANDTest(OUT UINT8 *mtdName)
{
    FILE *fd;
    UINT8 buf[10];
    UINT32 len = 0;
    UINT32 ret = E_TYPE_SUCCESS;
    
    fd = popen("cat /proc/mtd | grep nand_test | cut -c 1-4", "r");
    if(fd ==NULL)
        return E_TYPE_IO_ERROR;

   /* check mtd name if match "nand_test" */
   if(fgets(buf, sizeof(buf), fd) != NULL) 
   {
        log_dbgPrintf("Find nand_test mtd partition is %s\n", buf);
        strcpy(mtdName,buf);
        /* remove the \n */
        len = strlen(mtdName);
        if (mtdName[len - 1] == '\n')
        	mtdName[len - 1] = '\0';
   }
   else
   {
        /* no found nand_test partition */
        mtdName[0] = '\0';
        ret =  E_TYPE_DEV_UNINSTALLED;
   }
   
   pclose(fd);
   return ret;
}

INT32 nand_halOpen
(
    IN INT32 mtdNum,
    OUT INT32 *flashHandle
)
{
    mtdTable_t *pmtdInfo;
    struct mtd_ecc_stats oldstats;
    UINT8 buf[10], mtdbuf[10];
    UINT32 ret = E_TYPE_SUCCESS;
    mtdTable_t mtdlookupTmp[1] = {0};
	
    /* find /proc/mtd if match "nand_test" device */	
    ret = nand_halFindNANDTest(buf);

    /* if find mtdtest name, replace the first mtdlookup device  */	
    if (ret == E_TYPE_SUCCESS);
    {
        sprintf(mtdbuf, "/dev/%s", buf);  	
        pmtdInfo = &mtdlookup[0];

        /* if mtd5 is not matched, swapped to mtd2 for (NAND_BOOT) board */ 
        strcpy(buf, pmtdInfo->name);
        if(strcmp(buf, mtdbuf) != 0)
        {
            /* swapped between mtdlookup mtd5 and mtd2 (NAND_BOOT) */
            memset(&mtdlookupTmp[0], 0, sizeof(mtdTable_t));
            memcpy(&mtdlookupTmp[0], &mtdlookup[0], sizeof(mtdTable_t));
            memcpy(&mtdlookup[0], &mtdlookup[1], sizeof(mtdTable_t));
            memcpy(&mtdlookup[1], &mtdlookupTmp[0], sizeof(mtdTable_t));
            mtdlookup[0].index = 1;
            mtdlookup[1].index = 2;
		 
            log_dbgPrintf("nand_halOpen, mtd_test mismatched %s, %s\n", buf, mtdbuf);	
        } 
		 
	 log_dbgPrintf("nand_halOpen: pmtdInfo->name=%s from new mtfbuf=%s, mtdlookup=%s\n", \
	                           pmtdInfo->name, mtdbuf, mtdlookup[0].name);	
    }
    
    for(pmtdInfo = &mtdlookup[0] ;(pmtdInfo&&pmtdInfo->index !=-1);pmtdInfo++)
    {
        if(pmtdInfo->index == mtdNum)
        {
            log_dbgPrintf("nand_halOpen: open mtdNum device -> pmtdInfo->name=%s\n", \
                                  pmtdInfo->name);
            /* open device */
            pmtdInfo->fd= open(pmtdInfo->name, O_RDWR);
            if(pmtdInfo->fd<0)
            {
                goto nand_open_err;
            }

            /* get mtd meminfo */
            if (ioctl(pmtdInfo->fd, MEMGETINFO, pmtdInfo->meminfo)) 
            {
    		    close(pmtdInfo->fd);
    		    pmtdInfo->fd=-1;
    		    goto nand_open_err;
            }
			
            ioctl(pmtdInfo->fd, ECCGETSTATS, &oldstats);
            *flashHandle=pmtdInfo->fd;
            return E_TYPE_SUCCESS;
        }
    }

nand_open_err:
    *flashHandle=-1;
    return E_TYPE_OUT_SPEC;
}

INT32 nand_halClose
(
    IN INT32 *flashHandle
)
{
    if(*flashHandle != -1)
        close(*flashHandle);
    *flashHandle=-1;
    return E_TYPE_SUCCESS;
}


UINT32 nand_halSectorSizeGet
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

INT32 nand_halTotalBlockNumGet
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


BOOL nand_halIsReserveSector
(
    IN INT32 flashHandle,
    IN UINT32 Block
)
{
    return FALSE;
}

INT32 nand_halIsBad
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


INT32 nand_halErase
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

    nand_halSectorSizeGet(flashHandle,&sector);

    er.start = eraseOffset;
    er.length = sector;
	
    if (ioctl(flashHandle, MEMERASE, &er)) 
    {
        log_printf("\nMTD Erase failure: %d\n",ret);
        return E_TYPE_OUT_SPEC;
    }
    return E_TYPE_SUCCESS;
}

INT32 nand_halWrite
(
    IN INT32 	flashHandle,
    IN UINT32 	writeOffset,
    IN UINT8 	*inBuf,
    IN UINT32 	len
)
{
    int ret;
    UINT32 writelen;
    
    fflush(stdout);

    writelen = pwrite(flashHandle, inBuf, len, writeOffset);
    if (writelen < 0) 
    {
        log_printf ("NAND write to offset %x failed %d\n",writeOffset, ret);
        return E_TYPE_OUT_SPEC;
    }
	
    if (writelen < len) 
    {
        log_printf ("NAND write to offset %x failed %d\n",writeOffset, ret);
        return E_TYPE_OUT_SPEC;
    }

    return E_TYPE_SUCCESS;
}

INT32 nand_halRead
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


UINT8 *nand_halWriteBufGet
(
    IN UINT32 len
)
{
    return (UINT8 *)malloc(len);
}

UINT8 *nand_halReadBufGet
(
    IN UINT32 len
)
{
    return (UINT8 *)malloc(len);
}


void nand_halFreeBuf
(
    IN UINT8 *freeBuf
)
{
    if(freeBuf)
        free(freeBuf);
    return;
}
void nand_halPattern
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




