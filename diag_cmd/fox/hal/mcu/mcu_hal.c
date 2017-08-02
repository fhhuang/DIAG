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
***      mcu_hal.c
***
***    DESCRIPTION :
***      for mcu hal
***
***    HISTORY :
***       - 2015/11/27, Eric Hsu
***             File Creation
***
***************************************************************************/
#include "cmn_type.h"
#include "porting.h"
#include "mcu_hal.h"

#include "err_type.h"
#include "log.h"

#define DBG
#ifdef DBG
#define DBG_PRINTF log_dbgPrintf
#else
#define DBG_PRINTF
#endif
/*==========================================================================
 *
 *      static library
 *
 *==========================================================================
 */

INT32 mcu_halPacketGen
(
    IN  UINT8       opcode,
    IN  UINT8       cmd,
    IN  UINT32      addr,
    IN  UINT8*      data,
    OUT UINT8*      pkt,
    OUT UINT32*     size
)
{
    INT32 ret = E_TYPE_SUCCESS;
    UINT32 addrH = MCU_BASE_ADDR, ptr;

    pkt[MCU_ADDR_HIGH] = (addrH >> 24) & 0xFF;
    pkt[MCU_ADDR_HIGH + 1] = (addrH >> 16) & 0xFF;
    pkt[MCU_ADDR_LOW] = (addr >> 8) & 0xFF;
    pkt[MCU_ADDR_LOW + 1] = addr & 0xFF;
    pkt[MCU_DATA_LEN] = 0x0;
    pkt[MCU_DATA_LEN + 1] = MCU_DATA_LEN + 2;

    switch (opcode)
    {
        case MCU_OPC_READ:
            *size = MCU_READ_SIZE;
            break;
        case MCU_OPC_ERPG:
            *size = MCU_READ_SIZE;
            pkt[MCU_DATA_LEN + 1] = data;
            break;
        case MCU_OPC_WRITE:
            pkt[MCU_DUM_BYTE] = 0xFF;
            pkt[MCU_DATA] = cmd;
            pkt[MCU_DATA + 1] = MCU_DATA_LEN;
            pkt[MCU_DATA + 2] = ((UINT32)data >> 24) & 0xFF;
            pkt[MCU_DATA + 3] = ((UINT32)data >> 16) & 0xFF;
            pkt[MCU_DATA + 4] = ((UINT32)data >> 8) & 0xFF;
            pkt[MCU_DATA + 5] = (UINT32)data & 0xFF;
            *size = MCU_WRITE_SIZE;
            break;
        case MCU_OPC_UPGRADE:
            pkt[MCU_DATA_LEN + 1] = *size + 2;
            pkt[MCU_DUM_BYTE] = 0xFF;
            pkt[MCU_DATA] = cmd;
            pkt[MCU_DATA + 1] = *size;
            for (ptr = 0; ptr < *size; ptr++)
                pkt[MCU_DATA + 2 + ptr] = data[ptr];
            break;
        default:;
    }

    return ret;
}

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */

/* User-defined Library */
#include "cmn_type.h"
#include "porting.h"

