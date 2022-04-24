// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>
#include <Wire.h>
#include "audio_interface.h"
#include <lcdDisplay/lcdDriver.h>

display::lcdDriver hud = display::lcdDriver();

#include <Audio.h>

// GUItool: begin automatically generated code
AudioPlayMemory          hitPlayer;       //xy=195,136
AudioPlaySdWav           playSdWav1;     //xy=197,98.00000190734863
AudioPlaySdRaw           shotPlayer;     //xy=202.00000381469727,55.000000953674316
AudioMixer4              mixer1;         //xy=482.00000762939453,98.00000190734863
AudioOutputI2S           board_out;           //xy=682.0000076293945,90.00000762939453
AudioConnection          patchCord1(hitPlayer, 0, mixer1, 3);
AudioConnection          patchCord2(playSdWav1, 0, mixer1, 1);
AudioConnection          patchCord3(playSdWav1, 1, mixer1, 2);
AudioConnection          patchCord4(shotPlayer, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, 0, board_out, 1);
AudioConnection          patchCord6(mixer1, 0, board_out, 0);
// GUItool: end automatically generated code

uint16_t next_loop_time = 0;


void setup() {
    sounds::audio_interface::init(); // Initialize the audio interface
    tagger_init(); // Initialize the tagger
    display::lcdDriver::displayInit(); // Initialize the LCD display
    hud.pass_data_ptr(get_tagger_data_ptr()); // pass the tagger data pointer to the lcd driver
}


// Main loop this runs once every 20ms or 50Hz while the tagger and hud updates run audio interrupts are disabled
void loop() {
    next_loop_time = millis() + 20;
    AudioNoInterrupts(); // Disable audio interrupts while running main loop
    tagger_loop(); // Run all main tagger functions
    hud.update_hud(); // Update the HUD
    AudioInterrupts();
    while (millis() < next_loop_time) {
        // Wait for the next loop
    }
}