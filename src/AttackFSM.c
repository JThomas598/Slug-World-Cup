#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MotorControl.h"
#include "Shooter.h"
#include "AttackFSM.h"
#include "MinspecHSM.h"
#include <IO_Ports.h>
#include <BOARD.h>
#include <stdbool.h>
//Uncomment these for the Roaches
//#include "roach.h"
//#include "RoachFrameworkEvents.h"
#include <stdio.h>


/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine.*/


/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/

/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

typedef enum {
    INIT,
    INITIAL_ORIENT,
    LEAVE_HOME,
    FACE_WALL,
    HIT_WALL,
    REAL_ORIENT,
    BACKSTEP,
    CLEAR_EMF,
    FIND_TAPE,
    LEAVE_TAPE,
    TURN_FROM_OBST,
    RECOIL_FROM_OBST,
    CROSS_FIELD,
    TWO_POINT_TRAVEL,
    AIM,
    FIRE,
} TemplateFSMState_t;

static const char *StateNames[] = {
	"INIT",
	"INITIAL_ORIENT",
	"LEAVE_HOME",
	"FACE_WALL",
	"HIT_WALL",
	"REAL_ORIENT",
	"BACKSTEP",
	"CLEAR_EMF",
	"FIND_TAPE",
	"LEAVE_TAPE",
	"TURN_FROM_OBST",
	"RECOIL_FROM_OBST",
	"CROSS_FIELD",
	"TWO_POINT_TRAVEL",
	"AIM",
	"FIRE",
};


static TemplateFSMState_t CurrentState = INIT; // <- change enum name to match ENUM
static uint8_t MyPriority;
extern uint16_t SIDE;
extern bool secAttack;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint16_t CheckSide(){
    return (IO_PortsReadPort(PORTW) & PIN7);
}
/**
 * @Function InitTemplateFSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitAttackFSM(void) {
    ES_Event returnEvent;
    CurrentState = INIT;
    if(CheckSide() == RIGHT){
        SIDE = RIGHT;
    }
    else{
        SIDE = LEFT;
    }
    returnEvent = RunAttackFSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function PostTemplateFSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostAttackFSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateFSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the flat state machine,
 *        as this is called any time a new event is passed to the event queue. This
 *        function will be called recursively to implement the correct order for a
 *        state transition to be: exit current state -> enter next state using the
 *        ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed."
 * @author J. Edward Carryer, 2011.10.23 19:25 */
ES_Event RunAttackFSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateFSMState_t nextState; // <- need to change enum type here

    //ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case INIT:
            if (ThisEvent.EventType == SUPER_ENTRY) {
                nextState = INITIAL_ORIENT;
                makeTransition = TRUE;
            }
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case INITIAL_ORIENT:
           switch(ThisEvent.EventType){
                case ES_ENTRY:
                    if(SIDE == LEFT){
                        RightTankTurn(SPEED/2);
                    }
                    else{
                        LeftTankTurn(SPEED/2);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case GOAL_DETECTOR_TRIPPED:
                    nextState = FACE_WALL;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
               case DOUBLE_DETECTOR_TRIPPED:
                    nextState = FACE_WALL;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case FACE_WALL:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 500);
                    if(SIDE == LEFT){
                        LeftTankTurn(SPEED);
                    }
                    else{
                        RightTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = HIT_WALL;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case HIT_WALL:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Forward(SPEED/2);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FL_BUMP_HIT:
                    nextState = BACKSTEP;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT: //Very High Probability that this will be obstacle
                    nextState = BACKSTEP;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;          
                case DB_BUMP_HIT:
                    nextState = BACKSTEP;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
        }
        break;
        case BACKSTEP:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 700);
                    Reverse(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = REAL_ORIENT;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case REAL_ORIENT:
           switch(ThisEvent.EventType){
                case ES_ENTRY:
                    if(SIDE == LEFT){
                        RightTankTurn(SPEED/3);
                    }
                    else{
                        LeftTankTurn(SPEED/3);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case GOAL_DETECTOR_TRIPPED:
                    nextState = LEAVE_HOME;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LEAVE_HOME:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Forward(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = TWO_POINT_TRAVEL;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case TWO_POINT_TRAVEL:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if(SIDE == RIGHT){
                        VeerLeft(SPEED, 0.9);
                    }
                    Forward(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case LEFT_TAPE_TRIPPED:
                    if(SIDE == RIGHT){
                        nextState = LEAVE_TAPE;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case RIGHT_TAPE_TRIPPED:
                    if(SIDE == LEFT){
                        nextState = LEAVE_TAPE;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = INIT;
                    makeTransition = TRUE;
                    break;
            }
            break;
        case FIND_TAPE:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (SIDE == LEFT) {
                        VeerRight(SPEED, 0.1);
                    } else {
                        VeerLeft(SPEED, 0.1);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case RIGHT_TAPE_TRIPPED:
                    if (SIDE == LEFT) {
                        nextState = LEAVE_TAPE;
                        makeTransition = TRUE;
                    }   
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case LEFT_TAPE_TRIPPED:
                    if (SIDE == RIGHT) {
                        nextState = LEAVE_TAPE;
                        makeTransition = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = INIT;
                    makeTransition = TRUE;
                    break;
                case FL_BUMP_HIT:
                    nextState = TURN_FROM_OBST;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT:
                    nextState = TURN_FROM_OBST;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case DB_BUMP_HIT:
                    nextState = TURN_FROM_OBST;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break; 
            }
            break;
        case LEAVE_TAPE:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (SIDE == LEFT) {
                        VeerLeft(SPEED, 0.1);
                    } else {
                        VeerRight(SPEED, 0.1);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case RIGHT_TAPE_SLEEP:
                    if (SIDE == LEFT) {
                        nextState = FIND_TAPE;
                        makeTransition = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case LEFT_TAPE_SLEEP:
                    if (SIDE == RIGHT) {
                        nextState = FIND_TAPE;
                        makeTransition = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = INIT;
                    makeTransition = TRUE;
                    break;
                case FL_BUMP_HIT:
                    nextState = RECOIL_FROM_OBST;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT:
                    nextState = RECOIL_FROM_OBST;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case DB_BUMP_HIT:
                    nextState = RECOIL_FROM_OBST;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break; 
            }
            break;
        case TURN_FROM_OBST:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 600);
                    if(SIDE == LEFT){
                        RightTankTurn(SPEED);
                    }
                    else{
                        LeftTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = CROSS_FIELD;
                        makeTransition = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case RECOIL_FROM_OBST:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 300);
                    Reverse(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = TURN_FROM_OBST;
                        makeTransition = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case CROSS_FIELD:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    Forward(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FL_BUMP_HIT:
                    if(SIDE == RIGHT){
                        SIDE = LEFT;
                    }
                    else{
                        SIDE = RIGHT;
                    }
                    secAttack = true; 
                    nextState = INIT;
                    makeTransition = TRUE;
                    break;
                case FR_BUMP_HIT:
                    if(SIDE == RIGHT){
                        SIDE = LEFT;
                    }
                    else{
                        SIDE = RIGHT;
                    }
                    secAttack = true; 
                    nextState = INIT;
                    makeTransition = TRUE;
                    break;
            }
            break;
    }
    if (makeTransition == TRUE) {
        RunAttackFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunAttackFSM(ENTRY_EVENT);
    }
    //ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
