#ifndef __LIB_CRC8_H__
#define __LIB_CRC8_H__
#include <Arduino.h>
volatile uint8_t calcCrc8(volatile uint8_t *DataArray, const uint16_t Length);

#endif //__LIB_CRC8_H__