/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MotorControl.h"
#include "Shooter.h"
#include "SecAttackFSM.h"
#include "MinspecHSM.h"
#include <IO_Ports.h>
#include <BOARD.h>
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
    ORIENT,
    LEAVE_HOME,
    ADJUST, 
    FACE_WALL,
    HIT_WALL,
    REAL_ORIENT,
    BACKSTEP,
    CLEAR_EMF,
    FIND_TAPE,
    LEAVE_TAPE,
    TURN_FROM_OBST,
    CROSS_FIELD,
    TWO_POINT_TRAVEL,
    AIM,
    FIRE,
} TemplateFSMState_t;

static const char *StateNames[] = {
	"INIT",
	"ORIENT",
	"LEAVE_HOME",
	"ADJUST",
	"FACE_WALL",
	"HIT_WALL",
	"REAL_ORIENT",
	"BACKSTEP",
	"CLEAR_EMF",
	"FIND_TAPE",
	"LEAVE_TAPE",
	"TURN_FROM_OBST",
	"CROSS_FIELD",
	"TWO_POINT_TRAVEL",
	"AIM",
	"FIRE",
};


static TemplateFSMState_t CurrentState = INIT; // <- change enum name to match ENUM
static uint8_t MyPriority;
extern uint16_t SIDE;


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
uint8_t InitSecAttackFSM(void) {
    ES_Event returnEvent;
    CurrentState = INIT;
    returnEvent = RunSecAttackFSM(INIT_EVENT);
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
uint8_t PostSecAttackFSM(ES_Event ThisEvent) {
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
ES_Event RunSecAttackFSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateFSMState_t nextState; // <- need to change enum type here

    //ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case INIT:
            if (ThisEvent.EventType == SUPER_ENTRY) {
                nextState = BACKSTEP;
                makeTransition = TRUE;
            }
            ThisEvent.EventType = ES_NO_EVENT;
        case BACKSTEP:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 100);
                    Reverse(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == HSM_TIMER) {
                        nextState = ADJUST;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case ADJUST:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 150);
                    if (SIDE == LEFT) {
                        RightTankTurn(SPEED);
                    } else {
                        LeftTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == HSM_TIMER) {
                        nextState = TWO_POINT_TRAVEL;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case ORIENT:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (SIDE == LEFT) {
                        RightTankTurn(SPEED / 2);
                    } else {
                        LeftTankTurn(SPEED / 2);
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
        case TWO_POINT_TRAVEL:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Forward(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FL_BUMP_HIT:
                    nextState = BACKSTEP;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT:
                    nextState = BACKSTEP;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = INIT;
                    makeTransition = TRUE;
                    break;
            }
            break;
    }
    if (makeTransition == TRUE) {
        RunSecAttackFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunSecAttackFSM(ENTRY_EVENT);
    }
    //ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
