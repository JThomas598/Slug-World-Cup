
#include "ES_Configure.h"
#include "EventCheckers.h"
#include "ES_Events.h"
#include "ES_Timers.h"
#include "serial.h"
#include "AD.h"
#include "IO_Ports.h"
#include  <stdbool.h>
#include <stdio.h>
#include <sensors.h>
#include"MinspecHSM.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define BATTERY_DISCONNECT_THRESHOLD 175

/*******************************************************************************
 * EVENTCHECKER_TEST SPECIFIC CODE                                                             *
 ******************************************************************************/

//#define EVENTCHECKER_TEST
#ifdef EVENTCHECKER_TEST

#define SaveEvent(x) do {eventName=__func__; storedEvent=x;} while (0)

static const char *eventName;
static ES_Event storedEvent;
#endif

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this EventChecker. They should be functions
   relevant to the behavior of this particular event checker */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

/* Any private module level variable that you might need for keeping track of
   events would be placed here. Private variables should be STATIC so that they
   are limited in scope to this module. */

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function TemplateCheckBattery(void)
 * @param none
 * @return TRUE or FALSE
 * @brief This function is a prototype event checker that checks the battery voltage
 *        against a fixed threshold (#defined in the .c file). Note that you need to
 *        keep track of previous history, and that the actual battery voltage is checked
 *        only once at the beginning of the function. The function will post an event
 *        of either BATTERY_CONNECTED or BATTERY_DISCONNECTED if the power switch is turned
 *        on or off with the USB cord plugged into the Uno32. Returns TRUE if there was an 
 *        event, FALSE otherwise.
 * @note Use this code as a template for your other event checkers, and modify as necessary.
 * @author Gabriel H Elkaim, 2013.09.27 09:18
 * @modified Gabriel H Elkaim/Max Dunne, 2016.09.12 20:08 */
 uint16_t prevTape = 0x0000;

typedef enum TapeState{
    LEFT_TAPE_PATTERN = 1,
    CENTER_TAPE_PATTERN = 2,
    RIGHT_TAPE_PATTERN = 4,
    REAR_TAPE_PATTERN = 8
}TapeState;

uint8_t CheckCenterTapeSensor(void){
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    static uint16_t prevCenterTape = 0x0000;
    uint16_t tapeVal = ReadTapeSensors() & CENTER_TAPE_PATTERN;
    if(tapeVal != prevCenterTape){
        if (tapeVal > prevCenterTape){
            curEvent = CENTER_TAPE_TRIPPED;
        }
        else{
            curEvent = CENTER_TAPE_SLEEP;
        }
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = tapeVal;
        returnVal = TRUE;
        prevCenterTape = tapeVal;
        #ifndef EVENTCHECKER_TEST
            PostMinspecHSM(thisEvent);
        #else
            SaveEvent(thisEvent);
        #endif   
    }
    return (returnVal);
}
uint8_t CheckRightTapeSensor(void){
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    static uint16_t prevRightTape = 0x0000;
    uint16_t tapeVal = ReadTapeSensors() & RIGHT_TAPE_PATTERN;
    if(tapeVal != prevRightTape){
        if (tapeVal > prevRightTape){
            curEvent = RIGHT_TAPE_TRIPPED;
        }
        else{
            curEvent = RIGHT_TAPE_SLEEP;
        }
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = tapeVal;
        returnVal = TRUE;
        prevRightTape = tapeVal;
        #ifndef EVENTCHECKER_TEST
            PostMinspecHSM(thisEvent);
        #else
            SaveEvent(thisEvent);
        #endif   
    }
    return (returnVal);
}
uint8_t CheckLeftTapeSensor(void){
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    static uint16_t prevLeftTape = 0x0000;
    uint16_t tapeVal = ReadTapeSensors() & LEFT_TAPE_PATTERN;
    if(tapeVal != prevLeftTape){
        if (tapeVal > prevLeftTape){
            curEvent = LEFT_TAPE_TRIPPED;
        }
        else{
            curEvent = LEFT_TAPE_SLEEP; 
        }
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = tapeVal;
        returnVal = TRUE;
        prevLeftTape = tapeVal;
        #ifndef EVENTCHECKER_TEST
            PostMinspecHSM(thisEvent);
        #else
            SaveEvent(thisEvent);
        #endif   
    }
    return (returnVal);
}

#define REAR_SAMPLE_WINDOW_SIZE 1000
static uint16_t rearTapeVal[REAR_SAMPLE_WINDOW_SIZE];

void RearTapeAddSample(uint16_t sample){
    for(int i = REAR_SAMPLE_WINDOW_SIZE - 2; i >= 0; i--){
        rearTapeVal[i] = rearTapeVal[i + 1];
    }
    rearTapeVal[REAR_SAMPLE_WINDOW_SIZE - 1] = sample;
}

bool RearTapeIsStable(void){
    for(int i = REAR_SAMPLE_WINDOW_SIZE - 1; i > 0; i--){
        if(rearTapeVal[i] != rearTapeVal[i - 1]){
            return false;
        }
    }
    return true;
}

uint8_t CheckRearTapeSensor(void){
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    static uint16_t prevRearTape = 0x0000;
    RearTapeAddSample(ReadTapeSensors() & REAR_TAPE_PATTERN);
    if(RearTapeIsStable() && rearTapeVal[0] != prevRearTape){
        if (rearTapeVal[0] > prevRearTape){
            curEvent = REAR_TAPE_TRIPPED;
        }
        else{
            curEvent = REAR_TAPE_SLEEP; 
        }
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = rearTapeVal[0];
        returnVal = TRUE;
        prevRearTape = rearTapeVal[0];
        #ifndef EVENTCHECKER_TEST
            PostMinspecHSM(thisEvent);
        #else
            SaveEvent(thisEvent);
        #endif   
    }  
    return (returnVal);
}

#define SAMPLE_WINDOW_SIZE 100

typedef enum BumpState{
    BACK_BUMP_PRESSED = 3,
    FRONT_BUMP_PRESSED,
    FRONT_RIGHT_BUMP_PRESSED,
    FRONT_LEFT_BUMP_PRESSED
}BumpState;

static uint16_t prevBump = 0x7;
static uint16_t bumpVal[SAMPLE_WINDOW_SIZE];

void BumperAddSample(uint16_t sample){
    for(int i = SAMPLE_WINDOW_SIZE - 2; i >= 0; i--){
        bumpVal[i] = bumpVal[i + 1];
    }
    bumpVal[SAMPLE_WINDOW_SIZE - 1] = sample;
}

bool BumperIsStable(void){
    for(int i = SAMPLE_WINDOW_SIZE - 1; i > 0; i--){
        if(bumpVal[i] != bumpVal[i - 1]){
            return false;
        }
    }
    return true;
}

uint8_t CheckBumpers(void) {
    ES_EventTyp_t curEvent = ES_NO_EVENT;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    BumperAddSample(ReadBumpers());
    if(BumperIsStable() && bumpVal[0] != prevBump){
        if (bumpVal[0] < prevBump){
            switch(bumpVal[0]){
                case BACK_BUMP_PRESSED:
                    curEvent = BK_BUMP_HIT;
                    printf("BACK BUMP HIT\r\n");
                    break;
                case FRONT_RIGHT_BUMP_PRESSED:
                    curEvent = FR_BUMP_HIT;
                    printf("FRONT RIGHT BUMP HIT\r\n");
                    break;
                case FRONT_LEFT_BUMP_PRESSED:
                    curEvent = FL_BUMP_HIT;
                    printf("FRONT LEFT BUMP HIT\r\n");
                    break;
                case FRONT_BUMP_PRESSED:
                    curEvent = DB_BUMP_HIT;
                    break;
                default:
                    break;
            }
        }
        else if(bumpVal[0] > prevBump){
            switch(prevBump){
                case BACK_BUMP_PRESSED:
                    curEvent = BK_BUMP_REL;
                    printf("BACK BUMP RELEASE\r\n");
                    break;
                case FRONT_RIGHT_BUMP_PRESSED:
                    curEvent = FR_BUMP_REL;
                    printf("FRONT RIGHT BUMP RELEASE\r\n");
                    break;
                case FRONT_LEFT_BUMP_PRESSED:
                    curEvent = FL_BUMP_REL;
                    printf("FRONT LEFT BUMP RELEASE\r\n");
                    break;
                default:
                    break;
            }
        }
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = bumpVal[0];
        returnVal = TRUE;
        prevBump = bumpVal[0];
        #ifndef EVENTCHECKER_TEST
            PostMinspecHSM(thisEvent);
        #else
            SaveEvent(thisEvent);
        #endif   
    }

    return (returnVal);
}

typedef enum BeacState{
   DOUBLE_DETECTOR_PATTERN = 0,
   GOAL_DETECTOR_PATTERN = 4,
   SAMMY_DETECTOR_PATTERN = 1,
}BeacState;

uint8_t CheckBeacons(){
    ES_EventTyp_t curEvent = ES_NO_EVENT;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    static uint16_t prevBeac = 0x0005;
    uint16_t beacVal = ReadBeacons();
    if(beacVal != prevBeac){
        switch(beacVal){
            case GOAL_DETECTOR_PATTERN:
                curEvent = GOAL_DETECTOR_TRIPPED;
                break;
            case SAMMY_DETECTOR_PATTERN:
                curEvent = SAMMY_DETECTOR_TRIPPED;
                break;
            case DOUBLE_DETECTOR_PATTERN:
                curEvent = DOUBLE_DETECTOR_TRIPPED;
                break;
            default:
                break;
        }
        prevBeac = beacVal;
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = beacVal;
        returnVal = TRUE;
        #ifndef EVENTCHECKER_TEST
            PostMinspecHSM(thisEvent);
        #else
            SaveEvent(thisEvent);
        #endif   
        
    }

    
    return (returnVal);
}



/* 
 * The Test Harness for the event checkers is conditionally compiled using
 * the EVENTCHECKER_TEST macro (defined either in the file or at the project level).
 * No other main() can exist within the project.
 * 
 * It requires a valid ES_Configure.h file in the project with the correct events in 
 * the enum, and the correct list of event checkers in the EVENT_CHECK_LIST.
 * 
 * The test harness will run each of your event detectors identically to the way the
 * ES_Framework will call them, and if an event is detected it will print out the function
 * name, event, and event parameter. Use this to test your event checking code and
 * ensure that it is fully functional.
 * 
 * If you are locking up the output, most likely you are generating too many events.
 * Remember that events are detectable changes, not a state in itself.
 * 
 * Once you have fully tested your event checking code, you can leave it in its own
 * project and point to it from your other projects. If the EVENTCHECKER_TEST marco is
 * defined in the project, no changes are necessary for your event checkers to work
 * with your other projects.
 */
#ifdef EVENTCHECKER_TEST
#include <stdio.h>
static uint8_t(*EventList[])(void) = {EVENT_CHECK_LIST};

void PrintEvent(void);

void main(void) {
    BOARD_Init();
    AD_Init();
    SERIAL_Init();
    ES_Timer_Init();
    /* user initialization code goes here */
    InitSensors();
    // Do not alter anything below this line
    int i;
    printf("\r\nEvent checking test harness for %s", __FILE__);

    while (1) {
        if (IsTransmitEmpty()) {
            for (i = 0; i< sizeof (EventList) >> 2; i++) {
                if (EventList[i]() == TRUE) {
                    PrintEvent();
                    break;
                }

            }
        }
    }
}

void PrintEvent(void) {
    printf("\r\nFunc: %s\tEvent: %s\tParam: 0x%X", eventName,
            EventNames[storedEvent.EventType], storedEvent.EventParam);
}
#endif