#include "power.h"
#include "setup.h"

//#include <stdint.h>
//#include <stdio.h>
//#include <avr/pgmspace.h>

uint8_t lst_bat_lvl = 100;

// Return supply voltage in mV
uint16_t getSplyVoltage() {
  return analogRead(BAT_PIN_ADC) * 10;  // voltage divider returns voltage in cV
}

// Return type of currently used power source:
// either on battery or on grid
uint8_t getPowerSource() {
  if (getSplyVoltage() > EXT_MIN_VOLT) {
    lst_bat_lvl = 100;
    return PWR_ON_GRD;
  } else {
    return PWR_ON_BAT;
  }
}

// Return battery power level in percent
uint8_t getBatLevel() {
  if (getPowerSource() == PWR_ON_GRD)
    return PWR_BAT_LVL_UNKOWN;

  uint8_t level = (float) 100.0f / ((float) (BAT_MAX_VOLT) - (float) (BAT_MIN_VOLT) ) * (float) (getSplyVoltage() - BAT_MIN_VOLT);

  // return level in steps of 10, because a voltage divider
  // will otherwise deliver very inaccurate values
  level /= 10;
  level *= 10;
    
  if (level > 100)
    level = 100;

  if (level < 0)
    level = 0;

  if (level < lst_bat_lvl)
    lst_bat_lvl = level;

  return lst_bat_lvl < level ? lst_bat_lvl : level;
}

bool initPower()
{
  pinMode(BAT_PIN_ADC, INPUT);
  //pinMode(BAT_PIN_SRC, INPUT);
  
  return true;
}
