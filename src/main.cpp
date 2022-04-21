// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>
#include <Wire.h>
#include "audio_interface.h"
#include <lcdDisplay/lcdDriver.h>

display::lcdDriver hud = display::lcdDriver();

void setup() {
    sounds::audio_interface::init(); // Initialize the audio interface
    tagger_init(); // Initialize the tagger
    display::lcdDriver::displayInit(); // Initialize the LCD display
    hud.pass_data_ptr(get_tagger_data_ptr()); // pass the tagger data pointer to the lcd driver
}

void loop() {

    tagger_loop(); // Run all main tagger functions
    hud.update_hud(); // Update the HUD
}