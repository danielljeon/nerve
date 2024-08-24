# nerve

STM32F446RE with telemetry ICs.

---

<details markdown="1">
  <summary>Table of Contents</summary>



</details>

---

## 1 Overview

### 1.1 Block Diagram

### 1.2 Pin Configurations

| STM32F446RE | Peripheral            | Config            | Connection                     | Notes                                             |
|-------------|-----------------------|-------------------|--------------------------------|---------------------------------------------------|
| PC7         | SPI2_SCK              |                   | BNO085 Pin 19: H_SCL/SCK/RX    |                                                   |
| PB4         | GPIO_Output (SPI2 CS) | Pull-up, set high | BNO085 Pin 18: H_CSN           | PB4 can be configured for SPI4 NSS (hardware CS). |
| PC2         | SPI2_MISO             |                   | BNO085 Pin 20: H_SDA/H_MISO/TX |                                                   |
| PC1         | SPI2_MOSI             |                   | BNO085 Pin 17: SA0/H_MOSI      |                                                   |
| ?TBD?       | GPIO_Output           | Pull-up, set high | BNO085 Pin 6: PS0/Wake         | Pull low to enable.                               |
| ?TBD?       | GPIO_Output           | Pull-up, set high | BNO085 Pin 5: PS1              | Pull low to enable.                               |
| ?TBD?       | GPIO_EXTI2            |                   | BNO085 Pin 14: H_INTN          |                                                   |
| ?TBD?       | GPIO_Output           | Pull-up, set high | BNO085 Pin 11: NRST            | Pull low to reset.                                |
| PC10        | SPI3_SCK              |                   | RFM95CW Pin 4: SCK             |                                                   |
| PA4         | SPI3_NSS              |                   | RFM95CW Pin 5: NSS             |                                                   |
| PC11        | SPI3_MISO             |                   | RFM95CW Pin 2: MISO            |                                                   |
| PB0         | SPI3_MOSI             |                   | RFM95CW Pin 3: MOSI            |                                                   |
| ?TBD?       | GPIO_Output           |                   | RFM95CW Pin 6: NRESET          | Default reset, pull low to turn on.               |
| PB6         | I2C1_SCL              |                   | BMP390 Pin 2: SCK              |                                                   |
| PB7         | I2C1_SDA              |                   | BMP390 Pin 4: SDI              |                                                   |
| PB10        | I2C2_SCL              |                   | Reserved                       |                                                   |
| PC12        | I2C2_SDA              |                   | Reserved                       |                                                   |
| PA11        | CAN1_RX               |                   | TJA1051T/3 (1 of 2) Pin 4: RXD |                                                   |
| PA12        | CAN1_TX               |                   | TJA1051T/3 (1 of 2) Pin 1: TXD |                                                   |
| PB12        | CAN2_RX               |                   | TJA1051T/3 (2 of 2) Pin 4: RXD |                                                   |
| PB13        | CAN2_TX               |                   | TJA1051T/3 (2 of 2) Pin 1: TXD |                                                   |

---

## 2 BNO085 9-DOF IMU

A 9-axis Inertial Measurement Unit (IMU) combining an accelerometer, gyroscope,
and magnetometer, based on Bosch Sensortec's BNO080 hardware, with sensor fusion
firmware developed by CEVA, Inc. (formerly Hillcrest Laboratories).

> Utilized reference documents:
> 1. `1000-3535 - Sensor Hub Transport Protocol v1.8`
> 2. `1000-3600 - SH-2 SHTP Reference Manual v1.5_1`
> 3. `1000-3625 - SH-2 Reference Manual v1.4`
> 4. `1000-3918 - BNO080 Migration_1`
> 5. `1000-3927 - BNO080 Datasheet v1.6`
> 6. `1000-4044 - BNO080-BNO085 Sensor Calibration Procedure v1.3`
> 7. `1000-4045 - App Note - BNO080-BNO085 Tare Function Usage Guide_1`
> 8. `HillcrestLabs BNO080-085 DataSheet_C`

### 2.1 Background

The BNO085 runs the same hardware as the BNO080, however runs custom Sensor Hub
2 (SH-2) firmware to reduce development overhead on features related to sensor
fusion and optimization. SH-2 is designed around the Sensor Hub Transport
Protocol (SHTP), which runs on SPI, I2C, etc.

