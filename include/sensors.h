#ifndef SENSORS_H
#define	SENSORS_H
#include <BOARD.h>

uint8_t InitSensors(void);
uint16_t ReadBumpers(void);
uint16_t ReadTapeSensors(void);
uint16_t ReadBeacons(void);


#endif