#include "i2c_hal.h"
#include "err_type.h"
#include "log.h"
#include "mcu_hal.h"

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mcu_halDataGet
 *
 *  DESCRIPTION :
 *      Get the data from MCU
 *
 *  INPUT :
 *      cmd    -   type of mcu commands
 *
 *  OUTPUT :
 *      read_data   -   the data from MCU
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
INT32 mcu_halDataGet
(
  IN E_MCU_CMD cmd,
  IN UINT32    addr,
  OUT UINT32 *read_data
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  idx, data= 0, pktsize, pktData[4] = {0};
    UINT8   packet[MCU_MAX_SIZE], opcode;

    memset(packet,0,sizeof(MCU_MAX_SIZE));

    if( read_data == NULL )
        return E_TYPE_INVALID_DATA;

    switch(cmd)
    {
        case MCU_GET_DATE:
        case MCU_GET_TIME:
        case MCU_GET_VOL_3_3:
        case MCU_GET_VOL_2_5:
        case MCU_GET_VOL_1_8:
        case MCU_GET_VOL_1_5:
        case MCU_GET_VOL_0_99:
        case MCU_GET_VOL_0_9:
        case MCU_GET_MAC_TEMP:
        case MCU_GET_PHY_TEMP:
        case MCU_GET_DDR_TEMP:
        case MCU_GET_SPEED:
            opcode = MCU_OPC_READ;
            break;
        default:
            log_printf("undefined type of MCU command\n");
            return E_TYPE_INVALID_CMD;
    }

    mcu_halPacketGen(opcode, cmd, addr, (UINT8*)data, packet, &pktsize);

    ret=i2c_halRegSet(MCU_DEV_ADDR, opcode, 1, packet, pktsize) ;

    log_dbgPrintf("Write: dev_addr=0x%X, addr=0x%08X, data=0x%08X\n", MCU_DEV_ADDR, addr, data);
    for(idx = 0; idx < pktsize; idx++ )
    {
        pktData[idx/4] |= (UINT32)packet[idx];
        if ((idx == 0) || (idx % 4) != 3)
           pktData[idx/4] <<= 8;
    }
    log_dbgPrintf("packet = %08X %08X %08X %08X (size = %d)\n", pktData[0], pktData[1], pktData[2], pktData[3], pktsize);

    usdelay(1000000);
    data = 0;
    ret=mcu_halRegGet(MCU_DEV_ADDR, opcode, 1, (UINT8 *)&data, 4);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to do MCU data get.\n");
        goto __CMD_ERROR;
    }


    log_dbgPrintf("Read: dev_addr=0x%x, addr=0x%x, data=0x%08x\n", MCU_DEV_ADDR, addr, data);
    *read_data = data;

