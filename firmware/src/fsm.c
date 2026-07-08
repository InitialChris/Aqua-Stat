#include "fsm.h"

extern uint32_t millis(void);  // Forward declaration for the millis() function

void FSM_ComputeNextState(FSM_Context_t *state, FSM_Input_t *input) {
    SensorLevel_t level = input->level;
    uint32_t elapsed_time = millis() - state->fsm_state_timestamp;

    // Default assumption: stay in the current state unless a condition is met
    state->next = state->current;

    // --- Global Overrides (In order of priority) ---

    // Manual override for draining
    if (input->switch_pressed) {
        state->next = MANUAL_DRAIN_STATE;
        // Return immediately so normal FSM logic is bypassed while pressed
        return; 
    }

    // Error override
    if (level == LEVEL_ERROR) {
        state->next = ERROR_STATE;
        return;
    }
    
    // Reset override
    if (level == LEVEL_UNKNOWN) {
        state->next = RESET_STATE;
        return;
    }

    
    // --- Normal FSM Operation ---
    switch (state->current) {
        
        case RESET_STATE:
            state->next = IDLE_STATE;  // Transition to IDLE_STATE after reset
        case IDLE_STATE:
            if (level == LEVEL_HIGH) {
                state->next = DRAIN_STATE;          // If the level is above the high sensor, start draining to reach the desired level.
            } else if (level == LEVEL_LOW) {
                state->next = FILL_STATE;           // If the level is below the low sensor, start filling to reach the desired level.
            }

            // Otherwise remain in IDLE_STATE
            break;

        case FILL_STATE:
            if (level == LEVEL_HIGH) {
                state->next = WAIT_HIGH_STATE;      // If this condition is met, the sensors are too close to each other, possible loop.
            } else if (level == LEVEL_MID) {
                state->next = FILL_TIMER_STATE;     // The level is above the low sensor, start the fill timer to reach the desired level.
            }
            // If level is still low, remain in FILL_STATE
            break;

        case DRAIN_STATE:
            if (level == LEVEL_LOW) {
                state->next = WAIT_LOW_STATE;       // If this condition is met, the sensors are too close to each other, possible loop.
            } else if (level == LEVEL_MID) {
                state->next = DRAIN_TIMER_STATE;    // The level is below the high sensor, start the drain timer to reach the desired level.
            }
            // If level is still high, remain in DRAIN_STATE
            break;

        case FILL_TIMER_STATE:
            if (level == LEVEL_HIGH) {
                state->next = WAIT_HIGH_STATE;      // If this condition is met, the sensors are too close, or the fill time was too long, possible loop.
            } else if (level == LEVEL_LOW) {
                state->next = FILL_STATE;           // Return to FILL_STATE if the level drops back to low, possibly due to a wave or sensor fluctuation.
            } else if (elapsed_time >= T_FILL_MS) {
                state->next = IDLE_STATE;           // Filling completed, the level should now be at the desired level.
            }
            // Otherwise remain in FILL_TIMER_STATE until the timer expires
            break;

        case DRAIN_TIMER_STATE:
            if (level == LEVEL_LOW) {
                state->next = WAIT_LOW_STATE;       // If this condition is met, the sensors are too close, or the drain time was too long, possible loop.
            } else if (level == LEVEL_HIGH) {
                state->next = DRAIN_STATE;          // Return to DRAIN_STATE if the level rises back to high, possibly due to a wave or sensor fluctuation.
            } else if (elapsed_time >= T_DRAIN_MS) {
                state->next = IDLE_STATE;           // Draining completed, the level should now be at the desired level.
            }
            // Otherwise remain in DRAIN_TIMER_STATE until the timer expires
            break;

        case WAIT_HIGH_STATE:
        case WAIT_LOW_STATE:
            if (elapsed_time >= T_WAIT_MS) {
                state->next = IDLE_STATE;       // Wait time to prevent looping.
            }
            break;

        case MANUAL_DRAIN_STATE:
            // If we are here, switch_pressed is false (due to the global override check above)
            state->next = RESET_STATE;  // Return to RESET_STATE when the switch is released.
            break;

        case ERROR_STATE:
            // Reset the FSM if the error condition is cleared (level is no longer LEVEL_ERROR)
            state->next = RESET_STATE;  // Return to RESET_STATE when the error condition is cleared.
            break;

        default:
            state->next = RESET_STATE;
            break;
    }
}

