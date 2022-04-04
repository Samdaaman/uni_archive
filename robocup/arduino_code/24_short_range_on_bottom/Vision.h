#ifndef VISION_H
#define VISION_H

#include "Sensor.h"
#include "Movement.h"

enum Height {SHORT, TALL};

typedef struct GenericReadings_t
{
    Reading *sameHeightSameSide;
    Reading *sameHeightOppositeSide;
    Reading *oppositeHeightSameSide;
    Reading *oppositeHeightOppositeSide;
} GenericReadings;

GenericReadings getGenericReadings(Height height, Side side);

class VisionClass {
    public:
        // Whether an object is found on a side at a height at a target distance and within a certain threshold
        bool found(Height height, Side side, int targetDistance, int threshold) {return found(height, side, targetDistance, threshold, getGenericReadings(height, side));}
        
        // Whether an object is found on a side at a height within a certain distance
        bool found(Height height, Side side, int cuttOffDistance) {return found(height, side, cuttOffDistance, getGenericReadings(height, side));}
        
        // Whether an object is found on a side at a height
        bool found(Height height, Side side) {return found(height, side, getGenericReadings(height, side));}

        // Whether an object is found on both sides
        bool foundOnBothSides(Height height);
        
        // Whether an object is found on both sides and the difference between the distances is within a threshold
        bool foundOnBothSides(Height height, int threshold);

        // Gets the closest side for an object
        Side getClosestSide(Height height);

    private:
        bool found(Height height, Side side, int targetDistance, int threshold, GenericReadings genericReadings);
        bool found(Height height, Side side, int cuttOffDistance, GenericReadings genericReadings);
        bool found(Height height, Side side, GenericReadings genericReadings);
};

extern VisionClass Vision;

#endif