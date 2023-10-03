/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MotorControl.h"
#include "Shooter.h"
#include "AimFSM.h"
#include <BOARD.h>
#include <stdbool.h>
//Uncomment these for the Roaches
//#include "roach.h"
//#include "RoachFrameworkEvents.h"
#include <stdio.h>


/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define LOAD_TIME 500

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
    ORIENT_1,
    ORIENT_2,
    LEAVE_BEACON,
    FACE_POSITION,
    APPROACH_POSITION,
    SWEET_SPOT,
    SAMMY_CHECK,
    WAIT,
    AIM,
    REV,
    HOP_FORWARD,
    HOP_BACK,
    LOAD_1,
    FIRE_1,
    LOAD_2,
    FIRE_2,
    LOAD_3,
    FIRE_3,
} TemplateFSMState_t;

static const char *StateNames[] = {
	"INIT",
	"ORIENT_1",
	"ORIENT_2",
	"LEAVE_BEACON",
	"FACE_POSITION",
	"APPROACH_POSITION",
	"SWEET_SPOT",
	"SAMMY_CHECK",
	"WAIT",
	"AIM",
	"REV",
	"HOP_FORWARD",
	"HOP_BACK",
	"LOAD_1",
	"FIRE_1",
	"LOAD_2",
	"FIRE_2",
	"LOAD_3",
	"FIRE_3",
};


static TemplateFSMState_t CurrentState = INIT; // <- change enum name to match ENUM
static uint8_t MyPriority;
extern uint16_t SIDE; 
bool hopForward = true;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

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
uint8_t InitAimFSM(void) {
    ES_Event returnEvent;
    CurrentState = INIT;
    returnEvent = RunAimFSM(INIT_EVENT);
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
uint8_t PostAimFSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateFSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and p                            aram) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the flat state machine,
 *        as this is called any time a new event is passed to the event queue. This
 *        function will be called recursively to implement the correct order for a
 *        state transition to be: exit current state -> enter next state using the
 *        ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed."
 * @author J. Edward Carryer, 2011.10.23 19:25 */

ES_Event RunAimFSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateFSMState_t nextState; // <- need to change enum type here

    //ES_Tattle(); // trace call stack
    switch (CurrentState) {
        case INIT:
            if (ThisEvent.EventType == SUPER_ENTRY){
                nextState = LEAVE_BEACON;
                makeTransition = TRUE;
            }
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case LEAVE_BEACON:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 700);
                    if(SIDE == RIGHT){
                        LeftTankTurn(SPEED);
                    }
                    else{
                        RightTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = ORIENT_1;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case ORIENT_1:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    if(SIDE == LEFT){
                        LeftTankTurn(SPEED/4);
                    }
                    else{
                        RightTankTurn(SPEED/4);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case GOAL_DETECTOR_TRIPPED:
                    nextState = FACE_POSITION;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case DOUBLE_DETECTOR_TRIPPED:
                    nextState = REV;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    /*
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = ORIENT_2;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                    */
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case FACE_POSITION:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 250);
                    if(SIDE == LEFT){
                        LeftTankTurn(SPEED);
                    }
                    if(SIDE == RIGHT){
                        RightTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    nextState = APPROACH_POSITION;    
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case APPROACH_POSITION:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 1000);
                    Forward(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    nextState = SWEET_SPOT;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case SWEET_SPOT:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    if(SIDE == LEFT){
                        ES_Timer_InitTimer(HSM_TIMER, 200);
                        RightTankTurn(SPEED);
                    }
                    if(SIDE == RIGHT){
                        ES_Timer_InitTimer(HSM_TIMER, 150);
                        LeftTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    nextState = WAIT;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case WAIT:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    StopMovement();
                    ES_Timer_SetTimer(HSM_TIMER, 10000);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case DOUBLE_DETECTOR_TRIPPED:
                    nextState = REV;
                    makeTransition = TRUE; 
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case SAMMY_DETECTOR_TRIPPED:
                    nextState = REV;
                    makeTransition = TRUE; 
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
            /*
        case ORIENT_2:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    if(SIDE == LEFT){
                        LeftTankTurn(SPEED/3);
                    }
                    else{
                        RightTankTurn(SPEED/3);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case GOAL_DETECTOR_TRIPPED:
                    nextState = AIM;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case DOUBLE_DETECTOR_TRIPPED:
                    nextState = AIM;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
            */
        case REV:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleShooterMotor();
                    StopMovement();
                    ES_Timer_InitTimer(HSM_TIMER, 4000);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = LOAD_1;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LOAD_1:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleActuator();
                    ES_Timer_InitTimer(HSM_TIMER, LOAD_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = FIRE_1;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case FIRE_1:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleActuator();
                    ES_Timer_InitTimer(HSM_TIMER, LOAD_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = LOAD_2;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LOAD_2:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleActuator();
                    ES_Timer_InitTimer(HSM_TIMER, LOAD_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = FIRE_2;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case FIRE_2:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleActuator();
                    ES_Timer_InitTimer(HSM_TIMER, LOAD_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = LOAD_3;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LOAD_3:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleActuator();
                    ES_Timer_InitTimer(HSM_TIMER, LOAD_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = FIRE_3;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case FIRE_3:
            switch (ThisEvent.EventType){
                case ES_ENTRY:
                    ToggleActuator();
                    ES_Timer_InitTimer(HSM_TIMER, LOAD_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        ToggleShooterMotor();
                        StopMovement();
                        nextState = INIT;
                        makeTransition = TRUE;
                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break; 
        default:
            break;
    }
    if (makeTransition == TRUE) {
        RunAimFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunAimFSM(ENTRY_EVENT);
    }
    //ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/