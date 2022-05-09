//
// Created by Jay on 5/9/2022.
//

#include "clone_configurer.h"
#include "mt2_protocol.h"
#include <lcdDisplay/lcdDriver.h>

void edit_health(int health) {

}

void edit_clips_from_ammobox(int amount){

}

display::menu_holder* create_clone_config_menu(mt2::clone* clone_ptr) {
    char* formatted_text_ptr = new char[100];
    sprintf(formatted_text_ptr, "Editing\nClone %s", clone_ptr->name);
    auto* clone_menu = display::lcdDriver::make_menu(formatted_text_ptr);
    display::menu_option_item* menu = display::lcdDriver::add_option_menu(clone_menu, "Clips from ammobox",
                                                                         edit_clips_from_ammobox);
    display::lcdDriver::add_option_menu_values(menu, 0xFF, 1);
    display::lcdDriver::option_menu_set_selected(menu, clone_ptr->clips_from_ammo_box);
    menu = display::lcdDriver::add_option_menu(clone_menu, "Health", edit_health);
    // For value in the respawn_health table in the mt2_protocol.h
    for (int i = 0; i < 0x48; i++) {
        int health = mt2::health_lookup(static_cast<mt2::respawn_health>(i));
        sprintf(formatted_text_ptr, "%d", health);
        display::lcdDriver::add_option_menu_item(menu, formatted_text_ptr);
        if (health == clone_ptr->respawn_health)
            display::lcdDriver::option_menu_set_selected(menu, i);
    }

    return clone_menu;
}