#ifndef __STM8S_CONF_H
#define __STM8S_CONF_H

/* --- Peripheral Includes --- */
/* Only include the drivers needed for our FSM to save Flash memory */
#include "stm8s_clk.h"
#include "stm8s_gpio.h"
#include "stm8s_tim4.h"

/* --- Assert Macro --- */
/* SPL functions use this macro to check parameter validity. 
 * Defining it as empty ((void)0) disables runtime checks, 
 * which saves a massive amount of flash space. 
 */
#define assert_param(expr) ((void)0)

#endif /* __STM8S_CONF_H */