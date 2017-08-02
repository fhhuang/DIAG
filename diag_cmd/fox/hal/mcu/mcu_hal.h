#ifndef __MCU_HAL_H_
#define __MCU_HAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define MCU_ADDR_HIGH   0
#define MCU_ADDR_LOW    2
#define MCU_DATA_LEN    4
#define MCU_DUM_BYTE    6
#define MCU_DATA        7
#define MCU_WRITE_SIZE  13
#define MCU_READ_SIZE   6
#define MCU_MAX_SIZE    13
#define MCU_OPC_EMPTY   0x00
#define MCU_OPC_READ    0x03
#define MCU_OPC_WRITE   0x06
#define MCU_OPC_UPGRADE 0x30
#define MCU_OPC_ERPG    0x20
#define MCU_OPC_ERUSM   0x60
#define MCU_OPC_USERCD  0x77
#define MCU_OPC_IAPCD   0x88

#define MCU_BASE_ADDR   0x8000000
#define TEST_DATA_ADDR  0x8007800

#define MCU_MAX_ADDR    0x7FFF
#define MCU_AP_ADDR     0x8004000
#define MCU_DEV_ADDR    0x10
#define MCU_BUF_SIZE    200
#define MCU_PACKET_SIZE 209 /* MCU_BUF_SIZE + Address (4 bytes) + Data Length (2 byte) + Dummy (1 byte) + command + length */

typedef struct MCUPkt
{
    UINT8   addr_high[2];
    UINT8   addr_low[2];
    UINT8   len[2];
    UINT8   dum_byte;
    UINT8   data[MCU_DATA_LEN];
}MCU_PKT;


/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef enum {
    MCU_GET_DATE = 0x1,
    MCU_GET_TIME,
    MCU_SET_DATE,
    MCU_SET_TIME,
    MCU_SET_LED,
    MCU_GET_VOL_3_3,
    MCU_GET_VOL_2_5,
    MCU_GET_VOL_1_8,
    MCU_GET_VOL_1_5,
    MCU_GET_VOL_0_99,
    MCU_GET_VOL_0_9,
    MCU_SET_VOL_MAR,
    MCU_GET_MAC_TEMP,
    MCU_GET_PHY_TEMP,
    MCU_GET_DDR_TEMP,
    MCU_SET_SPEED,
    MCU_GET_SPEED,
    MCU_WRITE_DATA   = 0x30,
    MCU_INT_TOGGLE   = 0x80,
    MCU_GET_VER      = 0x90,
    MCU_GET_VER_DATE = 0x91,
    MCU_RESET_READ   = 0x98,
    MCU_ERASE_WRITE  = 0x99
}E_MCU_CMD;

typedef enum {
    MCU_IAP_MODE,
    MCU_AP_MODE
}E_MCU_MODE;
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
  IN UINT32  addr,
  OUT UINT32 *read_data
);

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
);


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
);


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
);

#ifdef __cplusplus
}
#endif

#endif /* __MCU_HAL_H_ */
