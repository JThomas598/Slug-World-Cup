#include "xc.h"
#include "MotorControl.h"
#include "Shooter.h"
#include "sensors.h"
#include "EventCheckers.h"
#include <BOARD.h>
#include <AD.h>
#include <pwm.h>
#include <stdio.h>
#include <IO_Ports.h>


#define Y_OUTPUT_PINS PIN6 | PIN8 | PIN9 | PIN10
#define IN1 PORTY09_LAT 
#define IN2 PORTY10_LAT 
#define IN3 PORTY08_LAT 
#define IN4 PORTY06_LAT 
#define EN_A PWM_PORTY12
#define EN_B PWM_PORTY04

void sleep(uint32_t seconds);


int main(void) {
    MotorInit();
    InitSensors();
    uint16_t prevBump = 0x0000;
    uint16_t bumpVal;
    while(1){
        printf("bumpVal: %X\n", bumpVal);
        if((bumpVal) != prevBump){
            Forward(SPEED);
            sleep(1);
            StopMovement();
            prevBump = bumpVal;
        }
    }
    return 0;
}

void sleep(uint32_t seconds){
    for(int i = 0; i < seconds * 500000; i++){asm("nop");}
}