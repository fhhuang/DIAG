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
***      i2c_hal.c
***
***    DESCRIPTION :
***      for i2c test
***
***    HISTORY :
***       - 2015/08/01, 13:30:52, Wed Chen
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
#include "porting.h"
#include "i2c_hal.h"
#include "i2c_fpga.h"
#include "err_type.h"
#include "log.h"

#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define MAX_RETRY 10

#define I2C_READ     1       /*read flag*/
#define I2C_WRITE    0       /*write flag*/

#define I2C_RETRIES 0x0701  /* number of times a device address      */
                    /* should be polled when not            */
                                    /* acknowledging            */
#define I2C_TIMEOUT 0x0702  /* set timeout - call with int      */
#define I2C_SLAVE_FORCE 0x0706  /* Change slave address         */

typedef struct _MARGIN_INFO_T
{
    UINT32 bus;
    UINT8 path[12];
}BUS_INFO_T;

static BUS_INFO_T i2c_Info[] =
{
    { 0,  "/dev/i2c-0"}
//    { 1,  "/dev/i2c-1"},
//    { 2,  "/dev/i2c-2"}

};

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

//static UINT8 g_busNum = 0;

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

/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
/*-------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_halRegGet
 *
 *  DESCRIPTION :
 *      get register value of i2c device
 *
 *  INPUT :
 *      devAddr -i2c device address
 *      addr - register address
 *      addLen - register length
 *      getLen - register length
 *
 *  OUTPUT :
 *      data - output register data
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
INT32 i2c_halRegGet
(
    IN UINT8 dev_addr,
    IN UINT32 reg_addr,
    IN INT32 addLen,
    OUT UINT8 *value,
    IN INT32 getLen
)
{
    UINT32 ret = 0, fd;
    UINT32 bus_num =0;
    struct i2c_rdwr_ioctl_data i2c_dev;

    if (addLen > 2) {
        log_printf( "Unsupport addrLen %d \n",addLen);
        return -4;
    }

    fd = open(i2c_Info[bus_num].path, O_RDWR);
    if (fd < 0)
    {
        perror("open device fail\n");
        return -1;
    }

    i2c_dev.msgs = (struct i2c_msg*)malloc(2*sizeof(struct i2c_msg));
    if (!i2c_dev.msgs)
    {
        perror("malloc fail\n");
        close(fd);
        return -2;
    }

    /* Set the timeout to 1 second */
    ioctl(fd,I2C_TIMEOUT,1000);
    ioctl(fd,I2C_RETRIES,2);

    i2c_dev.nmsgs = 2;
    i2c_dev.msgs[0].buf = (UINT8 *)malloc(addLen);
    i2c_dev.msgs[0].len = addLen;
    if (addLen == 2) {
        i2c_dev.msgs[0].buf[0] = (reg_addr >> 8) & 0xff;
        i2c_dev.msgs[0].buf[1] = reg_addr & 0xff;
    } else {
        i2c_dev.msgs[0].buf[0] = reg_addr;
    }
    i2c_dev.msgs[0].addr = dev_addr;
    i2c_dev.msgs[0].flags = I2C_WRITE;
    i2c_dev.msgs[1].len = getLen;
    i2c_dev.msgs[1].addr = dev_addr;
    i2c_dev.msgs[1].flags = I2C_READ;
    i2c_dev.msgs[1].buf = (UINT8 *)value;

    ret = ioctl(fd, I2C_RDWR, (UINT32)&i2c_dev);
    if (fd < 0)
    {
        perror("read ioctl error\n");
        close(fd);
        free(i2c_dev.msgs);
        free(i2c_dev.msgs[0].buf);
        return -3;
    }

    close(fd);
    free(i2c_dev.msgs);
    free(i2c_dev.msgs[0].buf);

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_halRegSet
 *
 *  DESCRIPTION :
 *      set register value to i2c device
 *
 *  INPUT :
 *      devAddr -i2c device address
 *      addr - register address
 *      addLen - register length
 *      data - register data
 *      getLen - register length
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
INT32 i2c_halRegSet
(
    IN UINT8 dev_addr,
    IN UINT32 reg_addr,
    IN INT32 addLen,
    IN UINT8 *data,
    IN INT32 setLen
)
{
    int ret, fd,i;
    UINT32 bus_num = 0;
    struct i2c_rdwr_ioctl_data i2c_dev;

    if (addLen > 2) {
        log_printf( "Unsupport addrLen %d \n",addLen);
        return -4;
    }

    fd = open(i2c_Info[bus_num].path, O_RDWR);
    if (fd < 0)
    {
        perror("open device fail\n");
        return -1;
    }

    i2c_dev.msgs = (struct i2c_msg*)malloc(2*sizeof(struct i2c_msg));
    if (!i2c_dev.msgs)
    {
        perror("malloc fail\n");
        close(fd);
        return -2;
    }

    /* Set the timeout to 1 second */
    ioctl(fd,I2C_TIMEOUT,1000);
    ioctl(fd,I2C_RETRIES,10);

    i2c_dev.nmsgs = 1;
    i2c_dev.msgs[0].addr = dev_addr;
    i2c_dev.msgs[0].flags = I2C_WRITE;


    i2c_dev.msgs[0].buf = (UINT8 *)malloc(setLen+addLen);
    i2c_dev.msgs[0].len = setLen+addLen;
    if (addLen == 2) {
        i2c_dev.msgs[0].buf[0] = (reg_addr >> 8) & 0xff;
        i2c_dev.msgs[0].buf[1] = reg_addr & 0xff;
    } else {
        i2c_dev.msgs[0].buf[0] = reg_addr;
    }
    for(i = 0; i<setLen; i++)
        i2c_dev.msgs[0].buf[i+addLen] = data[i];

    ret = ioctl(fd, I2C_RDWR, (UINT32)&i2c_dev);
    if (ret < 0)
    {
        perror("write ioctl error\n");
        close(fd);
        free(i2c_dev.msgs);
        free(i2c_dev.msgs[0].buf);
        return -3;
    }

    close(fd);
    free(i2c_dev.msgs);
    free(i2c_dev.msgs[0].buf);

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_halSfpClkSel
 *
 *  DESCRIPTION :
 *      1x4 SFP clock channel select
 *
 *  INPUT :
 *      i2cSfpClk - i2c SFP clock channel number.(valid value 1-4)
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
INT32 i2c_halSfpClkSel
(
    IN UINT32 i2cSfpClk
)
{
    INT32 ret = E_TYPE_SUCCESS;

    /* Check the valid sfp clock channel number */
    if ( i2cSfpClk < 1 || i2cSfpClk > 4)
    {
        log_printf("Invalid SFP clock is 1-4.\n");
        return E_TYPE_INVALID_PARA;
    }

    ret = fpga_sfpClkSel(i2cSfpClk);
    if( ret < 0 )
    {
        log_printf("failed to select the sfp clock channel.\n");
        ret = E_TYPE_REG_WRITE;
        return ret;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_halSfpInfoGet
 *
 *  DESCRIPTION :
 *      i2c_halSfpInfoGet
 *
 *  INPUT :
 *      lPort - logic port number
 *
 *  OUTPUT :
 *      info  - SFP information data
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
INT32 i2c_halSfpInfoGet
(
    IN  UINT32      lPort,
    OUT S_SFP_MSA * info
)
{
    UINT8       devAddr, offset, addrLen;

    devAddr = SFP_EEPROM_I2C_ADDR;
    offset  = 0;
    addrLen = 1;

    memset(info, 0, sizeof(S_SFP_MSA));

    if ( i2c_halRegGet(devAddr, offset, addrLen, (UINT8 *)info, 256) < E_TYPE_SUCCESS )
    {
        return E_TYPE_REG_READ;
    }
#if 0
    udelay(100*1000);
    /* Store A2 content */
    if ( i2c_halRegGet(devAddr+1, offset, addrLen, (UINT8 *)info->sfpInfoA2, 256)  < E_TYPE_SUCCESS )
    {
        return E_TYPE_REG_READ;
    }
#endif
    return 0;
}

INT32 mcu_halRegGet
(
    IN UINT8 dev_addr,
    IN UINT32 reg_addr,
    IN INT32 addLen,
    OUT UINT8 *value,
    IN INT32 getLen
)
{
    UINT32 ret = 0, fd;
    UINT32 bus_num =0;
    struct i2c_rdwr_ioctl_data i2c_dev;

    if (addLen > 2) {
        log_printf( "Unsupport addrLen %d \n",addLen);
        return -4;
    }

    fd = open(i2c_Info[bus_num].path, O_RDWR);
    if (fd < 0)
    {
        perror("open device fail\n");
        return -1;
    }

    i2c_dev.msgs = (struct i2c_msg*)malloc(2*sizeof(struct i2c_msg));
    if (!i2c_dev.msgs)
    {
        perror("malloc fail\n");
        close(fd);
        return -2;
    }

    ioctl(fd,I2C_TIMEOUT,5);
    ioctl(fd,I2C_RETRIES,2);

    i2c_dev.nmsgs = 1;
    i2c_dev.msgs[0].len = getLen;
    i2c_dev.msgs[0].addr = dev_addr;
    i2c_dev.msgs[0].flags = I2C_READ;
    i2c_dev.msgs[0].buf = (UINT8 *)value;

    ret = ioctl(fd, I2C_RDWR, (UINT32)&i2c_dev);
    if (fd < 0)
    {
        perror("read ioctl error\n");
        close(fd);
        free(i2c_dev.msgs);
        return -3;
    }

    close(fd);
    free(i2c_dev.msgs);

    return E_TYPE_SUCCESS;
}
