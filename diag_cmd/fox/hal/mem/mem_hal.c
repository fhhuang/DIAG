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
***      mem_hal.c
***
***    DESCRIPTION :
***      for mem ahl
***
***    HISTORY :
***       - 2009/11/06, 13:30:52, Eden Weng
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
#include "mem_hal.h"

#include <fcntl.h>
#include <stdarg.h>
#include <sys/mman.h>
#include "memtest.h"

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
 *      Local Function segment
 *
 *==========================================================================
 */
/*==========================================================================
 *
 *      External Funtion segment
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
)
{
    volatile UINT8 *pu8Addr;
    UINT8 u8Data;
    volatile UINT16 *pu16Addr;
    UINT16 u16Data;
    volatile UINT32 *pu32Addr;
    UINT32 u32Data;
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 tempAddr=addr;
    UINT32 offset=0;
    
    UINT32 tempSize=0;
    int fd=0;
    
    if( isPhy == TRUE )
    {
        // Map address into kernel space
        /* Must be O_SYNC, non cache */
        if( (fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1 )
            return E_TYPE_IO_ERROR;

        tempSize = getpagesize();
        tempAddr = (UINT32)mmap(0, tempSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr&(~(tempSize-1)));
        if( tempAddr == 0xffffffff )
        {
            close(fd);
            return E_TYPE_UNKNOWN_IO;
        }
        
        offset = addr - (addr&(~(tempSize-1)));
    }

    switch (dataSize)
    {
        case 8:
            pu8Addr = (volatile UINT8*)((UINT8*)tempAddr+offset);
            u8Data = *pu8Addr;
            *data = (UINT8)u8Data;
            break;
        case 16:
            pu16Addr = (volatile UINT16*)((UINT8*)tempAddr+offset);
            u16Data = *pu16Addr;
             *data = (UINT16)u16Data;
            break;
        case 32:
            pu32Addr = (volatile UINT32*)((UINT8*)tempAddr+offset);
            u32Data = *pu32Addr;
             *data = (UINT32)u32Data;
            break;
        default:
            ret = E_TYPE_INVALID_PARA;
    }

    if( isPhy == TRUE )
    {
        munmap((void*)tempAddr, tempSize);
        
        close(fd);
    }

    return ret;
}

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
)
{
    volatile UINT8 *pu8Addr;
    UINT8 u8Data;
    volatile UINT16 *pu16Addr;
    UINT16 u16Data;
    volatile UINT32 *pu32Addr;
    UINT32 u32Data;
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 tempAddr=addr;
    UINT32 offset = 0;
    
    UINT32 tempSize=0;
    int fd=0;
    
    if( isPhy == TRUE )
    {
        // Map address into kernel space
        /* Must be O_SYNC, non cache */
        if( (fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1 )
            return E_TYPE_IO_ERROR;

        tempSize = getpagesize();
        tempAddr = (UINT32)mmap(0, tempSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr&(~(tempSize-1)));
        if( tempAddr == 0xffffffff )
        {
            close(fd);
            return E_TYPE_UNKNOWN_IO;
        }
        
        offset = addr - (addr&(~(tempSize-1)));
    }

    switch (dataSize)
    {
        case 8:
            pu8Addr = (volatile UINT8*)((UINT8*)tempAddr+offset);
            u8Data = (UINT8)data;
            *pu8Addr = u8Data;
            break;
        case 16:
            pu16Addr = (volatile UINT16*)((UINT8*)tempAddr+offset);
            u16Data = (UINT16)data;
            *pu16Addr = u16Data;
            break;
        case 32:
            pu32Addr = (volatile UINT32*)((UINT8*)tempAddr+offset);
            u32Data = (UINT32)data;
            *pu32Addr = u32Data;
            break;
        default:
            ret = E_TYPE_INVALID_PARA;
    }
    
    if( isPhy == TRUE )
    {
        munmap((void*)tempAddr, tempSize);
        
        close(fd);
    }

    return ret;
}

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
)
{
    *dramSize = MEM_TEST_SIZE;
    if( *dramSize == 0 )
        return E_TYPE_INVALID_DATA;

    return E_TYPE_SUCCESS;;
}

