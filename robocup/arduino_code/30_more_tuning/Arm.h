#ifndef ARM_H
#define ARM_H

class ArmClass {
    public:
        void initialise();
        void clawOpen();
        void clawClose();
        void elbowPickup();
        void elbowPutDown();
        void backToStarting();
    private:
        void elbowMoveToMicroseconds(int nextMicroseconds);
        void clawOpenNoDelay();
        void clawDelayAndStopOnError(long milliseconds);
};

extern ArmClass Arm;

#endif