__CMD_ERROR:
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mcu_halDataSet
 *
 *  DESCRIPTION :
 *      Set the data to MCU
 *
 *  INPUT :
 *      cmd    -   type of mcu commands
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
INT32 mcu_halDataSet
(
  IN E_MCU_CMD cmd,
  IN UINT32 addr,
  IN UINT32 write_data
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  idx, pktsize;
    UINT8   packet[MCU_MAX_SIZE], opcode = MCU_OPC_WRITE;
    UINT32  data[4] = {0};

    memset(packet,0,sizeof(MCU_MAX_SIZE));

    switch(cmd)
    {
        case MCU_GET_DATE:log_dbgPrintf("MCU Command : Get Date\n"); break;
        case MCU_GET_TIME:log_dbgPrintf("MCU Command : Get Time\n"); break;
        case MCU_SET_DATE:log_dbgPrintf("MCU Command : Set Date\n"); break;
        case MCU_SET_TIME:log_dbgPrintf("MCU Command : Set Time\n"); break;
        case MCU_SET_LED:log_dbgPrintf("MCU Command : Set LED\n"); break;
        case MCU_GET_VOL_3_3:log_dbgPrintf("MCU Command : Get Voltage - 3.3V/5V\n"); break;
        case MCU_GET_VOL_1_8:log_dbgPrintf("MCU Command : Get Voltage - 1.8V\n"); break;
        case MCU_GET_VOL_1_5:log_dbgPrintf("MCU Command : Get Voltage - 1.5V\n"); break;
        case MCU_GET_VOL_0_99:log_dbgPrintf("MCU Command : Get Voltage - 0.99V\n"); break;
        case MCU_GET_VOL_0_9:log_dbgPrintf("MCU Command : Get Voltage - 0.9V\n"); break;
        case MCU_SET_VOL_MAR:log_dbgPrintf("MCU Command : Set Voltage Margin\n"); break;
        case MCU_GET_MAC_TEMP:log_dbgPrintf("MCU Command : Get MAC Temp\n"); break;
        case MCU_GET_PHY_TEMP:log_dbgPrintf("MCU Command : Get PHY Temp\n"); break;
        case MCU_GET_DDR_TEMP:log_dbgPrintf("MCU Command : Get DDR Temp\n"); break;
        case MCU_SET_SPEED:log_dbgPrintf("MCU Command : Set FAN Speed\n"); break;
        case MCU_GET_SPEED:log_dbgPrintf("MCU Command : Get FAN Speed\n"); break;
        case MCU_WRITE_DATA:log_dbgPrintf("MCU Command : Write Data without erase\n"); break;
        case MCU_INT_TOGGLE:log_dbgPrintf("MCU Command : Set MCU INT Toggle\n"); break;
        case MCU_GET_VER:log_dbgPrintf("MCU Command : Get MCU version\n"); break;
        case MCU_GET_VER_DATE:log_dbgPrintf("MCU Command : Get MCU version date\n"); break;
        case MCU_RESET_READ:log_dbgPrintf("MCU Command : Reset MCU Read Flag\n"); break;
        case MCU_ERASE_WRITE:log_dbgPrintf("MCU Command : Write Data with erase\n"); break;
        case MCU_OPC_ERPG: opcode = MCU_OPC_ERPG; break;
        case MCU_OPC_USERCD:log_dbgPrintf("MCU Command : Jump to AP mode\n"); break;
        case MCU_OPC_IAPCD:log_dbgPrintf("MCU Command : Jump to IAP mode\n"); break;
        default: log_dbgPrintf("Undefined type of MCU command (0x%X)\n", cmd);
            /*return E_TYPE_INVALID_CMD;*/
    }

    mcu_halPacketGen(opcode, cmd, addr, (UINT8*)write_data, packet, &pktsize);

    ret=i2c_halRegSet(MCU_DEV_ADDR, opcode, 1, packet, pktsize);

    log_dbgPrintf("Write: dev_addr=0x%X, addr=0x%08X, write_data=0x%08X\n", MCU_DEV_ADDR, addr, write_data);
    for(idx = 0; idx < pktsize; idx++ )
    {
        data[idx/4] |= (UINT32)packet[idx];
        if ((idx == 0) || (idx % 4) != 3)
           data[idx/4] <<= 8;
    }
    log_dbgPrintf("packet = %08X %08X %08X %08X (size = %d)\n", data[0], data[1], data[2], data[3], pktsize);

    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to do MCU data set.\n");
        goto __CMD_ERROR;
    }

__CMD_ERROR:
    return ret;

}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mcu_halModeGet
 *
 *  DESCRIPTION :
 *      Get the mode of MCU
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      mcu_mode   -   the mode of MCU current running firmware; 0: IAP, 1: AP
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
INT32 mcu_halModeGet
(
  OUT UINT32 *mcu_mode
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  data;

    ret = mcu_halDataSet(MCU_GET_VER, 0, 0);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to set MCU data.\n");
        goto __CMD_ERROR;
    }

    ret = mcu_halDataGet(0x1, 0, &data);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to get MCU version\n");
        goto __CMD_ERROR;
    }

    *mcu_mode = (data >> 24) & 0xFF;

__CMD_ERROR:
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mcu_halModeSet
 *
 *  DESCRIPTION :
 *      Set the mode of MCU
 *
 *  INPUT :
 *      mode    -   the mode of MCU; 0: IAP, 1: AP
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
INT32 mcu_halModeSet
(
  IN UINT32 mcu_mode
)
{
    INT32   ret = E_TYPE_SUCCESS;
    UINT32  mode;

    ret = mcu_halModeGet(&mode);
    if( ret != E_TYPE_SUCCESS )
    {
        log_printf("Failed to get MCU mode.\n");
        goto __CMD_ERROR;
    }

    if (mode != mcu_mode)
    {
        ret = (mcu_mode == MCU_AP_MODE) ? mcu_halDataSet(MCU_OPC_USERCD, 0, 0) : mcu_halDataSet(MCU_OPC_IAPCD, 0, 0);
        if( ret != E_TYPE_SUCCESS )
        {
            log_printf("Failed to set MCU mode.\n");
            goto __CMD_ERROR;
        }
    }

__CMD_ERROR:
    return ret;
}
