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
***      hal_pcamap.c
***
***    DESCRIPTION :
***      for pcamap test
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
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
#include "sys_utils.h"
#include "err_type.h"
#include "porting.h"
#include "hal_pcamap.h"
#include "i2c_hal.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if_arp.h>
#include "log.h"
#include "foxCommand.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

//extern UINT16 const crc16_table[256];
//extern _pcamap_field const pcamap_field[];

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

/*
* Precondition the CRC accumulator with all ones. Then loop through
* each byte of the data. Each nibble is handled individually by XORing
* it with the current accumulator and using the result as an index into
* the CRC table. The value found in the table is XORed with the current
* CRC accumulator after it's been shifted left one nibble. The result
* is the new CRC accumulator value. When we're done with all of the
* data, the accumulator is postconditioned by inverting it.
*/
UINT16 generate_seeded_crc (UINT16 crc_accum, INT32 length, void *vdata)
{
    INT32 table_index;
    INT8 *data = vdata;

    crc_accum = ~crc_accum;

    for ( ; length != 0; length--, data++ )
    {
        table_index = ( (crc_accum >> 12) ^ (*data >> 4) ) & 0x000F;
        crc_accum = ( (crc_accum << 4) & 0xFFF0 ) ^ crc_table[table_index];

        table_index = ( (crc_accum >> 12) ^ *data ) & 0x000F;
        crc_accum = ( (crc_accum << 4) & 0xFFF0 ) ^ crc_table[table_index];
    }

    return( ~crc_accum );
}

/**
 * crc16 - compute the CRC-16 for the data buffer
 * @crc:    previous CRC value
 * @buffer: data pointer
 * @len:    number of bytes in the buffer
 *
 * Returns the updated CRC value.
 */
