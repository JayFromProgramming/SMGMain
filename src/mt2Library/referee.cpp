//
// Created by Jay on 6/5/2022.
//

#include "referee.h"
#include "tag_communicator.h"


void ref_kill() {
    sendCommand(mt2::ADMIN_KILL);
}

void ref_respawn() {
    sendCommand(mt2::RESPAWN);
}

void ref_reset() {
    sendCommand(mt2::FULL_HEALTH);
}

void ref_explode() {
    sendCommand(mt2::EXPLODE_PLAYER);
}

display::menu_holder *create_ref_menu(void (*boot_menu_callback)(), void (*clone_config_callback)(), void (*clone_menu_callback)()) {

    auto* ref_menu = display::lcdDriver::make_menu("Ref Menu");
    display::lcdDriver::add_menu_item(ref_menu, "Admin Kill", &ref_kill);
    display::lcdDriver::add_menu_item(ref_menu, "Respawn Player", &ref_respawn);
    display::lcdDriver::add_menu_item(ref_menu, "Reset Player", &ref_reset);
    display::lcdDriver::add_menu_item(ref_menu, "Explode Player", &ref_explode);

    display::lcdDriver::add_menu_item(ref_menu, "Enter Clone Mode", clone_menu_callback);
    display::lcdDriver::add_menu_item(ref_menu, "Edit Presets", clone_config_callback);
    display::lcdDriver::add_menu_item(ref_menu, "Exit to boot", boot_menu_callback);

    return ref_menu;

}


