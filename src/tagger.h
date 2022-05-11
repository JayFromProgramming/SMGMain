//
// Created by Jay on 4/18/2022.
//

#ifndef SMGMAIN_TAGGER_H
#define SMGMAIN_TAGGER_H

//#define TRIGGER_PIN_NUMBER 3
//#define TRIGGER_PIN_MODE INPUT_PULLUP
//#define TRIGGER_PIN_ACTIVE LOW
//#define TRIGGER_PIN_RELEASED HIGH
//#define TRIGGER_INTERRUPT_MODE FALLING
//
//#define RELOAD_PIN_NUMBER 2
//#define RELOAD_PIN_MODE INPUT_PULLUP
//#define RELOAD_PIN_ACTIVE LOW
//#define RELOAD_PIN_RELEASED HIGH
//#define RELOAD_INTERRUPT_MODE FALLING
//
//#define SELECT_PIN_NUMBER 6
//#define SELECT_PIN_MODE INPUT_PULLUP
//#define SELECT_PIN_ACTIVE LOW
//#define SELECT_PIN_RELEASED HIGH
//#define SELECT_INTERRUPT_MODE RISING

#define HIT_LED_PIN_NUMBER 4
#define HIT_LED_PIN_MODE OUTPUT
#define HIT_LED_PIN_ACTIVE HIGH
#define HIT_LED_PIN_INACTIVE LOW

#define IR_RECEIVER_PIN_NUMBER 5

#define MUZZLE_RED_FLASH_PIN_NUMBER 7
#define MUZZLE_BLUE_FLASH_PIN_NUMBER 8
#define MUZZLE_FLASH_ACTIVE HIGH
#define MUZZLE_FLASH_INACTIVE LOW

#include <Arduino.h>

#include <mt2Library/mt2_protocol.h>
#include <audio/audio_interface.h>
#include <mt2Library/tag_communicator.h>
#include <Bounce.h>

struct tagger_state {
    mt2::clone *currentConfig = nullptr;
    elapsedMillis last_shot;
    uint32_t last_hit = 0;
    uint16_t hit_delay_ms = 0;
    uint16_t shot_interval = 0;
    volatile uint16_t max_health = 0;
    volatile uint16_t health = 0;
    uint8_t  player_id = 0;
    volatile uint8_t ammo_count = 0;
    volatile uint8_t clip_count = 0;
    volatile uint8_t clip_size = 0;
    uint8_t current_burst_count = 0;
    volatile bool reloading = false;
    volatile uint32_t reload_time = 0.0;
    volatile bool paused = false;
    volatile bool started = true;
    volatile uint8_t max_respawns = 0;
    volatile uint8_t respawns_remaining = 0;
};

struct score_data {
    volatile uint16_t      rounds_fired_game = 0;
    volatile uint16_t      rounds_fired_life = 0;
    volatile uint16_t      total_hits_game = 0;
    volatile uint16_t      total_hits_life = 0;
    elapsedMillis          game_time;  // Game elapsed time in milliseconds
    elapsedMillis          alive_time; // Time player was alive in milliseconds
    volatile uint32_t      last_alive_time = 0; // Time player was alive in milliseconds
    volatile uint16_t      respawn_count = 0;
    volatile uint32_t      respawn_time = 0; // Time of last respawn in ms
    volatile uint8_t       last_killed_by = 0; // Which player killed this player during this life
    volatile uint16_t*     killed_by_game = nullptr; // Which players killed this player during the game
    volatile uint32_t*     damage_from_players_game = nullptr; // How much damage this player has done to other players during the game
    volatile uint16_t*     hits_from_players_game = nullptr; // How many times this player was hit by each player during the game
    volatile uint32_t*     damage_from_players_life = nullptr; // How much damage this player has received from other players during the game
    volatile uint16_t*     hits_from_players_life = nullptr; // How many times this player was hit by each player during this life

    String* killer_name = nullptr;
    String* assist_name = nullptr;
};

void shot_check(Bounce *bounce);

void on_reload();

void tagger_init(audio_interface::audio_interface* audio_interface);

void tagger_loop();

tagger_state* get_tagger_data_ptr();

event_handlers* get_event_handler_ptr();

score_data* get_score_data_ptr();

#endif //SMGMAIN_TAGGER_H
