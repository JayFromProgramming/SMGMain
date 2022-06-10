//
// Created by Jay on 5/9/2022.
//

#include "clone_configurer.h"
#include "mt2_protocol.h"
#include <lcdDisplay/lcdDriver.h>
#include <eeprom_handler.h>

#define REBOOT SCB_AIRCR = 0x05FA0004

mt2::clone_t* temp_clone;
uint8_t temp_clone_id;

void (*temp_callback)();

void edit_health(int health) {
    temp_clone->respawn_health = static_cast<mt2::respawn_health>(health);
}

void edit_damage(int damage) {
    temp_clone->damage_per_shot = static_cast<mt2::damage_table>(damage);
}

void edit_firerate(int firerate) {
    temp_clone->cyclic_rpm = static_cast<mt2::fire_rate_table>(firerate);
}

void edit_reload_time(int reload_time) {
    temp_clone->reload_time = reload_time;
}

void edit_clips_from_ammobox(int amount){
    temp_clone->clips_from_ammo_box = amount;
}

void respawn_delay(int delay) {
    temp_clone->respawn_delay = delay;
}

void max_respawns(int max_respawns) {
    temp_clone->max_respawns = max_respawns;
}

void edit_number_of_clips(int) {
    temp_clone->number_of_clips = 0;
}

void edit_hit_led_timeout(int timeout) {
    temp_clone->hit_led_timout_seconds = timeout;
}

void edit_hit_delay(int delay) {
    temp_clone->hit_delay = static_cast<mt2::hit_delays>(delay);
}


void edit_clip_size(int size) {
    temp_clone->clip_size = size;
}

void save_config() {
    save_preset(temp_clone_id, temp_clone);
    REBOOT;
}

void edit_start_delay(int delay) {
    temp_clone->start_delay = delay;
}

void edit_death_delay(int delay) {
    temp_clone->death_delay = delay;
}

void edit_armor_value(int value) {
    temp_clone->armour_value = value;
}

void edit_fire_mode(int mode) {
    temp_clone->fire_selector = static_cast<mt2::fire_mode>(mode);
}

void edit_burst_size(int size) {
    temp_clone->burst_size = size;
}

void edit_friendly_fire(int value) { // TODO: Fix
//    set_game_flag(temp_clone, GAME_FRIENDLY_FIRE, value, 0);
}

display::menu_holder *create_clone_config_menu(mt2::clone_t *clone, uint8_t clone_id, void (*config_menu_callback)()) {
    // Due to the nature of adding a bunch of different items to a menu this method is very large

    temp_clone = clone;
    temp_clone_id = clone_id;
    temp_callback = config_menu_callback;

    char *formatted_text_ptr = new char[100];
    sprintf(formatted_text_ptr, "Editing Clone\n%s", clone->name);
    auto *clone_menu = display::lcdDriver::make_menu(formatted_text_ptr);
    delete[] formatted_text_ptr;
    display::lcdDriver::add_menu_item(clone_menu, "Save & Return", save_config);
    display::lcdDriver::add_menu_item(clone_menu, "Exit without\nsaving", config_menu_callback);
    display::menu_option_item *menu = nullptr;

    menu = display::lcdDriver::add_submenu(clone_menu, "Respawn Health", edit_health);
    // For value in the respawn_health table in the mt2_protocol.h
    for (int i = 0; i <= 0x48; i++) {
        int health = mt2::health_lookup(static_cast<mt2::respawn_health>(i));
        String health_str = String(health) + " HP";
        display::lcdDriver::add_submenu_item(menu, health_str.c_str());
        if (i == clone->respawn_health)
            display::lcdDriver::submenu_set_selected(menu, i);
    }

    // Friendly fire (True/False)
    menu = display::lcdDriver::add_submenu(clone_menu, "Team Damage", edit_friendly_fire);
    display::lcdDriver::add_submenu_item(menu, "Disabled");
    display::lcdDriver::add_submenu_item(menu, "Enabled");
    display::lcdDriver::submenu_set_selected(menu,temp_clone->game_bool_flags_1 & GAME_FRIENDLY_FIRE);

    // Damage per shot
    menu = display::lcdDriver::add_submenu(clone_menu, "Shot Damage", edit_damage);
    for (int i = 0; i <= 0x0F; i++) {
        int damage = mt2::damage_table_lookup(static_cast<mt2::damage_table>(i));
        String damage_str = String(damage) + " DMG";
        display::lcdDriver::add_submenu_item(menu, damage_str.c_str());
        if (i == clone->damage_per_shot)
            display::lcdDriver::submenu_set_selected(menu, i);
    }

    // Firerate
    menu = display::lcdDriver::add_submenu(clone_menu, "Rate of fire", edit_firerate);
    for (int i = 0; i <= 0x0B; i++) {
        int firerate = mt2::fire_rate_table_lookup(static_cast<mt2::fire_rate_table>(i));
        String firerate_str = String(firerate) + " RPM";
        display::lcdDriver::add_submenu_item(menu, firerate_str.c_str());
        if (i == clone->cyclic_rpm)
            display::lcdDriver::submenu_set_selected(menu, i);
    }

    // Firemode
    menu = display::lcdDriver::add_submenu(clone_menu, "Fire selector", edit_fire_mode);
    display::lcdDriver::add_submenu_item(menu, "Single shot");
    display::lcdDriver::add_submenu_item(menu, "Burst fire");
    display::lcdDriver::add_submenu_item(menu, "Full Auto");
    display::lcdDriver::add_submenu_item(menu, "Selector - Burst");
    display::lcdDriver::add_submenu_item(menu, "Selector - Auto");
    display::lcdDriver::submenu_set_selected(menu, clone->fire_selector);


    // Burst size
    menu = display::lcdDriver::add_submenu(clone_menu, "Burst size", edit_burst_size);
    display::lcdDriver::add_submenu_values(menu, 30, 1);
    display::lcdDriver::submenu_set_selected(menu, clone->burst_size);

    // Reload time
    menu = display::lcdDriver::add_submenu(clone_menu, "Reload Time", edit_reload_time);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, "Seconds");
    display::lcdDriver::submenu_set_selected(menu, clone->reload_time);

    // Respawn delay
    menu = display::lcdDriver::add_submenu(clone_menu, "Respawn Delay", respawn_delay);
    display::lcdDriver::add_submenu_values(menu, 0xFF * 10, 10, "Seconds");
    display::lcdDriver::submenu_set_selected(menu, clone->respawn_delay / 10);

    // Max respawns
    menu = display::lcdDriver::add_submenu(clone_menu, "Max Respawns", max_respawns);
    display::lcdDriver::add_submenu_values(menu, 0xFE, 1, "Times");
    display::lcdDriver::add_submenu_item(menu, "Unlimited");
    display::lcdDriver::submenu_set_selected(menu, clone->max_respawns);

    // Clips from ammo box
    menu = display::lcdDriver::add_submenu(clone_menu, "Ammobox quant.",
                                           edit_clips_from_ammobox);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, " Clips");
    display::lcdDriver::submenu_set_selected(menu, clone->clips_from_ammo_box);
