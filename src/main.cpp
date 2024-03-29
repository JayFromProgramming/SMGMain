// This project implements the MilesTag 2 IR laser tag protocol.
// This is to function as a programmable IR gun cloner for game admins.

#include <Arduino.h>
#include <tagger.h>
#include <Bounce.h>

#include "audio/audio_interface.h"
#include "InternalTemperature.h"
#include "eeprom_handler.h"
#include <mt2Library/clone_configurer.h>
#include <lcdDisplay/lcdDriver.h>
//#include <radio/radioInterface.h>
#include "settings_configurer.h"
#include "mt2Library/referee.h"
#include "eventTimer.h"

#include <pinout.h>

//#define DEBUG_MODE


#define VERSION "0.1.0"


#define REBOOT SCB_AIRCR = 0x05FA0004

extern "C" uint32_t set_arm_clock(uint32_t frequency); // required prototype for cpu throttling

void boot_mode_game();
void boot_mode_ref();
void boot_mode_set_defaults();
void boot_mode_sys_info();
void boot_mode_clone_config();
void boot_menu();

display::lcdDriver hud = display::lcdDriver();
//wireless::radioInterface radio = wireless::radioInterface();
audio_interface::audio_interface audio = audio_interface::audio_interface();
uint32_t next_loop_time = 0;
event_handlers* tagger_events = nullptr;


//IntervalTimer io_refresh_timer;

Bounce trigger_button = Bounce(IO_TRIGGER, 10);
Bounce reload_button = Bounce(IO_RELOAD, 10);
Bounce mode_button = Bounce(IO_MODE, 10);
Bounce select_button = Bounce(IO_SELECT, 10);
eventTimer trigger_held_timer;

struct button_methods {
  void (*trigger_method)(bool state) = nullptr; //!< The method to call when the trigger button is pressed.
  void (*shot_check_method)(Bounce* passthrough) = nullptr; //!< The method to call when the trigger button is pressed.
  void (*trigger_held_method)() = nullptr; //!< The method to call when the trigger has been held for 1 second.
  void (*reload_method)()  = nullptr; //!< method to call when reload button is pressed
  void (*mode_method)()  = nullptr; //!< method to call when mode button is pressed
  void (*mode_method_secondary)()  = nullptr; //!< secondary mode method for when the mode button is depressed
  void (*select_method)(bool state) = nullptr; //!< method to call when select button is pressed
};

button_methods io_actions;

void clear_io_actions() {
    // Read button states to clear any waiting
    io_actions.trigger_method = nullptr;
    io_actions.reload_method = nullptr;
    io_actions.mode_method = nullptr;
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

void io_refresh(){
  trigger_button.update();
  reload_button.update();
  mode_button.update();
  if (io_actions.shot_check_method != nullptr) {
        io_actions.shot_check_method(&trigger_button);
  }
  if (trigger_button.fallingEdge()){
      if (io_actions.trigger_method != nullptr) {
          io_actions.trigger_method(true);
      }
      trigger_held_timer.set(1000);
  }
  if (trigger_button.risingEdge()){
      if (io_actions.trigger_method != nullptr){
        io_actions.trigger_method(false);
      }
      trigger_held_timer.stop();
  }
  if (trigger_held_timer.check()){
      if (io_actions.trigger_held_method != nullptr){
        io_actions.trigger_held_method();
      }
  }
  if (reload_button.fallingEdge()){
      if (io_actions.reload_method != nullptr){
        io_actions.reload_method();
      }
  }
  if (mode_button.fallingEdge()){
      if (io_actions.mode_method != nullptr){
        io_actions.mode_method();
      }
      if (io_actions.mode_method_secondary != nullptr){
        io_actions.mode_method_secondary();
      }
  }
  if (select_button.fallingEdge()){
      if (io_actions.select_method != nullptr){
        io_actions.select_method(true);
      }
  }
if (select_button.risingEdge()){
      if (io_actions.select_method != nullptr){
        io_actions.select_method(false);
      }
  }
}

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
    float batVal = analogRead(BATT_VOLT);
    float batVoltage = (batVal * 3.3f) / 1024;
    return batVoltage;
}

void overheat_method() {
    // In the event of the main cpu overheating we reduce the system clock to .75MHz and stop the main loop.
    // We will also attach a low temp interrupt to the main cpu to restart the system clock once the cpu has cooled down.

    char format_text[124];

    sprintf(format_text, "CPU TEMP: %.2f\n"
                         "MAX TEMP: 70C\n", InternalTemperatureClass::readTemperatureC());

    String info(format_text);

    hud.display_alert((String *) "CPU OVERHEATING", &info);
    set_arm_clock(750000);
}

void bad_battery_method(float_t battery_volts){
    char format_text[124];

    sprintf(format_text, "VOLTAGE: %.2f Volts\n"
                         "Min Volts: %.2f Volts\n"
                         "Max Volts: %.2f Volts", battery_volts, 7.f, 9.f);

    String info(format_text);

    hud.display_alert((String *) "LOW BATTERY!", &info);
    set_arm_clock(750000);
}

