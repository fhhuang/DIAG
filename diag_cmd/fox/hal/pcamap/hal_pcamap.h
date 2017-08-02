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
***             hal_pcamap.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
***             File Creation
***
***************************************************************************/

#ifndef __HAL_PCAMAP_H_
#define __HAL_PCAMAP_H_

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
#include "cmn_type.h"
#include "port_defs.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Macro Definition Segment
 *
 *==========================================================================
 */

#define TYPE_MACADDR    1
#define TYPE_ASCII      2

#define PCAMAP_SIZE     0x400  /* base on pcamap_field */
#define EEPROM_SIZE     0x4000 /* 16k bytes */
#define STR_CMP         "fffffffffffffffffffffffffffffffffff"
#define STRING_LEN      512
#define CRC_DATA        0xBABA

#define _stricmp strcasecmp

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */

typedef struct {
    UINT32   idx;
    UINT8    *field_name;
    UINT8    *full_name;
    UINT32   type;
    UINT32   size;       /* size in byte */
} _pcamap_field;


/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */

/***************************************************************************
 * Parameter name           Format      Example         Source
 * BOARD_CONFIG_REV     ASCII (32 max)  0               Required to boot
 * MAC_ADDR             6 ASCII         XX:XX:XX:XX:XX:XX assigned by manufacturing
 * BOARD_ASSEMBLY_NUM   ASCII (32 max)                  Cisco or JDM, TBD
 * BOARD_SERIAL_NUM     ASCII (32 max)  XXXXXXXXXXX     assigned by manufacturing
 * BOARD_REVISION_NUM   ASCII (32 max)  XX              Cisco or JDM, TBD
 * MODEL_NUM            ASCII (32 max)  WS-CXXXXX       Cisco PID, TBD
 * MODEL_REVISION_NUM   ASCII (32 max)  XX              Cisco BOM
 * MODEL _SERIAL_NUM    ASCII (32 max)  XXXXXXXXXXX     assigned by manufacturing
 * TAN_NUM              ASCII (32 max)  800-XXXXX-XX    Cisco BOM
 * TAN_REVISION_NUMBER  ASCII (32 max)  XX              Cisco BOM
 * VERSION_ID           ASCII (32 max)  VXX             Cisco BOM
 * CLEI_CODE_NUMBER     ASCII (32 max)  XXXXXXXXXX      Cisco BOM
 * [Reserved for future usage] ASCII (160 max)
 ***************************************************************************
 */

#if 0
_pcamap_field pcamap_field[] = {
    {0,  "board_rev",   "BOARD_CONFIG_REV",             TYPE_ASCII,     32},
    {1,  "mac_addr",    "MAC_ADDR",                     TYPE_MACADDR,   6},
    {2,  "model_num",   "MODEL_NUM",                    TYPE_ASCII,     32},
    {3,  "model_rn",    "MODEL_REVISION_NUM",           TYPE_ASCII,     32},
    {4,  "model_sn",    "MODEL_SERIAL_NUM",             TYPE_ASCII,     32},
    {5,  "ver_id",      "VERSION_ID",                   TYPE_ASCII,     32},
    {6,  "code_num",    "CLEI_CODE_NUMBER",             TYPE_ASCII,     32},
    {7,  "tan_num",     "TAN_NUM",                      TYPE_ASCII,     32},
    {8,  "tan_rn",      "TAN_REVISION_NUMBER",          TYPE_ASCII,     32},
    {9,  "mb_an",       "MAINBOARD_ASSEMBLY_NUM",       TYPE_ASCII,     32},
    {10, "mb_sn",       "MAINBOARD_SERIAL_NUM",         TYPE_ASCII,     32},
    {11, "mb_rn",       "MAINBOARD_REVISION_NUM",       TYPE_ASCII,     32},
    {12, "db_an",       "DAUGHTER_BOARD_ASSEMBLY_NUM",  TYPE_ASCII,     32},
    {13, "db_sn",       "DAUGHTER_BOARD_SERIAL_NUM",    TYPE_ASCII,     32},
    {14, "db_rn",       "DAUGHTER_BOARD_REVISION_NUM",  TYPE_ASCII,     32},
    {15, "ps_pn",       "POWER_SUPPLY_PART_NUM",        TYPE_ASCII,     32},
    {16, "ps_sn",       "POWER_SUPPLY_SERIAL_NUM",      TYPE_ASCII,     32}, /* 518 bytes */
    {17, "reserved",    "RESERVED",                     TYPE_ASCII,     256},
    {18, "checksum",    "CHECKSUM",                     TYPE_ASCII,       2},
    {19, NULL,          NULL,                           TYPE_ASCII,       0} /* end of list */
};
#endif

