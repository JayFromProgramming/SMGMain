// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>
#include <Bounce.h>

#include "audio/audio_interface.h"
#include "InternalTemperature.h"
//#include "mt2Library/tag_communicator.h"
#include <lcdDisplay/lcdDriver.h>
//#include <radio/radioInterface.h>

#define TRIGGER_PIN_NUMBER 3
#define RELOAD_PIN_NUMBER 2
#define SELECT_PIN_NUMBER 6

#define DEVICE_ID 0x01

extern "C" uint32_t set_arm_clock(uint32_t frequency); // required prototype


IntervalTimer io_refresh_timer;

Bounce trigger_button = Bounce(TRIGGER_PIN_NUMBER, 5);
Bounce reload_button = Bounce(RELOAD_PIN_NUMBER, 5);
Bounce select_button = Bounce(SELECT_PIN_NUMBER, 5);

struct button_methods {
  void (*trigger_method)(bool state) = nullptr;
  void (*reload_method)()  = nullptr;
  void (*select_method)()  = nullptr;
};

button_methods io_actions;

void io_refresh(){ // Called every .25 ms
  trigger_button.update();
  reload_button.update();
  select_button.update();
  if (trigger_button.fallingEdge()){
      if (io_actions.trigger_method != nullptr){
        io_actions.trigger_method(true);
      }
  }
  if (trigger_button.risingEdge()){
      if (io_actions.trigger_method != nullptr){
        io_actions.trigger_method(false);
      }
  }
  if (reload_button.fallingEdge()){
      if (io_actions.reload_method != nullptr){
        io_actions.reload_method();
      }
  }
  if (select_button.fallingEdge()){
      if (io_actions.select_method != nullptr){
        io_actions.select_method();
      }
  }
}


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

    tagger_init(&audio); // Initialize the tagger

    // Initialize all debounced IO methods
    io_actions.trigger_method = &shot_check;
    io_actions.reload_method =  &on_reload;
    io_actions.select_method =  &display::lcdDriver::toggle_backlight;

    display::lcdDriver::displayInit(); // Initialize the LCD display
    hud.pass_data_ptr(get_tagger_data_ptr(), get_score_data_ptr()); // pass the tagger data pointer to the lcd driver
    tagger_events = get_event_handler_ptr() ; // get the tagger event pointer
//    digitalWrite(LED_BUILTIN, LOW);
    hud.clear();
    io_refresh_timer.begin(io_refresh, 250);
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

    tagger_loop(); // Run all main tagger functions
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
    }

}