//
    // Clip size
    menu = display::lcdDriver::add_submenu(clone_menu, "Clip size",
                                           edit_clip_size);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, " Bullets");
    display::lcdDriver::submenu_set_selected(menu, clone->clip_size);

    // Number of clips
    menu = display::lcdDriver::add_submenu(clone_menu, "Number of clips",
                                               edit_number_of_clips);
    display::lcdDriver::add_submenu_values(menu, 0xCA - 1, 1, " Clips");
    display::lcdDriver::add_submenu_item(menu, "Unlimited");
    display::lcdDriver::submenu_set_selected(menu, clone->number_of_clips);

    // hit_led_timout_seconds
    menu = display::lcdDriver::add_submenu(clone_menu, "Hit LED timeout",
                                               edit_hit_led_timeout);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, " Seconds");
    display::lcdDriver::submenu_set_selected(menu, clone->hit_led_timout_seconds);

    // hit_delay
    menu = display::lcdDriver::add_submenu(clone_menu, "Hit delay",
                                                   edit_hit_delay);
    for (int i = 0; i < 0x17; i++) {
        float hit_delay = mt2::hit_delay_to_seconds(static_cast<mt2::hit_delays>(i));
        String hit_delay_str = String(hit_delay) + " Seconds";
        display::lcdDriver::add_submenu_item(menu, hit_delay_str.c_str());
        if (i == clone->hit_delay)
            display::lcdDriver::submenu_set_selected(menu, i);
    }

    // Start delay
    menu = display::lcdDriver::add_submenu(clone_menu, "Start delay",
                                                       edit_start_delay);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, " Seconds");
    display::lcdDriver::submenu_set_selected(menu, clone->start_delay);

    // Death delay
    menu = display::lcdDriver::add_submenu(clone_menu, "Death delay",
                                                       edit_death_delay);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, " Seconds");
    display::lcdDriver::submenu_set_selected(menu, clone->death_delay);

    // Armor value
    menu = display::lcdDriver::add_submenu(clone_menu, "Armor value",
                                                           edit_armor_value);
    display::lcdDriver::add_submenu_values(menu, 0xFF, 1, " Points");
    display::lcdDriver::submenu_set_selected(menu, clone->armour_value);
//
//    for (int i = 0 ; i < 0x17; i++) {
//        float delay = mt2::hit_delay_to_seconds(static_cast<mt2::hit_delays>(i));
//        String delay_str = String(delay) + " Seconds";
//        display::lcdDriver::add_submenu_item(menu, delay_str.c_str());
//        if (i == clone_t->hit_delay)
//            display::lcdDriver::submenu_set_selected(menu, i);
//    }

    display::lcdDriver::add_menu_item(clone_menu, "Save", save_config);

    return clone_menu;
}


