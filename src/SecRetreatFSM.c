/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MotorControl.h"
#include "MinspecHSM.h"
#include "SecRetreatFSM.h"
#include <stdbool.h>
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
    RESET,
    TURN_AROUND,
    LEAVE_TAPE,
    FIND_TAPE,
    ADJUST,
    BACKSTEP,
    LEAVE_ONE_POINT,
    LEAVE_TWO_POINT,
    PRE_DOCK,
    DOCK,
    SPACE,
    HEAD_HOME,
    LEAVE_CENTER_LINE,
} TemplateFSMState_t;

static const char *StateNames[] = {
	"INIT",
	"RESET",
	"TURN_AROUND",
	"LEAVE_TAPE",
	"FIND_TAPE",
	"ADJUST",
	"BACKSTEP",
	"LEAVE_ONE_POINT",
	"LEAVE_TWO_POINT",
	"PRE_DOCK",
	"DOCK",
	"SPACE",
	"HEAD_HOME",
	"LEAVE_CENTER_LINE",
};


static TemplateFSMState_t CurrentState = INIT; // <- change enum name to match ENUM
static uint8_t MyPriority;
static Zone zone = ONE;
extern uint8_t SIDE;
static bool inOne = false;


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
uint8_t InitSecRetreatFSM(void) {
    ES_Event returnEvent;
    CurrentState = INIT;
    returnEvent = RunSecRetreatFSM(INIT_EVENT);
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
uint8_t PostSecRetreatFSM(ES_Event ThisEvent) {
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

ES_Event RunSecRetreatFSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateFSMState_t nextState; // <- need to change enum type here

    //ES_Tattle(); // trace call stack
    switch (CurrentState) {
        case INIT:
            if (ThisEvent.EventType == SUPER_ENTRY){
                nextState = TURN_AROUND;
                zone = ONE;
                makeTransition = TRUE;
            }
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case TURN_AROUND:
            switch (ThisEvent.EventType)
            {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 1000);
                    if(SIDE == RIGHT){
                        RightTankTurn(SPEED);
                    }
                    else{
                        LeftTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = LEAVE_ONE_POINT;
                        inOne = true;
                        makeTransition = TRUE;
                        break;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LEAVE_ONE_POINT:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Forward(SPEED);
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT:
                    if(SIDE == LEFT){
                        nextState = BACKSTEP;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case FL_BUMP_HIT:
                    if(SIDE == RIGHT){
                        nextState = BACKSTEP;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = LEAVE_TWO_POINT;
                    inOne = false;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LEAVE_TWO_POINT:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Forward(SPEED);
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT:
                    if(SIDE == LEFT){
                        nextState = BACKSTEP;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case FL_BUMP_HIT:
                    if(SIDE == RIGHT){
                        nextState = BACKSTEP;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case REAR_TAPE_SLEEP:
                    nextState = PRE_DOCK;
                    inOne = false;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case BACKSTEP:
            switch (ThisEvent.EventType)
            {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 200);
                    Reverse(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = ADJUST;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case REAR_TAPE_TRIPPED:
                        inOne = true;    
                        nextState = LEAVE_ONE_POINT;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                case REAR_TAPE_SLEEP:
                        inOne = true;
                        nextState = LEAVE_ONE_POINT;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case ADJUST:
            switch (ThisEvent.EventType)
            {
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 150);
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
                        nextState = LEAVE_TWO_POINT;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case REAR_TAPE_TRIPPED:
                        nextState = LEAVE_ONE_POINT;
                        inOne = true;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                case REAR_TAPE_SLEEP:
                        nextState = LEAVE_ONE_POINT;
                        inOne = true;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case PRE_DOCK:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 500);
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
                        nextState = DOCK;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;
        case DOCK:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    Forward(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FL_BUMP_HIT:
                    nextState = SPACE;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case FR_BUMP_HIT:
                    nextState = SPACE;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case DB_BUMP_HIT:
                    nextState = SPACE;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case SPACE:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 200);
                    Reverse(SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = INIT;
                        makeTransition = TRUE;
                    }
                    break;
            }
            break;
        default:
            break;
    }
    if (makeTransition == TRUE) {
        RunSecRetreatFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunSecRetreatFSM(ENTRY_EVENT);
    }
    //ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
