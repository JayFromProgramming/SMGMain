//
// Created by Jay on 6/5/2022.
//

#ifndef SMGMAIN_REFEREE_H
#define SMGMAIN_REFEREE_H

#include "../lcdDisplay/lcdDriver.h"

display::menu_holder *create_ref_menu(void (*boot_menu_callback)(), void (*clone_config_callback)(), void (*clone_menu_callback)());

#endif //SMGMAIN_REFEREE_H
