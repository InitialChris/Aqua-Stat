#include "sensors_debouncer.h"

extern uint32_t millis(void);  // Forward declaration for the millis() function

void sensorsDebouncer_init(SensorsDebouncer *debouncer, uint32_t debounce_ms, bool inverted) {
    debouncer->low_sensor_state     = inverted;
    debouncer->high_sensor_state    = inverted;
    debouncer->inverted             = inverted;
    debouncer->sensor_state         = LEVEL_UNKNOWN;
    debouncer->last_sensor_state    = LEVEL_UNKNOWN;
    debouncer->debounce_ms          = debounce_ms;
    debouncer->timestamp_ms         = 0;
}

void sensorsDebouncer_update(SensorsDebouncer *debouncer, bool low_state, bool high_state) {
    // Apply the correct logic if sensors are active LOW
    debouncer->low_sensor_state  = low_state  ^ debouncer->inverted;
    debouncer->high_sensor_state = high_state ^ debouncer->inverted;

    // If both sensors are 0   -> LOW
    // If only low sensor is 1 -> MID
    // If both sensors are 1   -> HIGH
    // Other combinations are considered as ERR (0x02)
    SensorLevel_t current_state = (SensorLevel_t)((debouncer->high_sensor_state << 1) | debouncer->low_sensor_state);

    // Debouncing
    if (current_state != debouncer->last_sensor_state) {
        debouncer->timestamp_ms = millis();
    } else if (millis() - debouncer->timestamp_ms > debouncer->debounce_ms) {
        debouncer->sensor_state = current_state;
    }

    debouncer->last_sensor_state = current_state;
}

SensorLevel_t sensorsDebouncer_get(SensorsDebouncer *debouncer) {
    return debouncer->sensor_state;
}
