//
// Created by Jay on 5/28/2022.
//

#ifndef SMGMAIN_STATE_STRUCTS_H
#define SMGMAIN_STATE_STRUCTS_H

#include <Arduino.h>
#include <mt2Library/mt2_protocol.h>

/**
 * @brief This struct contains current game state values.
 */
typedef struct tagger_state {
    mt2::clone *currentConfig = nullptr; //!< The current clone configuration
    elapsedMillis last_shot; //!< How long since the last shot was fired, in milliseconds (auto-increments)
    uint32_t last_hit = 0; //!< The last time a hit was registered, in milliseconds
    uint16_t hit_delay_ms = 0; //!< The delay between hits, in milliseconds
    uint16_t shot_interval = 0; //!< The interval between shots, in milliseconds
    volatile uint16_t max_health = 0; //!< The maximum health the tagger can have
    volatile uint16_t health = 0;  //!< The current health the tagger has
    volatile uint16_t shield_health = 0; //!< The current shield health the tagger has
    uint8_t  player_id = 0; //!< The player id of the tagger
    volatile uint8_t ammo_count = 0; //!< The current ammo count
    volatile uint8_t clip_count = 0; //!< The current clip count
    volatile uint8_t clip_size = 0; //!< The current clip size
    uint8_t current_burst_count = 0; //!< The current burst size count
    volatile bool reloading = false; //!< Whether the tagger is currently reloading
    volatile uint32_t reload_time = 0.0; //!< If reloading, the time the reload will be completed, in milliseconds
    volatile bool paused = false; //!< Whether the tagger is paused
    volatile bool started = true; //!< Whether the tagger game has started
    volatile bool disarmed = false; //!< Whether the tagger is disarmed (triggered by admin)
    volatile uint8_t max_respawns = 0; //!< The maximum number of respawns the tagger can have
    volatile uint8_t respawns_remaining = 0; //!< The number of respawns the tagger has remaining
    volatile uint32_t auto_respawn_time = 0; //!< The time the tagger will respawn, in milliseconds (if auto-respawn is enabled)
} tagger_state;

/**
 * @brief This struct contains the current game scoring values.
 */
typedef struct score_data {
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
    volatile uint16_t*     killed_by_game = nullptr; //!< The number of times each player has killed the tagger in the game
    volatile uint32_t*     damage_from_players_game = nullptr; //!< The total damage the tagger has taken from each player in the game
    volatile uint16_t*     hits_from_players_game = nullptr; //!< The total number of hits the tagger has taken from each player in the game
    volatile uint32_t*     damage_from_players_life = nullptr; //!< The total damage the tagger has taken from each player in the current life
    volatile uint16_t*     hits_from_players_life = nullptr; //!< The total number of hits the tagger has taken from each player in the current life

    String* killer_name = nullptr; //!< The name of the player who killed the tagger
    String* assist_name = nullptr; //!< The name of the player who assisted the killer (if any)
} score_data;

#endif //SMGMAIN_STATE_STRUCTS_H
