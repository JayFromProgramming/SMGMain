//
// Created by Jay on 5/9/2022.
//

#include "clone_configurer.h"
#include "mt2_protocol.h"
#include <lcdDisplay/lcdDriver.h>

void edit_health(int health) {

}

void edit_damage(int damage) {

}

void edit_firerate(int firerate) {

}

void edit_reload_time(int reload_time) {

}

void edit_clips_from_ammobox(int amount){

}

void respawn_delay(int delay) {

}

display::menu_holder *create_clone_config_menu(mt2::clone *clone, display::lcdDriver driver) {
    // Due to the nature of adding a bunch of different items to a menu this method is very large
    char *formatted_text_ptr = new char[100];
    sprintf(formatted_text_ptr, "Editing Clone\n%s", clone->name);
    auto *clone_menu = display::lcdDriver::make_menu(formatted_text_ptr);
    display::lcdDriver::add_menu_item(clone_menu, "Return to menu");

    display::menu_option_item *menu = nullptr;

    menu = display::lcdDriver::add_submenu(clone_menu, "Respawn Health", edit_health);
    // For value in the respawn_health table in the mt2_protocol.h
    for (int i = 0; i < 0x48; i++) {
        int health = mt2::health_lookup(static_cast<mt2::respawn_health>(i));
        String health_str = String(health) + " HP";
        display::lcdDriver::add_option_menu_item(menu, health_str.c_str());
        if (i == clone->respawn_health)
            display::lcdDriver::option_menu_set_selected(menu, i);
    }

    // Add damage per shot
    menu = display::lcdDriver::add_submenu(clone_menu, "Shot Damage", edit_damage);
    for (int i = 0; i < 0x0F; i++) {
        int damage = mt2::damage_table_lookup(static_cast<mt2::damage_table>(i));
        String damage_str = String(damage) + " DMG";
        display::lcdDriver::add_option_menu_item(menu, damage_str.c_str());
        if (i == clone->damage_per_shot)
            display::lcdDriver::option_menu_set_selected(menu, i);
    }

    // Add firerate to menu
    menu = display::lcdDriver::add_submenu(clone_menu, "Rate of fire", edit_firerate);
    for (int i = 0; i < 0x0B; i++) {
        int firerate = mt2::fire_rate_table_lookup(static_cast<mt2::fire_rate_table>(i));
        String firerate_str = String(firerate) + " RPM";
        display::lcdDriver::add_option_menu_item(menu, firerate_str.c_str());
        if (i == clone->cyclic_rpm)
            display::lcdDriver::option_menu_set_selected(menu, i);
    }

    menu = display::lcdDriver::add_submenu(clone_menu, "Reload Time", edit_reload_time);
    display::lcdDriver::add_option_menu_values(menu, 0xFF, 1, "Seconds");
    display::lcdDriver::option_menu_set_selected(menu, clone->reload_time);

    menu = display::lcdDriver::add_submenu(clone_menu, "Respawn Delay", respawn_delay);
    display::lcdDriver::add_option_menu_values(menu, 0xFF * 10, 10, "Seconds");
    display::lcdDriver::option_menu_set_selected(menu, clone->respawn_delay / 10);

    menu = display::lcdDriver::add_submenu(clone_menu, "Ammobox quant.",
                                           edit_clips_from_ammobox);
    display::lcdDriver::add_option_menu_values(menu, 0xFF, 1, " Clips");
    display::lcdDriver::option_menu_set_selected(menu, clone->clips_from_ammo_box);

    display::lcdDriver::add_menu_item(clone_menu, "Save");

    return clone_menu;
}