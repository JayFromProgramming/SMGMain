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

#endif //TLTPROJECT1_EEPROM_HANDLER_H
