// millis() function implementation for STM8S microcontrollers using TIM4 for 1ms ticks.
#ifndef SYSTICK_H
#define SYSTICK_H

#include "stm8s.h"

extern volatile uint32_t system_ticks;

/**
 * @brief Get the system uptime in milliseconds.
 * Disables interrupts locally to ensure atomic read of the 32-bit value on an 8-bit MCU.
 * @return The number of milliseconds since the system started.
 */
 uint32_t millis(void);


/**
 * @brief Interrupt Service Routine for TIM4 OVF.
 * Increments the system tick counter every 1ms.
 */
void TIM4_UPD_OVF_IRQHandler(void) __interrupt(23);

#endif // SYSTICK_H