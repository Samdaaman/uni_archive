#ifndef WATCHDOG_H
#define WATCHDOG_H

class Watchdog {
    public:
        Watchdog(unsigned long timeout, void (*callback)());
        void update();
        void reset();
        static void enableAll();
    private:
        unsigned long timeout;
        void (*callback)();
        unsigned long lastReset;
};

#endif