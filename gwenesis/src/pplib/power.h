#ifndef POWER_H
#define POWER_H

#include <Arduino.h>

// Physical parameters
#define BAT_MIN_VOLT 3600  // voltage (mV) when empty
#define BAT_MAX_VOLT 3900  // voltage (mV) when full
#define EXT_MIN_VOLT 4600  // min. voltage for ext. supply

// Communication
#define PWR_ON_UNKOWN 0
#define PWR_ON_BAT 1
#define PWR_ON_GRD 2

#define PWR_BAT_LVL_UNKOWN 255

bool initPower();
uint8_t getPowerSource();
uint8_t getBatLevel();
uint16_t getSplyVoltage();

#endif //POWER_H
