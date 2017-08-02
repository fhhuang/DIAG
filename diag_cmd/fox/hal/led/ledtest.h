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
***			ledtest.h
***
***    DESCRIPTION :
***
***
***    HISTORY :
***       - 2009/05/12, 10:02:52, Eden Weng
***             File Creation
***
***************************************************************************/

#ifndef __LEDTEST_H_
#define __LEDTEST_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*==========================================================================
 *                                                                          
 *      Function Definition Segment
 *                                                                          
 *==========================================================================
 */
INT32 ledtest
(
	IN  E_LED_TEST_MODE    mode
);

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
);

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
);

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
);

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
);



#ifdef __cplusplus
}
#endif

#endif /* __LEDTEST_H_ */
