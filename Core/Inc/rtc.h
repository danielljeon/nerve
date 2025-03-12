/*******************************************************************************
 * @file rtc.h
 * @brief General driver abstracting STM32 HAL: RTC.
 *******************************************************************************
 */

#ifndef NERVE__RTC_H
#define NERVE__RTC_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** STM32 port and pin configs. ***********************************************/

extern RTC_HandleTypeDef hrtc;

/** Public functions. *********************************************************/

/**
 * @brief Set RTC date.
 *
 * @param year Year index.
 * @param month Month index.
 * @param date Day of the month number.
 * @param day Weekday number (Monday = 1, Tuesday = 2, ..., Sunday = 7).
 */
void set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t day);

/**
 * @brief Get RTC date.
 *
 * @param time Character based time value.
 * @param date Character based date value.
 */
void get_time_date(char *time, char *date);

#endif
