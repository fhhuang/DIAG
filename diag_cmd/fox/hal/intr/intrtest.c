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
***      intrtest.c
***
***    DESCRIPTION :
***      for interrupt test
***
***    HISTORY :
***       - 2009/05/11, 10:30:52, Eden Weng
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
#include "intr_hal.h"
#include "i2c_hal.h"
#include "port_utils.h"
#include "sys_utils.h"

#include "err_type.h"
#include "log.h"
#include "gpio_hal.h"
#include "intr_hal.h"
#include "port_utils.h"
#include "i2c_fpga.h"
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
 *      intrtest_init
 *
 *  DESCRIPTION :
 *      init device setting
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
static INT32 intrtest_init
(
    IN  E_EXT_INTR                  extIntr
)
{
    INT32                           ret = E_TYPE_SUCCESS;

    switch(extIntr)
    {
        case E_EXT_INTR_PUSH_BUTTON:
            break;
        case E_EXT_INTR_SB:
        case E_EXT_INTR_SFP_PRESENT:
        case E_EXT_INTR_SFP_RX_LOSS:
        case E_EXT_INTR_SFP_TX_FAULT:
        case E_EXT_INTR_MCU:
            fpgaRegWrite(FPGA_INT_MASK_REG, 0x0); /* unmask all interrupt */
            break;
        default:
            ret = E_TYPE_INT_NO_SUPPORT;
            goto __INTR_ERROR;
    }

    if (intr_halIntrClear(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_CLEAR;
        goto __INTR_ERROR;
    }
    
__INTR_ERROR:
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intrtest_restore
 *
 *  DESCRIPTION :
 *      restore device setting
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
static INT32 intrtest_restore
(
    IN  E_EXT_INTR                  extIntr
)
{
    INT32                           ret = E_TYPE_SUCCESS;

    /* clear interrupt after test */
    if (intr_halIntrClear(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_CLEAR;
        goto __INTR_ERROR;
    }

    switch(extIntr)
    {
        case E_EXT_INTR_SFP_PRESENT:
        case E_EXT_INTR_SFP_RX_LOSS:
        case E_EXT_INTR_SFP_TX_FAULT:
        case E_EXT_INTR_SB:
        case E_EXT_INTR_PUSH_BUTTON:
            break;
        case E_EXT_INTR_MCU:
            /* retore MCU interrupt setting */
            ret = mcu_halDataSet(0x98, 0, 0x0);

            if( ret != E_TYPE_SUCCESS )
            {
                goto __INTR_ERROR;
            }
            break;
        default:
            ret = E_TYPE_INT_NO_SUPPORT;
            goto __INTR_ERROR;
    }

__INTR_ERROR:
    
    /* mask all interrupt to cpu */
    fpgaRegWrite(FPGA_INT_MASK_REG, 0xff); 

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intrtest_restore
 *
 *  DESCRIPTION :
 *      restore device setting
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
static BOOL intrtest_checkGpioStatus
(
    IN  E_EXT_INTR                  extIntr,
    IN  int                         expect
)
{
    int                             status;
    UINT8                           count;

    for (count = 0 ; count < 5 ; count++)
    {
        /* cpu interrupt is low active */
        if ((intr_halIntStatusGet(extIntr, &status)) == E_TYPE_SUCCESS && 
            (status == expect))
            break;
    }

    if (status != expect)
    {
        log_dbgPrintf("status=%d; expect=%d!\n", status, expect);
        return FALSE;
    }
        
    return TRUE; 
}
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_mgmtTest
 *
 *  DESCRIPTION :
 *      Generate management port interrupt to CPLD and CPU
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
static INT32 intr_PushButtonTest
(
    E_EXT_INTR                      extIntr,
    INT32                               timeout
)
{
    int                                gpioStatus = GPIO_HIGH;
    UINT8                           count=0;
    INT32                           ret = E_TYPE_SUCCESS;

    if (intrtest_init(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }

    /* Check if default status is HIGH , no interrupt */
    if (!intrtest_checkGpioStatus(extIntr, GPIO_HIGH))
    {
        log_printf("CPU can't clear interrupt!\n");
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }
    
    log_printf("\nStart Push Button Interrupt Test\n");
    log_printf("Please press the push button at least 1s\n");
    log_printf("Trigger interrupt...\n");

    while(1)
    {
        if (!intrtest_checkGpioStatus(extIntr, GPIO_LOW))
        {      
                if( count < timeout)
                {
                    log_printf(".");
    
                    /* delay 1000 ms */
                    udelay(1000000);
                    count++;
                }
                else
                {
                    log_printf("\nNo interrupt is triggerd\n");
                    ret = E_TYPE_INT_EN;
                    goto __INTR_ERROR;
                }    
        }    
        else
        {
            log_printf("\nCPU receives interrupt!\n");
            break;
        }
    }    

    log_printf("\nRestore and clear interrupt...");
    if (intrtest_restore(extIntr) != E_TYPE_SUCCESS)
    {
        log_printf("Fail to restore original setting!\n");
        ret = E_TYPE_INT_CLEAR;
        goto __INTR_ERROR;
    }

    log_printf("Done\n");
    return E_TYPE_SUCCESS;
    
__INTR_ERROR:
    if (intrtest_restore(extIntr) != E_TYPE_SUCCESS)
    {
        log_printf("Fail to restore original setting!\n");
    }
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_sfpTest
 *
 *  DESCRIPTION :
 *      To generate an SFP interrupt to CPLD and CPU
 *
 *  INPUT :
 *      extIntr - SFP interrupt type
 *
 *  OUTPUT :
 *
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
static INT32 intr_sfpTest
(
    E_EXT_INTR                      extIntr, 
    INT32                              timeout
)
{
    int                             count=0;
    int                             gpioStatus = GPIO_HIGH;
    INT32                           ret = E_TYPE_SUCCESS;
    UINT16                          regVal, intr_status;
    S_BOARD_INFO                    boardInfo;
    UINT32                          sfpNum;
    UINT32                          i;
    UINT8                            portShift;
    
    
    sys_utilsBoardInfoGet(&boardInfo);

    switch (extIntr)
    {
        case E_EXT_INTR_SFP_RX_LOSS:
            log_printf("Start SFP/SFP+ RX_LOSS interrupt test\n");
            break;
        case E_EXT_INTR_SFP_PRESENT:
            log_printf("Start SFP/SFP+ Present interrupt test\n");
            break;
        case E_EXT_INTR_SFP_TX_FAULT:
            log_printf("Start SFP/SFP+ TX_FAULT interrupt test\n");
            break;
        default:
            break;
    }
    log_printf("Please make sure SFP modules are removed!\n");
    log_printf("Press any key to continue...\n");
    GET_CHAR;

    if (intrtest_init(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }
    
    
    log_printf("Please make sure SFP modules are plugged!\n");
    log_printf("Press any key to continue...\n");
    GET_CHAR;
    log_printf("Trigger interrupt...");
    
    while(1)
    {
        if (!intrtest_checkGpioStatus(extIntr, GPIO_LOW))
        {      
                if( count < timeout)
                {
                    log_printf(".");
    
                    /* delay 1000 ms */
                    udelay(1000000);
                    count++;
                }
                else
                {
                    log_printf("\nNo interrupt is triggerd\n");
                    ret = E_TYPE_INT_EN;
                    goto __INTR_ERROR;
                }    
        }    
        else
        {
            log_printf("\nCPU receives interrupt!\n");
            break;
        }
    }    
    
    log_printf("\nCheck if sfp modules are present\n");
    fpgaRegRead(FPGA_SFP_PRE_REG, &regVal);
    fpgaRegRead(FPGA_INT_STA_REG, &intr_status);
    
    switch(boardInfo.boardId)
    {
         /* 16,8 sku sfp1, sfp2 maps to FPGA bit2,bit3. Ohters are bit0,bit1,bit2,bit3 */
        case E_BOARD_ID_HAYWARDS_8G2G_T:
        case E_BOARD_ID_HAYWARDS_8G2G_P:
        case E_BOARD_ID_HAYWARDS_16G2G_T:
        case E_BOARD_ID_HAYWARDS_16G2G_P:
            portShift =2;   
            sfpNum = 2;
            break;
        default:
            portShift =0;
            sfpNum = 4;
            break;
    }

    for( i=portShift; i< sfpNum+portShift; i++)
    {
        /* 0:present, 1:absent */
        if(((regVal & (1<<i)) >> i))
        {
            if( sfpNum == 4 ) /* 16,8 sku sfp1, sfp2 maps to FPGA bit2,bit3. Ohters are bit0,bit1,bit2,bit3 */
                log_printf("SFP%d is not present\n", i+1);
            else
                log_printf("SFP%d is not present\n", i-1);
            ret = E_TYPE_SFP_NO_PRESENT;
            goto __INTR_ERROR;
        }

        /* check the interrupt status in FPGA */
        if( !((intr_status & (1<<i))>>i))
        {
            if( sfpNum == 4 )  /* 16,8 sku sfp1, sfp2 maps to FPGA bit2,bit3. Ohters are bit0,bit1,bit2,bit3 */
                log_printf("SFP%d does not generate interrupt event\n", i+1);
            else
                log_printf("SFP%d does not generate interrupt event\n", i-1); 
            ret = E_TYPE_INT_DIS;
            goto __INTR_ERROR;
        }   
    }
    
    log_printf("Restore and clear interrupt...");
    if (intrtest_restore(extIntr) != E_TYPE_SUCCESS)
    {
        log_printf("Fail to restore original setting!\n");
        ret = E_TYPE_INT_CLEAR;
        goto __INTR_ERROR;
    }
    
    if (!intrtest_checkGpioStatus(extIntr, GPIO_HIGH))
    {
        log_printf("CPU can't clear interrupt!\n");
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }
    log_printf("Done\n");
    return E_TYPE_SUCCESS;
    
__INTR_ERROR:
    if (intrtest_restore(extIntr) != E_TYPE_SUCCESS)
    {
        log_printf("Fail to restore original setting!\n");
    }
    return ret;
}



/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intr_mcuTest
 *
 *  DESCRIPTION :
 *      To generate an MCU interrupt to FPGA and CPU
 *
 *  INPUT :
 *      extIntr - MCU interrupt type
 *
 *  OUTPUT :
 *
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
static INT32 intr_mcuTest
(
    E_EXT_INTR                     extIntr, 
    INT32                              timeout
)
{
    int                                gpioStatus = GPIO_HIGH;
    UINT8                           count=0;
    INT32                           ret = E_TYPE_SUCCESS;

    if (intrtest_init(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }

     /* Check if default status is HIGH , no interrupt */
    if (!intrtest_checkGpioStatus(extIntr, GPIO_HIGH))
    {
        log_printf("CPU can't clear interrupt!\n");
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }
    
    log_printf("\nStart MCU interrupt test\n");
    log_printf("Trigger interrupt...");
    if (intr_halIntrGen(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }

    while(1)
    {
        if (!intrtest_checkGpioStatus(extIntr, GPIO_LOW))
        {      
                if( count < timeout)
                {
                    log_printf(".");
    
                    /* delay 1000 ms */
                    udelay(1000000);
                    count++;
                }
                else
                {
                    log_printf("\nNo interrupt is triggerd\n");
                    ret = E_TYPE_INT_EN;
                    goto __INTR_ERROR;
                }    
        }    
        else
        {
            log_printf("\nCPU receives interrupt!\n");
            break;
        }
    }    

    log_printf("\nRestore device original setting...");
    /* restore original setting of interrupt mask */
    if (intrtest_restore(extIntr) != E_TYPE_SUCCESS)
    {
        ret = E_TYPE_INT_CLEAR;
        goto __INTR_ERROR;
    }    
    
    if (!intrtest_checkGpioStatus(extIntr, GPIO_HIGH))
    {
        log_printf("CPU can't clear interrupt!\n");
        ret = E_TYPE_INT_EN;
        goto __INTR_ERROR;
    }
    log_printf("Done\n");


__INTR_ERROR:
    return ret;
    
}
/*==========================================================================
 *
 *      External Funtion Body segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      intrtest
 *
 *  DESCRIPTION :
 *      To test an specific interrupt 
 *
 *  INPUT :
 *      intrSource - interrupt type
 *
 *  OUTPUT :
 *
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
INT32 intrtest
(
    E_EXT_INTR                      intrSource,
    INT32                               timeout
)
{
    INT32 ret = E_TYPE_SUCCESS;


    switch (intrSource)
    {
        case E_EXT_INTR_SFP_PRESENT:
        case E_EXT_INTR_SFP_RX_LOSS:
        case E_EXT_INTR_SFP_TX_FAULT:
            if ( (ret = intr_sfpTest(intrSource, timeout)) < E_TYPE_SUCCESS )
            {
                log_printf("SFP interrupt test fail\n");
                goto __INTR_ERROR;
            }
            break;
        case E_EXT_INTR_SB:
            break;  /* After Secure boot ready, we'll checking this */
        case E_EXT_INTR_MCU:
            if ( (ret = intr_mcuTest(intrSource, timeout)) < E_TYPE_SUCCESS )
            {
                log_printf("MCU interrupt test fail\n");
                goto __INTR_ERROR;
            }
            break; 
        case E_EXT_INTR_PUSH_BUTTON:
            if ( (ret = intr_PushButtonTest(intrSource, timeout)) < E_TYPE_SUCCESS )
            {
                log_printf("Push Button  interrupt test fail\n");
                goto __INTR_ERROR;
            }
            break;
        default:
            log_printf("No support this interrupt type\n");
            ret = E_TYPE_INT_NO_SUPPORT;
            break;
    }

__INTR_ERROR:
    return ret;
}

