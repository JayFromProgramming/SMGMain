//
// Created by Jay on 5/30/2022.
//

#ifndef SMGMAIN_EVENTTIMER_H
#define SMGMAIN_EVENTTIMER_H

#if ARDUINO >= 100

#include <climits>
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/**
 * @brief The eventTimer class is used to track when an event should be fired.
 * @details To use set the class value to the time from now in milliseconds when the event should be fired.
 *         The class will return true once if the event should be fired.
 */
class eventTimer {
private:
    uint32_t _time;
    elapsedMillis timer;
    bool event_active;
    bool always_active;
    bool _repeat;
public:

    explicit eventTimer(uint32_t time) {
        timer = 0;
        _time = time;
        event_active = true; // Event has already happened and also never happened
        always_active = false;
        _repeat = false;
    }

    eventTimer() {
        timer = 0;
        _time = 0;
        event_active = true; // Event has already happened and also never happened
        always_active = false;
        _repeat = false;
    }

    void repeat() {
        always_active = true;
        event_active = false;
        _repeat = true;
    }

    /**
     * @brief Sets the time in milliseconds when the event should be fired.
     * @param time - The time in milliseconds from now when the event should be fired.
     */
    void set(uint32_t time) {
        _time = time;
        timer = 0;
        event_active = false;
    }

    /**
     * @brief Sets the time in milliseconds when the event should be fired.
     * @param time - The time in milliseconds from now when the event should be fired.
     */
    void set(const elapsedMillis& time) {
        _time = time;
        timer = 0;
        event_active = false;
    }

   /**
    * @brief Checks if the event should be fired, and returns true if it should
    * @return true if the event should be fired, false if it should not
    */
    bool check() {
        if (event_active) return false;
        if (timer >= _time) {
            if (!always_active || !_repeat) event_active = true;
            if (_repeat) timer = 0;
            return true;
        }
        return false;
    }

    /**
     *
     */
     void stop() {
        event_active = true;
     }
};

#endif //SMGMAIN_EVENTTIMER_H
