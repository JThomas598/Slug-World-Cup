#include <sensors.h>
#include <IO_Ports.h>
#define SENSOR_PORT PORTZ
#define BEACON_PORT PORTX
#define BUMPER_PINS (PIN3 | PIN4 | PIN5)
#define TAPE_SENSOR_PINS (PIN6 | PIN7 | PIN8 | PIN9)
#define SAMMY_DETECTOR_PIN PIN11
#define GOAL_DETECTOR_PIN PIN12
#define BEACON_PINS (PIN3 | PIN5)
#define SENSOR_PINS (BUMPER_PINS | TAPE_SENSOR_PINS | BEACON_PINS)

uint8_t InitSensors(void){
    if(IO_PortsSetPortInputs(SENSOR_PORT, SENSOR_PINS) == ERROR){
        return 0;
    }
    if(IO_PortsSetPortInputs(BEACON_PORT, BEACON_PINS) == ERROR){
        return 0;
    }
    return 1;
}

uint16_t ReadBumpers(void){
    uint16_t port = IO_PortsReadPort(SENSOR_PORT);
    return (port & BUMPER_PINS) >> 3;
}

uint16_t ReadTapeSensors(void){
    uint16_t port = IO_PortsReadPort(SENSOR_PORT);
    return (port & TAPE_SENSOR_PINS) >> 6;
}

uint16_t ReadBeacons(void){
    uint16_t port = IO_PortsReadPort(BEACON_PORT);
    return (port & BEACON_PINS) >> 3;
}

