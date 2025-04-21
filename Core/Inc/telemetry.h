/*******************************************************************************
 * @file telemetry.h
 * @brief General telemetry functions including CAN and XBee radio.
 *******************************************************************************
 */

#ifndef NERVE__TELEMETRY_H
#define NERVE__TELEMETRY_H

/** Public functions. *********************************************************/

void can_tx_state(void);
void can_tx_barometric(void);
void can_tx_gps1(void);
void can_tx_gps2(void);
void can_tx_gps3(void);

#endif
