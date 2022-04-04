#include "Brain.h"
#include "Sensor.h"
#include "Constants.h"
#include "Vision.h"
#include "Movement.h"
#include "Arm.h"

// TODO add debug flag
// TODO maybe change volatile to static methods?

Reading topLeftReading;
Reading topRightReading;
Reading bottomLeftReading;
Reading bottomRightReading;
Reading middleProximityReading;

typedef struct AllStates_t {
    State stopAndWaitAFewSec;
    State forwardsMindlessly;
    State scanningLeft;
    State scanningRight;
    State towardsOnLeft;
    State towardsOnRight;
    State pickingUpLeft;
    State pickingUpRight;
    State avoidingFront;
    State avoidingLeft;
    State avoidingRight;
} AllStates;

static AllStates allStates;

static void blackhole() {
    return;
    Movement::stop();
    while (1) {}
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
void actionStopAndWaitAFewSec() {
    Movement::stop();
    delay(1000);
    Movement::together(MEDIUM, REVERSE);
    delay(500);
    Movement::spin(MEDIUM, RIGHT);
    delay(500);
    Movement::stop();
    Brain.stateChange(&allStates.scanningLeft, C_FORCED);
}

void actionForwardsMindlessly() {
    Movement::together(SLOW);
}

void actionGenericScanningOnSide(Side side) {
    Movement::spin(SLOW, side);
}

void actionScanningLeft() {
    actionGenericScanningOnSide(LEFT);
}

void actionScanningRight() {
    actionGenericScanningOnSide(RIGHT);
}

void actionGenericTowardsSide(Side side) {
    Movement::slightTurn(MEDIUM, FORWARD, side);
}

void actionTowardsOnLeft() {
    actionGenericTowardsSide(LEFT);
}

void actionTowardsOnRight() {
    actionGenericTowardsSide(RIGHT);
}

void actionGenericPickingUpOnSide(Side side) {
    Movement::spin(SLOW, side);
    bottomLeftReading = irBottomLeftMedium.getReading();
    bottomRightReading = irBottomRightMedium.getReading();
    while ((side == LEFT ? bottomLeftReading.distanceMM : bottomRightReading.distanceMM) < 300) {
        bottomLeftReading = irBottomLeftMedium.getReading();
        bottomRightReading = irBottomRightMedium.getReading();
        delay(1);
    }
    unsigned long startTime = millis();
    while ((side == LEFT ? bottomRightReading.distanceMM : bottomLeftReading.distanceMM) < 300) {
        bottomLeftReading = irBottomLeftMedium.getReading();
        bottomRightReading = irBottomRightMedium.getReading();
        delay(1);
    }
    unsigned long endTime = millis();
    Movement::spin(SLOW, (Side)-side);
    while (millis() < (endTime - startTime) / 2 + endTime) {}
    Movement::stop();
    delay(1000);
    Movement::together(SLOW, REVERSE);
    delay(1500);
    Movement::stop();
    Arm.elbowPutDown();
    Movement::together(SLOW, FORWARD);
    delay(3000);
    Movement::stop();
    Arm.clawClose();
    Arm.elbowPickup();
    Arm.clawOpen();
    Brain.stateChange(&allStates.scanningLeft, C_RESET_FROM_PREVIOUS);
}

void actionPickingUpLeft() {
    actionGenericPickingUpOnSide(LEFT);
}

void actionPickingUpRight() {
    actionGenericPickingUpOnSide(RIGHT);
}

void actionAvoidFront() {
    // TODO: Make this better
    Movement::together(SLOW, REVERSE);
    delay(1000);
    Movement::spin(SLOW, LEFT);
    delay(1000);
    Movement::stop();
    Brain.stateChange(&allStates.scanningLeft, C_RESET_FROM_PREVIOUS);
}

void actionGenericAvoidOnSide(Side side) {
    // TODO: Make this better
    Movement::together(SLOW, REVERSE);
    delay(1000);
    Movement::spin(SLOW, (Side)-side);
    delay(1000);
    Movement::stop();
    Brain.stateChange(&allStates.scanningLeft, C_RESET_FROM_PREVIOUS);
}

void actionAvoidLeft() {
    actionGenericAvoidOnSide(LEFT);
}

void actionAvoidRight() {
    actionGenericAvoidOnSide(RIGHT);
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
bool foundWallCloseOnSide(bool shouldLog, Side side) {
    // If there is a low object on both sides within 20cm difference between sensors
    volatile bool found = Vision.found(TALL, side, 110 /* TODO: Tune wall max distance */);
    return found;
}

bool foundWallCloseBothSides(bool shouldLog) {
    volatile bool found = foundWallCloseOnSide(shouldLog, LEFT) && foundWallCloseOnSide(shouldLog, RIGHT);
    return found;
}

bool foundWallCloseEitherSide(bool shouldLog) {
    volatile bool found = foundWallCloseOnSide(shouldLog, LEFT) || foundWallCloseOnSide(shouldLog, RIGHT);
    return found;
}

/* Returns the distance to a weight if there is one and negative (a status code type thing) if there is no weight */
int weightDistanceOnSide(bool shouldLog, Side side) {
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
    if (foundWallCloseEitherSide(shouldLog)) {
        return -3;
    }

    // See if we found something on the bottom sensor
    if (Vision.found(SHORT, side, 700 /* TODO: Tune max distance */)) {
        int distanceToWeight = getGenericReadings(SHORT, side).sameHeightSameSide->distanceMM;
        // Check that it isn't also a wall (within a certain threshold)
        if (!Vision.found(TALL, side, distanceToWeight, 200 /* TODO: Tune distance threshold between top and bottom */)) {
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

bool foundWeightFarOnSide(bool shouldLog, Side side) {
    volatile bool found = 200 /* TODO: Tune weight close distance */ < weightDistanceOnSide(shouldLog, side);
    return found;
}

bool foundWeightCloseOnSide(bool shouldLog, Side side) {
    int weightDistance = weightDistanceOnSide(shouldLog, side);
    volatile bool found = 0 < weightDistance && weightDistance <= 200; /* TODO: Tune weight close distance */
    return found;
}

bool checkTowardsOnLeft(bool shouldLog) {
    volatile bool found = foundWeightFarOnSide(shouldLog, LEFT);
    return found;
}

bool checkTowardsOnRight(bool shouldLog) {
    volatile bool found = foundWeightFarOnSide(shouldLog, RIGHT);
    return found;
}

bool checkPickingUpLeft(bool shouldLog) {
    volatile bool found = foundWeightCloseOnSide(shouldLog, LEFT);
    return found;
}

bool checkPickingUpRight(bool shouldLog) {
    volatile bool found = foundWeightCloseOnSide(shouldLog, RIGHT);
    return found;
}

bool checkAvoidingFront(bool shouldLog) {
    volatile bool found = foundWallCloseBothSides(shouldLog);
    return found;
}

bool checkAvoidingLeft(bool shouldLog) {
    volatile bool found = foundWallCloseOnSide(shouldLog, LEFT);
    return found;
}

bool checkAvoidingRight(bool shouldLog) {
    volatile bool found = foundWallCloseOnSide(shouldLog, RIGHT);
    return found;
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
    allStates.stopAndWaitAFewSec.Initialise("Stopping and waiting", actionStopAndWaitAFewSec);
    allStates.forwardsMindlessly.Initialise("Forwards mindlessly", actionForwardsMindlessly);
    allStates.scanningLeft.Initialise("Scanning left", actionScanningLeft);
    allStates.scanningRight.Initialise("Scanning right", actionScanningRight);
    allStates.towardsOnLeft.Initialise("Towards on left", actionTowardsOnLeft, checkTowardsOnLeft);
    allStates.towardsOnRight.Initialise("Towards on right", actionTowardsOnRight, checkTowardsOnRight);
    allStates.pickingUpLeft.Initialise("Picking up on left", actionPickingUpLeft, checkPickingUpLeft);
    allStates.pickingUpRight.Initialise("Picking up on right", actionPickingUpRight, checkPickingUpRight); 
    allStates.avoidingFront.Initialise("Avoiding front", actionAvoidFront, checkAvoidingFront);
    allStates.avoidingLeft.Initialise("Avoiding left", actionAvoidLeft, checkAvoidingLeft);
    allStates.avoidingRight.Initialise("Avoiding right", actionAvoidRight, checkAvoidingRight);

    allStates.forwardsMindlessly.addWatchdog(&allStates.scanningLeft, 3000);
    allStates.scanningLeft.addWatchdog(&allStates.forwardsMindlessly, 5000);
    allStates.scanningRight.addWatchdog(&allStates.forwardsMindlessly, 5000);
    allStates.towardsOnLeft.addWatchdog(&allStates.scanningRight, 2500);
    allStates.towardsOnRight.addWatchdog(&allStates.scanningLeft, 2500);

    allStates.stopAndWaitAFewSec.setAutomaticTransision();
    allStates.pickingUpLeft.setAutomaticTransision();
    allStates.pickingUpRight.setAutomaticTransision();
    allStates.avoidingFront.setAutomaticTransision();
    allStates.avoidingLeft.setAutomaticTransision();
    allStates.avoidingRight.setAutomaticTransision();

    allStates.avoidingFront.setResetSensorsPost();
    allStates.avoidingLeft.setResetSensorsPost();
    allStates.avoidingRight.setResetSensorsPost();
    allStates.pickingUpLeft.setResetSensorsPost();
    allStates.pickingUpRight.setResetSensorsPost();

    currentStatePointer = &allStates.scanningLeft;
    stateChange(currentStatePointer, C_FORCED);
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

    topLeftReading = irTopLeftSmall.getReading();
    topRightReading = irTopRightSmall.getReading();
    bottomLeftReading = irBottomLeftMedium.getReading();
    bottomRightReading = irBottomRightMedium.getReading();
    middleProximityReading = irProximity.getReading();

    if (!silent) {
        logSensors();
    }
}

void BrainClass::logSensors() {
    Serial.print("sensors=");
    Serial.print(topLeftReading.distanceMM);
    Serial.print(",");
    Serial.print(topRightReading.distanceMM);
    Serial.print(",");
    Serial.print(bottomLeftReading.distanceMM);
    Serial.print(",");
    Serial.print(bottomRightReading.distanceMM);
    Serial.print(",");
    Serial.print(middleProximityReading.foundObject ? 1 : 0);
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
            if (ithStatePointer->transisionCallback(ithStatePointer->shouldLog)) {
                return stateChange(ithStatePointer, C_CONDITIONAL);
            }
        }
    }
}

void BrainClass::stateChange(State *nextStatePointer, Cause cause) {
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