void boot_mode_game(){

    if (boot_mode != BOOT_MODE_GAME){
        set_boot_mode(BOOT_MODE_GAME);
        REBOOT;
    }

    tagger_init(&audio, &hud); // Initialize the tagger

    // Initialize all debounced IO methods
    clear_io_actions();
    io_actions.trigger_method = nullptr;
    io_actions.shot_check_method = &shot_check;
    io_actions.reload_method =  &on_reload;
    io_actions.mode_method =  &display::lcdDriver::toggle_backlight;
    io_actions.mode_method_secondary = &on_mode_select;
    io_actions.select_method = &on_fire_select;
    io_actions.trigger_held_method = &on_held_trigger;

    hud.pass_data_ptr(get_tagger_data_ptr(), get_score_data_ptr()); // pass the tagger data pointer to the lcd driver
    tagger_events = get_event_handler_ptr(); // get the tagger event pointer
//    digitalWrite(LED_BUILTIN, LOW);
    hud.clear();
}

void transmit_clone(int clone_id){
    auto* to_send = load_preset(clone_id);
    digitalWriteFast(MUZZLE_RED_FLASH, HIGH);
    sendClone(to_send);
    digitalWriteFast(MUZZLE_RED_FLASH, LOW);
    delete to_send;
//    hud.clear();
}

