/*******************************************************************************
 * @file configuration.h <- TODO: DEV CONFIGURATIONS!
 * @brief High level hardcoded configuration related declarations/definitions.
 *******************************************************************************
 */

#ifndef NERVE__CONFIGURATION_H
#define NERVE__CONFIGURATION_H

/** Configuration definitions. ************************************************/

// XBee communications destination.
#define XBEE_DESTINATION_64 0x0123456789ABCDEF
#define XBEE_DESTINATION_16 0xFFFE

// Full telemetry flood on CAN bus intended for debug/development purposes.
//#define NERVE_DEBUG_FULL_CAN_TELEMETRY

// Full reset of GPS prior to initialization, triggers cold start.
// The 3.3 V backup cell powers the RTC and u-blox ephemeris RAM normally.
//#define NERVE_GPS_COLD_START

#endif