### 2.2 Serial Peripheral Interface (SPI)

In an SPI setup, there is always one controller (master) connected to one or
more peripherals (slaves). The controller controls the communication by
generating a clock signal (SCK) and selecting which slave to communicate with
using the Chip Select (CS) line. Data is exchanged between the controller and
peripheral(s) over two data lines: COPI / MOSI and CIPO or MISO.

Basic pinouts:

1. Clock (SCK)
    - The clock signal generated by the master device that synchronizes data
      transfer in SPI communication.

2. Chip Select (CS) or Slave Select (NSS)
    - A signal used to select a specific slave device in SPI communication. When
      the CS line is active (usually low), the selected slave device is enabled
      to communicate with the master.

3. Controller Out Peripheral In (COPI) or Master Out Slave In (MOSI)
    - The data line used to transfer data from the master device to the slave
      device. The master outputs data on this line, which the slave reads.

4. Controller In Peripheral Out (CIPO) or Master In Slave Out (MISO)
    - The data line used to transfer data from the slave device to the master
      device. The slave outputs data on this line, which the master reads.

#### 2.2.1 Full-Duplex vs. Half-Duplex

Full-Duplex: Data can be sent and received simultaneously.

Half-Duplex: Data is either sent or received at any given time, not both
simultaneously.

#### 2.2.2 Clock Polarity, Phase and Modes

CPOL (Clock Polarity): determines the idle state of the clock signal (SCK).

- CPOL = 0: The clock is low (0) when idle.
- CPOL = 1: The clock is high (1) when idle.

CPHA (Clock Phase): determines when data is sampled relative to the clock
signal.

- CPHA = 0: Data is sampled on the leading (first) clock edge.
- CPHA = 1: Data is sampled on the trailing (second) clock edge.

SPI Modes (Combination of CPOL and CPHA):

| Mode | CPOL | CPHA | SCK idle state | Data captured on               | Data output on |
|:----:|:----:|:----:|:--------------:|--------------------------------|----------------|
|  0   |  0   |  0   |    Low (0)     | Rising edge of SCK (1st edge)  | Falling edge   |
|  1   |  0   |  1   |    Low (0)     | Falling edge of SCK (2nd edge) | Rising edge    |
|  2   |  1   |  0   |    High (1)    | Falling edge of SCK (1st edge) | Rising edge    |
|  3   |  1   |  1   |    High (1)    | Rising edge of SCK (2nd edge)  | Falling edge   |

### 2.3 General-Purpose Input/Output (GPIO) Output

### 2.4 Timer

TIM5 is configured to be used for timing operations (1 µs time base) in the SH2
SHTP.

#### 2.4.1 Timer Prescaler Calculation

TODO!!!

~~TIM5 runs based on the APB1 timer clocks which are set to 275 MHz. The
prescaler (PSC) must be calculated accordingly to achieve a 1 µs (1 MHz) time
base.~~

$$PSC = \frac{APB1}{Target} - 1 = \frac{ 275 \space \mathrm{MHz} }{ 1 \space
\mathrm{MHz} } - 1 = 274$$

### 2.5 Direct Memory Access (DMA)

### 2.6 Nested Vectored Interrupt Controller (NVIC)

#### 2.6.1 GPIO External Interrupt/Event Controller (EXTI)

### 2.7 BNO085 Driver

#### 2.7.1 State Machine

```
↓
```

### 2.8 Sensor Fusion Concepts

#### 2.8.1 Euler Angles

#### 2.8.2 Quaternions

---

## 3 BMP390 Barometric Pressure Sensor

---

## 4 RFM95CW (SX1276) LoRa Module

---

## 5 TJA1051T/3 CAN Bus Transceiver

### 5.1 Controller Area Network (CAN)

### 5.1.1 Bit Time Calculation

CAN peripherals run on APB1 (45 MHz), the goal is for a 500 kHz CAN bus.

```
Prescaler                     = 5
Time Quanta in Bit Segment 1  = 15        times
Time Quanta in Bit Segment 2  = 2         times
Time Quantum                  = 111.111   ns
```

> Good resources and calculators online, example here:
> (http://www.bittiming.can-wiki.info/)[http://www.bittiming.can-wiki.info/].

---
