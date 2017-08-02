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
***      i2c_fpga.h.c
***
***    DESCRIPTION :
***      for FPGA test
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
#include "i2c_fpga.h"
#include "i2c_hal.h"
#include "act2test.h"
#include "tam_library.h"
#include "tam_lib_manufacturing.h"

#include "log.h"
#include "foxCommand.h"

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

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaRegRead
 *
 *  DESCRIPTION :
 *      FPGA register read
 *
 *  INPUT :
 *      regAddr - register address
 *
 *  OUTPUT :
 *      regVal - register value
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
INT32 fpgaRegRead
(
    IN UINT32 regAddr,
    OUT UINT16 *regVal
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT8 val[2] = {0};

    /* Read the value */
    ret = i2c_halRegGet(FPGA_I2C_ADDR, regAddr, FPGA_REG_LEN, val, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to read the value.\n");
        return ret;
    }

    log_dbgPrintf("val[0]=0x%02x, val[1]=0x%02x\n", val[0], val[1]);
    *regVal = ((val[1]<<8)| val[0]);
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaRegWrite
 *
 *  DESCRIPTION :
 *      FPGA register write
 *
 *  INPUT :
 *      regAddr - register address
 *      regVal - register value
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
INT32 fpgaRegWrite
(
    IN UINT32 regAddr,
    IN UINT16 regVal
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT8  val[2] = {0};

    val[0] = (regVal & 0xFF);
    val[1] = ((regVal>>8) & 0xFF);
    log_dbgPrintf("val[0]=0x%02x, val[1]=0x%02x\n", val[0], val[1]);
    /* Set the value */
    ret = i2c_halRegSet(FPGA_I2C_ADDR, regAddr, FPGA_REG_LEN, val, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to set the value.\n");
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaBitSet
 *
 *  DESCRIPTION :
 *      FPGA bit set
 *
 *  INPUT :
 *      regAddr - register address
 *      bitNum  - bit number
 *      enable  - enable/disable
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
INT32 fpgaBitSet
(
    IN UINT32 regAddr,
    IN UINT32 bitNum,
    IN BOOL   enable
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 regVal = 0;

    /* Read default value */
    ret = i2c_halRegGet(FPGA_I2C_ADDR, regAddr, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to read the default value.\n");
        return ret;
    }

    regVal |= (1<<bitNum);

    /* Set the value */
    ret = i2c_halRegSet(FPGA_I2C_ADDR, regAddr, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to set the value.\n");
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaBitGet
 *
 *  DESCRIPTION :
 *      FPGA bit get
 *
 *  INPUT :
 *      regAddr - register address
 *      bitNum  - bit number
 *      enable  - enable/disable
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
INT32 fpgaBitGet
(
    IN UINT32 regAddr,
    IN UINT32 bitNum,
    IN BOOL   *bitVal
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 regVal = 0;

    /* Read default value */
    ret = i2c_halRegGet(FPGA_I2C_ADDR, regAddr, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to read the default value.\n");
        return ret;
    }

    *bitVal = ((regVal>>bitNum) & 0x1);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      sfpClkSel
 *
 *  DESCRIPTION :
 *      FPGA to switch the SFP clock channel
 *
 *  INPUT :
 *      clkNum - sfp clock channel
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
INT32 fpga_sfpClkSel
(
    IN UINT32 clkNum
)
{
    INT32  ret = E_TYPE_SUCCESS;

    UINT16 regVal = 0;

    /* Read default value */
    ret = i2c_halRegGet(FPGA_I2C_ADDR, FPGA_SFP_DISABLE_REG, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to read the default value.\n");
        return ret;
    }

    /* bit[5:4] sfp channel select*/
    regVal &= FPGA_SFP_SEL_MASK;
    regVal |= ((clkNum-1)<<4);

    /* Set the value */
    ret = i2c_halRegSet(FPGA_I2C_ADDR, FPGA_SFP_DISABLE_REG, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to set the value.\n");
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpga_boardIdGet
 *
 *  DESCRIPTION :
 *      FPGA to get boardId
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      boardId - boardId
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
INT32 fpga_boardIdGet
(
    OUT UINT32 *boardId
)
{
    INT32  retry_time=10, ret = E_TYPE_SUCCESS;
    UINT16 regVal = 0;

    /* Fixed QSGMII init as for board id not matched HW design during CPSS init */
    while(retry_time--)
    {
        regVal = 0;
        /* Read board id value */
        ret = i2c_halRegGet(FPGA_I2C_ADDR, FPGA_BOARD_ID_REG, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
        if ( ret != E_TYPE_SUCCESS )
        {
            log_printf("Fail to read the default value.\n");
            return ret;
        }

        udelay(10000);
        
        /*HwRevId = bit[4:3] == 0x1(Pilot build), BoardId = bit[2:0]*/
        if((regVal != 0xFFFF) && (regVal != 0x0))
            break;
    }

    /* bit[2:0] is the baord Id */
    *boardId = (regVal & 0x7);

    if(retry_time == 0)
    {
        log_printf("[Err]: retry 10 tiems, failed to read board id register.\n");
        ret = E_TYPE_INVALID_DATA;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpga_hwVerGet
 *
 *  DESCRIPTION :
 *      FPGA to get HW version
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      hwVer - HW version
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
INT32 fpga_hwVerGet
(
    OUT UINT32 *hwVer
)
{
    INT32  ret = E_TYPE_SUCCESS;
    UINT16 regVal = 0;

    /* Read HW version value */
    ret = i2c_halRegGet(FPGA_I2C_ADDR, FPGA_HW_VER_REG, FPGA_REG_LEN, (UINT8 *)&regVal, 2);
    if ( ret != E_TYPE_SUCCESS )
    {
        log_printf("Fail to read the default value.\n");
        return ret;
    }

   *hwVer = (regVal & 0xFFFF);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fox_Dump
 *
 *  DESCRIPTION :
 *      fox_Dump
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
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
INT32 fox_Dump
(
    IN UINT32 base_addr,
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
            log_printf("%08x: ", i+base_addr);
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

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaUploadImage
 *
 *  DESCRIPTION :
 *      fpgaUploadImage
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
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
 INT32 fpgaUploadImage
 (
    void *tam_handle,
    uint8_t *file_path,
    uint32_t s_offset,
    uint32_t e_offset
 )
{
    tam_lib_status_t status=TAM_RC_OK;
    uint32_t i=0, file_size=0, addr_offset=0;
    uint32_t idx=0, percent=0;;
    uint16_t data_len = WR_DATA_LEN;
    uint8_t *bitstream_buf=NULL;
    uint8_t tmp_buf[WR_DATA_LEN] = {0};
    FILE *fp;

    bitstream_buf = (uint8_t *)malloc(BITSTREAM_BUF_SIZE);
    if(bitstream_buf == NULL)
    {
        log_printf("malloc %d bytes memory failed.\n",BITSTREAM_BUF_SIZE);
        return E_TYPE_ALLOC_MEM_FAIL;
    }

    memset(bitstream_buf, 0x0, BITSTREAM_BUF_SIZE);
    /* Open Upgrade bitstream, copy to internal buffer */
    fp = fopen(file_path,"rb");
    if (fp == NULL)
    {
        log_printf("ERROR: Bitstream file open failed!\n");
        free(bitstream_buf);
        return E_TYPE_DEV_BUSY;
    }

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    if(file_size > BITSTREAM_BUF_SIZE)
    {
        log_printf("Err: file size %d bytes more than BITSTREAM_BUF_SIZE %d bytes.\n",file_size,BITSTREAM_BUF_SIZE);
        free(bitstream_buf);
        return E_TYPE_OUT_RANGE;
    }
    fseek(fp, 0L, SEEK_SET);
    log_dbgPrintf("file %s, Size = %d bytes!\n", file_path, file_size);

    fread(bitstream_buf, 1, file_size, fp);

    fclose (fp);

    log_dbgPrintf("Erase the SPI from 0x%x to 0x%0x\n", s_offset, e_offset);
    /* Erase FPGA upgrade image area, any write should perform erase first */

    for (i = s_offset; i < e_offset; i += SIZE_64K) {
        status = tam_lib_espi_erase_64k(tam_handle, i);
        if (status != TAM_RC_OK)
        {
            log_printf("Err: erase spi flash offset 0x%08x.(status=%d)\n", i, status);
            free(bitstream_buf);
            return status;
        }
        percent=(i-s_offset+SIZE_64K)*100/(e_offset-s_offset);

        log_printf("\rErase size %d @%p -- %3d%% complete", SIZE_64K, (void *)i, percent);
        fflush (stdout);
    }
    log_printf("\n");

    /* Write FPGA Upgrade bitstream to SPI */
    addr_offset = s_offset;

    /*Write data*/
    while (addr_offset < s_offset+file_size)
    {
        if (addr_offset+data_len > s_offset+file_size) {
            data_len = s_offset + file_size - addr_offset;
        } else {
            data_len = WR_DATA_LEN;
        }

        status = tam_lib_espi_write(tam_handle, addr_offset, data_len, (uint8_t *)&bitstream_buf[idx]);
        if (status != TAM_RC_OK)
        {
            log_printf("Err spi write: Write fpga fw imgage.(status=%d)\n", status);
            free(bitstream_buf);
            return status;
        }

        percent=(idx+data_len)*100/file_size;

        log_printf("\rWrite data size %d @%p -- %3d%% complete", data_len, (void *)addr_offset, percent);
        fflush (stdout);

        memset(tmp_buf, 0, WR_DATA_LEN);
        status = tam_lib_espi_read(tam_handle, addr_offset, data_len, tmp_buf);
        if (status != TAM_RC_OK)
        {
            log_printf("Err spi read: read fpga image buffer back.(status=%d)\n", status);
            free(bitstream_buf);
            return status;
        }

        /* Verify the write data */
        if (memcmp(tmp_buf, (uint8_t *)&bitstream_buf[idx], data_len))
        {
            log_printf("Err: Verify data offset 0x%x size 0x%x failed.\n",addr_offset, data_len);
            log_printf("Raw data:\n");
            fox_Dump(addr_offset, (uint8_t *)&bitstream_buf[idx], WR_DATA_LEN);
            log_printf("Verify data:\n");
            fox_Dump(addr_offset, tmp_buf,WR_DATA_LEN);
            free(bitstream_buf);
            return  E_TYPE_DATA_MISMATCH;
        }

        addr_offset += data_len;
        idx += data_len;
    }

    log_printf("\n");

    free(bitstream_buf);
    return status;
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaUpgrade
 *
 *  DESCRIPTION :
 *      fpgaUpgrade
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
INT32 fpgaUpgrade
(
    uint8_t *file_path,
    uint8_t *fw_file_path
)
{
    act2_device_t act2_device;
    tam_lib_status_t status=TAM_RC_OK;
    int  i=0, ret = 0;
    void *tam_handle = NULL;
    uint16_t mfg_len=0, dev_len=0;
    uint8_t mfg_data[32]={0};
    uint8_t dev_data[32]={0};
    tam_lib_scc_id_t scc_id={0};
    dir_tbl_t dir_tbl;
    uint8_t fpga_status, fw_status;
    void *platform_opaque_handle = NULL;

    memset((void *) &act2_device, 0x00, sizeof(act2_device));
    act2_device.addr = ACT2_I2C_ADDR;
    act2_device.delay_flag = 0;

    /* open device */
    status = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : (&act2_device)), &tam_handle);
    if (TAM_RC_OK != status) {
        log_printf("\n Error: FPGA Device Open Status = 0x%0x-%s\n",
                   status,tam_lib_rc2string(status));
        return (status);
    }

    /* get the SCC_FW_ID data */
    status = tam_lib_scc_read_id(tam_handle, &scc_id);
    if (status != TAM_RC_OK) {
        log_printf("\n%s-line %u ERROR read id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        tam_lib_device_close(&tam_handle);
        return status;
    }
    log_printf("Chip Information:\n");
    log_printf(" Chip Type  : %02x\n", scc_id.chip_type);
    log_printf(" Chip Vendor: %02x\n", scc_id.chip_vendor);
    log_printf(" Chip FW Ver: %02x\n", scc_id.firmware_version);
    log_printf(" Post Result: %02x\n", scc_id.post_result);


    /* Only perform this test for AIkido device */
    if (scc_id.firmware_version == TAM_ACT2_AIKIDO_FW) {

        /* Get MFG ID */
        status = tam_lib_espi_get_mfg_id(tam_handle, &mfg_len, mfg_data);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi mfg id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_printf("MFG ID = ");
        for (i = 0; i < mfg_len; i++) {
            log_printf("%02x ", mfg_data[i]);
        }
        log_printf("\n");

        /* Get DEV ID */
        status = tam_lib_espi_get_dev_id(tam_handle, &dev_len, dev_data);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi dev id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_printf("DEV ID = ");
        for (i = 0; i < dev_len; i++) {
            log_printf("%02x ", dev_data[i]);
        }
        log_printf("\n");

        /* Read SPI Directory Table, size of 12 Bytes */
        status = tam_lib_espi_read(tam_handle, SPI_DIR_TABLE_START, SPI_DIR_TABLE_SIZE, (uint8_t *)&dir_tbl);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi directory table status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_printf("SPI Directory Table:\n");
        log_printf("  Golden Start Address = %p\n", (void *)dir_tbl.golden_start);
        log_printf("  Golden Version = 0x%04x\n", dir_tbl.golden_ver);
        log_printf("  Update Start Address = %p\n", (void *)dir_tbl.update_start);
        log_printf("  Update Version = 0x%04x\n", dir_tbl.update_ver);

        /* fpga image: from 0x180000-0x200000, Size 0x80000 512KB */
        log_printf("Upgrade fpga image...\n");
        ret = fpgaUploadImage(tam_handle,file_path, UPDATE_FPGA_START, USER_SPI_ADDR_END);
        if (ret != TAM_RC_OK)
        {
            log_printf("Err: Upload image %s to spi failed.\n", file_path);
            act2_local_close_device(&tam_handle);
            return ret;
        }

        /* fpga fw image: from 0x180000-0x200000, Size 0x80000 512KB */
        log_printf("Upgrade the FW image...\n");
        ret = fpgaUploadImage(tam_handle,fw_file_path, UPDATE_FW_START, GOLDEN_FPGA_START);
        if (ret != TAM_RC_OK)
        {
            log_printf("Err: Upload image %s to spi failed.\n", fw_file_path);
            act2_local_close_device(&tam_handle);
            return ret;
        }

        /* Perform the eSPI authentication for upgrade image */
       // FIXME:comment out for now, the release version need to call this cmd to authenticate image
#if 0
        status = tam_lib_espi_authenticate(tam_handle, UPGRADE_IMAGE, &fpga_status, &fw_status);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR spi authenticate status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            tam_lib_device_close(&tam_handle);
            return status;
        }

        if (fpga_status != TAM_RC_OK || fw_status != TAM_RC_OK)
        {
            log_printf("Err: Authentication Failed!! FPGA_STATUS=0x%02x FW_STATUS=0x%02x\n",
                   fpga_status, fw_status);
            tam_lib_device_close(&tam_handle);
            return fpga_status? fpga_status:fw_status;
        }
#endif
        act2_local_close_device(&tam_handle);

        log_printf("FPGA image upgrade completely.\n");
    } else {
        /* Not Aikido FW, skip the test */
        log_printf("This is not AIkido FW, skip this test.\n");
        act2_local_close_device(&tam_handle);
        return E_TYPE_UNSUPPORT;
    }

    log_printf("\n");
    return status;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaUpdateSpiDir
 *
 *  DESCRIPTION :
 *      fpgaUpdateSpiDir
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
INT32 fpgaUpdateSpiDir
(
    uint16_t golden_ver,
    uint16_t update_ver
)
{
    act2_device_t act2_device;
    tam_lib_status_t status=TAM_RC_OK;
    int  i=0, ret = 0;
    void *tam_handle = NULL;
    uint16_t mfg_len=0, dev_len=0;
    uint8_t mfg_data[32]={0};
    uint8_t dev_data[32]={0};
    tam_lib_scc_id_t scc_id={0};
    dir_tbl_t dir_tbl, verify_dir_tbl;
    void *platform_opaque_handle = NULL;

    memset((void *) &act2_device, 0x00, sizeof(act2_device));
    act2_device.addr = ACT2_I2C_ADDR;
    act2_device.delay_flag = 0;

    /* open device */
    status = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : &act2_device), &tam_handle);
    if (TAM_RC_OK != status) {
        log_printf("\n Error: FPGA Device Open Status = 0x%0x-%s\n",
                   status,tam_lib_rc2string(status));
        return (status);
    }

    /* get the SCC_FW_ID data */
    status = tam_lib_scc_read_id(tam_handle, &scc_id);
    if (status != TAM_RC_OK) {
        log_printf("\n%s-line %u ERROR read id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        act2_local_close_device(&tam_handle);
        return status;
    }
    log_dbgPrintf("Chip Information:\n");
    log_dbgPrintf(" Chip Type  : %02x\n", scc_id.chip_type);
    log_dbgPrintf(" Chip Vendor: %02x\n", scc_id.chip_vendor);
    log_dbgPrintf(" Chip FW Ver: %02x\n", scc_id.firmware_version);
    log_dbgPrintf(" Post Result: %02x\n", scc_id.post_result);


    /* Only perform this test for AIkido device */
    if (scc_id.firmware_version == TAM_ACT2_AIKIDO_FW) {

        /* Get MFG ID */
        status = tam_lib_espi_get_mfg_id(tam_handle, &mfg_len, mfg_data);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi mfg id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_printf("MFG ID = ");
        for (i = 0; i < mfg_len; i++) {
            log_printf("%02x ", mfg_data[i]);
        }
        log_printf("\n");

        /* Get DEV ID */
        status = tam_lib_espi_get_dev_id(tam_handle, &dev_len, dev_data);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi dev id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_printf("DEV ID = ");
        for (i = 0; i < dev_len; i++) {
            log_printf("%02x ", dev_data[i]);
        }
        log_printf("\n");

        memset((uint8_t *)&dir_tbl, 0x0, sizeof(dir_tbl_t));
        /* Read SPI Directory Table, size of 12 Bytes */
        status = tam_lib_espi_read(tam_handle, SPI_DIR_TABLE_START, SPI_DIR_TABLE_SIZE, (uint8_t *)&dir_tbl);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi directory table status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_printf("Default SPI Directory Table:\n");
        log_printf("  Golden Start Address = %p\n", (void *)dir_tbl.golden_start);
        log_printf("  Golden Version = 0x%04x\n", dir_tbl.golden_ver);
        log_printf("  Update Start Address = %p\n", (void *)dir_tbl.update_start);
        log_printf("  Update Version = 0x%04x\n", dir_tbl.update_ver);


        log_printf("Update SPI directory table...\n");
        /* Update SPI Directory Table */
        dir_tbl.golden_start = GOLDEN_FPGA_START;
        dir_tbl.golden_ver = golden_ver;
        dir_tbl.update_start = UPDATE_FPGA_START;
        dir_tbl.update_ver = update_ver;

        log_printf("\nUpdate SPI Directory Table:\n");
        log_printf("  Golden Start Address = %p\n", (void *)dir_tbl.golden_start);
        log_printf("  Golden Version = 0x%04x\n", dir_tbl.golden_ver);
        log_printf("  Update Start Address = %p\n", (void *)dir_tbl.update_start);
        log_printf("  Update Version = 0x%04x\n", dir_tbl.update_ver);

        /* first erase the spi directory table */
        status = tam_lib_espi_erase_4k(tam_handle, 0);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR erase spi dir table status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        /* Update spi directory table */
        status = tam_lib_espi_write(tam_handle, SPI_DIR_TABLE_START, SPI_DIR_TABLE_SIZE,
                                        (uint8_t *)&dir_tbl);
        if (status != TAM_RC_OK)
        {
            log_printf("ERROR: SPI write: Update SPI directory table.\n");
            log_printf("\n%s-line %u ERROR update SPI directory table status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        memset((uint8_t *)&verify_dir_tbl, 0x0, sizeof(dir_tbl_t));
        /* Read SPI Directory Table, size of 12 Bytes */
        status = tam_lib_espi_read(tam_handle, SPI_DIR_TABLE_START, SPI_DIR_TABLE_SIZE, (uint8_t *)&verify_dir_tbl);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi directory table status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        /* Verify the write data */
        if (memcmp((uint8_t *)&dir_tbl, (uint8_t *)&verify_dir_tbl, SPI_DIR_TABLE_SIZE))
        {
            log_printf("Err: Verify spi dir table data failed.\n");
            log_printf("Raw data:\n");
            fox_Dump(0, (uint8_t *)&dir_tbl, SPI_DIR_TABLE_SIZE);
            log_printf("Verify data:\n");
            fox_Dump(0, (uint8_t *)&verify_dir_tbl,SPI_DIR_TABLE_SIZE);
            return  E_TYPE_DATA_MISMATCH;
        }

        act2_local_close_device(&tam_handle);

        log_printf("FPGA SPI table update completely.\n");

    } else {
        /* Not Aikido FW, skip the test */
        log_printf("This is not AIkido FW, skip this test.\n");
        act2_local_close_device(&tam_handle);
        return E_TYPE_UNSUPPORT;
    }

    log_printf("\n");
    return status;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fpgaSpiDump
 *
 *  DESCRIPTION :
 *      fpgaSpiDump
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
INT32 fpgaSpiDump
(
    uint32_t start_offt,
    uint32_t length
)
{
    act2_device_t act2_device;
    tam_lib_status_t status=TAM_RC_OK;
    int  i=0, ret = 0;
    void *tam_handle = NULL;
    uint16_t mfg_len=0, dev_len=0;
    uint8_t mfg_data[32]={0};
    uint8_t dev_data[32]={0};
    tam_lib_scc_id_t scc_id={0};
    dir_tbl_t dir_tbl, verify_dir_tbl;
    uint8_t fpga_status, fw_status;
    uint8_t *buf = NULL;
    void *platform_opaque_handle = NULL;

    memset((void *) &act2_device, 0x00, sizeof(act2_device));
    act2_device.addr = ACT2_I2C_ADDR;
    act2_device.delay_flag = 0;

    /* open device */
    status = act2_local_open_device(((gSpiFlag) ? platform_opaque_handle : &act2_device), &tam_handle);
    if (TAM_RC_OK != status) {
        log_printf("\n Error: FPGA Device Open Status = 0x%0x-%s\n",
                   status,tam_lib_rc2string(status));
        return (status);
    }

    /* get the SCC_FW_ID data */
    status = tam_lib_scc_read_id(tam_handle, &scc_id);
    if (status != TAM_RC_OK) {
        log_printf("\n%s-line %u ERROR read id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
        act2_local_close_device(&tam_handle);
        return status;
    }
    log_dbgPrintf("Chip Information:\n");
    log_dbgPrintf(" Chip Type  : %02x\n", scc_id.chip_type);
    log_dbgPrintf(" Chip Vendor: %02x\n", scc_id.chip_vendor);
    log_dbgPrintf(" Chip FW Ver: %02x\n", scc_id.firmware_version);
    log_dbgPrintf(" Post Result: %02x\n", scc_id.post_result);


    /* Only perform this test for AIkido device */
    if (scc_id.firmware_version == TAM_ACT2_AIKIDO_FW) {

        /* Get MFG ID */
        status = tam_lib_espi_get_mfg_id(tam_handle, &mfg_len, mfg_data);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi mfg id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_dbgPrintf("MFG ID = ");
        for (i = 0; i < mfg_len; i++) {
            log_dbgPrintf("%02x ", mfg_data[i]);
        }
        log_dbgPrintf("\n");

        /* Get DEV ID */
        status = tam_lib_espi_get_dev_id(tam_handle, &dev_len, dev_data);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi dev id status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            return status;
        }

        log_dbgPrintf("DEV ID = ");
        for (i = 0; i < dev_len; i++) {
            log_dbgPrintf("%02x ", dev_data[i]);
        }
        log_dbgPrintf("\n");

        buf = (UINT8 *)malloc(length);
        if (!buf)
        {
            log_printf("Malloc 0x%x bytes fail.\n", length);
            return E_TYPE_ALLOC_MEM_FAIL;
        }

        memset(buf, 0x00, length);

        /* Read SPI content */
        status = tam_lib_espi_read(tam_handle, start_offt, length, buf);
        if (status != TAM_RC_OK)
        {
            log_printf("\n%s-line %u ERROR read spi content status=0x%0x - %s \n",
                 __FUNCTION__, __LINE__, status, tam_lib_rc2string(status));
            act2_local_close_device(&tam_handle);
            free(buf);
            return status;
        }

        log_printf("SPI Content:\n\n");
        fox_Dump(start_offt, buf, length);
        tam_lib_device_close(&tam_handle);
        free(buf);

    } else {
        /* Not Aikido FW, skip the test */
        log_printf("This is not AIkido FW, skip this test.\n");
        act2_local_close_device(&tam_handle);
        return E_TYPE_UNSUPPORT;
    }

    log_printf("\n");
    return status;
}
