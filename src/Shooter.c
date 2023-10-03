#include "Shooter.h"
#include "xc.h"
#include <pwm.h>
#include <IO_Ports.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

//Definitions--------------------------------------------------
#define SHOOTER_PORT PORTW
#define SHOOTER_ENABLE PIN8
#define ACTUATOR_ENABLE PIN6

//Functions
int ShooterInit(){
    if(IO_PortsSetPortOutputs(SHOOTER_PORT, (SHOOTER_ENABLE | ACTUATOR_ENABLE)) == ERROR){
        printf("FAILURE\r\n");
        return 0;
    }
    IO_PortsClearPortBits(SHOOTER_PORT, (SHOOTER_ENABLE | ACTUATOR_ENABLE));
    printf("SUCCESS\r\n");
    return 1;
}

int ToggleShooterMotor(){
    IO_PortsTogglePortBits(SHOOTER_PORT, SHOOTER_ENABLE);
}

int ToggleActuator(){
    IO_PortsTogglePortBits(SHOOTER_PORT, ACTUATOR_ENABLE);
}