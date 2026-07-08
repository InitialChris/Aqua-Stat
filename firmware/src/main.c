#include "stm8s.h"
#include <stdint.h>
#include <stdbool.h>
#include "pin_map.h"
#include "config.h"
#include "systick.h"
#include "fsm.h"
#include "sensors_debouncer.h"

// Helper macro to set GPIO pin state based on a boolean value
#define GPIO_Set(port_pin, value)  ((value) == RESET ? GPIO_WriteLow(port_pin) : GPIO_WriteHigh(port_pin))

SensorsDebouncer sensors_debouncer;
FSM_Context_t fsm_context = {RESET_STATE, RESET_STATE, 0};
FSM_Output_t fsm_output = {false, false, false, false, false};

extern uint32_t millis(void);  // Forward declaration for the millis() function

void setupMCU();
bool readSwitchState();
void applyOutputToHardware(FSM_Output_t *output);


void main(void){
    setupMCU();
    sensorsDebouncer_init(&sensors_debouncer, T_SENSOR_DEBOUNCE_MS, ACTIVE_LOW_SENSORS);
    
    while(1){
        // Input handling
        sensorsDebouncer_update(&sensors_debouncer, (bool)GPIO_ReadInputPin(GPIO_SENSOR_LOW), (bool)GPIO_ReadInputPin(GPIO_SENSOR_HIGH));
        SensorLevel_t sensor_level = sensorsDebouncer_get(&sensors_debouncer);
        bool switch_state = readSwitchState();

        // FSM processing
        FSM_Input_t fsm_input = {sensor_level, switch_state};
        FSM_ComputeNextState(&fsm_context, &fsm_input);
        FSM_SaveState(&fsm_context);
        FSM_GetOutputForState(&fsm_context, &fsm_output);

        // Output handling
        applyOutputToHardware(&fsm_output);
    }
}

// Setup the microcontroller's clock, GPIO pins, and timer
void setupMCU(){
    // 16 MHz internal (HSI) clock
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

     // Input configuration: no interrupt, enable pull-up for switch if active low
    GPIO_Init(GPIO_SENSOR_LOW,  GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIO_SENSOR_HIGH, GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIO_SWITCH,      ACTIVE_LOW_SWITCH ? GPIO_MODE_IN_PU_NO_IT : GPIO_MODE_IN_FL_NO_IT);

    // Output configuration: push-pull, low speed, initialize to inactive state
    GPIO_Init(GPIO_VALVE_DRAIN, ACTIVE_LOW_VALVES ? GPIO_MODE_OUT_PP_HIGH_SLOW : GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_VALVE_FILL,  ACTIVE_LOW_VALVES ? GPIO_MODE_OUT_PP_HIGH_SLOW : GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_HIGH, ACTIVE_LOW_LEDS ? GPIO_MODE_OUT_PP_HIGH_SLOW : GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_MID,  ACTIVE_LOW_LEDS ? GPIO_MODE_OUT_PP_HIGH_SLOW : GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIO_LED_LOW,  ACTIVE_LOW_LEDS ? GPIO_MODE_OUT_PP_HIGH_SLOW : GPIO_MODE_OUT_PP_LOW_SLOW);

    // TIM4 configuration: 1ms tick (16MHz / 128 / 125 = 1kHz)
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    TIM4_Cmd(ENABLE);

    enableInterrupts();
}

// Read the state of the manual override switch applying debounce logic
bool readSwitchState(){
    static bool last_switch_state = false;
    static bool current_switch_state = false;
    static uint32_t switch_ms = 0;
    bool switch_state = ((bool)GPIO_ReadInputPin(GPIO_SWITCH)) ^ ACTIVE_LOW_SWITCH;

    if (switch_state != last_switch_state){
        switch_ms = millis();
    } else if (millis() - switch_ms > T_SWITCH_DEBOUNCE_MS){
        current_switch_state = switch_state;
    }

    last_switch_state = switch_state;
    return current_switch_state;
}

// Apply the computed FSM output to the hardware GPIO pins based on the active low/high configuration for valves and LEDs.
void applyOutputToHardware(FSM_Output_t *output){
    GPIO_Set(GPIO_VALVE_FILL,  output->valve_fill  ^ ACTIVE_LOW_VALVES);
    GPIO_Set(GPIO_VALVE_DRAIN, output->valve_drain ^ ACTIVE_LOW_VALVES);
    GPIO_Set(GPIO_LED_HIGH,    output->led_high    ^ ACTIVE_LOW_LEDS);
    GPIO_Set(GPIO_LED_MID,     output->led_mid     ^ ACTIVE_LOW_LEDS);
    GPIO_Set(GPIO_LED_LOW,     output->led_low     ^ ACTIVE_LOW_LEDS);

    // Prevent compiler optimization, for a strange reason the optimizer removes the last GPIO_Set() call.
    __asm__("nop");
}