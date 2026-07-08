#include "systick.h"

// Definition of the global variable allocated in RAM
volatile uint32_t system_ticks = 0;

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