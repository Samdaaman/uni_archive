#ifndef WATCHDOG_H
#define WATCHDOG_H

class Watchdog {
    public:
        Watchdog(unsigned long timeout, void (*callback)());
        static void allUpdate(Watchdog *watchdogs);
        void update();
        void reset();
    private:
        unsigned long timeout;
        void (*callback)();
        unsigned long lastUpdated;
};

#endif