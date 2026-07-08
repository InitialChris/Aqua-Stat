/* Sensors Debouncer 
 * This module provides a debouncing mechanism for two sensors (low and high level sensors) to ensure stable readings. 
 * It handles the logic for determining the current sensor level based on the states of the two sensors.
 * The value returned is based on the following logic:
 * 
 * 
 *            |                |                |                |                  |                |
 *     H[OFF]-|                |         H[OFF]-|                |            H[ON]-|~~~~~~~~~~~~~~~~|
 *            |                |                |                |                  |████████████████|
 *            |                |                |~~~~~~~~~~~~~~~~|                  |████████████████|
 *            |                |                |████████████████|                  |████████████████|  
 *     L[OFF]-|                |          L[ON]-|████████████████|            L[ON]-|████████████████|
 *            |~~~~~~~~~~~~~~~~|                |████████████████|                  |████████████████|
 *            |████████████████|                |████████████████|                  |████████████████|
 *            └────────────────┘                └────────────────┘                  └────────────────┘
 *                LEVEL_LOW                         LEVEL_MID                           LEVEL_HIGH
 * 
 * Ensure that the debounce time is set appropriately to filter out waves and noise from the sensor readings 
 * especially when the water level is near the sensors.
 * */


#ifndef SENSORS_DEBOUNCER_H
#define SENSORS_DEBOUNCER_H

#include <stdint.h>
#include <stdbool.h>

/// Sensor level definitions
typedef enum {
    LEVEL_LOW     = 0x00,   /*!< No sensor is active */
    LEVEL_MID     = 0x01,   /*!< Only low sensor is active */
    LEVEL_HIGH    = 0x03,   /*!< Both low and high sensors are active */
    LEVEL_ERROR   = 0x02,   /*!< High sensor is active while low sensor is not */
    LEVEL_UNKNOWN = 0xFF    /*!< Unknown sensor level, set when the debouncer is initialized */
} SensorLevel_t;

typedef struct {
    bool low_sensor_state;
    bool high_sensor_state;
    bool inverted;
    SensorLevel_t sensor_state;
    SensorLevel_t last_sensor_state;
    uint32_t timestamp_ms;
    uint32_t debounce_ms;
} SensorsDebouncer;

/**
 * @brief Initialize the SensorsDebouncer structure.
 * @param debouncer Pointer to the SensorsDebouncer structure to initialize.
 * @param debounce_ms Debounce time in milliseconds.
 * @param inverted If true, the sensor logic is inverted (active LOW).
 */
void sensorsDebouncer_init(SensorsDebouncer *debouncer, uint32_t debounce_ms, bool inverted);

/**
 * @brief Update the debouncer with the current sensor states.
 * @param debouncer Pointer to the SensorsDebouncer structure.
 * @param low_sensor_state Current state of the low level sensor (true for HIGH, false for LOW).
 * @param high_sensor_state Current state of the high level sensor (true for HIGH, false for LOW).
 */
void sensorsDebouncer_update(SensorsDebouncer *debouncer, bool low_sensor_state, bool high_sensor_state);

/**
 * @brief Get the current debounced sensor level.
 * @param debouncer Pointer to the SensorsDebouncer structure.
 * @return The current debounced sensor level as a SensorLevel_t value.
 */
SensorLevel_t sensorsDebouncer_get(SensorsDebouncer *debouncer);

#endif // SENSORS_DEBOUNCER_H
