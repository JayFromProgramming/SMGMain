//
// Created by Jay on 3/26/2022.
//

#ifndef SMGMAIN_TAG_COMMUNICATOR_H
#define SMGMAIN_TAG_COMMUNICATOR_H

#include <Arduino.h>
#include "mt2Library/ir_handler.h"
#include "mt2_protocol.h"
//#include <clone_preset_builder.h>

// A struct for the tagger.cpp to pass its event handlers to the tag_communicator.cpp
struct event_handlers {
    void (*on_hit)(uint_least8_t playerID, uint_least8_t teamID, uint_least8_t dmg) = nullptr;
    void (*on_clone)(mt2::clone *clone) = nullptr;
    void (*on_respawn)() = nullptr;
    void (*on_full_health)() = nullptr;
    void (*on_admin_kill)() = nullptr;
    void (*on_start_game)() = nullptr;
    void (*on_new_game)() = nullptr;
    void (*on_end_game)() = nullptr;
    void (*on_stun)() = nullptr;
    void (*on_explode)() = nullptr;

};

void IR_init();

event_handlers* get_handlers();

void signalScan();

void sendCommand(mt2::system_commands command);

void sendShot(uint_least8_t playerID, uint_least8_t teamID, uint_least8_t dmg);

void shoot();

void sendClone(mt2::clone *clone);

mt2::clone * buildClone(String json);

void printClone(mt2::clone *clone);

void decodeMT2Data(uint_least8_t* data);

void set_game_flag(mt2::clone* preset, unsigned char flag, unsigned char val,
                   unsigned char flag_select);


#endif //SMGMAIN_TAG_COMMUNICATOR_H
