//
// Created by Jay on 5/9/2022.
//

#ifndef SMGMAIN_CLONE_CONFIGURER_H
#define SMGMAIN_CLONE_CONFIGURER_H

#include "mt2_protocol.h"
#include "lcdDisplay/lcdDriver.h"

display::menu_holder *create_clone_config_menu(mt2::clone *clone, uint8_t clone_id, void(*config_callback)());

#endif //SMGMAIN_CLONE_CONFIGURER_H
