/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include <ES_Framework.h>
#include <BOARD.h>
#include <stdio.h>
#include "MinspecHSM.h"
#include "MotorControl.h"
#include "Shooter.h"
#include "AttackFSM.h" //#include all sub state machines called
#include "SecAttackFSM.h"
#include "AimFSM.h"
#include "RetreatFSM.h" 
#include "SecRetreatFSM.h"
#include <stdbool.h>
/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
//Include any defines you need to do

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


typedef enum {
    INIT,
    ATTACK,
    SEC_ATTACK,
    AIM,
    RETREAT,
    SEC_RETREAT,
    LOADING,
} TemplateHSMState_t;

static const char *StateNames[] = {
	"INIT",
	"ATTACK",
	"SEC_ATTACK",
	"AIM",
	"RETREAT",
	"SEC_RETREAT",
	"LOADING",
};




/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
   Example: char RunAway(uint_8 seconds);*/



/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TemplateHSMState_t CurrentState = INIT; // <- change enum name to match ENUM`
static uint8_t MyPriority;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitMinspecHSM(uint8_t Priority) {
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = INIT;
    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostMinspecHSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
uint16_t SIDE;
bool secAttack = false;

ES_Event RunMinspecHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateHSMState_t nextState; // <- change type to correct enum

    // ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case INIT: // If current state is initial Pseudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state
                // Initialize all sub-state machines
                InitAttackFSM();
                InitRetreatFSM();
                InitAimFSM();
                // now put the machine into the actual initial state
                nextState = LOADING;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case ATTACK:
            ThisEvent = RunAttackFSM(ThisEvent);
            switch (ThisEvent.EventType)
            {
            case REAR_TAPE_SLEEP:
                nextState = AIM;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                break;
            case FL_BUMP_HIT:
                nextState = SEC_ATTACK;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                break;
            case FR_BUMP_HIT:
                nextState = SEC_ATTACK;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                break;
            default:
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case SEC_ATTACK:
            ThisEvent = RunSecAttackFSM(ThisEvent);
            switch (ThisEvent.EventType)
            {
                case REAR_TAPE_SLEEP:
                    nextState = AIM;
                    makeTransition = TRUE;
                    break;

                default:
                    ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case SEC_RETREAT:
            ThisEvent = RunSecRetreatFSM(ThisEvent);
            switch (ThisEvent.EventType)
            {
                case REAR_TAPE_SLEEP:
                    nextState = AIM;
                    makeTransition = TRUE;
                    break;
                case ES_TIMEOUT:
                    nextState = LOADING;
                    makeTransition = TRUE;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case RETREAT:
            ThisEvent = RunRetreatFSM(ThisEvent);
            switch (ThisEvent.EventType)
            {
                case ES_TIMEOUT:
                    nextState = LOADING;
                    makeTransition = TRUE;
                    break;
            default:
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case AIM:
            ThisEvent = RunAimFSM(ThisEvent);
            switch (ThisEvent.EventType)
            {
                case ES_TIMEOUT:
                    if(secAttack){
                        nextState = SEC_RETREAT;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                    }
                    nextState = RETREAT;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case LOADING:
            switch (ThisEvent.EventType)
            {
                case SUPER_ENTRY:
                    ES_Timer_InitTimer(HSM_TIMER, 2500);
                    StopMovement();
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == HSM_TIMER){
                        nextState = ATTACK;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;

                    }
                    break;
                default:
                    ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunMinspecHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunMinspecHSM(SUPER_ENTRY_EVENT); // <- rename to your own Run function
    }

    //ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
