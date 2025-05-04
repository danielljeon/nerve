/*******************************************************************************
 * @file configuration.h <- TODO: DEV CONFIGURATIONS!
 * @brief High level hardcoded configuration related declarations/definitions.
 *******************************************************************************
 */

#ifndef NERVE__CONFIGURATION_H
#define NERVE__CONFIGURATION_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal_rtc.h"

/** Configuration definitions. ************************************************/

// RTC.
//#define NERVE_RTC_SET_FLAG                   // Set RTC to new datetime.
//#define NERVE_RTC_YEAR 2                     // Year: 2002.
//#define NERVE_RTC_MONTH RTC_MONTH_APRIL      // Month: April.
//#define NERVE_RTC_DATE 1                     // Date: 1st.
//#define NERVE_RTC_WEEKDAY RTC_WEEKDAY_MONDAY // Weekday: Monday.
//#define NERVE_RTC_HOUR 14                    // Hour: 14 (2 PM).
//#define NERVE_RTC_MINUTE 59                  // Minute: 59.
//#define NERVE_RTC_SECOND 59                  // Second: 59.

// XBee communications destination.
#define XBEE_DESTINATION_64 0x0123456789ABCDEF
#define XBEE_DESTINATION_16 0xFFFE

// Full telemetry flood on CAN bus intended for debug/development purposes.
//#define NERVE_DEBUG_FULL_CAN_TELEMETRY

// Full reset of GPS prior to initialization, triggers cold start.
// The 3.3 V backup cell powers the RTC and u-blox ephemeris RAM normally.
//#define NERVE_GPS_COLD_START

#endif
