#include "Brain.h"
#include "Sensor.h"
#include "Vision.h"
#include "Movement.h"
#include "Arm.h"

// TODO add debug flag
// TODO maybe change volatile to static methods?

#define IGNORE_WEIGHTS false

Reading topLeftReading;
Reading topRightReading;
Reading bottomLeftReading;
Reading bottomRightReading;
Reading middleProximityReading;

typedef struct AllStates_t {
    State forwardAfterASec;
    State forwardsMindlessly;
    State scanningRandom;
    State scanningLeft;
    State scanningRight;
    State towardsOnLeft;
    State towardsOnRight;
    State pickingUpMiddle;
    State pickingUpLeft;
    State pickingUpRight;
    State avoidingFront;
    State avoidingLeft;
    State avoidingRight;
} AllStates;

static AllStates allStates;

static void blackhole() {
    Movement::stop();
    while (1) {}
}

static Side randomSide() {
    return millis() % 2 ? LEFT : RIGHT;
}


/***********************************************************************
 *    _____ _               _        
 *   / ____| |             | |       
 *  | |    | |__   ___  ___| | _____ 
 *  | |    | '_ \ / _ \/ __| |/ / __|
 *  | |____| | | |  __/ (__|   <\__ \
 *   \_____|_| |_|\___|\___|_|\_\___/                              
 *
 **********************************************************************/ 
// TODO make private and static?
bool foundWallCloseOnSide(Side side) {
    // If there is a low object on both sides within 20cm difference between sensors
    volatile bool found = Vision.found(TALL, side, 200 /* TODO: Tune wall max distance */);
    return found;
}

bool foundWallCloseBothSides() {
    volatile bool found = foundWallCloseOnSide(LEFT) && foundWallCloseOnSide(RIGHT);
    return found;
}

bool foundRampClose() {
    volatile bool found = Vision.found(SHORT, LEFT, 150) && Vision.found(SHORT, RIGHT, 150);
    return found;
}

bool foundWallCloseEitherSide() {
    volatile bool found = foundWallCloseOnSide(LEFT) || foundWallCloseOnSide(RIGHT);
    return found;
}

/* Returns the distance to a weight if there is one and negative (a status code type thing) if there is no weight */
int weightDistanceOnSide(Side side) {
    if (Vision.foundOnBothSides(SHORT)) {
        // If there is a low object on both sides within 20cm difference between sensors
        if (Vision.foundOnBothSides(SHORT, 200 /* TODO: Tune threshold */)) {
            return -1;
        }
    } else {
        if (side != Vision.getClosestSide(SHORT)) {
            // If there is two objects but this side isn't the closer one
            return -2;
        }
    }


    // If we find a wall close on either side then return
    if (foundWallCloseEitherSide()) {
        return -3;
    }

    // See if we found something on the bottom sensor
    if (Vision.found(SHORT, side, 600 /* TODO: Tune max distance */)) {
        int distanceToWeight = getGenericReadings(SHORT, side).sameHeightSameSide->distanceMM;
        // Check that it isn't also a wall (within a certain threshold)
        if (!Vision.found(TALL, side, distanceToWeight, 200 /* TODO: Tune distance threshold between top and bottom */)) {
        // if (!Vision.found(TALL, side) && !Vision.found(TALL, (Side)-side)) {
            Serial.print("Found weight at distance=");
            Serial.print(distanceToWeight);
            Serial.println();
            return distanceToWeight; // Yay found a weight
        } else {
            return -4; // As there is a wall close to or near the weight
        }
    }

    return -5;
}

bool foundWeightFarOnSide(Side side) {
    volatile bool found = 200 /* TODO: Tune weight close distance */ < weightDistanceOnSide(side);
    return found;
}

bool foundWeightCloseOnSide(Side side) {
    int weightDistance = weightDistanceOnSide(side);
    volatile bool found = 0 < weightDistance && weightDistance <= 200; /* TODO: Tune weight close distance */
    return found;
}

bool foundWeightCloseMiddle() {
    if (middleProximityReading.foundObject) {
        const int thresholdBottom = 350;
        volatile bool foundWideObject = bottomLeftReading.foundObjectCloser(thresholdBottom) || bottomRightReading.foundObjectCloser(thresholdBottom);
        const int thresholdWall = 350;
        volatile bool foundWall = Vision.found(TALL, LEFT, thresholdWall) || Vision.found(TALL, RIGHT, thresholdWall);
        return !foundWideObject && !foundWall;
    } else {
        return false;
    }
}

bool checkTowardsOnLeft() {
    volatile bool found = foundWeightFarOnSide(LEFT);
    return found;
}

