// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>

#include "audio/audio_interface.h"
#include "InternalTemperature.h"
//#include "mt2Library/tag_communicator.h"
#include <lcdDisplay/lcdDriver.h>
//#include <radio/radioInterface.h>

#define DEVICE_ID 0x01

extern "C" uint32_t set_arm_clock(uint32_t frequency); // required prototype

display::lcdDriver hud = display::lcdDriver();
//wireless::radioInterface radio = wireless::radioInterface();
audio_interface::audio_interface audio = audio_interface::audio_interface();
uint16_t next_loop_time = 0;

event_handlers* tagger_events = nullptr;

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
//    pinMode(LED_BUILTIN, OUTPUT);
//    digitalWrite(LED_BUILTIN, HIGH);
//    InternalTemperatureClass::attachHighTempInterruptCelsius(80, &overheat_method);
//    audio.init();
//    radio.init(DEVICE_ID);
    tagger_init(&audio); // Initialize the tagger
    display::lcdDriver::displayInit(); // Initialize the LCD display
    hud.pass_data_ptr(get_tagger_data_ptr()); // pass the tagger data pointer to the lcd driver
    tagger_events = get_event_handler_ptr() ; // get the tagger event pointer
//    digitalWrite(LED_BUILTIN, LOW);
    hud.clear();
}

int split(const String& command, String pString[4], int i, char delimiter, int max_length) {
    int pos = command.indexOf(delimiter, 0);
    if (pos == -1) {
        pString[i] = command;
        return i + 1;
    }
    if (i > max_length) {
        return i;
    }
    pString[i] = command.substring(0, pos);
    return split(command.substring(pos + 1, command.length()), pString, i + 1, delimiter, max_length);
}



// Main loop this runs once every 20ms or 50Hz while the tagger and hud updates run audio interrupts are disabled
void loop() {
    next_loop_time = micros() + (20 * 1000);
//    digitalWriteFast(LED_BUILTIN, HIGH);
    status_LED = true;
    tagger_loop(); // Run all main tagger functions
//    hud.clear();
    hud.update_hud(); // Update the HUD

    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        // Split the command string into an array of strings delimited by spaces
        String command_array[4];
        int command_array_length = split(command, command_array, 0, ' ', 4);

        if (command_array[0] == "hit") {

            tagger_events->on_hit(0, 1, 1);
            Serial.printf("hit! Remaining health: %d\n", get_tagger_data_ptr()->health);
        } else if (command_array[0] == "shoot") {
            Serial.println("SHOOT");

        } else if (command_array[0] == "respawn") {
            Serial.println("RESPAWN");
            tagger_events->on_respawn();
        }
    }

    while (micros() < next_loop_time) {
        if (status_LED) {
//            digitalWriteFast(LED_BUILTIN, LOW); // Set LED off while waiting for next loop
            status_LED = false;
        }
    }

}