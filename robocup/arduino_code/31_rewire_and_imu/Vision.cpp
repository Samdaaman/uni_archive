#include "Vision.h"
#include "Brain.h"

GenericReadings getGenericReadings(Height height, Side side) {
    Reading *p_sameHeightLeftReading;
    Reading *p_sameHeightRightReading;
    Reading *p_oppositeHeightLeftReading;
    Reading *p_oppositeHeightRightReading;

    if (height == SHORT) {
        p_sameHeightLeftReading = &bottomLeftReading;
        p_sameHeightRightReading = &bottomRightReading;
        p_oppositeHeightLeftReading = &topLeftReading;
        p_oppositeHeightRightReading = &topRightReading;
    } else {
        p_sameHeightLeftReading = &topLeftReading;
        p_sameHeightRightReading = &topRightReading;
        p_oppositeHeightLeftReading = &bottomLeftReading;
        p_oppositeHeightRightReading = &bottomRightReading;
    }

    return {
        side == LEFT ? p_sameHeightLeftReading : p_sameHeightRightReading,
        side == LEFT ? p_sameHeightRightReading : p_sameHeightLeftReading,
        side == LEFT ? p_oppositeHeightLeftReading : p_oppositeHeightRightReading,
        side == LEFT ? p_oppositeHeightRightReading : p_oppositeHeightLeftReading
    };
}

Side VisionClass::getClosestSide(Height height) {
    GenericReadings gr = getGenericReadings(height, LEFT);

    if (gr.sameHeightSameSide->foundObject && gr.sameHeightOppositeSide->foundObject) {
        return gr.sameHeightSameSide->distanceMM < gr.sameHeightOppositeSide->distanceMM ? LEFT : RIGHT;
    } else if (gr.sameHeightSameSide->foundObject) {
        return LEFT;
    } else if (gr.sameHeightOppositeSide->foundObject) {
        return RIGHT;
    } else {
        return NEITHER;
    }
}

bool VisionClass::foundOnBothSides(Height height) {
    GenericReadings gr = getGenericReadings(height, NEITHER);

    if (gr.sameHeightSameSide->foundObject && gr.sameHeightOppositeSide->foundObject) {
        return true;
    }
    return false;
}

bool VisionClass::foundOnBothSides(Height height, int threshold) {
    GenericReadings gr = getGenericReadings(height, NEITHER);

    if (gr.sameHeightSameSide->foundObject && gr.sameHeightOppositeSide->foundObject) {
        if (abs(gr.sameHeightSameSide->distanceMM - gr.sameHeightOppositeSide->distanceMM) <= threshold) {
            return true;
        }
    }
    return false;
}

bool VisionClass::found(Height height, Side side, int targetDistance, int threshold, GenericReadings gr) {
    if (!gr.sameHeightSameSide->foundObject) {
        return false;
    }

    if (abs(gr.sameHeightSameSide->distanceMM - targetDistance) <= threshold) {
        return true;
    } else {
        return false;
    }
}

bool VisionClass::found(Height height, Side side, int cuttOffDistance, GenericReadings gr) {
    return gr.sameHeightSameSide->foundObjectCloser(cuttOffDistance);
}

bool VisionClass::found(Height height, Side side, GenericReadings gr) {
    return gr.sameHeightSameSide->foundObject;
}



VisionClass Vision;