/*******************************************************************************
 * @file sh2_hal_spi.h
 * @brief BNO085 SH2 HAL via SHTP SPI.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

#ifndef __SH2_HAL_SPI_H
#define __SH2_HAL_SPI_H

/* Includes. */

#include "sh2_err.h"
#include "sh2_hal.h"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_tim.h"

/* Definitions. */

extern SPI_HandleTypeDef hspi4;
extern TIM_HandleTypeDef htim5;

// STM32 port and pin configs.
#define SPI_HANDLER hspi4
#define TIM_HANDLER htim5
#define RSTN_PORT GPIOB
#define RSTN_PIN GPIO_PIN_4
#define PS0_WAKEN_PORT GPIOF
#define PS0_WAKEN_PIN GPIO_PIN_0
#define PS1_PORT GPIOF
#define PS1_PIN GPIO_PIN_1
#define INTN_PORT GPIOF
#define INTN_PIN GPIO_PIN_2
#define CSN_PORT GPIOE
#define CSN_PIN GPIO_PIN_4

// Keep reset asserted this long.
// (Some targets have a long RC decay on reset.)
#define RESET_DELAY_US (10000)

// Wait up to this long to see first interrupt from SH.
#define START_DELAY_US (2000000)

// How many bytes to read when reading the length field.
#define READ_LEN (4)

// Macros.
#define ARRAY_LEN(a) ((sizeof(a)) / (sizeof(a[0])))

/* Functions. */

static int sh2_spi_hal_open(sh2_Hal_t *self);
static void sh2_spi_hal_close(sh2_Hal_t *self);
static int sh2_spi_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len,
                            uint32_t *t);
static int sh2_spi_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len);
static uint32_t sh2_spi_hal_get_time_us(sh2_Hal_t *self);
sh2_Hal_t *sh2_hal_init(void);

#endif