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
***      mpp_i2c.c
***
***    DESCRIPTION :
***      for MPP simulation I2C
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
#include <sys/mman.h>
#include "cmn_type.h"
#include "sys_utils.h"
#include "err_type.h"
#include "porting.h"
#include "mem_hal.h"
#include "mpp_i2c.h"

#include "log.h"
#include "foxCommand.h"

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */

INT32 devmemFd=0;
UINT32 *reg_vaddr=NULL;

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
 *      fabonaci
 *
 *  DESCRIPTION :
 *      fabonaci
 *
 *  INPUT :
 *      n - n
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
UINT32 recure(UINT8 n)
{
    if (n <= 2)
        return 1;
    else
        return recure(n - 1) + recure(n - 2);
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      usdelay
 *
 *  DESCRIPTION :
 *      usdelay
 *
 *  INPUT :
 *      usec - usdelay time
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void usdelay
(
    IN UINT32 usec
)
{
    UINT32 i=0;

    for( i=0; i<usec; i++ )
    {
        recure(2);
    }
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      memMap
 *
 *  DESCRIPTION :
 *      memMap
 *
 *  INPUT :
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
static __inline memMap(UINT8 flag)
{
    if(flag == 1)
    {
        if ((devmemFd=open("/dev/mem", O_RDWR)) < 0)
        {
            printf("Failed to open /dev/mem. ");
            exit (1);
        }

        reg_vaddr = (UINT32 *) mmap(0, (size_t) MAP_SPACE_SIZE, PROT_READ|PROT_WRITE, \
                                    MAP_SHARED, devmemFd, (off_t)IO_BASE_ADDR);

        if (reg_vaddr < 0) {
            printf("mmap() fail\n");
            exit (1);
        }
    }
    else
    {
        munmap(reg_vaddr, MAP_SPACE_SIZE);
        close(devmemFd);
    }
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mppRegRead
 *
 *  DESCRIPTION :
 *      memory register read
 *
 *  INPUT :
 *      regOffset - register offset
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
INT32 mppRegRead
(
    IN UINT32 regOffset,
    IN UINT32 regLength
)
{
    UINT32 regValue = 0;
    regOffset = regOffset & 0xffff;

    regValue = reg_vaddr[regOffset>>2];

    return regValue;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      mppRegWrite
 *
 *  DESCRIPTION :
 *      memory register read
 *
 *  INPUT :
 *      regOffset - register offset
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
INT32 mppRegWrite
(
    IN UINT32 regOffset,
    IN UINT32 regBit,
    IN UINT32 regValue
)
{
    regOffset = regOffset & 0xffff;

    *((UINT32 *)((UINT32)reg_vaddr + regOffset)) = (UINT32)(regValue) & 0xFFFFFFFF;

    return TRUE;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      SetSDASCL_output
 *
 *  DESCRIPTION :
 *      set the SDA and SDC to output mode
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void SetSDASCL_output()
{
    UINT32 reg = 0;

    reg = mppRegRead(MPP_IO_SEL, 1);
    usdelay(DELAY_TIME);
    reg &= ~(SDA | SCL);
    mppRegWrite(MPP_IO_SEL, MPP_ACCESS, reg);
    usdelay(DELAY_TIME);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      SetSDASCL_input
 *
 *  DESCRIPTION :
 *      set the SDA and SDC to input mode
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void SetSDASCL_input()
{
    UINT32 reg = 0;

    reg = mppRegRead(MPP_IO_SEL, 1);
    usdelay(DELAY_TIME);
    reg |= (SDA | SCL);
    mppRegWrite(MPP_IO_SEL, MPP_ACCESS, reg);
    usdelay(DELAY_TIME);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      OSDA
 *
 *  DESCRIPTION :
 *      set the SDA high or low
 *
 *  INPUT :
 *      hl - high or low
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void OSDA(INT32 hl)
{
    UINT32 reg = 0;

    reg = mppRegRead(MPP_OUT_LVL, 1);
    usdelay(DELAY_TIME);
    if(hl==0)
        reg &= ~SDA;
    else
        reg |= SDA;

    mppRegWrite(MPP_OUT_LVL, MPP_ACCESS, reg);
    usdelay(DELAY_TIME);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      OSCL
 *
 *  DESCRIPTION :
 *      set the SCL high or low
 *
 *  INPUT :
 *      hl - high or low
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void OSCL(INT32 hl)
{
    UINT32 reg = 0;

    reg = mppRegRead(MPP_OUT_LVL, 1);
    usdelay(DELAY_TIME);

    if(hl==0)
        reg &= ~SCL;
    else
        reg |= SCL;

    mppRegWrite(MPP_OUT_LVL, MPP_ACCESS, reg);
    usdelay(DELAY_TIME);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      SDA_OutIn
 *
 *  DESCRIPTION :
 *      set the SDA input or output mode
 *
 *  INPUT :
 *      hl - input or output
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void SDA_OutIn (INT32 hl)
{
    UINT32 reg = 0;

    reg = mppRegRead(MPP_IO_SEL, 1);
    usdelay(DELAY_TIME);

    if(hl==0)
            reg &= ~SDA;
    else
            reg |= SDA;

    mppRegWrite(MPP_IO_SEL, MPP_ACCESS, reg);
    usdelay(DELAY_TIME);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_start
 *
 *  DESCRIPTION :
 *      i2c start signal
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void i2c_start()
{

    memMap(1);

    /* SDA SCL output mode */
    SetSDASCL_output();

    OSDA(1);
    OSCL(1);
    OSDA(0);
    OSCL(0);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_dev_addr
 *
 *  DESCRIPTION :
 *      i2c transfer device address
 *
 *  INPUT :
 *      hl  - read or write mode
 *      dev - device address
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void i2c_dev_addr (INT32 hl,INT32 dev)
{
    INT32 device_address=dev;
    INT32 hl_bit;
    INT32 bit_num;

    for(bit_num=0;bit_num<8;bit_num++)
    {
        /*7bit addr, skip the 7bit, chang 7-index to 6-index
          Notice: the address would take the high 7bit or low 7bit + one r/w bit */
        hl_bit = (device_address>>(6-bit_num)) & 0x01;

        if(bit_num==7)
        {
                hl_bit = hl;
        }

        OSDA(hl_bit);
        OSCL(1);
        OSCL(0);
        OSDA(0);
    }

    /* set SDA input mode */
    SDA_OutIn(1);
    OSCL(1);
    OSCL(0);
    /* set SDA output mode */
    SDA_OutIn(0);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      read_data_to_buffer
 *
 *  DESCRIPTION :
 *      read the data back
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      read data bit
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 read_data_to_buffer()
{
    UINT32 reg = 0;

    reg = mppRegRead(MPP_IN_LVL, 1); //read out
    usdelay(DELAY_TIME);
    reg &= SDA;
    reg >>= SDA_BIT;

    return reg;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_write_value
 *
 *  DESCRIPTION :
 *      send the data
 *
 *  INPUT :
 *      value - send data value
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void i2c_write_value (INT32 value)
{
    INT32 bit_num;
    INT32 hl_bit;

    for(bit_num=0 ; bit_num<8 ; bit_num++)
    {
        hl_bit = (value >>(7 - bit_num)) & 0x01;
        OSDA(hl_bit);
        OSCL(1);
        OSCL(0);
        OSDA(0);
    }

    /* set SDA input mode */
    SDA_OutIn(1);
    OSCL (1);
    OSCL (0) ;
    /* set SDA output mode */
    SDA_OutIn(0);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_read_data
 *
 *  DESCRIPTION :
 *      read the data back
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      value - read data value
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
INT32 i2c_read_data()
{
    /*set SDA input mode */
    INT32 read_data = 0;
    INT32 bit_num;
    SDA_OutIn(1);

    for (bit_num=0;bit_num<8;bit_num++)
    {
        read_data <<= 1;
        OSCL(1);
        /* write the data of buffer to "read_data" */
        read_data |= read_data_to_buffer();
        OSCL(0);
    }

    return read_data;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      i2c_stop
 *
 *  DESCRIPTION :
 *      i2c stop signal
 *
 *  INPUT :
 *      None
 *
 *  OUTPUT :
 *      None
 *
 *  RETURN :
 *      None
 *
 *  COMMENT :
 *      none
 *
 *--------------------------------------------------------------------------
 */
void i2c_stop()
{
    OSDA(0);
    OSCL(1);
    OSDA(1);
    /* set SDA SCL input mode  */
    SetSDASCL_input();

    memMap(0);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      I2Cgpio_write
 *
 *  DESCRIPTION :
 *      I2c data write by MPP simulation I2c
 *
 *  INPUT :
 *      dev - devic address
 *      buff - write data buffer
 *      size - write data size
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
UINT32 I2Cgpio_read
(
    IN UINT16   dev,
    IN UINT8   *buff,
    IN UINT32   size
)
{
    UINT32 read_times=0;

    i2c_start();
    i2c_dev_addr (1,dev);
    for(read_times=0 ; read_times<size ; read_times++)
    {
        buff[read_times] = i2c_read_data();

        /* Host send ACK or NACK */
        SDA_OutIn(0);

        /*fist data = 0xFF is not valid, skip, make sure the OP code is not 0xFF, 
          if the first data is 0xFF, the operation would invalid. */
        if(read_times ==(size-1) || buff[0] == 0xFF)
        {
            OSDA(1);
            read_times = size;
        }
        else
            OSDA(0);

        OSCL(1);
        OSCL(0);
        /* set SDA output mode */
        SDA_OutIn(0);
    }
    i2c_stop();

    return ((buff[0] == 0xFF)? 0 : size);
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      I2Cgpio_write
 *
 *  DESCRIPTION :
 *      I2c data read by MPP simulation I2c
 *
 *  INPUT :
 *      dev - devic address
 *      buff - read data buffer
 *      size - read data size
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
UINT32 I2Cgpio_write
(
    IN UINT16 dev,
    IN UINT8 *buff,
    IN UINT32 size
)
{
    UINT32 write_times=0;

    i2c_start();
    i2c_dev_addr (0,dev);
    for(write_times = 0; write_times < size ; write_times++)
        i2c_write_value( buff[write_times] );

    i2c_stop();

    return size;
}
