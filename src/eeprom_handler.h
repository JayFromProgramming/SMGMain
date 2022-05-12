//
// Created by Jay on 4/1/2022.
//

#ifndef TLTPROJECT1_EEPROM_HANDLER_H
#define TLTPROJECT1_EEPROM_HANDLER_H

#include "mt2Library/mt2_protocol.h"

using namespace mt2;

int eepromPresetSize();

clone* load_preset(uint8_t preset_num);

void save_preset(uint8_t preset_num, clone* preset);

clone** load_presets(int* length);

void set_defaults();

int get_remaining_space();

unsigned char get_boot_mode();

void set_boot_mode(unsigned char mode);

void set_temp_boot_mode(unsigned char mode);

unsigned short get_device_id();

void set_device_id(unsigned short id);

void init_eeprom();

typedef struct device_configs {
    uint8_t boot_mode;
    uint8_t temp_boot_mode; // Indicates if the device is temporarily in switching boot modes

    uint8_t screen_orientation; // 0 for left, 1 for right, 2 for up, 3 for down
    uint8_t device_type; // Will be used by the radio library once it's implemented
    uint8_t device_id; // Used by the radio to identify the device and by IR shots

    bool one_color_flash; // Indicates if the device only supports one color flash (Red or Blue)
    bool lock_team_color; // If true, the team color will not be changed when being cloned

    float standard_battery_voltage; // The voltage of the standard battery
    float max_battery_voltage; // The maximum voltage of the battery
    float min_battery_voltage; // The minimum voltage of the battery

    uint8_t radio_channel; // The channel the radio is on
    uint8_t radio_power; // The transmission power of the radio

    uint8_t current_preset;
    teams current_team;

} device_configs;


device_configs* get_device_configs();

void set_device_configs(device_configs* configs);

#endif //TLTPROJECT1_EEPROM_HANDLER_H
