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
***         act2test.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2015/11/14, 16:30:52, Lowell Li
***             File Creation
***
***************************************************************************/

#ifndef __ACT2_TEST_H_
#define __ACT2_TEST_H_

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

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
 /* added virt_reg_addr 0620216*/
unsigned int *global_virt_reg_addr;
unsigned int gSpiFlag;
unsigned int gDebug;

#define DISPLAY_LEN             0x10
#define E_TYPE_DIAG_SUCCESS     0
#define ACT2_I2C_READ_RETRIES   200
#define ACT2_I2C_ADDR           0x38
#define DATA_BUF_LEN            1024
typedef unsigned char uint8_t;
typedef unsigned char boolean;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define REPORT_TAM_ERROR(rc) \
do { \
    if (TAM_RC_OK != rc) { \
        log_printf("Error: in %s() Line: %d rc: %d(%s)\n", \
               __FUNCTION__, __LINE__, rc, tam_lib_rc2string(rc)); \
        return (rc); \
    } \
} while(0)

#define MFG_NONCE_OFFSET 0
#define TOKEN_CERT_CHAIN_OFFSET MFG_NONCE_SIZE
/*Lowell: TO be define 2016-02-02 */
#define SIG_LENGTH                  256
#define SIG_FROM_OAT_LENGTH         513
#define CERT_CHAIN_MAX_COUNT        2
#define CLIIP_MAX_COUNT             3
#define SUDI_MAX_COUNT              1
#define CHASSIS_SERIAL_NUM_SIZE     32
#define TAM_LIB_PLATFORM_BUF_SIZE   259
#define PID_SIZE                    32
#define ACT2_PAGESIZE               4096
#define ACT2_ECSKMP_SIZE            8092


/* Endianess macros.                                                        */
#define MV_16BIT_LE(X)  (X)
#define MV_32BIT_LE(X)  (X)
#define MV_64BIT_LE(X)  (X)

/* Added by Alex, 2016/06/13 */
#if 1
#define MAP_SPACE_SIZE                0xFFFFF

#define INTER_REGS_VIRT_BASE          0xF1000000
#define INTER_REGS_BASE               INTER_REGS_VIRT_BASE /* For compatibility */
#define MV_SPI_BYTE_LENGTH_OFFSET     5   /* bit 5 */
#define MV_SPI_BYTE_LENGTH_MASK       (0x1  << MV_SPI_BYTE_LENGTH_OFFSET)
#define MV_SPI_16_BIT_CHUNK_SIZE      2
#define MV_SPI_DUMMY_WRITE_16BITS     0xFFFF
#define MV_SPI_REGS_OFFSET(unit)      (0x10600 + (unit * 0x80))
#define MV_SPI_REGS_BASE(unit)        (MV_SPI_REGS_OFFSET(unit))
#define MV_SPI_IF_CONFIG_REG(spiId)   (MV_SPI_REGS_BASE(spiId) + 0x04)
#define MV_32BIT_LE_FAST(val)         MV_32BIT_LE(val)

#define   MV_SPI_CS_NUM_OFFSET        2
#define   MV_SPI_CS_NUM_MASK          (0x7 << MV_SPI_CS_NUM_OFFSET)

#define MV_MEMIO32_READ(addr) \
        ((*((unsigned int *)(addr))))

#define MV_MEMIO32_WRITE(addr, data) \
        ((*((unsigned int *)(addr))) = ((unsigned int)(data)))

#define MV_MEMIO_LE32_WRITE(addr, data) \
        MV_MEMIO32_WRITE(addr, MV_32BIT_LE_FAST(data))

static inline unsigned int MV_MEMIO_LE32_READ(unsigned int addr)
{
  unsigned int data;

  data = (unsigned int)MV_MEMIO32_READ(addr);

  return (unsigned int)MV_32BIT_LE_FAST(data);
}

#define MV_REG_READ(offset)             \
      (global_virt_reg_addr[offset/4])

#define MV_REG_WRITE(offset, val)    \
    (*((unsigned int*)((unsigned int)global_virt_reg_addr + offset)) = (unsigned int)(val) & 0xFFFFFFFF)

