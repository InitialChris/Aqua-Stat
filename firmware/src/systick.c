#include "systick.h"

// Definition of the global variable allocated in RAM
volatile uint32_t system_ticks = 0;

void SysTick_Init(){
    // TIM4 configuration: 1ms tick (16MHz / 128 / 125 = 1kHz)
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    TIM4_Cmd(ENABLE);
}

uint32_t millis(void) {
    uint32_t ms;
    disableInterrupts();
    ms = system_ticks;
    enableInterrupts();
    return ms;
}

void TIM4_UPD_OVF_IRQHandler(void) __interrupt(23) {
    system_ticks++;
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}