bool checkTowardsOnRight() {
    volatile bool found = foundWeightFarOnSide(RIGHT);
    return found;
}

bool checkPickingUpMiddle() {
    volatile bool found = foundWeightCloseMiddle();
    return found;
}

bool checkPickingUpLeft() {
    return false;
    volatile bool found = foundWeightCloseOnSide(LEFT);
    return found;
}

bool checkPickingUpRight() {
    return false;
    volatile bool found = foundWeightCloseOnSide(RIGHT);
    return found;
}

bool checkAvoidingFront() {
    volatile bool found = foundWallCloseBothSides() || foundRampClose();
    return found;
}

bool checkAvoidingLeft() {
    volatile bool found = foundWallCloseOnSide(LEFT);
    return found;
}

bool checkAvoidingRight() {
    volatile bool found = foundWallCloseOnSide(RIGHT);
    return found;
}


/***********************************************************************
 *                _   _                 
 *      /\       | | (_)                
 *     /  \   ___| |_ _  ___  _ __  ___ 
 *    / /\ \ / __| __| |/ _ \| '_ \/ __|
 *   / ____ \ (__| |_| | (_) | | | \__ \
 *  /_/    \_\___|\__|_|\___/|_| |_|___/
 *
 **********************************************************************/                              
void actionNothing() {}

void actionForwardsMindlessly() {
    Movement::together(FAST);
}

void actionGenericScanningOnSide(Side side) {
    Movement::spin(MEDIUM, side);
}

