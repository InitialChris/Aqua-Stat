/* ----------------------------------------------------------------------------
 * Pin mapping for the STM8S003F3P6 MCU
 * ---------------------------------------------------------------------------
 *                                  
 *                                    STM8S003F3P6
 *                         ┌───────────────────────────────┐
 *   GPIO_LED_HIGH (OUT) ──|  1 [PD4]    [USB]    [PD3] 1  |── GPIO_VALVE_DRAIN (OUT)
 *    GPIO_LED_MID (OUT) ──|  2 [PD5]             [PD2] 2  |── GPIO_VALVE_FILL  (OUT)
 *    GPIO_LED_LOW (OUT) ──|  3 [PD6]             [PD1] 3  |── PD1
 *                  NRST ──|  4 [RST]             [PC7] 4  |── PC7
 *  GPIO_SENSOR_LOW (IN) ──|  5 [PA1]             [PC6] 5  |── PC6
 * GPIO_SENSOR_HIGH (IN) ──|  6 [PA2]             [PC5] 6  |── PC5
 *                   GND ──|  7 [GND]             [PC4] 7  |── PC4
 *                   5V  ┬─|  8 [5V]              [PC3] 8  |── PC3
 *                       └─|  9 [3V3]             [PB4] 9  |── PB4
 *    GPIO_SWITCH   (IN) ──| 10 [PA3]             [PB5] 10 |── PB5
 *                         └───────────────────────────────┘
 *                                 O    O    O    O
 *                               NRST  GND SWIM  3V3
 * - 3.3V Regulator is bypassed
*/
#pragma once

// Inputs
#define GPIO_SENSOR_LOW     GPIOA, GPIO_PIN_1
#define GPIO_SENSOR_HIGH    GPIOA, GPIO_PIN_2
#define GPIO_SWITCH         GPIOA, GPIO_PIN_3

// Outputs 
#define GPIO_VALVE_FILL     GPIOD, GPIO_PIN_2
#define GPIO_VALVE_DRAIN    GPIOD, GPIO_PIN_3
#define GPIO_LED_HIGH       GPIOD, GPIO_PIN_4
#define GPIO_LED_MID        GPIOD, GPIO_PIN_5
#define GPIO_LED_LOW        GPIOD, GPIO_PIN_6