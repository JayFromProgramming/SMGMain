//
// Created by Jay on 4/18/2022.
//

#ifndef SMGMAIN_TAGGER_H
#define SMGMAIN_TAGGER_H

#define TRIGGER_PIN_NUMBER 2
#define TRIGGER_PIN_MODE INPUT_PULLUP
#define TRIGGER_PIN_ACTIVE LOW
#define TRIGGER_PIN_RELEASED HIGH
#define TRIGGER_INTERRUPT_MODE RISING

#define RELOAD_PIN_NUMBER 3
#define RELOAD_PIN_MODE INPUT_PULLUP
#define RELOAD_PIN_ACTIVE LOW
#define RELOAD_PIN_RELEASED HIGH
#define RELOAD_INTERRUPT_MODE RISING

#define SELECT_PIN_NUMBER 6
#define SELECT_PIN_MODE INPUT_PULLUP
#define SELECT_PIN_ACTIVE LOW
#define SELECT_PIN_RELEASED HIGH
#define SELECT_INTERRUPT_MODE RISING

#define HIT_LED_PIN_NUMBER 4
#define HIT_LED_PIN_MODE OUTPUT
#define HIT_LED_PIN_ACTIVE HIGH
#define HIT_LED_PIN_INACTIVE LOW

#define IR_RECEIVER_PIN_NUMBER 5

#include <Arduino.h>

#include <mt2Library/mt2_protocol.h>
#include <audio/audio_interface.h>

struct tagger_state {
    mt2::clone *currentConfig = nullptr;
    uint32_t last_shot = 0;
    uint32_t last_hit = 0;
    unsigned int hit_delay_ms = 0;
    long shot_interval = 0;
    volatile short max_health = 0;
    volatile short health = 0;
    unsigned char player_id = 0;
    volatile unsigned char ammo_count = 0;
    volatile unsigned char clip_count = 0;
    unsigned char current_burst_count = 0;
    volatile bool reloading = false;
    volatile unsigned long reload_time = 0.0;
    volatile bool paused = false;
};

struct score_data {
    volatile unsigned short  rounds_fired = 0;
    volatile unsigned short  total_hits = 0;
    volatile unsigned int    game_start_time = 0; // Time of game start in ms
    volatile unsigned int    game_time = 0; // Time of game as of game end in ms
    volatile unsigned char   respawn_count = 0;
    volatile unsigned char   killed_by = 0; // Which player killed this player during this life
    volatile unsigned short* kills_by = nullptr; // Which players killed this player during the game
    volatile unsigned short* hits_from_players_game = nullptr; // How many times this player was hit by each player during the game
    volatile unsigned short* hits_from_players_life = nullptr; // How many times this player was hit by each player during this life
};

void tagger_init(audio_interface::audio_interface* audio_interface);

void tagger_loop();

tagger_state* get_tagger_data_ptr();

#endif //SMGMAIN_TAGGER_H
