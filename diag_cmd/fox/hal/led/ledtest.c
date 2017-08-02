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
***      ledtest.c
***
***    DESCRIPTION :
***      for led test
***
***    HISTORY :
***       - 2009/05/12, 10:30:52, Eden Weng
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
#include "port_utils.h"
#include "err_type.h"
#include "log.h"
#include "porting.h"
#include "sys_utils.h"
#include "led_hal.h"

/*==========================================================================
 *
 *      Static Variable segment
 *
 *==========================================================================
 */
static UINT32  FirstRandNum = 0;
static UINT32  SecondRandNum = 0;
static UINT32  RandArray1[8*2+1];
static UINT32  RandArray2[8*2+1];


static INT32 ledPortRandOff
(
    IN UINT32 stage
)
{
    UINT32      i, count, j, p;
    UINT32      randNum;
    UINT32      modNum;
    S_BOARD_INFO  boardInfo;


    sys_utilsBoardInfoGet(&boardInfo);
    modNum = boardInfo.lPortMaxNum;
    
    randNum = get_timer(0)%MAX_RAND_NUM + 1;
    
    if ( stage == 1 )
    {
        FirstRandNum = randNum;
    }
    else
    {
        SecondRandNum = randNum;
    }

    if( stage == 2 )
    {
        while(SecondRandNum == FirstRandNum)
        {
            SecondRandNum = get_timer(0)%MAX_RAND_NUM + 1;
        }    
    }
    
    if(stage == 1)
    {    
        count = FirstRandNum;
        for(i=1 ; i<=count; i++)
        {
            RandArray1[i] = get_timer(0)%modNum + 1;

            /* check if the random number alread been used in RandArray1 */
            for(j =1; j < i; j++)
            {
                while(RandArray1[i] ==  RandArray1[j])
                {
                    RandArray1[i] = get_timer(0)%modNum + 1;
                }
            }
        }       
            
    }
    else
    {    
        count = SecondRandNum; 
        for(i = 1 ; i<=count; i++)
        {
            RandArray2[i] = get_timer(0)%modNum + 1;

            /* check if the random number alread been used in RandArray1 */
            for(j=1;j<=FirstRandNum;j++)
            {
                while(RandArray2[i] ==  RandArray1[j])
                {
                    RandArray2[i] = get_timer(0)%modNum + 1;
                    
                    /* check if the random number alread been used in RandArray2 */
                    for(p =1; p < i; p++)
                    {
                        while(RandArray2[i] ==  RandArray2[p])
                        {
                            RandArray2[i] = get_timer(0)%modNum + 1;
                        }
                    }
                }    
            }

            /* check if the random number alread been used in RandArray2 */
            for(p =1; p < i; p++)
            {
                while(RandArray2[i] ==  RandArray2[p])
                {
                    RandArray2[i] = get_timer(0)%modNum + 1;
                }
            }
        }
    }
    
#ifdef DBG /* debug used */  
    log_printf("modNum = %d\n", modNum);
    log_printf("The FirtRandNum number is %d\n", FirstRandNum);
    log_printf("The SecondRandNum number is %d\n", SecondRandNum);
    for( i = 1; i<=count; i++)
    {
        log_printf("RandArray1[%d] = %d\n", i, RandArray1[i]);
        log_printf("RandArray2[%d] = %d\n", i, RandArray2[i]);
    }
#endif    
     
    /* turn off the fornt port RJ45 led */
    for ( i = 1; i <=count ; i ++ )
    {
        switch_halPhyPageNumSet(((stage == 1)?RandArray1[i]:RandArray2[i]), 3);
        switch_halSMIRegSet(((stage == 1)?RandArray1[i]:RandArray2[i]), 16, 0x1188);
        switch_halPhyPageNumSet(((stage == 1)?RandArray1[i]:RandArray2[i]), 0);                  
    } 

    return count;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      ledAllOn
 *
 *  DESCRIPTION :
 *      Force led on green
 *
 *  INPUT :
 *      led type, copper, sfp , sys led, or all
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
INT32 ledAllOn
(
    E_LED_TYPE led_type
)
{
    UINT32      lPort, ret =E_TYPE_SUCCESS ;
    S_PORT_INFO portInfo;
    S_BOARD_INFO  boardInfo;
    E_LED_SFP_PORT i;
    INT32 j;

    sys_utilsBoardInfoGet(&boardInfo);

    if( led_type & LED_TYPE_SYS )
    {
        /* led port 0 - SYSLED */
        ret = led_halSet(0, LED_TYPE_GREEN, FALSE);
        
        if( ret != E_TYPE_SUCCESS)
            log_printf("Light sys led fail\n");

        for( j=5; j<=7;j++)
        {
            /* led port 5-7*/
            ret = led_halSet(j, LED_TYPE_GREEN, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light sys led fail\n");
        }
    }

    if( led_type & LED_TYPE_SFP )
    {
        for( i= LED_TYPE_SFP_PORT_1; i<=LED_TYPE_SFP_PORT_4; i++)
        {
            ret = led_halSet(i, LED_TYPE_GREEN, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light SFP led_%d led fail\n", i);
        }
    }

    if( led_type & LED_TYPE_COPPER_PORT )
    {
        for ( lPort = 1; lPort <= boardInfo.lPortMaxNum; lPort++ )
        {
    	    switch_halPhyPageNumSet(lPort, 3);
            switch_halSMIRegSet(lPort, 16, 0x1189); /* Force green */
            switch_halPhyPageNumSet(lPort, 0);
        }
    }

    return 0;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      ledChangeColor
 *
 *  DESCRIPTION :
 *      change the led color, green or amber
 *
 *  INPUT :
 *      led type, copper, sfp , sys led, or all
 *      led color
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
INT32 ledChangeColor
(
    IN E_LED_TYPE led_type,
    IN E_LED_COLOR led_color
)
{
    UINT32      lPort;
    S_PORT_INFO portInfo;
    S_BOARD_INFO  boardInfo;
    UINT16 regValue;
    INT32   ret = E_TYPE_SUCCESS;
    E_LED_SFP_PORT i;
    INT32 j;

    sys_utilsBoardInfoGet(&boardInfo);

    if( led_type & LED_TYPE_SYS )
    {
        /* led port 0 - SYSLED */
        ret = led_halSet(0, led_color, FALSE);
        
        if( ret != E_TYPE_SUCCESS)
            log_printf("Light sys led fail\n");

        for( j=5; j<=7;j++)
        {
            /* led port 5-7*/

            /* Speed doesn't support amber */
            if (led_color == LED_TYPE_AMBER && (j == 7))
            {
                led_halSet(j, LED_TYPE_OFF, FALSE);
                continue;
            }
            
            ret = led_halSet(j, led_color, FALSE);
        
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light sys led fail\n");
        }
    }

    if( led_type & LED_TYPE_SFP )
    {
        for( i= LED_TYPE_SFP_PORT_1; i<=LED_TYPE_SFP_PORT_4; i++)
        {
            ret = led_halSet(i, (led_color == LED_TYPE_GREEN) ? led_color : LED_TYPE_OFF, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light SFP led_%d led fail\n", i);
        }
    }    
    
    if( led_type & LED_TYPE_COPPER_PORT )
    {
        for ( lPort = 1; lPort <= boardInfo.lPortMaxNum; lPort++ )
        {
            if( led_color == LED_TYPE_GREEN )
                regValue = 0x1189;
            else if( led_color == LED_TYPE_AMBER )
                regValue = 0x1198;
            else if( led_color == LED_TYPE_OFF )
                regValue = 0x1188;
            
    	    switch_halPhyPageNumSet(lPort, 3);
            switch_halSMIRegSet(lPort, 16, regValue);
            switch_halPhyPageNumSet(lPort, 0);
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      ledAllOff
 *
 *  DESCRIPTION :
 *      Force led off
 *
 *  INPUT :
 *      led type, copper, sfp , sys led, or all
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
INT32 ledAllOff
(
   E_LED_TYPE led_type
)
{
    UINT32      lPort;
    INT32       ret = E_TYPE_SUCCESS;
    S_PORT_INFO portInfo;
    S_BOARD_INFO  boardInfo;
    E_LED_SFP_PORT i;
    INT32 j;

    sys_utilsBoardInfoGet(&boardInfo);

    if( led_type & LED_TYPE_SYS )
    {
        /* led port 0 - SYSLED */
        ret = led_halSet(0, LED_TYPE_OFF, FALSE);
        if( ret != E_TYPE_SUCCESS)
            log_printf("Light off sys led fail\n");

        for( j=5; j<=7;j++)
        {
            /* led port 5-7*/
            ret = led_halSet(j, LED_TYPE_OFF, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light sys led fail\n");
        }    
    }

    if( led_type & LED_TYPE_SFP )
    {
        for( i= LED_TYPE_SFP_PORT_1; i<=LED_TYPE_SFP_PORT_4; i++)
        {
            ret = led_halSet(i, LED_TYPE_OFF, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light SFP led_%d led fail\n", i);
        }
    }

    if( led_type & LED_TYPE_COPPER_PORT )
    {
        for ( lPort = 1; lPort <= boardInfo.lPortMaxNum; lPort++ )
        {
    	    switch_halPhyPageNumSet(lPort, 3);
            switch_halSMIRegSet(lPort, 16, 0x1188);
            switch_halPhyPageNumSet(lPort, 0);
        }
    }

    return 0;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      ledReset
 *
 *  DESCRIPTION :
 *      Reset to factory default
 *
 *  INPUT :
 *      led type, copper, sfp , sys led, or all
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
INT32 ledReset
(
   IN E_LED_TYPE led_type
)
{

    UINT32      lPort;
    S_PORT_INFO portInfo;
    S_BOARD_INFO  boardInfo;
    E_LED_SFP_PORT i;
    INT32       ret = E_TYPE_SUCCESS;
    INT32 j;

    sys_utilsBoardInfoGet(&boardInfo);

    if( led_type & LED_TYPE_SYS )
    {
        /* led port 0 - SYSLED */
        ret = led_halSet(0, LED_TYPE_GREEN, FALSE);
        
        if( ret != E_TYPE_SUCCESS)
            log_printf("Light off sys led fail\n");

        for( j=5; j<=7;j++)
        {
            /* led port 5 -7 STAT */
            ret = led_halSet(j, LED_TYPE_OFF, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light sys led fail\n");
        }    
    }

    if( led_type & LED_TYPE_SFP )
    {
        for( i= LED_TYPE_SFP_PORT_1; i<=LED_TYPE_SFP_PORT_4; i++)
        {
            ret = led_halSet(i, LED_TYPE_OFF, FALSE);
            if( ret != E_TYPE_SUCCESS)
                log_printf("Light SFP led_%d led fail\n", i);
        }
    }

    if( led_type & LED_TYPE_COPPER_PORT )
    {
        for ( lPort = 1; lPort <= boardInfo.lPortMaxNum; lPort++ )
        {
    	    switch_halPhyPageNumSet(lPort, 3);
            switch_halSMIRegSet(lPort, 16, 0x1117);
            switch_halPhyPageNumSet(lPort, 0);
        }
    }

    return 0;
}


/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      ledtest
 *
 *  DESCRIPTION :
 *      Doing the led test, force all on , and randomly off some ports
 *
 *  INPUT :
 *      led test mode - manual, or auto mode
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
INT32 ledtest
(
    IN  E_LED_TEST_MODE    mode
)
{
    INT32 ret = 0, count;

    log_printf("Light on  all LEDs\n");
    ledChangeColor(LED_TYPE_ALL, LED_TYPE_GREEN);
    
    if ( mode == LED_TEST_MANUAL)
    {
        log_printf("press any key to continue.\n");
        GET_CHAR;
    }    
    else
        udelay(2000000);    

    /* Firt random off some port led */
    count = ledPortRandOff(1);
    log_printf("First turn off %d ports\n", count);
    if ( mode == LED_TEST_MANUAL)
    {
        log_printf("press any key to continue.\n");
        GET_CHAR;
    }    
    else
        udelay(2000000);

    ledChangeColor(LED_TYPE_ALL, LED_TYPE_GREEN);
    log_printf("Light on  all LEDs\n");
    if ( mode == LED_TEST_MANUAL)
    {
        log_printf("press any key to continue.\n");
        GET_CHAR;
    }    
    else
        udelay(2000000);
    
    /* Second random off some port led */
    count = ledPortRandOff(2);
    log_printf("Second turn off %d ports\n", count);
    if ( mode == LED_TEST_MANUAL)
    {
        log_printf("press any key to continue.\n");
        GET_CHAR;
    }    
    else
        udelay(2000000);
    
    log_printf("Light off all LEDs\n");
    ledChangeColor(LED_TYPE_ALL, LED_TYPE_OFF);
    if ( mode == LED_TEST_MANUAL)
    {
        log_printf("press any key to continue.\n");
        GET_CHAR;
    }    
    else
        udelay(2000000);
    
    log_printf("Change color\n");
    ledChangeColor(LED_TYPE_ALL, LED_TYPE_AMBER);
    if ( mode == LED_TEST_MANUAL)
    {
        log_printf("press any key to continue.\n");
        GET_CHAR;
    }    
    else
        udelay(2000000);
    
    log_printf("Reset all LEDs\n");
    ledReset(LED_TYPE_ALL);

    log_printf("LEDs test finish\n");

    return ret;
}

