#include "cmn_type.h"
#include "port_utils.h"
#include "err_type.h"
#include "log.h"
#include "porting.h"
#include "sys_utils.h"
#include "led_hal.h"
#include "mcu_hal.h"
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
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      led_halSet
 *
 *  DESCRIPTION :
 *      config the led behavior into MCU 
 *
 *  INPUT :
 *      port_num    -   0: system led, 1-4 maps to SFP 1-4
 *      color    -      0: off, 1:green, 2:amber
 *      blink    -      0: no blink, 1:blink
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
INT32 led_halSet
(
  IN UINT8 port_num, 
  IN E_LED_COLOR color, 
  IN BOOL blink
)
{
    E_BOARD_ID boardId = sys_utilsDevBoardIdGet();
    UINT32 ret= E_TYPE_SUCCESS;
    UINT32 write_data;

    write_data = (INT32)((blink << 16) | (color << 8) | port_num); 

    if( (boardId == E_BOARD_ID_HAYWARDS_16G2G_T) ||
        (boardId == E_BOARD_ID_HAYWARDS_16G2G_P) ||
        (boardId == E_BOARD_ID_HAYWARDS_8G2G_T) ||
        (boardId == E_BOARD_ID_HAYWARDS_8G2G_P) )
    {
        if(port_num == 1)
        {
		    port_num = port_num+3;
		}
        else if(port_num == 2)
        {
            port_num = port_num+1;
        }
    }

    if((ret = mcu_halDataSet(MCU_SET_LED, 0, write_data)) != E_TYPE_SUCCESS)
    {
        log_printf("%s: Get MCU data fail\n", __FUNCTION__);
    }
    
    return ret;
}

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      led_halInit
 *
 *  DESCRIPTION :
 *      config the default led behavior
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
INT32 led_halInit
(
  void
)
{
    UINT32    ret =E_TYPE_SUCCESS ;

    /* led port 0 - SYSLED */
    ret = led_halSet(0, LED_TYPE_GREEN, FALSE);
    
    return ret;     
}