#define   MV_SPI_CTRLR_OFST             0x10600
#define   MV_SPI_INT_CAUSE_MASK_REG     (MV_SPI_CTRLR_OFST + 0x14)
#define   MV_SPI_CS_ENABLE_OFFSET       0   /* bit 0 */
#define   MV_SPI_CS_ENABLE_MASK         (0x1  << MV_SPI_CS_ENABLE_OFFSET)

#define   MV_SPI_IF_CTRL_REG(spiId)     (MV_SPI_REGS_BASE(spiId) + 0x00)
#define   MV_SPI_INT_CAUSE_REG(spiId)   (MV_SPI_REGS_BASE(spiId) + 0x10)
#define   MV_SPI_DATA_OUT_REG(spiId)    (MV_SPI_REGS_BASE(spiId) + 0x08)
#define   MV_SPI_DATA_IN_REG(spiId)     (MV_SPI_REGS_BASE(spiId) + 0x0c)
#define   MV_SPI_WAIT_RDY_MAX_LOOP      100000
#define   MV_SPI_DUMMY_WRITE_8BITS      0xFF
#define   DEV_ID_REG                    0x1823C
#define   FPGA_CS_L                     0x18100
#define   DEVICE_ID_MASK                0xFF00
#define   DEVICE_ID_OFFS                0
#define   MV_SPI_CPOL_OFFSET            11
#define   MV_SPI_CPHA_OFFSET            12
#define   MV_SPI_TXLSBF_OFFSET          13
#define   MV_SPI_RXLSBF_OFFSET          14
#define   MV_SPI_CPOL_MASK              (0x1 << MV_SPI_CPOL_OFFSET)
#define   MV_SPI_CPHA_MASK              (0x1 << MV_SPI_CPHA_OFFSET)
#define   MV_SPI_TXLSBF_MASK            (0x1 << MV_SPI_TXLSBF_OFFSET)
#define   MV_SPI_RXLSBF_MASK            (0x1 << MV_SPI_RXLSBF_OFFSET)
#define   MV_FALSE                      0
#define   MV_TRUE                       (!(MV_FALSE))
#define   _2M                           0x00200000
#define   _8M                           0x00800000
#define   MV_SPI_SPR_OFFSET             0
#define   MV_SPI_SPR_MASK               (0xF << MV_SPI_SPR_OFFSET)
#define   MV_SPI_SPPR_0_OFFSET          4
#define   MV_SPI_SPPR_0_MASK            (0x1 << MV_SPI_SPPR_0_OFFSET)
#define   MV_SPI_SPPR_HI_OFFSET         6
#define   MV_SPI_SPPR_HI_MASK           (0x3 << MV_SPI_SPPR_HI_OFFSET)

#define MV_REG_BIT_SET(offset, bitMask)  \
    (MV_REG_WRITE((offset), \
    (MV_REG_READ(offset) | MV_32BIT_LE_FAST(bitMask))))

#define MV_REG_BIT_RESET(offset,bitMask) \
    (MV_REG_WRITE((offset), \
    (MV_REG_READ(offset) & MV_32BIT_LE_FAST(~bitMask))))

#ifndef MV_SPI_REG_WRITE
#define MV_SPI_REG_WRITE  MV_REG_WRITE
#endif

#ifndef MV_SPI_REG_READ
#define MV_SPI_REG_READ   MV_REG_READ
#endif
#endif

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef int MV_BOOL;
typedef unsigned int            MV_U32;

typedef struct {
    UINT16    addr;         /* I2C device address */
    UINT16    delay_flag;
    UINT32    bitrate;      /* 100 or 400 MHz */
    UINT8     bus_mode;     /* legacy of simple */
    UINT8     firmware_version;
    UINT32    debug_flag;
    UINT32    is_so64;
} act2_device_t;