UINT16 generate_crc (INT32 length, void *vdata)
{
    UINT16 checksum=0;


    checksum = (generate_seeded_crc (0, length, vdata));

    log_dbgPrintf("generate_seeded_crc = 0x%04X.\n",checksum);
    /* Big endian mode */
    checksum = ((checksum>>8) & 0xFF)| ((checksum<<8) & 0xFF00);

    return checksum;
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_getPcamapOffset
 *
 *  DESCRIPTION :
 *      hal_getPcamapOffset
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      none
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void hal_getPcamapOffset
(
    OUT UINT16 *size
)
{
    UINT32 idx = 0, pcamapSize = 0;

    while (strcmp(pcamap_field[idx].field_name,"checksum"))
    {
        pcamapSize += pcamap_field[idx].size;
        idx++;
    }

    *size = pcamapSize;
}


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_validCheckSum
 *
 *  DESCRIPTION :
 *      hal_validCheckSum
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_validCheckSum
(
    IN UINT8 *buf,
    IN UINT8 *valid
)
{
    UINT16 checksum_offt = 0, def_checksum = 0, checksum = 0;
    INT32 ret = E_TYPE_SUCCESS;

    /* Get the pcamap size */
    hal_getPcamapOffset(&checksum_offt);

    /* Computer the crc check sum */
    checksum = generate_crc(checksum_offt, buf);

    def_checksum = (buf[checksum_offt+1]<<8 | buf[checksum_offt]);

    log_dbgPrintf("[check valid]: checksum = 0x%04X, original = 0x%04X.\n",checksum, def_checksum);

    if ( def_checksum != checksum )
        *valid = FALSE;
    else
        *valid = TRUE;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_updateCheckSum
 *
 *  DESCRIPTION :
 *      hal_updateCheckSum
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_updateCheckSum()
{
    UINT8  buf[PCAMAP_SIZE]={0};
    UINT16 checksum_offt = 0, verify_checksum = 0, checksum = 0;
    INT32 ret = E_TYPE_SUCCESS;

    ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, 0, 2, buf, PCAMAP_SIZE);
    if (ret != E_TYPE_SUCCESS)
    {
        log_printf("Get pcamap data fail!\n");
        return E_TYPE_REG_READ;
    }

    /* Get the pcamap size */
    hal_getPcamapOffset(&checksum_offt);

    /* Computer the crc check sum */
    checksum = generate_crc(checksum_offt, buf);

    /* Set the checksum data */
    buf[0] = (checksum & 0xFF);
    buf[1] = ((checksum>>8) & 0xFF);
    ret = i2c_halRegSet(SYSTEM_EEPROM_I2C_ADDR, checksum_offt, 2, buf, 2);
    if (ret != E_TYPE_SUCCESS)
    {
        log_printf("Get default check sum fail!\n");
        return E_TYPE_REG_READ;
    }

    /* Verify the check sum */
    ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, checksum_offt, 2, buf, 2);
    if (ret != E_TYPE_SUCCESS)
    {
        log_printf("Get verify check sum fail!\n");
        return E_TYPE_REG_READ;
    }

    verify_checksum = ((buf[1]<<8)| buf[0]) & 0xFFFF;

    log_dbgPrintf("checksum = 0x%04X, verify checksum = 0x%04X\n", checksum, verify_checksum);

    if ( verify_checksum != checksum )
        return E_TYPE_DATA_MISMATCH;

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_pcamapShow
 *
 *  DESCRIPTION :
 *      Pcamap show
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_pcamapShow
(
    IN UINT8 *field,
    IN UINT8 *eeprom_buf
)
{
    INT32 i = 0, ret = E_TYPE_SUCCESS;
    BOOL  valid = FALSE;
    UINT8 value_str[STRING_LEN]={0};
    UINT8 *cmp_buf = "FF", temp_buf[10]={0};
    UINT8 *buf=NULL, eeprom_data[PCAMAP_SIZE]={0};

    if (eeprom_buf == NULL)
    {
        /* Read the data from eeprom if the eeprom_buf is null */
        ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, 0, 2, eeprom_data, PCAMAP_SIZE);
        if (ret != E_TYPE_SUCCESS)
        {
            log_printf("Get main EERPOM contents fail!\n");
            return E_TYPE_REG_READ;
        }
        buf=eeprom_data;

        /* Check the checksum data */
        ret = hal_validCheckSum(buf, &valid);
        if (ret != E_TYPE_SUCCESS )
            return ret;

        if (!valid)
        {
            log_printf("Err: checksum mismatch.\n");
            return E_TYPE_DATA_MISMATCH;
        }
    }
    else
        buf = eeprom_buf;

    if (field == NULL)
    {
        /* show all fields */
        while (strcmp(pcamap_field[i].field_name,"reserved"))
        {
            memset(value_str, 0, STRING_LEN);
            switch (pcamap_field[i].type)
            {
                case TYPE_MACADDR:
                    sprintf(value_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
                    break;
                case TYPE_ASCII:
                    strncpy(value_str, buf, pcamap_field[i].size);
                    break;
            }

            /*version is one byte hex data */
            if (! strcmp(pcamap_field[i].field_name,"version") )
                sprintf(value_str, "%02X", buf[0]);

            sprintf(temp_buf, "%02x", buf[0]);
            /* Fixed the mac address and version show abnomally with value 0xFF */
            /*i==0: version, i==1: mac_addr*/
            if (!strncasecmp(temp_buf, cmp_buf, 2) && ( i >= 2))
            {
                memset(value_str, 0, pcamap_field[i].size);
            }
            log_printf("%s: %s\n", pcamap_field[i].full_name, value_str);
            buf += pcamap_field[i].size;
            i++;
        }
    }
    else
    {
        /* show only specific field */
        while (strcmp(pcamap_field[i].field_name,"reserved"))
        {
            if (strcasecmp(field, pcamap_field[i].field_name) == 0)
            {
                switch (pcamap_field[i].type)
                {
                    case TYPE_MACADDR:
                        sprintf(value_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
                        break;
                    case TYPE_ASCII:
                        strncpy(value_str, buf, pcamap_field[i].size);
                        break;
                }

                /*version is one byte hex data */
                if (! strcmp(pcamap_field[i].field_name,"version") )
                    sprintf(value_str, "%02X", buf[0]);

                sprintf(temp_buf, "%02x", buf[0]);
                /* Fixed the mac address and version show abnomally with value 0xFF */
                /*i==0: version, i==1: mac_addr*/
                if (!strncasecmp(temp_buf, cmp_buf, 2) && ( i >= 2))
                {
                    memset(value_str, 0, pcamap_field[i].size);
                }
                log_printf("%s: %s\n", pcamap_field[i].full_name, value_str);
                eeprom_buf = value_str;
                break;
            }

            buf += pcamap_field[i].size;
            i++;
        }

        if (! strcmp(pcamap_field[i].field_name,"reserved") )
        {
            log_printf("Err: Invalid field name: %s\n", field);
            return E_TYPE_INVALID_CMD_FORMAT;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_pcamapItemGet
 *
 *  DESCRIPTION :
 *      hal_pcamapItemGet
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_pcamapItemGet
(
    IN UINT8 *field,
    OUT UINT8 *eeprom_buf
)
{
    INT32 i = 0, ret = E_TYPE_SUCCESS;
    BOOL  valid = FALSE;
    UINT8 value_str[STRING_LEN]={0};
    UINT8 *cmp_buf = "FF", temp_buf[10]={0};
    UINT8 *buf=NULL, eeprom_data[PCAMAP_SIZE]={0};

    /* Read the data from eeprom */
    ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, 0, 2, eeprom_data, PCAMAP_SIZE);
    if (ret != E_TYPE_SUCCESS)
    {
        log_printf("Get main EERPOM contents fail!\n");
        return E_TYPE_REG_READ;
    }
    buf=eeprom_data;

    /* Check the checksum data */
    ret = hal_validCheckSum(buf, &valid);
    if (ret != E_TYPE_SUCCESS )
        return ret;

    if (!valid)
    {
        log_printf("Err: checksum mismatch.\n");
        return E_TYPE_DATA_MISMATCH;
    }

    /* show only specific field */
    while (strcmp(pcamap_field[i].field_name,"reserved"))
    {
        if (strcasecmp(field, pcamap_field[i].field_name) == 0)
        {
            switch (pcamap_field[i].type)
            {
                case TYPE_MACADDR:
                    sprintf(value_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
                    break;
                case TYPE_ASCII:
                    strncpy(value_str, buf, pcamap_field[i].size);
                    break;
            }

           /*version is one byte hex data */
            if (! strcmp(pcamap_field[i].field_name,"version") )
                sprintf(value_str, "%02X", buf[0]);

            sprintf(temp_buf, "%02x", buf[0]);
            /* Fixed the mac address and version show abnomally with value 0xFF */
            /*i==0: version, i==1: mac_addr*/
            if (!strncasecmp(temp_buf, cmp_buf, 2) && ( i >= 2))
            {
                memset(value_str, 0, pcamap_field[i].size);
            }
            log_printf("%s: %s\n", pcamap_field[i].full_name, value_str);
            memcpy(eeprom_buf, value_str, pcamap_field[i].size);
            break;
        }

        buf += pcamap_field[i].size;
        i++;
    }

    if (! strcmp(pcamap_field[i].field_name,"reserved") )
    {
        log_printf("Err: Invalid field name: %s\n", field);
        return E_TYPE_INVALID_CMD_FORMAT;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_pcamapEdit
 *
 *  DESCRIPTION :
 *      write pcamap content
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_pcamapEdit
(
    OUT UINT8 *keep,
    OUT UINT8 *eeprom_buf
)
{
    UINT8 *eeprom_buf_ptr = eeprom_buf;
    UINT32 i = 0, j = 0, m = 0;
    UINT8 input_line[STRING_LEN]={0}, default_str[STRING_LEN]={0};
    UINT32 mac_buf[6]={0};

    while (strcmp(pcamap_field[i].field_name,"reserved"))
    {
        switch (pcamap_field[i].type)
        {
            case TYPE_MACADDR:
                sprintf(default_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                eeprom_buf_ptr[0], eeprom_buf_ptr[1], eeprom_buf_ptr[2],
                eeprom_buf_ptr[3], eeprom_buf_ptr[4], eeprom_buf_ptr[5]);
                break;
            case TYPE_ASCII:
                strncpy(default_str, eeprom_buf_ptr, pcamap_field[i].size);
                break;
            default:
                fprintf(stderr, "Bug: TYPE %d not supported.\n", pcamap_field[i].type);
        }

        /*version is one byte hex data */
        if (! strcmp(pcamap_field[i].field_name,"version") )
            sprintf(default_str, "%02X", eeprom_buf_ptr[0]);

        do
        {
            log_printf("%s [%s]:",pcamap_field[i].full_name, default_str);
            /* the following lines replace gets() */
            if( fgets(input_line, STRING_LEN, stdin) == NULL )
            {
                log_printf("Invalid input\n");
                continue;
            }

            if ((input_line[0] == 0xa) || (input_line[0] == 0xd))
            {
                break;
            }
            else
            {
                /* new value entered, parse it */
                if (pcamap_field[i].type == TYPE_MACADDR)
                {
                    if (sscanf(input_line, "%02X%02X%02X%02X%02X%02X", &(mac_buf[0]), &(mac_buf[1]), &(mac_buf[2]),
                        &(mac_buf[3]), &(mac_buf[4]), &(mac_buf[5])) != 6)
                    {
                        if (sscanf(input_line, "%02X:%02X:%02X:%02X:%02X:%02X", &(mac_buf[0]), &(mac_buf[1]), &(mac_buf[2]),
                            &(mac_buf[3]), &(mac_buf[4]), &(mac_buf[5])) != 6)
                        {
                            if (sscanf(input_line, "%02X-%02X-%02X-%02X-%02X-%02X", &(mac_buf[0]), &(mac_buf[1]), &(mac_buf[2]),
                                &(mac_buf[3]), &(mac_buf[4]), &(mac_buf[5])) != 6)
                            {
                                log_printf("Err: Invalid MAC address format\n");
                                return E_TYPE_INVALID_DATA_FORMAT;
                            }
                        }
                    }

                    /* copy the mac address to buf */
                    for(m=0;m<6;m++)
                        memcpy(&eeprom_buf_ptr[m], (UINT8 *)&mac_buf[m], 1);

                    break;
                }

                if (pcamap_field[i].type == TYPE_ASCII)
                {
                    /* trim cr/lf */
                    for(j = 0; j < pcamap_field[i].size; j++)
                    {
                        if ((input_line[j] == 0xa) || (input_line[j] == 0xd))
                        {
                            input_line[j] = 0x0;
                            break;
                        }
                    }

                    /*version is one byte hex data */
                    if (! strcmp(pcamap_field[i].field_name, "version") )
                    {
                        sscanf(input_line, "%02X", &(mac_buf[0]));
                        input_line[0]=mac_buf[0];
                    }

                    strncpy(eeprom_buf_ptr, input_line, pcamap_field[i].size);
                    break;
                }
            }
        } while (1);

        /* next field */
        eeprom_buf_ptr += pcamap_field[i].size;
        i++;
    }/* End of while (pcamap_field[i].field_name != "reserved") */

    log_printf("\nThe following content will be written to PCAMAP:\n");
    hal_pcamapShow(NULL, eeprom_buf);
    while (1)
    {
        log_printf("Are you sure (y/n)?");
        if( fgets(input_line, STRING_LEN, stdin) != NULL )
        {
            if ((input_line[0] == 'y') || (input_line[0] == 'Y'))
            {
                *keep = FALSE;
                return E_TYPE_SUCCESS;
            }
            if ((input_line[0] == 'n') || (input_line[0] == 'N'))
            {
                *keep = TRUE;
                return E_TYPE_SUCCESS;
            }
        }
    }

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_pcamapEditField
 *
 *  DESCRIPTION :
 *      write pcamap content
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_pcamapEditField
(
    IN UINT32 field_id,
    IN UINT8 *string_buf,
    IN UINT8 *argv[]
)
{
    UINT8 input_line[STRING_LEN]={0}, default_str[STRING_LEN]={0};
    UINT32 mac_buf[6]={0};
    UINT32 i, m = 0;

    switch (pcamap_field[field_id].type)
    {
        case TYPE_MACADDR:
            sprintf(default_str, "%02X:%02X:%02X:%02X:%02X:%02X",
            string_buf[0], string_buf[1], string_buf[2],
            string_buf[3], string_buf[4], string_buf[5]);
            break;
        case TYPE_ASCII:
            strncpy(default_str, string_buf, pcamap_field[field_id].size);
            break;
        default:
            fprintf(stderr, "Bug: TYPE %d not supported.\n", pcamap_field[field_id].type);
    }

    /*version is one byte hex data */
    if (! strcmp(pcamap_field[field_id].field_name,"version") )
        sprintf(default_str, "%02X", string_buf[0]);

    if (argv[3] == NULL)
    {
        do
        {
            log_printf("%s [%s]:",pcamap_field[field_id].full_name, default_str);
            if ((fgets(input_line, STRING_LEN, stdin)) == NULL)
            {
                log_printf("Invalid input\n");
                continue;
            }

        }while(strlen(input_line) <= 1);
    }
    else
    {
        log_printf("%s [%s]: %s\n",pcamap_field[field_id].full_name, default_str, argv[3]);
        memcpy(input_line, argv[3], strlen(argv[3]));
    }

    if (pcamap_field[field_id].type == TYPE_MACADDR)
    {
        /* if input MAC info */
        if (sscanf(input_line, "%02X%02X%02X%02X%02X%02X", &(mac_buf[0]), &(mac_buf[1]), &(mac_buf[2]),
            &(mac_buf[3]), &(mac_buf[4]), &(mac_buf[5])) != 6)
        {
            if (sscanf(input_line, "%02X:%02X:%02X:%02X:%02X:%02X", &(mac_buf[0]), &(mac_buf[1]), &(mac_buf[2]),
                &(mac_buf[3]), &(mac_buf[4]), &(mac_buf[5])) != 6)
            {
                if (sscanf(input_line, "%02X-%02X-%02X-%02X-%02X-%02X", &(mac_buf[0]), &(mac_buf[1]), &(mac_buf[2]),
                        &(mac_buf[3]), &(mac_buf[4]), &(mac_buf[5])) != 6)
                {
                    log_printf("Err: Invalid MAC address format\n");
                    return E_TYPE_INVALID_DATA_FORMAT;
                }
            }
        }

        /* copy mac address to buf */
        for(m=0; m < pcamap_field[field_id].size; m++)
            memcpy(&string_buf[m], (UINT8 *)&mac_buf[m], 1);
    }
    else
    {
        for(i = 0; i < pcamap_field[field_id].size; i++)
        {
            if ((input_line[i] == 0xa) || (input_line[i] == 0xd))
            {
                input_line[i] = 0x0;
                break;
            }
        }


        /*version is one byte hex data */
        if (! strcmp(pcamap_field[field_id].field_name, "version") )
        {
            sscanf(input_line, "%02X", &(mac_buf[0]));
            input_line[0]=mac_buf[0];
        }

        /*strncpy(string_buf, input_line, g_pcamapFieldInfo[field_id].size);*/
        /* fixed to copy input_line to string_buf incorrect data */
        for(m=0; m < strlen(input_line); m++)
            memcpy(&string_buf[m], &input_line[m], sizeof(UINT8));

        /* Set the excess data to zero */
        memset(&string_buf[m], 0x0, pcamap_field[field_id].size-m);
    }
    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      hal_pcamapProgram
 *
 *  DESCRIPTION :
 *      program pcamap content
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
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
INT32 hal_pcamapProgram
(
    IN UINT8 *argv[]
)
{
    UINT8 verify_buf[PCAMAP_SIZE]={0}, buf[PCAMAP_SIZE]={0};
    BOOL keepDefault = TRUE;
    INT32 ret = E_TYPE_SUCCESS;
    INT32 idx =0, fieldId = -1;
    INT32 fieldOffset = 0, fieldSize = 0;

    if (argv[2] == NULL)
    {
        ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, 0, 2, buf, PCAMAP_SIZE);
        if (ret != E_TYPE_SUCCESS)
        {
            log_printf("Get main EEPROM contents fail!\n");
            return E_TYPE_REG_READ;
        }

        /* Get the input value string */
        ret = hal_pcamapEdit(&keepDefault, buf);
        if (ret != E_TYPE_SUCCESS)
            return ret;

        /* user choice not program */
        if (keepDefault)
            return ret;

        log_printf("Writing to EEPROM...\n");
        ret = fox_eeprom_write(0, buf, PCAMAP_SIZE);
        if ( ret != E_TYPE_SUCCESS )
            return E_TYPE_REG_WRITE;

        log_printf("Verify the write content  -  ");
        ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, 0, 2, verify_buf, PCAMAP_SIZE);
        if (ret != E_TYPE_SUCCESS)
        {
            log_printf("\nGet EEPROM verify contents fail!\n");
            return E_TYPE_REG_READ;
        }

        if (memcmp(verify_buf, buf, PCAMAP_SIZE) != 0)
        {
            log_printf("fail\n");
            return E_TYPE_DATA_MISMATCH;
        }
        log_printf("pass\n");
    }
    else
    {
        while (strcmp(pcamap_field[idx].field_name,"reserved"))
        {
            if (!_stricmp(argv[2], pcamap_field[idx].field_name))
            {
                fieldSize =  pcamap_field[idx].size;
                fieldId = pcamap_field[idx].idx;
                break;
            }
            fieldOffset += pcamap_field[idx].size;
            idx++;
        }

        if (fieldId == -1)
        {
            log_printf("Err: Invalid field name: %s\n", argv[2]);
            return E_TYPE_INVALID_CMD_FORMAT;
        }

        log_dbgPrintf("Field offset is %d, size is %d\n", fieldOffset, fieldSize);
        ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, fieldOffset, 2, buf, fieldSize);
        if (ret != E_TYPE_SUCCESS)
        {
            log_printf("Get main EEPROM contents fail!\n");
            return E_TYPE_REG_READ;
        }

        /* Get the input value string */
        ret = hal_pcamapEditField(fieldId, buf, argv);
        if (ret != E_TYPE_SUCCESS)
            return ret;

        ret = fox_eeprom_write(fieldOffset, buf, fieldSize);
        if ( ret != E_TYPE_SUCCESS )
            return E_TYPE_REG_WRITE;

        log_printf("Verify the write content - ");
        ret = i2c_halRegGet(SYSTEM_EEPROM_I2C_ADDR, fieldOffset, 2, verify_buf, fieldSize);
        if (ret != E_TYPE_SUCCESS)
        {
            log_printf("\nGet EEPROM verify contents fail!\n");
            return E_TYPE_REG_READ;
        }

        if (memcmp(verify_buf, buf, fieldSize) != 0)
        {
            log_printf("fail\n");
            return E_TYPE_DATA_MISMATCH;
        }
        log_printf("pass\n");

    }

    log_printf("Update checksum - ");
    /* Update checksum */
    ret = hal_updateCheckSum();
    if (ret != E_TYPE_SUCCESS )
    {
        log_printf("fail\n");
        return ret;
    }
    log_printf("pass\n");

    return ret;
}

