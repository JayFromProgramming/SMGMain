//
// Created by Jay on 5/28/2022.
//

#ifndef SMGMAIN_STATE_STRUCTS_H
#define SMGMAIN_STATE_STRUCTS_H

#include <Arduino.h>
#include <mt2Library/mt2_protocol.h>
#include "eventTimer.h"

/**
 * @brief This struct contains current game state values.
 */
struct tagger_state_struct {
    elapsedMillis last_shot; //!< How long since the last shot was fired, in milliseconds (auto-increments)
    volatile uint32_t last_hit = 0; //!< The last time a hit was registered, in milliseconds
    volatile uint16_t hit_delay_ms = 0; //!< The delay between hits, in milliseconds
    volatile uint16_t health = 0;  //!< The current health the tagger has
    volatile uint8_t shield_health = 0; //!< The current shield health the tagger has
    volatile uint8_t ammo_count = 0; //!< The current ammo count
    volatile uint8_t clip_count = 0; //!< The current clip count
    volatile uint8_t clip_size = 0; //!< The current clip size
    volatile float_t barrel_temp = 0; //!< The current barrel temperature (in temp units)
    volatile bool reloading = false; //!< Whether the tagger is currently reloading
    volatile uint32_t reload_time = 0.0; //!< If reloading, the time the reload will be completed, in milliseconds
    volatile bool paused = false; //!< Whether the tagger is paused
    volatile bool keyed  = false; //!< Whether the tagger has held trigger down to start the game
    volatile bool started = false; //!< Whether the tagger game has started
    volatile bool starting = false; //!< Whether game start is in progress
    volatile bool disarmed = false; //!< Whether the tagger is disarmed (triggered by admin)
    volatile uint8_t respawns_remaining = 0; //!< The number of respawns the tagger has remaining
    eventTimer auto_respawn_time; //!< The time the tagger will respawn, in milliseconds (if auto-respawn is enabled)
    volatile bool jammed = false; //!< Whether the tagger is jammed (reload to clear jam)
    volatile uint8_t fire_selector = 0; //!< The current fire selector position
    volatile bool is_zombie = false; //!< Whether the tagger is a zombie (only used in zombie mode)

    // Below are static values not updated during game play
    mt2::clone_t *currentConfig = nullptr; //!< The current clone_t configuration
    uint16_t max_health = 0; //!< The maximum health the tagger can have
    uint8_t max_shield_health = 0; //!< The maximum shield health the tagger can have
    uint8_t max_respawns = 0; //!< The maximum number of respawns the tagger can have
    float_t max_barrel_temp = 0; //!< The maximum barrel temperature the tagger can have
    uint8_t player_id = 0; //!< The player id of the tagger
    uint16_t shot_interval = 0; //!< The interval between shots, in milliseconds
    mt2::teams team = mt2::TEAM_NONE; //!< The team the tagger is on
    bool medic_mode = false; //!< Whether the tagger is a medic
    bool zombie_mode = false; //!< Whether the tagger is in zombie mode
    bool friendly_fire = false; //!< Whether the tagger is in friendly fire mode
    uint8_t current_burst_count = 0; //!< The current burst size count

};

/**
 * @brief This struct contains the current game scoring values.
 */
struct score_data_struct {
    volatile uint16_t      rounds_fired_game = 0; //!< The number of rounds fired in the game
    volatile uint16_t      rounds_fired_life = 0; //!< The number of rounds fired in the current life
    volatile uint16_t      total_hits_game = 0; //!< The total number of hits in the game
    volatile uint16_t      total_hits_life = 0; //!< The total number of hits in the current life
    elapsedMillis          game_elapsed_time; //!< The elapsed time of the game, in milliseconds
    elapsedMillis          alive_time; //!< The time the tagger has been alive, in milliseconds
    volatile uint32_t      last_alive_time = 0; //!< How long the tagger survived during the last life, in milliseconds
    volatile uint16_t      respawn_count = 0; //!< The number of respawns the tagger has had
    volatile uint32_t      respawn_time = 0; //!< How long ago the last respawn was, in milliseconds
    volatile uint8_t       last_killed_by = 0; //!< The player id of the last player to kill the tagger
    volatile uint16_t*     killed_by_game = new uint_least16_t[MT2_MAX_PLAYERS]; //!< The number of times each player has killed the tagger in the game
    volatile uint32_t*     damage_from_players_game = new uint_least32_t[MT2_MAX_PLAYERS]; //!< The total damage the tagger has taken from each player in the game
    volatile uint16_t*     hits_from_players_game = new uint_least16_t[MT2_MAX_PLAYERS]; //!< The total number of hits the tagger has taken from each player in the game
    volatile uint32_t*     damage_from_players_life = new uint_least32_t[MT2_MAX_PLAYERS]; //!< The total damage the tagger has taken from each player in the current life
    volatile uint16_t*     hits_from_players_life = new uint_least16_t[MT2_MAX_PLAYERS]; //!< The total number of hits the tagger has taken from each player in the current life

    String* killer_name = nullptr; //!< The name of the player who killed the tagger
    String* assist_name = nullptr; //!< The name of the player who assisted the killer (if any)
};

#endif //SMGMAIN_STATE_STRUCTS_H
