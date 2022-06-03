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
    unsigned long _time;
    elapsedMillis timer;
    bool event_active;
    bool always_active;
public:

    explicit eventTimer(unsigned long time) {
        timer = 0;
        _time = 0;
        event_active = false;
        always_active = false;
    }

    eventTimer() {
        timer = 0;
        _time = 0;
        event_active = true; // Event has already happened and also never happened
        always_active = false;
    }

    void repeat() {
        always_active = true;
        event_active = false;
    }

    /**
     * @brief Sets the event time to when the event should occur how many milliseconds from now.
     * @param time The time in milliseconds from now when the event should occur.
     * @return A reference to this eventTimer object.
     */
    eventTimer & operator = (unsigned long time) {
        _time = time;
        timer = 0;
        event_active = false;
        return *this;
    }
    /**
     * @brief Sets this event timer to the value of another event timer.
     * @param time A pointer to another eventTimer object.
     * @return A reference to this eventTimer object.
     */
    eventTimer & operator = (const eventTimer & time) {
        _time = time._time;
        timer = time.timer;
        event_active = time.event_active;
        return *this;
    }
    /**
     * @brief Adds more time to the event time.
     * @param time - The time to add to the event time. (in milliseconds)
     * @return A reference to this eventTimer object.
     */
    eventTimer & operator += (unsigned long time) {
        _time += time;
        return *this;
    }

    /**
     * @brief Subtracts time from the event time.
     * @param time - The time to subtract from the event time. (in milliseconds)
     * @return A reference to this eventTimer object.
     */
    eventTimer & operator -= (unsigned long time) {
        _time -= time;
        return *this;
    }

    /**
     * @brief Checks if the remaining time is less than the given time.
     * @return True if the event time has passed, false otherwise.
     */
    bool operator < (unsigned long time) const {
        return (_time - timer) < time;
    }

    /**
     * @brief Checks if the remaining time is greater than the given time.
     * @return True if the event time has not passed, false otherwise.
     */
    bool operator > (unsigned long time) const {
        return (_time - timer) > time;
    }

    /**
     * @brief Checks if the remaining time is greater than or equal to the given time.
     * @return True if the event time has not passed, false otherwise.
     */
    bool operator >= (unsigned long time) const {
        return (_time - timer) >= time;
    }

    /**
     * @brief Checks if the remaining time is less than or equal to the given time.
     * @return True if the event time has passed, false otherwise.
     */
    bool operator <= (unsigned long time) const {
        return (_time - timer) <= time;
    }

    /**
     * @brief Checks if the remaining time is equal to the given time.
     * @param time - The time to compare to the event time. (in milliseconds)
     * @return True if the remaining time is equal to the given time, false otherwise.
     */
    bool operator == (unsigned long time) const {
        return (_time - timer) == time;
    }

    /**
     * @brief Checks if the remaining time is not equal to the given time.
     * @param time - The time to compare to the event time.
     * @return True if the event time does not equal the given time, false otherwise.
     */
    bool operator != (unsigned long time) const {
        return (_time - timer) != time;
    }

    /**
     * @brief Returns the remaining time in milliseconds.
     * @note This function returns zero if the event time has passed.
     * @return The remaining time in milliseconds. (zero if the event time has passed)
     */
    explicit operator unsigned long() const {
        return _time - timer;
    }

    /**
     * @brief Returns the remaining time in milliseconds.
     * @note This function returns a negative value if the event time has passed.
     * @return The remaining time in milliseconds.
     */
    explicit operator long() const{
        return _time - timer;
    }

    /**
     * @brief Checks if the event time has passed.
     * @note If this is a one time event, it will be set to false after it has passed and been checked.
     * changing the event time will reset the event to active.
     * @return True if the event time has passed, false otherwise.
     */
    explicit operator bool() {
        if (always_active) {
            if (timer >= _time) {
                return true;
            }
        } else {
            if (event_active) {
                if (timer >= _time) {
                    event_active = true;
                    return true;
                } else return false;
            } else{
                return false;
            }
        }
    }

    /**
     * @brief Checks if the event time has not passed.
     * @return True if the event time has not passed, false otherwise.
     */
    bool operator ! () {
        if (always_active) {
            if (timer >= _time) {
                return false;
            }
        } else {
            if (event_active) {
                if (timer >= _time) {
                    event_active = true;
                    return false;
                } else return true;
            } else{
                return true;
            }
        }
    }

};

#endif //SMGMAIN_EVENTTIMER_H
