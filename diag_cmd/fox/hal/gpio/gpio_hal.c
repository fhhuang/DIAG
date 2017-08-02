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
***      gpio_hal.c
***
***    DESCRIPTION :
***      for gpio test
***
***    HISTORY :
***       - 2015/08/03, 16:30:52, Wed Chen
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

/* HAL-dep */
#include "gpio_hal.h"
#include "err_type.h"
#include "log.h"
#include <fcntl.h>
#include <stdarg.h>
#include <sys/mman.h>


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halInit
 *
 *  DESCRIPTION :
 *      Init GPIO
 *
 *  INPUT :
 *      none
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
INT32 gpio_halInit
(
    void
)
{
    INT32 ret=E_TYPE_SUCCESS;
    UINT32 gpioPin=0, regval=0;

    for(gpioPin=5; gpioPin<=8; gpioPin++)
    {
        ret = gpio_halExport(gpioPin);

        /* Don't config the these pin because it already init in previous stage , MPP5, MPP7 used for FPGA. MPP9 used for interrupt */
        if((gpioPin == 5) || (gpioPin == 7))
            continue;
        
        ret = gpio_halSetDir(gpioPin, 1);
        /* Workaround to avoid NFS boot failed */
        gpio_halSetVal(gpioPin, 1);
    }

    for(gpioPin=17; gpioPin<=18; gpioPin++)
    {
        ret = gpio_halExport(gpioPin);
        ret = gpio_halSetDir(gpioPin, 0);
    }

    /* MPP9, MPP13 set it as GPIO input */
    ret = gpio_halExport(9);
    ret = gpio_halSetDir(9, 0);

    ret = gpio_halExport(13);
    ret = gpio_halSetDir(13, 0);
    

    return ret;
}

INT32 gpio_halReset(void)
{
    UINT32 gpioPin=13;

    gpio_halExport(gpioPin);
    gpio_halSetDir(gpioPin, 1);
    gpio_halSetVal(gpioPin, 0);

    return E_TYPE_SUCCESS;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halGetVal
 *
 *  DESCRIPTION :
 *      Get GPIO data in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
 *
 *  OUTPUT :
 *      *val       -   GPIO value
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
INT32 gpio_halGetVal
(
    UINT32 pin,
    UINT8  *val
)
{
    int ret=E_TYPE_SUCCESS;

    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    /* Enable GPIO pin 5~13, 17 and 18. */
    if( (pin <5) || (pin > 18) )
    {
        return E_TYPE_IO_ERROR;
    }
    else
    {
        if ((pin > 13) && (pin < 17))
        {
            return E_TYPE_IO_ERROR;
        }
    }

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        log_printf("Failed to open gpio value for reading!\n");
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }

    if (-1 == read(fd, value_str, 3)) {
        log_printf("Failed to read value!\n");
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }

    *val = (UINT8)atoi(value_str);

__FUN_RET:
    close(fd);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halSetVal
 *
 *  DESCRIPTION :
 *      Set GPIO data in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
 *      val        -   GPIO value
 *
 *  OUTPUT :
 *      ret
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
INT32 gpio_halSetVal
(
    UINT32 pin,
    UINT8  val
)
{
    int ret=E_TYPE_SUCCESS;

    static const char s_values_str[] = "01";
    char path[VALUE_MAX];
    int fd;

    /* Enable GPIO pin 5~13, 17 and 18. */
    if( (pin <5) || (pin > 18) )
    {
        return E_TYPE_IO_ERROR;
    }
    else
    {
        if ((pin > 13) && (pin < 17))
        {
            return E_TYPE_IO_ERROR;
        }
    }

    /* Set GPIO direction */
    if( (pin>=6) || (pin<=12) )
    {
        snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
        fd = open(path, O_WRONLY);
        if (-1 == fd) {
            log_printf("Failed to open gpio value for writing!\n");
            ret = E_TYPE_IO_ERROR;
            goto __FUN_RET;
        }
    
        if (1 != write(fd, &s_values_str[GPIO_LOW == val ? 0 : 1], 1)) {
            log_printf("Failed to write gpio value!\n");
            ret = E_TYPE_IO_ERROR;
            goto __FUN_RET;
        }

__FUN_RET:
        close(fd);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpiotest
 *
 *  DESCRIPTION :
 *      Execute gpio teset in SYSFS
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      ret
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
INT32 gpiotest
(
    void
)
{
    UINT8 val=0;
    INT32 ret=E_TYPE_SUCCESS;

    /* Set GPIO6 for testing*/
    ret = gpio_halSetVal(6, 0);
    if(ret != E_TYPE_SUCCESS)
    {
        return E_TYPE_REG_WRITE;
    }

    ret = gpio_halGetVal(6, &val);
    if(ret != E_TYPE_SUCCESS)
    {
        return E_TYPE_REG_READ;
    }

    if(val != 0)
    {
        return E_TYPE_REG_WRITE;
    }

    ret = gpio_halSetVal(6, 1);
    if(ret != E_TYPE_SUCCESS)
    {
        return E_TYPE_REG_WRITE;
    }

    log_printf("GPIO test - %s\n", (!ret)? "PASS":"FAIL" );
    return ret;
}


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halExport
 *
 *  DESCRIPTION :
 *      Export GPIO in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
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
INT32 gpio_halExport
(
    UINT32 gpioPin
)
{
    char buffer[BUFFER_MAX];
    int fd, ret=E_TYPE_SUCCESS;
    UINT32 bytes_written;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        log_printf("Failed to open fd export, %ld.\n", fd);
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", gpioPin);
    write(fd, buffer, bytes_written);

__FUN_RET:
    close(fd);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halUnExport
 *
 *  DESCRIPTION :
 *      Un-export GPIO in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
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
INT32 gpio_halUnExport
(
    UINT32 gpioPin
)
{
    char buffer[BUFFER_MAX];
    int fd, ret=E_TYPE_SUCCESS;
    UINT32 bytes_written;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        log_printf("Failed to open fd export, %ld.\n", fd);
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", gpioPin);
    write(fd, buffer, bytes_written);

__FUN_RET:
    close(fd);

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      gpio_halSetDir
 *
 *  DESCRIPTION :
 *      Set GPIO direction in SYSFS
 *
 *  INPUT :
 *      gpioPin    -   GPIO index
 *      dir        -   GPIO direction
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
INT32 gpio_halSetDir
(
    UINT32 gpioPin,
    UINT8  dir
)
{
    static const char s_directions_str[]  = "in\0out";
    char path[DIRECTION_MAX];
    int fd, ret;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", gpioPin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        log_printf("Failed to open gpio direction for writing, %d!\n", fd);
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }

    if (-1 == write(fd, &s_directions_str[0 == dir ? 0 : 3], 0 == dir ? 2 : 3)) {
        log_printf("Failed to set direction\n");
        ret = E_TYPE_IO_ERROR;
        goto __FUN_RET;
    }

__FUN_RET:
    close(fd);

    return ret;
}
