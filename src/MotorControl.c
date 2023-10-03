
#include "MotorControl.h"
#include "xc.h"
#include <pwm.h>
#include <IO_Ports.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

//Definitions--------------------------------------------------
#define MOTOR_PORT PORTY
#define Y_OUTPUT_PINS (PIN6 | PIN8 | PIN9 | PIN10)
#define IN1 PORTY09_LAT 
#define IN2 PORTY10_LAT 
#define IN3 PORTY08_LAT 
#define IN4 PORTY06_LAT 
#define EN_A PWM_PORTY12
#define EN_B PWM_PORTY04

//Functions

int MotorInit(){
    //PWM INIT
    printf("Running PWM_Init()...: ");
    if(PWM_Init() == ERROR){
        printf("FAILURE\r\n");
        return 0;
    }
    printf("SUCCESS\r\n");
    //PWM PINS
    printf("Adding PWM pins...: ");
    if(PWM_AddPins(EN_A |  EN_B) == ERROR){
        printf("FAILURE\r\n");
        return 0; 
    }
    printf("SUCCESS\r\n");
    //PWM FREQUENCY
    printf("Setting PWM frequency...: ");
    if(PWM_SetFrequency(MIN_PWM_FREQ) == ERROR){
        printf("FAILURE\r\n");
        return 0;
    }
    printf("SUCCESS\r\n");
    //PWM Duty Cycle
    printf("Setting PWM Duty Cycle to 0%%...: ");
    if(PWM_SetDutyCycle(EN_A, 0) == ERROR || 
       PWM_SetDutyCycle( EN_B, 0) == ERROR){
        printf("FAILURE\r\n");
        return 0; 
    }
    printf("SUCCESS\r\n");
    //IO Ports
    printf("Setting IO Ports to proper direction...: ");
    if(IO_PortsSetPortOutputs(MOTOR_PORT, Y_OUTPUT_PINS) == ERROR){
        printf("FAILURE\r\n");
        return 0;
    }
    IO_PortsClearPortBits(MOTOR_PORT, Y_OUTPUT_PINS);
    printf("SUCCESS\r\n");
    return 1;
}

int RightMtrSpd(int speed){
    if(speed < 0){
        IN1 = 0;
        IN2 = 1;
        speed = abs(speed);
    }
    else{
        IN1 = 1;
        IN2 = 0;
    }
    if(speed > 100){
        speed = 100;
    }
    PWM_SetDutyCycle(EN_A, MAX_PWM * ((double)abs(speed) / 100.0));
}

int LeftMtrSpd(int speed){
    if(speed < 0){
        IN3 = 0;
        IN4 = 1;
        speed = abs(speed);
    }
    else{
        IN3 = 1;
        IN4 = 0;
    }
    if(speed > 100){
        speed = 100;
    }
    PWM_SetDutyCycle(EN_B, MAX_PWM * ((double)speed / 100.0));
}

int Forward(int speed){
    RightMtrSpd(speed);
    LeftMtrSpd(speed);
}

int Reverse(int speed){
    RightMtrSpd(-speed);
    LeftMtrSpd(-speed);
}

int LeftTankTurn(int speed){
    RightMtrSpd(speed);
    LeftMtrSpd(-speed);
}

int RightTankTurn(int speed){
    RightMtrSpd(-speed);
    LeftMtrSpd(speed);
}

int VeerRight(int speed, double ratio){
    LeftMtrSpd(speed);
    RightMtrSpd(ratio * speed);
}

int VeerLeft(int speed, double ratio){
    LeftMtrSpd(ratio * speed);
    RightMtrSpd(speed);
}

int StopMovement(){
    RightMtrSpd(0);
    LeftMtrSpd(0);
}