/***************************************************************************
***
***    Copyright 2015  Hon Hai Precision Ind. Co. Ltd.
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
***      cmd_eeprom.c
***
***    DESCRIPTION :
***      for eeprom dump/program
***
***    HISTORY :
***       - 2011/05/20, 11:28:52, Sean Hsu
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */

#include "foxCommand.h"

#include "cmn_type.h"
#include "porting.h"
#include "i2c_hal.h"
#include "err_type.h"
#include "log.h"
#include "sys_utils.h"

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
static INT32 fox_eepromDump
(
    IN UINT8 *raw_data,
    IN UINT32 length
)
{
    UINT32 i, j;
    UINT8 printStr[20];

    for(i=0; i < length ;)
    {
        if( (i%16) == 0 )
        {
            log_printf("%08x: ", i);
        }

        log_printf("%02x", *(raw_data+i));

        i++;

        if( (i%4) == 0 )
        {
            log_printf(" ");
        }

        if( (i%16) == 0 )
        {
            memset(printStr, 0, sizeof(printStr));
            memcpy(printStr, &(raw_data[i-16]), 16);

            for(j=0; j<16 ;j++)
            {
                /* print 0xff will occur erro(can't new line) */
                if( printStr[j] >= 0x7e || printStr[j] < 0x20 )
                    printStr[j] = '.';
            }

            log_printf("%s\n", printStr);
        }
    }

    log_printf("\n");
    return E_TYPE_SUCCESS;
}

INT32 fox_eeprom_write
(
    UINT32 offset,
    UINT8  *buffer,
    UINT32  cnt
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32  i =0;

    for ( i=0 ; i< cnt; i++)
    {
        if (i2c_halRegSet(SYSTEM_EEPROM_I2C_ADDR, (offset+i), 2, &buffer[i], 1) != E_TYPE_SUCCESS)
        {
            ret = E_TYPE_IO_ERROR;
        }
        udelay(10000);
    }

    return ret;
}

static INT32 fox_eeprom_clear
(
    UINT32 offset,
    UINT8  *buffer,
    UINT32 cnt
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  i =0;

    for ( i=0 ; i< cnt; i++)
    {
        if (i2c_halRegSet(SYSTEM_EEPROM_I2C_ADDR, (offset+i), 2, buffer, 1) != E_TYPE_SUCCESS)
        {
            ret = E_TYPE_IO_ERROR;
        }
        udelay(10000);
    }

    return ret;
}

