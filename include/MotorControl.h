/* 
 * File:   MotorControl.h
 * Author: jaalthom 
 *
 * Created on May 24, 2023, 7:53 PM
 */

#ifndef MOTORCONTROL_H
#define	MOTORCONTROL_H

//Navigation Function Declarations
int MotorInit();
int RightMtrSpd(int);
int LeftMtrSpd(int);
int Forward(int);
int Reverse(int);
int LeftTankTurn(int);
int RightTankTurn(int);
int VeerRight(int,double);
int VeerLeft(int,double);
int StopMovement();

#define SPEED 80 //Value from 0 - 100

#endif	/* MOTORCONTROL_H */

