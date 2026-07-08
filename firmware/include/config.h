/*----------------------------------------------------------------------------
 * CONFIGURATION
 *----------------------------------------------------------------------------*/
#pragma once

/* User Configuration */

// Time required to fill the pool (from low sensor trigger to desired level)
#define T_FILL_MS               (22UL * 60UL * 1000UL)  /* 22 minutes */

// Time required to drain the pool (from high sensor trigger to desired level)
#define T_DRAIN_MS              (15UL * 60UL * 1000UL)  /* 15 minutes */

// Safety delay between cycles to prevent frequent sensor triggering
#define T_WAIT_MS               (60UL * 1000UL)         /* 60 seconds */

// Debounce time for water level sensors stabilization
#define T_SENSOR_DEBOUNCE_MS    (6UL * 1000UL)          /* 6 seconds */


/* System Configuration */

// Debounce time for the empty pool switch
#define T_SWITCH_DEBOUNCE_MS    (1000UL)    /* 1 second */

// Blink interval for the error signaling LED
#define T_ERROR_BLINK_MS        (500UL)     /* 500 milliseconds */
#define T_ANIMATION_MS          (250UL)     /* 250 milliseconds */

// Active low configuration for switches, sensors, valves, and LEDs
// If true, the corresponding hardware is considered active when the GPIO pin is low (0V).
#define ACTIVE_LOW_SENSORS   true
#define ACTIVE_LOW_SWITCH    true
#define ACTIVE_LOW_VALVES    false
#define ACTIVE_LOW_LEDS      true