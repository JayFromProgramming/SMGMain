//
// Created by Jay on 5/12/2022.
//

#include "settings_configurer.h"
#include <lcdDisplay/lcdDriver.h>
#include <eeprom_handler.h>

device_configs* temp_configs = nullptr;

void save_settings() {
    set_device_configs(temp_configs);
}

void change_device_id(int id) {
    temp_configs->device_id = id;
}

display::menu_holder *create_settings_config_menu(void (*boot_menu_callback)()) {

    device_configs *configs = get_device_configs();

    auto *settings_menu = display::lcdDriver::make_menu("Edit Settings");

    display::lcdDriver::add_menu_item(settings_menu, "Save & Return", save_settings);
    display::lcdDriver::add_menu_item(settings_menu, "Exit without\nsaving", boot_menu_callback);
    display::menu_option_item *menu = nullptr;


    // Device ID
    menu = display::lcdDriver::add_submenu(settings_menu, "Device ID", change_device_id);

    for (int i = 0; i < MT2_MAX_PLAYERS; i++) {
        String id = "Tagger " + String(i) + " (" + *mt2::get_player_name(i) + ")";
        display::lcdDriver::add_option_menu_item(menu, id.c_str());
    }

    display::lcdDriver::option_menu_set_selected(menu, configs->device_id);

    // Screen Orientation
    menu = display::lcdDriver::add_submenu(settings_menu, "Screen Orientation", nullptr);
    display::lcdDriver::add_option_menu_item(menu, "Up");
    display::lcdDriver::add_option_menu_item(menu, "Down");
    display::lcdDriver::add_option_menu_item(menu, "Left");
    display::lcdDriver::add_option_menu_item(menu, "Right");
    display::lcdDriver::option_menu_set_selected(menu, configs->screen_orientation);

    // Radio Frequency
    menu = display::lcdDriver::add_submenu(settings_menu, "Radio Frequency", nullptr);
    display::lcdDriver::add_option_menu_item(menu, "850 MHz");
    display::lcdDriver::add_option_menu_item(menu, "885 MHz");
    display::lcdDriver::add_option_menu_item(menu, "915 MHz");
    display::lcdDriver::add_option_menu_item(menu, "935 MHz");
    display::lcdDriver::add_option_menu_item(menu, "950 MHz");
    display::lcdDriver::option_menu_set_selected(menu, configs->radio_channel);

    // Radio Power
    menu = display::lcdDriver::add_submenu(settings_menu, "Radio Power", nullptr);
    display::lcdDriver::add_option_menu_values(menu, 14, 20, 1, "dBi");
    display::lcdDriver::option_menu_set_selected(menu, configs->radio_power);



    return settings_menu;
}