INT32 utils_halStr2MacAddr
(
    IN UINT8 *macStr,
    OUT UINT8 *macAddr
)
{
    UINT32 i;
    UINT32 j;
    UINT8 *pStrMac = macStr;
    UINT8 mac;


    /* Remove ':' from MAC affress */
    UINT8 enet_addr[6*2+1];

    for(i=0,j=0; i < sizeof(enet_addr) ;j++ )
    {
        if( pStrMac[j] != ':' )
        {
            enet_addr[i] = pStrMac[j];
            i++;
        }
    }

    for(i=0; i < 6 ;i++)
    {
        mac = enet_addr[i*2];
        if( (mac >= '0') && (mac <= '9') )
            mac = mac - '0';
        else if( (mac >= 'a') && (mac <= 'f') )
            mac = ( mac - 'a' + 10 );
        else if( (mac >= 'A') && (mac <= 'F') )
            mac = ( mac - 'A' + 10 );
        macAddr[i] = mac << 4;

        mac = enet_addr[i*2+1];
        if( (mac >= '0') && (mac <= '9') )
            mac = mac - '0';
        else if( (mac >= 'a') && (mac <= 'f') )
            mac = ( mac - 'a' + 10 );
        else if( (mac >= 'A') && (mac <= 'F') )
            mac = ( mac - 'A' + 10 );
        macAddr[i] += mac;
    }
    return E_TYPE_SUCCESS;
}

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
INT32 do_eeprom_info
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    UINT8               buf[2]={0}, *dump_buf=NULL;
    UINT32              startOffset=0, endOffset=0, size = 0;
    INT32               ret = E_TYPE_INVALID_CMD_FORMAT;

    if(argc == 1)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }

    if ( strcmp(argv[1], "dump") == 0 )
    {
        if(argc != 4)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        startOffset = simple_strtoul(argv[2], NULL, 16);
        size = simple_strtoul(argv[3], NULL, 16);
        dump_buf = (UINT8 *)malloc(size);
        if (!dump_buf)
        {
            log_printf("Malloc 0x%x bytes fail.\n", size);
            return E_TYPE_ALLOC_MEM_FAIL;
        }
        memset(dump_buf, 0x00, size);

        ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, startOffset, 2, dump_buf, size);
        if (ret != E_TYPE_SUCCESS)
       {
           log_printf("Get main EERPOM contents fail!\n");
           free(dump_buf);
           return E_TYPE_REG_READ;
       }
       log_printf("Dump EEPROM content\n");
       fox_eepromDump(dump_buf, size);
       free(dump_buf);
    }
    else if ( strcmp(argv[1], "show") == 0 )
    {
        ret = hal_pcamapShow(argv[2], NULL);
    }
    else if ( strcmp(argv[1], "edit") == 0 )
    {
        ret = hal_pcamapProgram(argv);
    }
    else if ( strcmp(argv[1], "clear_to_0") == 0 )
    {
        if(argc != 4)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        startOffset = simple_strtoul(argv[2], NULL, 16);
        endOffset = simple_strtoul(argv[3], NULL, 16);
        buf[0] = 0x0;
        log_printf("Clear eeprom to 0 from 0x%x to 0x%x\n", startOffset, endOffset);

        ret = fox_eeprom_clear(startOffset, &buf[0], endOffset - startOffset +1);
        if ( ret != E_TYPE_SUCCESS)
        {
            log_printf("Clear EEPROM from 0x%x to 0x%x failed\n", startOffset, endOffset);
            return E_TYPE_INVALID_DATA;
        }
    }
    else if( strcmp(argv[1], "clear_to_ff") == 0 )
    {
        if(argc != 4)
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        startOffset = simple_strtoul(argv[2], NULL, 16);
        endOffset = simple_strtoul(argv[3], NULL, 16);
        buf[0] = 0xff;
        log_printf("Clear eeprom to ff from 0x%x to 0x%x\n", startOffset, endOffset);

        ret = fox_eeprom_clear(startOffset, &buf[0], endOffset - startOffset + 1);
        if ( ret != E_TYPE_SUCCESS)
        {
            log_printf("Clear EEPROM from 0x%x to 0x%x failed\n", startOffset, endOffset);
            return E_TYPE_INVALID_DATA;
        }
    }
    else
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
    }

    log_printf("\n");

    return ret;

__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    eeprom,    CONFIG_SYS_MAXARGS,    1,  do_eeprom_info,
        "eeprom \t\t- EEPROM dump or program command\n",
        "<dump|clear_to_0|clear_to_ff|show|edit>]\n"
        "  - dump: dump <startOffset> <size>.\n"
        "  - clear_to_0: clear_to_0 <startOffset> <endOffset>.\n"
        "  - clear_to_ff: clear_to_ff <startOffset> <endOffset>.\n"
        "  - show|edit: show|edit <types> <value>.Valid types are <version\n"
        "  - .......... |mac_addr|mb_an|ps_pn|mb_sn|ps_sn|model_rn|mb_rn\n"
        "  - .......... |model_num|system_sn|db_an|db_sn|sfpb_apn|sfpb_rn\n"
        "  - .......... |sfpb_sn|top_apn|top_arn|ver_id|code_num|db_rn|mfg_cert\n"
        "  - .......... |sps_pn|sps_sn|board_rev|cfg_model>\n"
);