void boot_mode_clone(){

    if (boot_mode != BOOT_MODE_CLONE_GUN) {
        set_boot_mode(BOOT_MODE_CLONE_GUN);
        REBOOT;
    }

    auto* clone_menu = display::lcdDriver::make_menu("Select\nClone Preset");
    int presets;
    clone_t** presets_ptr = load_presets(&presets);
    for (int i = 0; i < presets; i++){
        display::lcdDriver::add_menu_item(clone_menu, presets_ptr[i]->name, transmit_clone, i);
    }
    display::lcdDriver::add_menu_item(clone_menu, "Exit", &boot_menu);
    hud.load_and_display_menu(clone_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.mode_method = decrement_menu;

}

void boot_mode_ref(){
    // Build the ref option menu

    if (boot_mode != BOOT_MODE_REF) {
        set_boot_mode(BOOT_MODE_REF);
        REBOOT;
    }

    auto* ref_menu =
            create_ref_menu(&boot_menu, &boot_mode_clone_config, &boot_mode_clone);
    hud.load_and_display_menu(ref_menu);

    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.mode_method = decrement_menu;

}

void launch_clone_config_menu(int clone_id){
    auto* clone = load_preset(clone_id);
    auto* menu = create_clone_config_menu(clone, clone_id, &boot_mode_clone_config);
    hud.load_free_display_menu(menu);

    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.mode_method = decrement_menu;
}

void boot_mode_clone_config(){

    if (boot_mode != BOOT_MODE_CLONE_CONFIG) {
        set_boot_mode(BOOT_MODE_CLONE_CONFIG);
        REBOOT;
    }

    auto* clone_menu = display::lcdDriver::make_menu("Select Preset\nto Edit");
    int presets;
    clone_t** presets_ptr = load_presets(&presets);
    for (int i = 0; i < presets; i++){
        display::lcdDriver::add_menu_item(clone_menu, presets_ptr[i]->name, &launch_clone_config_menu, i);
    }
    display::lcdDriver::add_menu_item(clone_menu, "Exit", &boot_menu);
    hud.load_free_display_menu(clone_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.mode_method = decrement_menu;

}

void boot_mode_options(){

    if (boot_mode != BOOT_MODE_GUN_CONFIG){
        set_temp_boot_mode(BOOT_MODE_GUN_CONFIG);
        REBOOT;
        return;
    }

    auto* menu = create_settings_config_menu(boot_mode_options);
    hud.load_and_display_menu(menu);

    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.mode_method = decrement_menu;


}

void boot_mode_set_defaults(){

    if (boot_mode != BOOT_MODE_SET_DEFAULTS){
        set_temp_boot_mode(BOOT_MODE_SET_DEFAULTS);
        REBOOT;
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
    io_actions.mode_method = decrement_menu;
}

void boot_menu(){

    if (boot_mode != BOOT_MODE_UNKNOWN){
        set_temp_boot_mode(BOOT_MODE_UNKNOWN);
        REBOOT;
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
    io_actions.mode_method = decrement_menu;
}

void boot_mode_sys_info() { // Display system information, does not override current boot mode

    if (boot_mode != BOOT_MODE_SYS_INFO){
        set_temp_boot_mode(BOOT_MODE_SYS_INFO); // Because this is a temp boot mode set the temp boot mode
        REBOOT;
        return;
    }

    auto* sys_info_menu = display::lcdDriver::make_menu("System Info");
    display::lcdDriver::add_menu_item(sys_info_menu, "Gun OS Version:\nV:" VERSION "\nBuild:\n"
            __DATE__ " " __TIME__, &boot_menu);
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
    display::lcdDriver::add_menu_item(sys_info_menu, "Copyright(c) 2022\nEMBEDDED\nENTERTAINMENT\nLLC\n"
                                                                 "All rights reserved.");
    display::lcdDriver::add_menu_item(sys_info_menu, "Exit", &boot_menu);
    hud.load_and_display_menu(sys_info_menu);
    clear_io_actions();
    io_actions.trigger_method = select_menu;
    io_actions.reload_method = increment_menu;
    io_actions.mode_method = decrement_menu;

    boot_mode = BOOT_MODE_SYS_INFO;
}

void setup() {

    pinMode(MUZZLE_IR_FLASH, OUTPUT);
    pinMode(MUZZLE_BLU_FLASH, OUTPUT);
    pinMode(MUZZLE_RED_FLASH, OUTPUT);
    pinMode(HIT_LED, OUTPUT);

    digitalWriteFast(MUZZLE_IR_FLASH, HIGH);
    digitalWriteFast(MUZZLE_BLU_FLASH, HIGH);
    digitalWriteFast(MUZZLE_RED_FLASH, HIGH);
    digitalWriteFast(HIT_LED, HIGH);

#ifdef DEBUG_MODE
    Serial.begin(9600);
    while (!Serial) {
    }
    Serial.println("Starting...");
#endif
    delayMicroseconds(1000); // Wait for all board components to initialize
    init_eeprom();
    pinMode(IO_TRIGGER, INPUT_PULLUP);
    pinMode(IO_MODE, INPUT_PULLUP);
    pinMode(IO_RELOAD, INPUT_PULLUP);
    pinMode(IO_SELECT, INPUT_PULLUP);
    pinMode(BATT_VOLT, INPUT);
    pinMode(DYNAMIC_IO, INPUT);
    pinMode(RADIO_CHIP_SELECT, OUTPUT);
    pinMode(RADIO_IRQ, INPUT);
    pinMode(RADIO_ENABLE, OUTPUT);


//     Make sure all spi cs pins are high before initialization
    digitalWriteFast(DISPLAY_CHIP_SELECT, LOW);
    digitalWriteFast(RADIO_CHIP_SELECT, HIGH);

    display::lcdDriver::displayInit(); // Initialize the LCD display
//    io_refresh_timer.begin(io_refresh, 250);

    InternalTemperatureClass::attachHighTempInterruptCelsius(70.f, overheat_method);

    boot_mode = static_cast<boot_modes>(get_boot_mode());

//    boot_mode = BOOT_MODE_UNKNOWN;

//    boot_mode = BOOT_MODE_GAME;

    // If the trigger is held down on startup, display the boot menu
    if (digitalReadFast(IO_TRIGGER) == LOW) {
        boot_mode = BOOT_MODE_UNKNOWN;
        // to start the main loop
    }

    // If the trigger and reload are held down on startup, display the factory reset menu
    if (digitalReadFast(IO_TRIGGER) == LOW && digitalReadFast(IO_RELOAD) == LOW ) {
        boot_mode = BOOT_MODE_SET_DEFAULTS;
    }
    // Else check the eeprom to see what the last boot mode was and run the appropriate boot mode

#ifdef DEBUG_MODE
    Serial.println("Setup complete");
    Serial.printf("Boot mode: %s\n", boot_mode_names[boot_mode].c_str());
#endif

    digitalWriteFast(MUZZLE_IR_FLASH, LOW);
    digitalWriteFast(MUZZLE_BLU_FLASH, LOW);
    digitalWriteFast(MUZZLE_RED_FLASH, LOW);
    digitalWriteFast(HIT_LED, LOW);

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
        case BOOT_MODE_SET_DEFAULTS:
            boot_mode_set_defaults();
            break;
        default:
            boot_menu();
            break;
    }
}

// Main loop this runs once every 20ms or 50Hz while the tagger and hud updates run audio interrupts are disabled
void loop() {

    next_loop_time = micros() + 20000;

    switch (boot_mode){
        case BOOT_MODE_GAME:
            tagger_loop(); // Run all main tagger functions
            hud.update_hud(); // Update the HUD
            break;
        case BOOT_MODE_REF: // All these boot modes are menu based so no loop is required
        case BOOT_MODE_CLONE_CONFIG:
        case BOOT_MODE_CLONE_GUN:
        case BOOT_MODE_GUN_CONFIG:
        case BOOT_MODE_SYS_INFO:
        case BOOT_MODE_UNKNOWN:
        default:
            break;
    }

    io_refresh();
#ifdef DEBUG_MODE
//    Serial.printf("Loop complete: %f\n", ((float) micros() - (float) next_loop_time) / 1000.f);
    if (micros() > next_loop_time) {
        Serial.printf("Loop took too long: %f ms\n", ((float) micros() - (float) next_loop_time) / 1000.f);
    }
#endif
//    while (micros() > next_loop_time) io_refresh(); // Just check IO while waiting for next loop

}