_pcamap_field pcamap_field[] = {
    {0,  "version",     "VERSION",                          TYPE_ASCII,     1},
    {1,  "mac_addr",    "MAC_ADDR",                         TYPE_MACADDR,   6},
    {2,  "mb_an",       "MOTHER_BOARD_ASSEMBLY_NUM",        TYPE_ASCII,     32},
    {3,  "ps_pn",       "POWER_SUPPLY_PART_NUM",            TYPE_ASCII,     32},
    {4,  "mb_sn",       "MOTHER_BOARD_SERIAL_NUM",          TYPE_ASCII,     32},
    {5,  "ps_sn",       "POWER_SUPPLY_SERIAL_NUM",          TYPE_ASCII,     32},
    {6,  "model_rn",    "MODEL_REVISION_NUM",               TYPE_ASCII,     32},
    {7,  "mb_rn",       "MOTHER_BOARD_REVISION_NUM",        TYPE_ASCII,     32},
    {8,  "model_num",   "MODEL_NUM",                        TYPE_ASCII,     32},
    {9,  "system_sn",   "SYSTEM_SERIAL_NUM",                TYPE_ASCII,     32},
    {10, "db_an",       "DAUGHTER_BOARD_ASSEMBLY_NUM",      TYPE_ASCII,     32},
    {11, "db_sn",       "DAUGHTER_BOARD_SERIAL_NUM",        TYPE_ASCII,     32},
    {12, "sfp_apn",     "SFP_BOARD_ASSEMBLY_PART_NUM",      TYPE_ASCII,     32},
    {13, "sfp_rn",      "SFP_BOARD_REVISION_NUM",           TYPE_ASCII,     32},
    {14, "sfpb_sn",     "SFP_BOARD_SERIAL_NUM",             TYPE_ASCII,     32},
    {15, "top_apn",     "TOP_ASSEMBLY_PART_NUM",            TYPE_ASCII,     32},
    {16, "top_arn",     "TOP_ASSEMBLY_REVISION_NUM",        TYPE_ASCII,     32},
    {17, "ver_id",      "VERSION_ID",                       TYPE_ASCII,     32},
    {18, "code_num",    "CLEI_CODE_NUMBER",                 TYPE_ASCII,     32},
    {19, "db_rn",       "DAUGHTER_BOARD_REVISION_NUM",      TYPE_ASCII,     32},
    {20, "mfg_cert",    "CISCO_MFG_CERTIFICATE",            TYPE_ASCII,     328},
    {21, "sps_pn",      "SECOND_POWER_SUPPLY_PART_NUM",     TYPE_ASCII,     32},
    {22, "sps_sn",      "SECOND_POWER_SUPPLY_SERIAL_NUM",   TYPE_ASCII,     32},
    {23, "board_rev",   "BOARD_CONFIG_REV",                 TYPE_ASCII,     4},
    {24, "cfg_model",   "CFG_MODEL_NUM",                    TYPE_ASCII,     32},
    {25, "reserved",    "SPARE",                            TYPE_ASCII,     11},
    {26, "checksum",    "CHECKSUM",                         TYPE_ASCII,     2},
    {27, NULL,          NULL,                               TYPE_ASCII,     0} /* 1024 bytes */
};

static UINT32 g_maxFieldCnt = (UINT32)(sizeof(pcamap_field)/sizeof(_pcamap_field));

static UINT16 crc_table[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
};

/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */

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
);

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
);

#endif /* __HAL_PCAMAP_H_ */