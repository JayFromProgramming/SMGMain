// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>
#include <Bounce.h>

#include "audio/audio_interface.h"
#include "InternalTemperature.h"
#include "eeprom_handler.h"
//#include "mt2Library/tag_communicator.h"
#include <mt2Library/clone_configurer.h>
#include <lcdDisplay/lcdDriver.h>
//#include <radio/radioInterface.h>

#define TRIGGER_PIN_NUMBER 3
#define RELOAD_PIN_NUMBER 2
#define SELECT_PIN_NUMBER 20

#define BATTERY_PIN_NUMBER 5

#define DEVICE_ID 0x01

extern "C" uint32_t set_arm_clock(uint32_t frequency); // required prototype

void boot_mode_game();
void boot_mode_ref();
void boot_mode_set_defaults();
void boot_mode_sys_info();
void boot_mode_clone_config();
void boot_menu();


//IntervalTimer io_refresh_timer;

Bounce trigger_button = Bounce(TRIGGER_PIN_NUMBER, 5);
Bounce reload_button = Bounce(RELOAD_PIN_NUMBER, 5);
Bounce select_button = Bounce(SELECT_PIN_NUMBER, 5);

struct button_methods {
  void (*trigger_method)(bool state) = nullptr;
  void (*reload_method)()  = nullptr;
  void (*select_method)()  = nullptr;
};

button_methods io_actions;

void clear_io_actions() {
    io_actions.trigger_method = nullptr;
    io_actions.reload_method = nullptr;
    io_actions.select_method = nullptr;
}

enum boot_modes: uint8_t {
    BOOT_MODE_UNKNOWN = 0,
    BOOT_MODE_GAME = 1,
    BOOT_MODE_REF = 2,
    BOOT_MODE_CLONE_CONFIG = 3,
    BOOT_MODE_CLONE_GUN = 4,
    BOOT_MODE_GUN_CONFIG = 5,
    BOOT_MODE_SET_DEFAULTS = 6,
    BOOT_MODE_SYS_INFO = 7,
};

String boot_mode_names[] = {
    "Unknown",
    "Game",
    "Ref",
    "Clone Config",
    "Clone Gun",
    "Gun Config",
    "Set Defaults",
    "Sys Info"
};

volatile boot_modes boot_mode;

