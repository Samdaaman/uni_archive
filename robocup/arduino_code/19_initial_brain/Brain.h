#ifndef BRAIN_H
#define BRAIN_H

#include "Sensor.h"
#include "Arduino.h"
#include "Vision.h"

typedef bool (*TransisionCallback)(bool shouldLog);
typedef void (*OnStateTransision)();

enum Cause {C_CONDITIONAL, C_RESET_FROM_PREVIOUS, C_FORCED, C_WATCHDOG};

extern Sensor irTopLeftSmall;
extern Sensor irTopRightSmall;
extern Sensor irBottomLeftMedium;
extern Sensor irBottomRightMedium;
extern Sensor irProximity;

extern Reading topLeftReading;
extern Reading topRightReading;
extern Reading bottomLeftReading;
extern Reading bottomRightReading;
extern Reading middleProximityReading;

class State {
    public:
        State() {};
        void Initialise(String name, OnStateTransision onStateTransision, TransisionCallback transisionCallback = NULL, int numberOfSuccesses = 1);
        void setResetSensorsPost();
        void setAutomaticTransision();
        void addWatchdog(State *timeoutStatePointer, unsigned long timeout);
        bool shouldLog = false;
        String name;
        OnStateTransision onStateTransision;
        TransisionCallback transisionCallback;
        int numberOfSuccesses;
        bool hasWatchdog = false;
        State *timeoutStatePointer;
        unsigned long timeout;
        bool resetSensorsPost = false;
        bool automaticTransision = false;
        bool initialised = false;
};

class BrainClass {
    public:
        void InitialiseStates();
        void tick(bool silent);
        State getCurrentState();
        void stateChange(State *nextStatePointer, Cause cause);
        void stateTick();
        void sensorsTick(bool clearBuffers = false, bool silent = false);
        void logSensors();
    private:
        State *currentStatePointer;
        State *lastStatePointer;
        unsigned long stateChangedTimestamp;
};


extern BrainClass Brain;

#endif