void FSM_SaveState(FSM_Context_t *state) {
    if (state->current != state->next) {
        // Save the timestamp of the transition to ensure timers are accurate for the new state
        state->fsm_state_timestamp = millis();
        state->current = state->next;
    }
}

void FSM_GetOutputForState(FSM_Context_t *state, FSM_Output_t *output) {

    static uint32_t last_animation_ms = 0;
    static uint32_t last_blink_ms = 0;
    static bool blink_state = false;
    static uint8_t animationFrame = 0;
    
    // Track the previous state to detect transitions
    static uint8_t previous_state = 0xFF; 

    uint32_t current_ms = millis();

    // --- Graceful Transition Alignment ---
    // If the state has just changed, reset visual phases
    if (state->current != previous_state) {
        previous_state = state->current;
        
        // Reset timers to the current instant
        last_animation_ms = current_ms;
        last_blink_ms = current_ms;
        
        // Force animation to start from the top
        animationFrame = 0; 
        
        // Force blink to start immediately ON for instant visual feedback
        blink_state = true; 
    } else {
        // Update timers normally only if we are stable in the state
        if (current_ms - last_animation_ms >= T_ANIMATION_MS) {
            last_animation_ms = current_ms; // Or += T_ANIMATION_MS as discussed
            animationFrame = animationFrame >= 2 ? 0 : animationFrame + 1;
        }

        if (current_ms - last_blink_ms >= T_ERROR_BLINK_MS) {
            last_blink_ms = current_ms;
            blink_state = !blink_state;
        }
    }

    // Default all outputs to inactive
    output->valve_fill = false;
    output->valve_drain = false;
    output->led_high = false;
    output->led_mid = false;
    output->led_low = false;

    switch (state->current) {
        case RESET_STATE:
            // FSM is waiting for the first valid sensor reading
            output->led_mid = blink_state;
            break;

        case IDLE_STATE:
            // The level is between the low and high sensors
            output->led_mid = true;  
            break;
        
        case FILL_STATE:
            // The level is below the low sensor
            output->valve_fill = true;
            output->led_low = true;
            break;

        case DRAIN_STATE:
            // The level is above the high sensor
            output->valve_drain = true;
            output->led_high = true;
            break;
        
        case FILL_TIMER_STATE:
            // The level is above the low sensor, filling to reach the desired level
            output->valve_fill = true;
            output->led_low = true;
            output->led_mid = blink_state;
            break;
        
        case DRAIN_TIMER_STATE:
            // The level is below the high sensor, draining to reach the desired level
            output->valve_drain = true;
            output->led_high = true;
            output->led_mid = blink_state;
            break;
        
        case WAIT_HIGH_STATE:
            // The level is above the high sensor, waiting to prevent looping
            output->led_high = blink_state;
            break;
        
        case WAIT_LOW_STATE:
            // The level is below the low sensor, waiting to prevent looping
            output->led_low = blink_state;
            break;
        
        case MANUAL_DRAIN_STATE:
            // The switch is pressed, manually draining
            output->valve_drain = true;
            if (animationFrame == 0) output->led_high = true;
            else if (animationFrame == 1) output->led_mid = true;
            else if (animationFrame == 2) output->led_low = true;
            break;

        case ERROR_STATE:
            // The level is in an error state, blinking all LEDs
            output->led_high = blink_state;
            output->led_mid = blink_state;
            output->led_low = blink_state;
            break;

        default:
            break;

    }       
}