void io_refresh(){ // Called every .25 ms
  trigger_button.update();
  reload_button.update();
  select_button.update();
  if (trigger_button.fallingEdge()){
      if (io_actions.trigger_method != nullptr) {
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

void increment_menu(){
    hud.menu_increment();
}

void decrement_menu(){
    hud.menu_decrement();
}

void select_menu(bool select){
    hud.menu_select(select);
}

float read_battery_voltage(){
    float batVal = analogRead(BATTERY_PIN_NUMBER);
    float batVoltage = (batVal * 3.3f) / 1024;
    return batVoltage;
}

//void overheat_method() {
//    // In the event of the main cpu overheating we reduce the system clock to .75MHz and stop the main loop.
//    // We will also attach a low temp interrupt to the main cpu to restart the system clock once the cpu has cooled down.
//    set_arm_clock(750000);
//    hud.override_text((String *) "Main CPU Overheating\nShutting Down");
//
//}

//#define DEBUG_MODE

void boot_mode_game(){

    if (boot_mode != BOOT_MODE_GAME){
        set_boot_mode(BOOT_MODE_GAME);
        SCB_AIRCR = 0x05FA0004;
    }

    tagger_init(&audio); // Initialize the tagger

    // Initialize all debounced IO methods
    clear_io_actions();
    io_actions.trigger_method = &shot_check;
    io_actions.reload_method =  &on_reload;
    io_actions.select_method =  &display::lcdDriver::toggle_backlight;

    hud.pass_data_ptr(get_tagger_data_ptr(), get_score_data_ptr()); // pass the tagger data pointer to the lcd driver
    tagger_events = get_event_handler_ptr() ; // get the tagger event pointer
//    digitalWrite(LED_BUILTIN, LOW);
    hud.clear();
}

void transmit_clone(int clone_id){
//    load_preset(clone_id);
      hud.clear();
}

void boot_mode_clone(){

    if (boot_mode != BOOT_MODE_CLONE_GUN) {
        set_boot_mode(BOOT_MODE_CLONE_GUN);
        SCB_AIRCR = 0x05FA0004;
    }

    auto* clone_menu = display::lcdDriver::make_menu("Select\nClone Preset");
    int presets;
    clone** presets_ptr = load_presets(&presets);
    for (int i = 0; i < presets; i++){
        display::lcdDriver::add_menu_item(clone_menu, presets_ptr[i]->name, transmit_clone, i);
    }
    display::lcdDriver::add_menu_item(clone_menu, "Exit", &boot_menu);
    hud.load_and_display_menu(clone_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;

}

void boot_mode_ref(){
    // Build the ref option menu

    if (boot_mode != BOOT_MODE_REF) {
        set_boot_mode(BOOT_MODE_REF);
        SCB_AIRCR = 0x05FA0004;
    }

    auto* ref_menu = display::lcdDriver::make_menu("Ref Menu");
    display::lcdDriver::add_menu_item(ref_menu, "Admin Kill");
    display::lcdDriver::add_menu_item(ref_menu, "Respawn Player");
    display::lcdDriver::add_menu_item(ref_menu, "Reset Player");
    display::lcdDriver::add_menu_item(ref_menu, "Explode Player");

    display::lcdDriver::add_menu_item(ref_menu, "Enter Clone Mode",&boot_mode_clone);
    display::lcdDriver::add_menu_item(ref_menu, "Edit Presets", &boot_mode_clone_config);
    display::lcdDriver::add_menu_item(ref_menu, "Exit to boot", &boot_menu);
    hud.load_and_display_menu(ref_menu);

    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;

}

void launch_clone_config_menu(int clone_id){
    auto* clone = load_preset(clone_id);
    auto* menu = create_clone_config_menu(clone, clone_id, &boot_mode_clone_config);
    hud.load_and_display_menu(menu);

    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;
}

void boot_mode_clone_config(){

    if (boot_mode != BOOT_MODE_CLONE_CONFIG) {
        set_boot_mode(BOOT_MODE_CLONE_CONFIG);
        SCB_AIRCR = 0x05FA0004;
    }

    auto* clone_menu = display::lcdDriver::make_menu("Select Preset\nto Edit");
    int presets;
    clone** presets_ptr = load_presets(&presets);
    for (int i = 0; i < presets; i++){
        display::lcdDriver::add_menu_item(clone_menu, presets_ptr[i]->name, launch_clone_config_menu, i);
    }
    display::lcdDriver::add_menu_item(clone_menu, "Exit", &boot_menu);
    hud.load_and_display_menu(clone_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;

}

void boot_mode_options(){

}

void boot_mode_set_defaults(){

    if (boot_mode != BOOT_MODE_SET_DEFAULTS){
        set_temp_boot_mode(BOOT_MODE_SET_DEFAULTS);
        SCB_AIRCR = 0x05FA0004;
        return;
    }

    auto* confirmation_menu = display::lcdDriver::make_menu(
            "Are you sure\nyou want to set\nall values\nto default?");
    display::lcdDriver::add_menu_item(confirmation_menu, "Yes", &set_defaults);
    display::lcdDriver::add_menu_item(confirmation_menu, "No", &boot_mode_options);
    display::lcdDriver::add_menu_item(confirmation_menu, "Cancel", &boot_menu);
    hud.load_and_display_menu(confirmation_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;
}

void boot_menu(){

    if (boot_mode != BOOT_MODE_UNKNOWN){
        set_temp_boot_mode(BOOT_MODE_UNKNOWN);
        SCB_AIRCR = 0x05FA0004;
        return;
    }

    auto* boot_menu = display::lcdDriver::make_menu("Select\nBoot Mode");
    display::lcdDriver::add_menu_item(boot_menu, "Game",        &boot_mode_game);
    display::lcdDriver::add_menu_item(boot_menu, "Referee",     &boot_mode_ref);
    display::lcdDriver::add_menu_item(boot_menu, "Clone Mode",  &boot_mode_clone);
    display::lcdDriver::add_menu_item(boot_menu, "Configure Clone",&boot_mode_clone_config);
    display::lcdDriver::add_menu_item(boot_menu, "Gun Options", &boot_mode_options);
    display::lcdDriver::add_menu_item(boot_menu, "Set Defaults", &boot_mode_set_defaults);
    display::lcdDriver::add_menu_item(boot_menu, "Sys Info",    &boot_mode_sys_info);
    hud.load_and_display_menu(boot_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;
}

void boot_mode_sys_info() { // Display system information, does not override current boot mode

    if (boot_mode != BOOT_MODE_SYS_INFO){
        set_temp_boot_mode(BOOT_MODE_SYS_INFO); // Because this is a temp boot mode set the temp boot mode
        SCB_AIRCR = 0x05FA0004;
        return;
    }

    auto* sys_info_menu = display::lcdDriver::make_menu("System Info");
    display::lcdDriver::add_menu_item(sys_info_menu, "Gun OS Version:\n" __TIME__ " " __DATE__);
    char* format_string = new char[100];
    sprintf(format_string, "Device ID:\nTagger-%d", get_device_id());
    display::lcdDriver::add_menu_item(sys_info_menu, format_string);
    sprintf(format_string, "Boot Mode:\n%s", boot_mode_names[get_boot_mode()].c_str());
    display::lcdDriver::add_menu_item(sys_info_menu, format_string);
    sprintf(format_string, "System Temp:\n%.2fC", InternalTemperatureClass::readTemperatureC());
    display::lcdDriver::add_menu_item(sys_info_menu, format_string);
    sprintf(format_string, "Battery Voltage:\n%.2f Volts", read_battery_voltage());
    display::lcdDriver::add_menu_item(sys_info_menu, format_string);
    display::lcdDriver::add_menu_item(sys_info_menu, "Radio Status:\nNo Radio Module");
    sprintf(format_string, "Remaining EEPROM:\n%d bytes", get_remaining_space());
    display::lcdDriver::add_menu_item(sys_info_menu, format_string);
    display::lcdDriver::add_menu_item(sys_info_menu, "Copyright(c) 2022\nLOAFCLAN TACTICAL\nEQUIPMENT, LLC\n"
                                                                 "All rights reserved.");
    display::lcdDriver::add_menu_item(sys_info_menu, "Exit", &boot_menu);
    hud.load_and_display_menu(sys_info_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.select_method = decrement_menu;

    boot_mode = BOOT_MODE_SYS_INFO;
}

void setup() {
#ifdef DEBUG_MODE
    Serial.begin(9600);
    Serial.println("Starting...");
#endif
    init_eeprom();
    pinMode(TRIGGER_PIN_NUMBER, INPUT_PULLUP);
    pinMode(RELOAD_PIN_NUMBER, INPUT_PULLUP);
    pinMode(SELECT_PIN_NUMBER, INPUT_PULLUP);
    pinMode(BATTERY_PIN_NUMBER, INPUT);

    display::lcdDriver::displayInit(); // Initialize the LCD display
//    io_refresh_timer.begin(io_refresh, 250);

    boot_mode = static_cast<boot_modes>(get_boot_mode());

    // If the trigger is held down on startup, display the boot menu
    if (digitalReadFast(TRIGGER_PIN_NUMBER) == LOW) {
        boot_mode = BOOT_MODE_UNKNOWN;
        // to start the main loop
    }

    // Else check the eeprom to see what the last boot mode was and run the appropriate boot mode

    switch (boot_mode) { // Run the appropriate boot mode initializer
        case BOOT_MODE_GAME:
            boot_mode_game();
            break;
        case BOOT_MODE_REF:
            boot_mode_ref();
            break;
        case BOOT_MODE_CLONE_CONFIG:
            boot_mode_clone_config();
            break;
        case BOOT_MODE_CLONE_GUN:
            boot_mode_clone();
            break;
        case BOOT_MODE_GUN_CONFIG:
            boot_mode_options();
            break;
        case BOOT_MODE_SYS_INFO:
            boot_mode_sys_info();
            break;
        case BOOT_MODE_UNKNOWN:
            boot_menu();
            break;
        default:
            boot_menu();
            break;
    }
}

// This method will be removed in the future, currently only used for testing
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
    io_refresh();

    switch (boot_mode){
        case BOOT_MODE_GAME:
            tagger_loop(); // Run all main tagger functions
            hud.update_hud(); // Update the HUD
            break;
        case BOOT_MODE_REF: // All Ref functions are interrupt based so no loop is required
            break;
        case BOOT_MODE_CLONE_CONFIG:
            // Not implemented
            break;
        case BOOT_MODE_CLONE_GUN:
            // Not implemented
            break;
        case BOOT_MODE_GUN_CONFIG:
            // Not implemented
            break;
        case BOOT_MODE_SYS_INFO:
            // Not implemented
            break;
        case BOOT_MODE_UNKNOWN:
            break;
        default:
            break;
            // Do nothing if no boot mode is selected
    }


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
        } else if (command_array[0] == "backlight") {
            display::lcdDriver::toggle_backlight();
        }
    }

    while (micros() < next_loop_time) {
    }

}