void actionScanningRandom() {
    Brain.stateChange(randomSide() == LEFT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
}

void actionScanningLeft() {
    actionGenericScanningOnSide(LEFT);
}

void actionScanningRight() {
    actionGenericScanningOnSide(RIGHT);
}

void routineTowardsSide(Side side) {
    Movement::largeTurn(FAST, FORWARD, side);
}

void actionTowardsOnLeft() {
    routineTowardsSide(LEFT);
}

void actionTowardsOnRight() {
    routineTowardsSide(RIGHT);
}

void routinePickingUpOnSide(Side side) {
    // Code for when the robot used to pick up off the IR sensors
    // if (!middleProximityReading.foundObject) {
    //     Movement::spin(SLOW, side);
    //     while (!middleProximityReading.foundObject) {
    //         Brain.sensorsTick();
    //     }
    // }

    const long timeout = 2000;
    const long start = millis();

    Movement::spin(MEDIUM, (Side)-side);
    while (middleProximityReading.foundObject) {
        Brain.sensorsTick();
        if (millis() > start + timeout) {
            Brain.stateChange(side == LEFT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
            return;
        }
    }
    
    Movement::spin(MEDIUM, side);
    while (!middleProximityReading.foundObject) {
        Brain.sensorsTick();
        if (millis() > start + timeout) {
            Brain.stateChange(side == RIGHT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
            return;
        }
    }
    while (middleProximityReading.foundObject) {
        Brain.sensorsTick();
        if (millis() > start + timeout) {
            Brain.stateChange(side == RIGHT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
            return;
        }
    }
    
    const long nowPlusHalfTime = millis() + (millis() - start) / 4;
    Movement::spin(MEDIUM, (Side)-side);
    while (millis() <= nowPlusHalfTime) {}
    Movement::stop();

    Movement::together(MEDIUM, REVERSE);
    int buffer = 0;
    while (buffer < 10) {
        Brain.sensorsTick();
        if (!middleProximityReading.foundObject) {
            buffer ++;
        } else {
            buffer = 0;
        }
    }
    delay(300);
    Movement::stop();
    Arm.elbowPutDown();
    Movement::together(MEDIUM, FORWARD);
    const int forwardTime = 1300;
    delay(forwardTime);
    Movement::stop();
    Arm.clawClose();

    // TODO make this section based off the top IR readings
    Movement::together(MEDIUM, REVERSE);
    delay(forwardTime);
    Movement::stop();

    Arm.elbowPickup();
    Arm.clawOpen();
    Brain.stateChange(&allStates.scanningRandom, C_RESET_FROM_PREVIOUS);
}

void actionPickingUpMiddle() {
    routinePickingUpOnSide(Brain.getCurrentStateP() == &allStates.towardsOnLeft ? LEFT : RIGHT);
}

void actionPickingUpLeft() {
    routinePickingUpOnSide(LEFT);
}

void actionPickingUpRight() {
    routinePickingUpOnSide(RIGHT);
}

void routineAvoidOnSide(Side side) {
    Movement::together(MEDIUM, REVERSE);
    delay(500);
    Movement::spin(MEDIUM, (Side)-side);
    Brain.sensorsTick();
    const int threshold = 400;
    while (bottomLeftReading.foundObjectCloser(threshold) || bottomRightReading.foundObjectCloser(threshold) || topLeftReading.foundObjectCloser(threshold) || topRightReading.foundObjectCloser(threshold)) {
        delay(10);
        Brain.sensorsTick();
    }
    delay(200); // Spin a bit further than it needs to spin
    Movement::stop();
    // Brain.stateChange(side == LEFT ? &allStates.scanningRight : &allStates.scanningLeft, C_RESET_FROM_PREVIOUS);
    Brain.stateChange(&allStates.forwardAfterASec, C_RESET_FROM_PREVIOUS);
}

void actionAvoidFront() {
    routineAvoidOnSide(randomSide());
}

void actionAvoidLeft() {
    routineAvoidOnSide(LEFT);
}

void actionAvoidRight() {
    routineAvoidOnSide(RIGHT);
}



/************************************************************
 *    _____ _        _            
 *   / ____| |      | |           
 *  | (___ | |_ __ _| |_ ___  ___ 
 *   \___ \| __/ _` | __/ _ \/ __|
 *   ____) | || (_| | ||  __/\__ \
 *  |_____/ \__\__,_|\__\___||___/
 *
 ***********************************************************/
void BrainClass::InitialiseStates() {
    allStates.forwardAfterASec.Initialise("Nothing for a sec", actionNothing);
    allStates.forwardsMindlessly.Initialise("Forwards mindlessly", actionForwardsMindlessly);
    allStates.scanningRandom.Initialise("Scanning random side", actionScanningRandom);
    allStates.scanningLeft.Initialise("Scanning left", actionScanningLeft);
    allStates.scanningRight.Initialise("Scanning right", actionScanningRight);
    allStates.towardsOnLeft.Initialise("Towards on left", actionTowardsOnLeft, checkTowardsOnLeft);
    allStates.towardsOnRight.Initialise("Towards on right", actionTowardsOnRight, checkTowardsOnRight);
    allStates.pickingUpMiddle.Initialise("Picking up middle", actionPickingUpMiddle, checkPickingUpMiddle);
    allStates.pickingUpLeft.Initialise("Picking up on left", actionPickingUpLeft, checkPickingUpLeft);
    allStates.pickingUpRight.Initialise("Picking up on right", actionPickingUpRight, checkPickingUpRight); 
    allStates.avoidingFront.Initialise("Avoiding front", actionAvoidFront, checkAvoidingFront);
    allStates.avoidingLeft.Initialise("Avoiding left", actionAvoidLeft, checkAvoidingLeft);
    allStates.avoidingRight.Initialise("Avoiding right", actionAvoidRight, checkAvoidingRight);
    
    allStates.forwardAfterASec.addWatchdog(&allStates.forwardsMindlessly, 1000);
    allStates.forwardsMindlessly.addWatchdog(&allStates.scanningRandom, 4000);
    allStates.scanningLeft.addWatchdog(&allStates.forwardsMindlessly, 4000);
    allStates.scanningRight.addWatchdog(&allStates.forwardsMindlessly, 4000);
    // allStates.scanningRight.addWatchdog(&allStates.forwardsMindlessly, 5000);
    // allStates.towardsOnLeft.addWatchdog(&allStates.scanningRight, 2500);
    // allStates.towardsOnRight.addWatchdog(&allStates.scanningLeft, 2500);

    // allStates.stopAndWaitAFewSec.setAutomaticTransision();
    // allStates.pickingUpMiddle.setAutomaticTransision();
    // allStates.pickingUpLeft.setAutomaticTransision();
    // allStates.pickingUpRight.setAutomaticTransision();
    // allStates.avoidingFront.setAutomaticTransision();
    // allStates.avoidingLeft.setAutomaticTransision();
    // allStates.avoidingRight.setAutomaticTransision();

    allStates.avoidingFront.setResetSensorsPost();
    allStates.avoidingLeft.setResetSensorsPost();
    allStates.avoidingRight.setResetSensorsPost();
    allStates.pickingUpMiddle.setResetSensorsPost();
    allStates.pickingUpLeft.setResetSensorsPost();
    allStates.pickingUpRight.setResetSensorsPost();

    currentStatePointer = &allStates.scanningLeft;
    stateChange(currentStatePointer, C_FORCED);
    // Brain.stateChange(&allStates.pickingUpMiddle, C_FORCED);
}

void State::Initialise(String name, OnStateTransision onStateTransision, TransisionCallback transisionCallback, int numberOfSuccesses) {
    this->name = name;
    this->onStateTransision = onStateTransision;
    this->transisionCallback = transisionCallback;
    this->numberOfSuccesses = numberOfSuccesses;
    this->initialised = true;
}

void State::setResetSensorsPost() {
    this->resetSensorsPost = true;
}

void State::setAutomaticTransision() {
    automaticTransision = true;
}

void State::addWatchdog(State *timeoutStatePointer, unsigned long timeout) {
    this->timeoutStatePointer = timeoutStatePointer;
    this->timeout = timeout;
    this->hasWatchdog = true;
}



/****************************************************************************
 *   ____            _       
 *  |  _ \          (_)      
 *  | |_) |_ __ __ _ _ _ __  
 *  |  _ <| '__/ _` | | '_ \ 
 *  | |_) | | | (_| | | | | |
 *  |____/|_|  \__,_|_|_| |_|
 *
 ***************************************************************************/
BrainClass Brain;

void BrainClass::tick(bool silent) {
    sensorsTick(false, silent);
    stateTick();
}

void BrainClass::sensorsTick(bool clearBuffers, bool silent) {
    if (clearBuffers) {
        for (int i = 0; i < BUFFER_SIZE; i ++) {
            sensorsTick(false, silent); // Loop BUFFER_SIZE times
        }
        return;
    }
    
    delay(10);

    topLeftReading = irTopLeftLong.getReading();
    topRightReading = irTopRightLong.getReading();
    bottomLeftReading = irBottomLeftMedium.getReading();
    bottomRightReading = irBottomRightMedium.getReading();
    middleProximityReading = irProximity.getReading();

    if (!silent) {
        logSensors();
    }
}

void BrainClass::logSensors() {
    // Serial.print("sensors=");
    static bool first = true;
    if (first) {
        Serial.print("TopLeft,TopRight,BottomLeft,BottomRight,Prox");
        Serial.println();
        first = false;
    }
    Serial.print(topLeftReading.distanceMM);
    Serial.print(",");
    Serial.print(topRightReading.distanceMM);
    Serial.print(",");
    Serial.print(bottomLeftReading.distanceMM);
    Serial.print(",");
    Serial.print(bottomRightReading.distanceMM);
    Serial.print(",");
    Serial.print(middleProximityReading.foundObject ? 1000 : 500);
    Serial.println();
}

void BrainClass::stateTick() {
    if (currentStatePointer->hasWatchdog) {
        if (millis() - stateChangedTimestamp >= currentStatePointer->timeout) {
            State *timeoutStatePointer = currentStatePointer->timeoutStatePointer;
            return stateChange(timeoutStatePointer, C_WATCHDOG);
        }
    };

    if (currentStatePointer->automaticTransision) {
        return; // Looks like the state will automatically transision to the next state
    }

    for (int i = 0; i < sizeof(allStates) / sizeof(State); i ++) {
        State *ithStatePointer = ((State *)(&allStates)) + i;
        if (ithStatePointer != currentStatePointer && ithStatePointer->transisionCallback != NULL) {
            if (ithStatePointer->transisionCallback()) {
                return stateChange(ithStatePointer, C_CONDITIONAL);
            }
        }
    }
}

void BrainClass::stateChange(State *nextStatePointer, Cause cause) {
    if (IGNORE_WEIGHTS) {
        if (
            nextStatePointer ==  &allStates.towardsOnLeft ||
            nextStatePointer == &allStates.towardsOnRight ||
            nextStatePointer == &allStates.pickingUpLeft ||
            nextStatePointer == &allStates.pickingUpMiddle ||
            nextStatePointer == &allStates.pickingUpRight
        ) {
            return; // If we are ignoring weights then don't go into those states
        }
    }

    logSensors();
    if (nextStatePointer->initialised == false) {
        Serial.print("Error next state is not initialised");
        Serial.println();

        // Just do something noticeable
        while (true) {
            Arm.clawOpen();
            delay(2000);
            Arm.clawClose();
            delay(2000);
        }
    }

    Serial.print("state-");
    switch (cause)
    {
    case C_CONDITIONAL:
        Serial.print("conditional=");
        break;
    
    case C_RESET_FROM_PREVIOUS:
        Serial.print("reset=");
        break;
    
    case C_FORCED:
        Serial.print("forced=");
        break;
    
    case C_WATCHDOG:
        Serial.print("watchdog=");
        break;
    
    default:
        Serial.print("unknown=");
        break;
    }
    Serial.print(nextStatePointer->name);
    Serial.println();

    if (currentStatePointer->resetSensorsPost) {
        sensorsTick(true, false);
    }

    lastStatePointer = currentStatePointer;
    currentStatePointer = nextStatePointer;
    stateChangedTimestamp = millis();

    if (nextStatePointer->onStateTransision != NULL) {
        nextStatePointer->onStateTransision();
    }
}
