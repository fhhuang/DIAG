#ifndef __RTC_HAL_H_
#define __RTC_HAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */
typedef struct {
    UINT8 sec;
    UINT8 min;
    UINT8 hour;
    UINT8 day;
    UINT8 date;
    UINT8 mon;
    UINT16 year;
} S_DATE;


/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halGetDate
 *
 *  DESCRIPTION :
 *      Get the date from MCU 
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      day, month, year 
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
INT32 rtc_halGetDate
(
  OUT UINT8 *day, 
  OUT UINT8 *month, 
  OUT UINT8 *year,
  OUT UINT8 *week
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halGetTime
 *
 *  DESCRIPTION :
 *      Get the Time from MCU 
 *
 *  INPUT :
 *      none
 *
 *  OUTPUT :
 *      second, min, hour
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
INT32 rtc_halGetTime
(
  OUT UINT8 *second, 
  OUT UINT8 *min, 
  OUT UINT8 *hour
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halSetDate
 *
 *  DESCRIPTION :
 *      Set the Date to MCU 
 *
 *  INPUT :
 *      day, month, year
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
INT32 rtc_halSetDate
(
  IN UINT8 day, 
  IN UINT8 month, 
  IN UINT8 year,
  IN UINT8 week
);

/*--------------------------------------------------------------------------
 *
 *  FUNCTION NAME :
 *      rtc_halSetTime
 *
 *  DESCRIPTION :
 *      Set the Time to MCU 
 *
 *  INPUT :
 *      second, min, hour
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
INT32 rtc_halSetTime
(
  IN UINT8 second, 
  IN UINT8 min, 
  IN UINT8 hour
);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_HAL_H_ */
