// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>
#include <Wire.h>
#include "audio_interface.h"
#include <lcdDisplay/lcdDriver.h>

display::lcdDriver hud = display::lcdDriver();

uint16_t next_loop_time = 0;

//#define DEBUG_MODE

void setup() {
    sounds::audio_interface::init(); // Initialize the audio interface
    tagger_init(); // Initialize the tagger
    display::lcdDriver::displayInit(); // Initialize the LCD display
    hud.pass_data_ptr(get_tagger_data_ptr()); // pass the tagger data pointer to the lcd driver
}


// Main loop this runs once every 20ms or 50Hz while the tagger and hud updates run audio interrupts are disabled
void loop() {
    next_loop_time = millis() + 20;
//    AudioNoInterrupts(); // Disable audio interrupts while running main loop
    tagger_loop(); // Run all main tagger functions
    hud.update_hud(); // Update the HUD
//    AudioInterrupts(); // Re-enable audio interrupts
    while (millis() < next_loop_time) {
        // Wait for the next loop
        // Interrupts will still run while waiting
        #ifdef DEBUG_MODE
        if (millis() > next_loop_time + 2) {
            Serial.printf("Loop took longer than 20ms %f\n", (millis() - next_loop_time) / 20.0);
        }
        #endif
    }
}