typedef enum {
    ACT2_PROBE,
    ACT2_PROBE_I2C,
    ACT2_PROGRAM,
    ACT2_PROGRAM_I2C,
    ACT2_ATHEN,
    ACT2_ATHEN_I2C,
    ACT2_ECSKMP,
    ACT2_ECSKMP_I2C,
    ACT2_SPITEST,
    ACT2_SPIREAD,
    ACT2_SPIWRITE,
    ACT2_SPIGPIO5,
    ACT2_MAX
} ACT2_TEST_TYPE;

typedef struct {
    uint16_t    ctrlModel;
    uint32_t  tclk;
} MV_SPI_HAL_DATA;

typedef struct {
    MV_BOOL   clockPolLow;
    enum {
      SPI_CLK_HALF_CYC,
      SPI_CLK_BEGIN_CYC
    }   clockPhase;
    MV_BOOL   txMsbFirst;
    MV_BOOL   rxMsbFirst;
} MV_SPI_IF_PARAMS;

typedef enum {
    SPI_TYPE_FLASH = 0,
    SPI_TYPE_SLIC_ZARLINK_SILABS,
    SPI_TYPE_SLIC_LANTIQ,
    SPI_TYPE_SLIC_ZSI,
    SPI_TYPE_SLIC_ISI
} MV_SPI_TYPE;

typedef struct {
    /* Does this device support 16 bits access */
    MV_BOOL en16Bit;
    /* should we assert / disassert CS for each byte we read / write */
    MV_BOOL byteCsAsrt;
    MV_BOOL clockPolLow;
    MV_U32  baudRate;
    MV_U32 clkPhase;
} MV_SPI_TYPE_INFO;

static MV_SPI_TYPE_INFO spiTypes[] = {
  {
    .en16Bit = MV_TRUE,
    .clockPolLow = MV_TRUE,
    .byteCsAsrt = MV_FALSE,
    .baudRate = (20 << 20), /*  20M */
    .clkPhase = SPI_CLK_BEGIN_CYC
  },
  {
    .en16Bit = MV_FALSE,
    .clockPolLow = MV_TRUE,
    .byteCsAsrt = MV_TRUE,
    .baudRate = _8M,
    .clkPhase = SPI_CLK_BEGIN_CYC
  },
  {
    .en16Bit = MV_FALSE,
    .clockPolLow = MV_TRUE,
    .byteCsAsrt = MV_FALSE,
    .baudRate = _8M,
    .clkPhase = SPI_CLK_BEGIN_CYC
  },
  {
    .en16Bit = MV_FALSE,
    .clockPolLow = MV_TRUE,
    .byteCsAsrt = MV_TRUE,
    .baudRate = _8M,
    .clkPhase = SPI_CLK_HALF_CYC
  },
  {
    .en16Bit = MV_FALSE,
    .clockPolLow = MV_FALSE,
    .byteCsAsrt = MV_FALSE,
    .baudRate = _2M,
    .clkPhase = SPI_CLK_HALF_CYC
  }
};

static MV_SPI_HAL_DATA  spiHalData;
/*==========================================================================
 *
 *      External Funtion Segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaACT2Test
 *
 *  DESCRIPTION :
 *      FPGA ACT2 test
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
INT32 fpgaACT2Test
(
    IN UINT32 probe
);

INT32 mvSpiRead(uint8_t spiId, uint8_t *pRxBuff, uint32_t buffSize);
INT32 mvSpiWrite(uint8_t spiId, uint8_t *pTxBuff, uint32_t buffSize);
INT32 mvSpiParamsSet(uint8_t spiId, uint8_t csId, MV_SPI_TYPE type);
INT32 mvSysSpiInit(uint8_t spiId, uint32_t serialBaudRate);
INT32 mvSpiReadAndWrite(uint8_t spiId, uint8_t *pRxBuff, uint8_t *pTxBuff, uint32_t buffSize);
uint32_t act2_local_open_device(void *platform_opaque_handle, void **tam_handle);
int act2_local_close_device(void **tam_handle);
int tam_lib_platform_spi_write(void *, uint16_t, uint8_t *);
uint32_t tam_lib_platform_spi_read(void *, uint16_t, uint8_t *, uint16_t, uint8_t *, uint16_t *);


#endif /* __MPP_I2C_H_ */
