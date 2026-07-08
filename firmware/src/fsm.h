/**
 * Water Level Control Finite State Machine (FSM)
 *
 * Manages water level using two discrete physical sensors (Low/High) and 
 * actuates two solenoid valves (Fill/Drain). Includes a manual override 
 * switch for unconditional maintenance draining,
 *
 * To eliminate the need for a third (middle) physical sensor, the FSM 
 * relies on dead-reckoning timers (T_FILL_MS, T_DRAIN_MS). It runs 
 * the valves for a fixed duration after crossing a known sensor 
 * threshold to reach the optimal intermediate setpoint. These 
 * constants require empirical calibration based on system flow rates.
 *
 * If sensors are placed too close together or timer values are 
 * excessive, the system could enter an infinite fill/drain loop. 
 * To mitigate this, a cooldown period (T_WAIT_MS) is 
 * enforced in case of invalid sensor conditions, preventing rapid cycling of the valves.
 *
 * System Status LED Indicators
 * Legend:  ( ) = OFF    (O) = ON    (*) = BLINKING   (#) = SEQUENTIAL ANIMATION
 *
 * LOW  MID  HIGH  | FSM State          | Description
 * ----------------|--------------------|--------------------------------------------------------------
 * ( )  ( )  (O)   | DRAIN_STATE        | Level is above the high sensor. Drain valve is active.
 * ( )  (*)  (O)   | DRAIN_TIMER_STATE  | Level dropped below the high sensor. Timed drain to reach setpoint.
 * ( )  (O)  ( )   | IDLE_STATE         | Target level reached. Water is stable between the two sensors.
 * (O)  (*)  ( )   | FILL_TIMER_STATE   | Level rose above the low sensor. Timed fill to reach setpoint.
 * (O)  ( )  ( )   | FILL_STATE         | Level is below the low sensor. Fill valve is active.
 * (*)  (*)  (*)   | ERROR_STATE        | Hardware fault: High sensor is active while low sensor is not.
 * ( )  (*)  ( )   | RESET_STATE        | Booting / Waiting for the first valid sensor reading.
 * (*)  ( )  ( )   | WAIT_LOW_STATE     | Dropped below low sensor during drain. Cooldown to prevent looping.
 * ( )  ( )  (*)   | WAIT_HIGH_STATE    | Rose above high sensor during fill. Cooldown to prevent looping.
 * (#)  (#)  (#)   | MANUAL_DRAIN_STATE | Manual override switch is closed. Drain valve is active.
 */

#ifndef FSM_H
#define FSM_H

#include <stdint.h>
#include <stdbool.h>
#include "sensors_debouncer.h"    
#include "config.h"
/**
 * @brief Enumeration of all possible FSM states for the water level controller.
 */
typedef enum {
    RESET_STATE,        /*!< Booting or waiting for first valid sensor data */
    IDLE_STATE,         /*!< Water level is optimal and stable between sensors */
    FILL_STATE,         /*!< Actively filling (level is below low sensor) */
    DRAIN_STATE,        /*!< Actively draining (level is above high sensor) */
    FILL_TIMER_STATE,   /*!< Timed filling to reach the virtual middle setpoint */
    DRAIN_TIMER_STATE,  /*!< Timed draining to reach the virtual middle setpoint */
    WAIT_HIGH_STATE,    /*!< Overshoot cooldown: triggered if water reaches the high sensor during a timed fill */
    WAIT_LOW_STATE,     /*!< Undershoot cooldown: triggered if water drops below the low sensor during a timed drain */
    MANUAL_DRAIN_STATE, /*!< Manual override engaged for unconditional draining */
    ERROR_STATE         /*!< Hardware fault detected (e.g., physical sensor paradox) */
} FSM_State_t;

/**
 * @brief Hardware inputs aggregated for FSM evaluation.
 */
typedef struct {
    SensorLevel_t level;    /*!< Debounced logical water level from sensors */
    bool switch_pressed;    /*!< Status of the manual override drain switch */
} FSM_Input_t;

/**
 * @brief Hardware outputs driven by the FSM state (Moore machine pattern).
 */
typedef struct {
    bool valve_fill;        /*!< Fill valve actuator command (true = ON) */
    bool valve_drain;       /*!< Drain valve actuator command (true = ON) */
    bool led_high;          /*!< High level indicator LED state */
    bool led_mid;           /*!< Middle level/Status indicator LED state */
    bool led_low;           /*!< Low level indicator LED state */
} FSM_Output_t;

/**
 * @brief Core context maintaining the FSM execution state and timing.
 */
typedef struct {
    FSM_State_t current;                /*!< The currently active state */
    FSM_State_t next;                   /*!< The computed state to transition to on the next update */
    uint32_t fsm_state_timestamp;  /*!< Millisecond timestamp of the last state transition */
} FSM_Context_t;

/**
 * @brief Determines the next FSM state based on current inputs and elapsed time.
 *
 * @param state Pointer to the FSM context holding current state and timing data.
 * @param input Pointer to the hardware inputs (sensor levels and physical switches).
 */
void FSM_ComputeNextState(FSM_Context_t *state, FSM_Input_t *input);

/**
 * @brief Commits the computed next state and updates transition timers.
 * * If a state transition occurred, it overwrites the current state and records 
 * the current timestamp. This ensures the timer for the new state starts exactly 
 * at the moment of the transition.
 *
 * @param state Pointer to the FSM context to be updated.
 */
void FSM_SaveState(FSM_Context_t *state);

/**
 * @brief Maps the current FSM state to physical hardware outputs.
 * * Implements a strict Moore machine pattern where outputs depend solely on the 
 * current state. Moreover, it handles visual feedback through LED indicators, including blinking and animation sequences
 * to reflect the system's operational status.
 *
 * @param state  Pointer to the FSM context containing the active state.
 * @param output Pointer to the output structure to be populated with actuator states.
 */
void FSM_GetOutputForState(FSM_Context_t *state, FSM_Output_t *output);


#endif // FSM_H