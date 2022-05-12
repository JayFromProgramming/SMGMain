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

display::menu_holder *create_settings_config_menu(void (*boot_menu_callback)()) {

    device_configs *configs = get_device_configs();

    auto *clone_menu = display::lcdDriver::make_menu("Edit Settings");

    display::lcdDriver::add_menu_item(clone_menu, "Save & Return", save_settings);
    display::lcdDriver::add_menu_item(clone_menu, "Exit without\nsaving", boot_menu_callback);
    display::menu_option_item *menu = nullptr;

    menu = display::lcdDriver::add_submenu(clone_menu, "", edit_health);

}