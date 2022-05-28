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
    void (*on_hit)(uint_least8_t playerID, mt2::teams teamID, mt2::damage_table dmg) = nullptr;
    void (*on_clone)(mt2::clone *clone) = nullptr; //!< This is the callback for when a clone is detected.
    void (*on_pause_unpause)() = nullptr; //!< Called when a pause/unpause signal is received.
    void (*on_respawn)() = nullptr; //!< Called when a respawn signal is received.
    void (*on_full_health)() = nullptr; //!< Called when a full health signal is received.
    void (*on_full_ammo)() = nullptr; //!< Called when a full ammo signal is received.
    void (*on_admin_kill)() = nullptr;  //!< Called when an admin kill signal is received.
    void (*on_start_game)() = nullptr; //!< Called when a start game signal is received.
    void (*on_new_game)() = nullptr; //!< Called when a new game signal is received.
    void (*on_end_game)() = nullptr; //!< Called when an end game signal is received.
    void (*on_stun)() = nullptr; //!< Called when a stun signal is received.
    void (*on_explode)() = nullptr; //!< Called when an explode signal is received.
    void (*on_clip_pickup)(uint8_t value) = nullptr; //!< Called when a clip pickup signal is received.
    void (*on_health_pickup)(uint8_t value) = nullptr; //!< Called when a health pickup signal is received.
    void (*on_flag_pickup)(uint8_t value) = nullptr; //!< Called when a flag pickup signal is received.
    void (*on_add_health)(uint8_t value) = nullptr; //!< Called when an add health signal is received.
    void (*on_add_rounds)(uint8_t value) = nullptr; //!< Called when an add rounds signal is received.
    void (*on_restore_defaults)() = nullptr; //!< Called when a restore defaults signal is received.
    void (*on_reset_clock)() = nullptr; //!< Called when a reset clock signal is received.
    void (*on_init_player)() = nullptr; //!< Called when a init tagger signal is received.
    void (*on_full_armor)() = nullptr; //!< Called when a full armor signal is received.
    void (*on_clear_scores)() = nullptr; //!< Called when a clear scores signal is received.
    void (*on_test_sensors)() = nullptr; //!< Called when a test sensors signal is received.
    void (*on_disarm_player)() = nullptr; //!< Called when a disarm player signal is received.
};

void IR_init();

event_handlers* get_handlers();

void signalScan();

void sendCommand(mt2::system_commands command);

void sendShot(uint_least8_t playerID, uint_least8_t teamID, uint_least8_t dmg);

bool shoot();

void sendClone(mt2::clone *clone);

mt2::clone * buildClone(String json);

void printClone(mt2::clone *clone);

void decodeMT2Data(uint_least8_t* data);

void set_game_flag(mt2::clone* preset, unsigned char flag, unsigned char val,
                   unsigned char flag_select);


#endif //SMGMAIN_TAG_COMMUNICATOR_H
