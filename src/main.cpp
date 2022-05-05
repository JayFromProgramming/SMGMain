// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>

#include "audio/audio_interface.h"
#include "InternalTemperature.h"
#include <lcdDisplay/lcdDriver.h>
#include <radio/radioInterface.h>

#define DEVICE_ID 0x01

extern "C" uint32_t set_arm_clock(uint32_t frequency); // required prototype

display::lcdDriver hud = display::lcdDriver();
wireless::radioInterface radio = wireless::radioInterface();
audio_interface::audio_interface audio = audio_interface::audio_interface();
uint16_t next_loop_time = 0;

bool status_LED = false;

//void overheat_method() {
//    // In the event of the main cpu overheating we reduce the system clock to .75MHz and stop the main loop.
//    // We will also attach a low temp interrupt to the main cpu to restart the system clock once the cpu has cooled down.
//    set_arm_clock(750000);
//    hud.override_text((String *) "Main CPU Overheating\nShutting Down");
//
//}

//#define DEBUG_MODE

void setup() {
#ifdef DEBUG_MODE
    Serial.begin(9600);
    Serial.println("Starting...");
#endif
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
//    InternalTemperatureClass::attachHighTempInterruptCelsius(80, &overheat_method);
    audio.init();
    radio.init(DEVICE_ID);
    tagger_init(&audio); // Initialize the tagger
//    display::lcdDriver::displayInit(); // Initialize the LCD display
    hud.pass_data_ptr(get_tagger_data_ptr()); // pass the tagger data pointer to the lcd driver
    digitalWrite(LED_BUILTIN, LOW);
}

// Main loop this runs once every 20ms or 50Hz while the tagger and hud updates run audio interrupts are disabled
void loop() {
    next_loop_time = micros() + (20 * 1000);
    digitalWriteFast(LED_BUILTIN, HIGH);
    status_LED = true;
    tagger_loop(); // Run all main tagger functions
//    hud.update_hud(); // Update the HUD
    while (micros() < next_loop_time) {
        if (status_LED) {
            digitalWriteFast(LED_BUILTIN, LOW); // Set LED off while waiting for next loop
            status_LED = false;
        }
    }

}