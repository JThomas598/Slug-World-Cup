/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MotorControl.h"
#include "OrientFSM.h"
#include "MinspecHSM.h"
#include <IO_Ports.h>
#include <BOARD.h>
#include <stdbool.h> 
#include "IO_Ports.h"
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
    SEEK_GOAL,
} TemplateFSMState_t;

static const char *StateNames[] = {
	"INIT",
	"SEEK_GOAL",
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
uint8_t InitOrientFSM(void) {
    ES_Event returnEvent;
    CurrentState = INIT;
    IO_PortsSetPortInputs(PORTW, PIN7);
    returnEvent = RunOrientFSM(INIT_EVENT);
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
uint8_t PostOrientFSM(ES_Event ThisEvent) {
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


ES_Event RunOrientFSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateFSMState_t nextState; // <- need to change enum type here

    //ES_Tattle(); // trace call stack
    switch (CurrentState) {
        case INIT:
            if (ThisEvent.EventType == ES_ENTRY){
                nextState = SEEK_GOAL;
                makeTransition = TRUE;
            }
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case SEEK_GOAL:
            switch(ThisEvent.EventType){
                case ES_ENTRY:
                    if(SIDE == LEFT){
                        RightTankTurn(SPEED);
                    }
                    else{
                        LeftTankTurn(SPEED);
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case GOAL_DETECTOR_TRIPPED:
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
        RunOrientFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunOrientFSM(ENTRY_EVENT);
    }
    //ES_Tail(); // trace call stack end
    return ThisEvent;
}
/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *;
 ******************************************************************************/
