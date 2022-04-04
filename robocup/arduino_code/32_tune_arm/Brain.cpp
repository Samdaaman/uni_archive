#include "Brain.h"
#include "Sensor.h"
#include "Vision.h"
#include "Movement.h"
#include "Arm.h"
#include "Switch.h"
#include "Accelerometer.h"

static unsigned long temporaryScanTime = 0;

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
    State closeOnLeft;
    State closeOnRight;
    State avoidingFront;
    State avoidingLeft;
    State avoidingRight;
    State avoidingOnLean;
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
static bool foundWallCloseOnSide(Side side) {
    // If there is a low object on both sides within 20cm difference between sensors
    // static int bufferLeft = 0;
    // static int bufferRight = 0;
    // int* bufferP = side == LEFT ? &bufferLeft : &bufferRight;
    
    // if (Vision.found(TALL, side, 200 /* TODO: Tune wall max distance */)) {
    //     if (*bufferP > 3) {
    //         *bufferP = 0;
    //         return true;
    //     } else {
    //         *bufferP ++;
    //     }
    // } else {
    //     *bufferP = 0;
    // }
    // return false;
    return Vision.found(TALL, side, 210 /* TODO: Tune wall max distance (adjusted from 200) */);
}

static bool foundWallCloseBothSides() {
    return foundWallCloseOnSide(LEFT) && foundWallCloseOnSide(RIGHT);
}

static bool foundRampClose() {
    return Vision.found(SHORT, LEFT, 100) && Vision.found(SHORT, RIGHT, 100);
}

static bool foundWallCloseEitherSide() {
    return foundWallCloseOnSide(LEFT) || foundWallCloseOnSide(RIGHT);
}

