#ifndef __FAN_HAL_H_
#define __FAN_HAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mcu_hal.h"
/*==========================================================================
 *
 *       Constant
 *
 *========================================================================== */
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
 *      External Funtion Body segment
 *
 *==========================================================================
 */

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fan_halSetSpeed
 *
 *  DESCRIPTION :
 *      write fan speed
 *
 *  INPUT :
 *      fan_source    -   fan index
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
INT32 fan_halSetSpeed
(
  IN UINT32 fan_speed
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      fantest
 *
 *  DESCRIPTION :
 *      perform fan test
 *
 *  INPUT :
 *      duration        - the duration during different phases of fan test
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
INT32 fantest
(
    void
);

#ifdef __cplusplus
}
#endif

#endif /* __FAN_HAL_H_ */