/* Returns the distance to a weight if there is one and negative (a status code type thing) if there is no weight */
static int weightDistanceOnSide(Side side) {
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
    if (Vision.found(SHORT, side, 500 /* TODO: Tune max distance */)) {
        int distanceToWeight = getGenericReadings(SHORT, side).sameHeightSameSide->distanceMM;
        // Check that it isn't also a wall (within a certain threshold)
        if (!Vision.found(TALL, side, distanceToWeight, 100 /* TODO: Tune distance threshold between top and bottom */)) {
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


#define closeDistance 0

static bool foundWeightFarOnSide(Side side) {
    return closeDistance /* TODO: Tune weight close distance */ < weightDistanceOnSide(side);
}

static bool foundWeightCloseOnSide(Side side) {
    int weightDistance = weightDistanceOnSide(side);
    return 0 < weightDistance && weightDistance <= closeDistance; /* TODO: Tune weight close distance */
}

static bool foundWeightCloseMiddle() {
    if (middleProximityReading.foundObject) {
        const int thresholdBottom = 300;
        bool foundWideObject = bottomLeftReading.foundObjectCloser(thresholdBottom) || bottomRightReading.foundObjectCloser(thresholdBottom);
        const int thresholdWall = 200;
        bool foundWall = Vision.found(TALL, LEFT, thresholdWall) || Vision.found(TALL, RIGHT, thresholdWall);
        return !foundWideObject && !foundWall;
    } else {
        return false;
    }
}

static bool checkTowardsOnLeft() {
    return foundWeightFarOnSide(LEFT);
}

static bool checkTowardsOnRight() {
    return foundWeightFarOnSide(RIGHT);
}

static bool checkPickingUpMiddle() {
    return foundWeightCloseMiddle();
}

static bool checkCloseOnLeft() {
    return foundWeightCloseOnSide(LEFT);
}

static bool checkCloseOnRight() {
    return foundWeightCloseOnSide(RIGHT);
}

static bool checkAvoidingFront() {
    // return false;
    return foundWallCloseBothSides() || foundRampClose();
}

static bool checkAvoidingLeft() {
    Serial.println("Wall left");
    return foundWallCloseOnSide(LEFT);
}

static bool checkAvoidingRight() {
    Serial.println("Wall right");
    return foundWallCloseOnSide(RIGHT);
}

static bool isOnLean() {
    return (Accelerometer.getLevelStatus() == L_ON_LEAN);
}

static bool checkOnLean() {
    static uint16_t buffer = 0;
    static unsigned firstTimestamp = 0;
    static unsigned long lastTriggered = 0;

    if (buffer < 50) {
        buffer ++;
    } else {
        buffer = 0;
        
        if (isOnLean()) {
            if (firstTimestamp != 0) {
                if (millis() - firstTimestamp > 500) {
                    firstTimestamp = 0;
                    if (lastTriggered == 0 || lastTriggered < millis() - 10000) {
                        lastTriggered = millis();
                    }
                    return true;
                }
            } else {
                firstTimestamp = millis();
            }
        } else {
            firstTimestamp = 0;
        }
    }
    return false;
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
static void actionNothing() {}

static void actionForwardsMindlessly() {
    Movement::together(FAST);
}

static void actionGenericScanningOnSide(Side side) {
    Movement::spin(MEDIUM, side);
}

static void actionScanningRandom() {
    Brain.stateChange(randomSide() == LEFT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
}

static void actionScanningLeft() {
    actionGenericScanningOnSide(LEFT);
}

static void actionScanningRight() {
    actionGenericScanningOnSide(RIGHT);
}

static void routineTowardsSide(Side side) {
    Movement::largeTurn(FAST, FORWARD, side);
}

static void routineCloseOnSide(Side side) {
    Movement::largeTurn(MEDIUM, FORWARD, side);
}

static void actionTowardsOnLeft() {
    routineTowardsSide(LEFT);
}

static void actionTowardsOnRight() {
    routineTowardsSide(RIGHT);
}

static void actionCloseOnLeft() {
    routineCloseOnSide(LEFT);
}

static void actionCloseOnRight() {
    routineCloseOnSide(RIGHT);
}

static bool checkIfIrProxHasReading(bool targetFoundValue) {
    for (int i = 0; i < 10; i++) {
        middleProximityReading = irProximity.getReading();
        if (middleProximityReading.foundObject != targetFoundValue) {
            return false;
        }
    }
    return true;
}

static void routinePickingUpOnSide(Side side) {
    const unsigned long timeoutJiggle = 500;

    Movement::spin(MEDIUM, (Side)-side);
    unsigned long start = millis();
    while (!checkIfIrProxHasReading(false)) {
        if (millis() > start + timeoutJiggle) {
            Brain.stateChange(side == LEFT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
            return;
        }
    }
    
    Movement::spin(MEDIUM, side);
    start = millis();
    while (!checkIfIrProxHasReading(true)) {
        if (millis() > start + timeoutJiggle) {
            Brain.stateChange(side == RIGHT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
            return;
        }
    }
    while (!checkIfIrProxHasReading(false)) {
        if (millis() > start + timeoutJiggle) {
            Brain.stateChange(side == RIGHT ? &allStates.scanningLeft : &allStates.scanningRight, C_RESET_FROM_PREVIOUS);
            return;
        }
    }
    
    const long nowPlusHalfTime = millis() + (millis() - start) / 2;
    Movement::spin(MEDIUM, (Side)-side);
    while (millis() <= nowPlusHalfTime) {}

    Movement::together(MEDIUM, REVERSE);
    int buffer = 0;
    const unsigned long timeoutBackup = 3000;
    start = millis();
    while (millis() < start + timeoutBackup) {
        while (buffer < 10 && millis() < start + timeoutBackup) {
            middleProximityReading = irProximity.getReading();
            if (!middleProximityReading.foundObject) {
                buffer ++;
            } else {
                buffer = 0;
            }
            delay(1);
        }

        long start2 = millis(); // for the 300ms delay below
        const unsigned long backExtraTime = 300;
        bool foundDuringBackup = false;
        while (millis() - backExtraTime < start2) {
            delay(1);
            if (irProximity.getReading().foundObject) {
                foundDuringBackup = true;
                break;
            }
        }
        if (!foundDuringBackup) {
            break; // completed naturally
        }
        // if we get here then we need to backup some more
    }
    Movement::stop();

    // TODO remove this section --------
    // waitForSwitch();
    // Brain.stateChange(&allStates.scanningLeft, C_FORCED);
    // return;
    // ---------------------------------
    
    Arm.elbowPutDown();
    Movement::together(SLOW, FORWARD);
    const int forwardTime = 1300;
    delay(forwardTime);
    Movement::stop();
    Arm.clawClose();

    // TODO make this section based off the top IR readings
    const int backwardTime = 600;
    Movement::together(FAST, REVERSE);
    delay(backwardTime);
    Movement::stop();

    Arm.elbowPickup();
    Arm.clawOpen();
    Brain.stateChange(&allStates.scanningRandom, C_RESET_FROM_PREVIOUS);
}

static void actionPickingUpMiddle() {
    routinePickingUpOnSide(Brain.getCurrentStateP() == &allStates.towardsOnLeft ? LEFT : RIGHT);
}

static void routineAvoidOnSide(Side side) {
    static unsigned long lastCalled = 0;
    static Side lastSide = LEFT;

    if (lastCalled != 0 && lastCalled + 4000 < millis()) {
        side = lastSide;
    } else {
        lastSide = side;
    }
    lastCalled = millis();

    Movement::together(FAST, REVERSE);
    delay(300);
    Movement::spin(FAST, (Side)-side);
    Brain.sensorsTick();
    const int threshold = 300;
    while (bottomLeftReading.foundObjectCloser(threshold) || bottomRightReading.foundObjectCloser(threshold) || topLeftReading.foundObjectCloser(threshold) || topRightReading.foundObjectCloser(threshold)) {
        Brain.sensorsTick();
    }
    // delay(100); // Spin a bit further than it needs to spin
    // delay(lastCalled % 500);
    if (lastCalled % 4 > 0) {
        Brain.stateChange(side == LEFT ? &allStates.scanningRight : &allStates.scanningLeft, C_RESET_FROM_PREVIOUS);
        temporaryScanTime = 300 + (millis() % 1000);
    } else {
        Brain.stateChange(&allStates.forwardsMindlessly, C_RESET_FROM_PREVIOUS);
    }
    // Brain.stateChange(&allStates.forwardsMindlessly, C_RESET_FROM_PREVIOUS);
}

static void actionAvoidFront() {
    routineAvoidOnSide(randomSide());
}

static void actionAvoidLeft() {
    routineAvoidOnSide(LEFT);
}

static void actionAvoidRight() {
    routineAvoidOnSide(RIGHT);
}

static void actionAvoidOnLean() {
    Movement::together(FAST, REVERSE);
    // delay(1000);

    const int timeout = 5000;
    unsigned long start = millis();
    int buffer = 0;
    while (buffer < 10 && start + timeout < millis()) {
        if (isOnLean) {
            buffer = 0;
        } else {
            buffer ++;
        }
        delay(25);
    }
    delay(1000);

    Brain.sensorsTick(true);
    Brain.stateChange(&allStates.avoidingLeft, C_RESET_FROM_PREVIOUS);
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
    allStates.closeOnLeft.Initialise("Close on left", actionCloseOnLeft, checkCloseOnLeft);
    allStates.closeOnRight.Initialise("Close on right", actionCloseOnRight, checkCloseOnRight); 
    allStates.avoidingFront.Initialise("Avoiding front", actionAvoidFront, checkAvoidingFront);
    allStates.avoidingLeft.Initialise("Avoiding left", actionAvoidLeft, checkAvoidingLeft);
    allStates.avoidingRight.Initialise("Avoiding right", actionAvoidRight, checkAvoidingRight);
    allStates.avoidingOnLean.Initialise("Avoiding on lean", actionAvoidOnLean, checkOnLean);
    
    allStates.forwardAfterASec.addWatchdog(&allStates.forwardsMindlessly, 1000);
    allStates.forwardsMindlessly.addWatchdog(&allStates.scanningRandom, 4000);
    allStates.scanningLeft.addWatchdog(&allStates.forwardsMindlessly, 4000);
    allStates.scanningRight.addWatchdog(&allStates.forwardsMindlessly, 4000);
    // allStates.scanningRight.addWatchdog(&allStates.forwardsMindlessly, 5000);
    allStates.towardsOnLeft.addWatchdog(&allStates.forwardsMindlessly, 1000);
    allStates.towardsOnRight.addWatchdog(&allStates.forwardsMindlessly, 1000);

    allStates.avoidingFront.setResetSensorsPost();
    allStates.avoidingLeft.setResetSensorsPost();
    allStates.avoidingRight.setResetSensorsPost();
    allStates.pickingUpMiddle.setResetSensorsPost();
    allStates.avoidingOnLean.setResetSensorsPost();

    currentStatePointer = &allStates.forwardsMindlessly;
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
    
    delay(1);

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
        } else if (currentStatePointer == &allStates.scanningLeft || currentStatePointer == &allStates.scanningRight) {
            if (temporaryScanTime != 0 && millis() - stateChangedTimestamp >= temporaryScanTime) {
                temporaryScanTime = 0;
                stateChange(&allStates.forwardsMindlessly, C_WATCHDOG);
            }
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
            nextStatePointer == &allStates.closeOnLeft ||
            nextStatePointer == &allStates.closeOnRight ||
            nextStatePointer == &allStates.pickingUpMiddle            
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
