//
// Created by Jay on 4/18/2022.
//
#include <tagger.h>
#include "audio/audio_interface.h"
#include <eeprom_handler.h>
#include <Bounce.h>
#include <pinout.h>
#include <eventTimer.h>

tagger_state_struct game_state = tagger_state_struct(); //!< The current game state and all associated data

event_handlers handles = event_handlers(); //!< The event handlers for the tagger (used by the tag_communicator)

score_data_struct score_data = score_data_struct(); //!< The score data for the tagger

audio_interface::audio_interface *audio_ptr = nullptr; //!< The audio interface pointer
display::lcdDriver *display_ptr = nullptr; //!< The display interface pointer

uint32_t muzzle_flash_timer; //!< The timer for the muzzle flash (used to indicate when the muzzle flash should turn off)
uint32_t hit_led_timer; //!< The timer for the hit led (used to indicate when the hit led should turn off)

uint8_t current_flash_bulb_pin = MUZZLE_RED_FLASH; //!< The current flash bulb pin (used to indicate which flash bulb should be turned on)
float_t temp_increase_per_shot = 0; //!< The temperature increase per shot

void move_life_scores();

void add_health(uint8_t value);

void add_ammo(uint8_t value);

void flag_pickup(uint8_t value);

void clip_pickup(uint8_t value);

void health_pickup(uint8_t value);

void reset_clock();

void full_armor();

void disarm_player();

void stun_player();

void explode_player();

void restore_defaults();

void respawn();

void hit_led_flash(){
    digitalWriteFast(HIT_LED, HIGH);
    hit_led_timer = millis() + 250;
}

int8_t trigger_down = 0; //!< The current state of the trigger [1 = Just pulled, 0 = Released,
//!< -1 = Not released but is being held]

/**
 * @brief Sets the taggers internal config states
 * @details Sets internal config states to the values in the new configuration
 * @usage This function is called when the tagger is first initialized, and when the gun gets cloned by a referee
 * @param newClone - The new config to set the tagger to
 */
void configure_from_clone(mt2::clone* newClone){
    game_state.currentConfig = newClone;

    // The config firerate is stored as Rounds per minute, so we need to calculate the delay in milliseconds
    game_state.shot_interval = ((float) (1.f / (float) mt2::fire_rate_table_lookup(newClone->cyclic_rpm)) * 60.f) * 1000;

    game_state.hit_delay_ms = mt2::hit_delay_to_micros(game_state.currentConfig->hit_delay);

    game_state.max_health = mt2::health_lookup(game_state.currentConfig->respawn_health);
    game_state.health = game_state.max_health;
    game_state.clip_size = game_state.currentConfig->clip_size;
    game_state.clip_count = game_state.currentConfig->number_of_clips;
    game_state.ammo_count = game_state.clip_size;
    game_state.started = false;
    game_state.team = game_state.currentConfig->team_id;
    game_state.max_barrel_temp = game_state.currentConfig->overheat_limit;
    buildShot(game_state.player_id, game_state.team, game_state.currentConfig->damage_per_shot);

    if (game_state.max_barrel_temp == 0) {
        temp_increase_per_shot = 0;
    } else temp_increase_per_shot = 0.5;

    switch (game_state.currentConfig->fire_selector){
        case FIRE_MODE_SINGLE:
            game_state.fire_selector = FIRE_MODE_SINGLE;
            break;
        case FIRE_MODE_BURST:
            game_state.fire_selector = FIRE_MODE_BURST;
            break;
        case FIRE_MODE_AUTO:
            game_state.fire_selector = FIRE_MODE_AUTO;
            break;
        case FIRE_MODE_SELECT_BURST:
            game_state.fire_selector = digitalReadFast(IO_SELECT) ? FIRE_MODE_BURST : FIRE_MODE_SINGLE;
            break;
        case FIRE_MODE_SELECT_AUTO:
            game_state.fire_selector = digitalReadFast(IO_SELECT) ? FIRE_MODE_AUTO : FIRE_MODE_SINGLE;
            break;
    }

    if (game_state.team == mt2::PASSTHROUGH || game_state.team == mt2::TEAM_NONE){
        game_state.team = get_device_configs()->current_team;
    }

    if (game_state.currentConfig->game_bool_flags_1 & GAME_FRIENDLY_FIRE){
        game_state.friendly_fire = true;
    } else {
        game_state.friendly_fire = false;
    }

    if (game_state.currentConfig->game_bool_flags_1 & GAME_MEDIC_ENABLE){
        game_state.medic_mode = true;
        display::lcdDriver::force_backlight(true, true); // Lock the backlight to on
    } else game_state.medic_mode = false;

    if (game_state.currentConfig->game_bool_flags_1 & GAME_ZOMBIE_MODE){
        game_state.zombie_mode = true;
        game_state.team = mt2::TEAM_BLUE;
    } else game_state.zombie_mode = false;

    switch (game_state.currentConfig->team_id){
        case TEAM_RED:
            current_flash_bulb_pin = MUZZLE_RED_FLASH;
            break;
        case TEAM_BLUE:
            current_flash_bulb_pin = MUZZLE_BLU_FLASH;
            break;
        case TEAM_YELLOW:
            current_flash_bulb_pin = MUZZLE_RED_FLASH;
            break;
        case TEAM_GREEN:
            current_flash_bulb_pin = MUZZLE_BLU_FLASH;
            break;
        case TEAM_NONE:
        case PASSTHROUGH:
            current_flash_bulb_pin = MUZZLE_RED_FLASH;
            break;
    }

}

/**
 * @brief Processes reload requests from the user
 * @details Called by the IO debouncer when the reload button is depressed, when reload is allowed it sets the reload
 * flag and sets the reload timer to time the reload should be completed in milliseconds
 */
void on_reload(){
    if (game_state.reloading) {
        return;
    }

    game_state.jammed = false; // Reset the jammed flag after a reload

    if (game_state.clip_count >= 1 && game_state.ammo_count < game_state.clip_size) {
        game_state.reloading = true;
        // Set the reload_time to the current time + the reload_time (in seconds)
        game_state.reload_time = millis() + (game_state.currentConfig->reload_time * 1000);
        audio_ptr->play_sound(audio_interface::SOUND_RELOAD);
        display_ptr->start_progress_circle(game_state.currentConfig->reload_time * 1000);
    }
}

/**
 * @brief Processes trigger state changes
 * @param bounce_ptr - The debouncer that triggered the event
 */
void shot_check(Bounce *bounce_ptr){


    if (bounce_ptr->fallingEdge()) {
        trigger_down = 1;
    } else if (bounce_ptr->risingEdge()) {
        trigger_down = 0; // Reset the trigger_down flag
    } else {
        if (trigger_down == 1) trigger_down = -1;
    }

//    game_state.currentConfig->fire_selector = mt2::FIRE_MODE_AUTO;
    if ((int) game_state.last_shot > game_state.shot_interval && !game_state.reloading) {
        if (game_state.ammo_count > 0 && !game_state.jammed) {

            // Check what fire mode we are in
            switch (game_state.fire_selector) {
                case mt2::FIRE_MODE_SINGLE: // Check if trigger_down is 1 and not -1
                    if (trigger_down == 1) {
                        if (shoot()) {
                            audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                            game_state.last_shot = 0;
                            game_state.ammo_count--;
                            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                            muzzle_flash_timer = millis() + 100;
                            score_data.rounds_fired_life++;
                            game_state.barrel_temp += temp_increase_per_shot;
                        }
                    }
                    break;
                case mt2::FIRE_MODE_BURST:
                    if (trigger_down == 1) {
                        if (shoot()) {
                            game_state.current_burst_count = game_state.currentConfig->burst_size;
                            audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                            game_state.last_shot = 0;
                            game_state.ammo_count--;
                            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                            muzzle_flash_timer = millis() + 100;
                            score_data.rounds_fired_life++;
                            game_state.barrel_temp += temp_increase_per_shot;
                        }
                    } else if (trigger_down == -1) {
                        if (game_state.current_burst_count > 0) {
                            if (shoot()) {
                                game_state.current_burst_count--;
                                audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                                game_state.last_shot = 0;
                                game_state.ammo_count--;
                                digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                                muzzle_flash_timer = millis() + 100;
                                score_data.rounds_fired_life++;
                                game_state.barrel_temp += temp_increase_per_shot;
                            }
                        }
                    }
                    break;
                case mt2::FIRE_MODE_AUTO:
                    if (trigger_down != 0) {
                        if (shoot()) { // If the IR transmitter is not busy preform shot actions
//                            audio_ptr.play_sound(audio_interface::SOUND_SHOOT);
                            game_state.last_shot = 0;
                            game_state.ammo_count--;
                            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                            muzzle_flash_timer = millis() + 100;
                            score_data.rounds_fired_life++;
                            game_state.barrel_temp += temp_increase_per_shot;
                        }
                    }
                    break;
                default:
                    break;
            }
        } else if (trigger_down == 1) {
            audio_ptr->play_sound(audio_interface::SOUND_EMPTY);
        }
    }
}

/**
 * @brief When the fire select pin changes state, this function is called
 * @param value - The value of the pin
 */
void on_fire_select(bool value){
    if (game_state.currentConfig->fire_selector == mt2::FIRE_MODE_SELECT_AUTO){ // If select auto is enabled
        // If the fire select pin is high, set the fire mode to auto else set it to single
        game_state.fire_selector = value ? mt2::FIRE_MODE_AUTO : mt2::FIRE_MODE_SINGLE;
    } else if (game_state.currentConfig->fire_selector == mt2::FIRE_MODE_SELECT_BURST) {
        // If the fire select pin is high, set the fire mode to burst else set it to single
        game_state.fire_selector = value ? mt2::FIRE_MODE_SINGLE : mt2::FIRE_MODE_BURST;
    }
}
/**
 * @brief When the mode pin changes state, this function is called
 */
void on_mode_select(){
    if (game_state.medic_mode){
        if ((int) game_state.last_shot > game_state.shot_interval && !game_state.reloading) {
            sendCommand(mt2::RESPAWN); // Send an admin respawn command
            game_state.last_shot = 0;
            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
            muzzle_flash_timer = millis() + 50;
            audio_ptr->play_sound(audio_interface::SOUND_SHOOT); // Temp: Plays shot sound will be a different sound later
        }
    }
}

void on_killed(uint_least8_t killer_id) {
    score_data.killed_by_game[killer_id]++; // Increment the kills by player
    score_data.last_killed_by = killer_id; // Set the killed by player
    score_data.killer_name = mt2::get_player_name(score_data.last_killed_by); // Set the killer name

    audio_ptr->play_sound(audio_interface::SOUND_DEATH); // Make a scream of death

    // Calculate who assisted the killer (whoever did the most amount of damage to the player not including the killer)
    uint_least16_t max_damage = 0;
    uint_least8_t max_damage_id = 0;
    for (uint_least8_t i = 0; i < MT2_MAX_PLAYERS; i++) {
        if (i != killer_id) {
            if (score_data.damage_from_players_life[i] > max_damage) {
                max_damage = score_data.damage_from_players_life[i];
                max_damage_id = i;
            }
        }
    }
    if (max_damage_id != killer_id || max_damage != 0) {
        score_data.assist_name = mt2::get_player_name(max_damage_id); // Set the assist name
    } else score_data.assist_name = nullptr; // No assist

    score_data.last_alive_time = score_data.alive_time;

    if (game_state.currentConfig->respawn_delay > 0 && game_state.respawns_remaining > 0) {
        // Set the respawn timer to the current time + the respawn_delay (in ten second increments)
        game_state.auto_respawn_time = game_state.currentConfig->respawn_delay * 10000;
        display_ptr->start_progress_circle(game_state.currentConfig->respawn_delay * 10000,
                                           (String *) "Auto Respawning in");
    }

    display::lcdDriver::force_backlight(true, true); // Force the backlight on to show the death screen
}

// This section contains the event handlers for the game

void on_clone(mt2::clone* clone){
    if (clone->checksum_valid){
        save_preset(0, clone);
        configure_from_clone(clone);
        audio_ptr->play_sound(audio_interface::SOUND_CLONE_OK);
    } else {
        audio_ptr->play_sound(audio_interface::SOUND_BEEP);
    }
    // audio_ptr.play_sound(audio_interface::S);
}

// Called when the player is hit
void on_hit(uint_least8_t playerID, mt2::teams teamID, mt2::damage_table dmg){

    Serial.printf("Player %d hit by %d\n", playerID, dmg);
    // Check if we are currently still in the hit delay period and if so, ignore the hit
    if (millis() - game_state.last_hit < game_state.hit_delay_ms) {
        return;
    }

    // Check if friendly fire is enabled, if so check if the hit was on a friendly team and if so ignore the hit
    if (game_state.friendly_fire || teamID != game_state.team) {
        Serial.println("Friendly fire enabled or hit was from enemy team");
        hit_led_flash();
        if (game_state.shield_health > 0){
            game_state.shield_health = max(0, game_state.shield_health - mt2::damage_table_lookup(dmg));
            audio_ptr->play_sound(audio_interface::SOUND_SHIELD_HIT);
        } else {
            game_state.health = max(0, game_state.health - mt2::damage_table_lookup(dmg));
            game_state.last_hit = millis(); // Set the last hit time
            score_data.hits_from_players_life[playerID]++;;
            score_data.damage_from_players_life[playerID] +=
                    mt2::damage_table_lookup(dmg);
            score_data.total_hits_life++;
            // Check if we are dead
            if (game_state.health <= 0) {
                on_killed(playerID);
            } else {
                audio_ptr->play_sound(audio_interface::SOUND_HIT);
                // Turn on the hit led
                digitalWriteFast(HIT_LED, game_state.is_zombie ? HIT_LED_PIN_ACTIVE : HIT_LED_PIN_INACTIVE);
                hit_led_timer = millis() + 250; // Set the hit led timer
            }
        }
    }
}
// Called when the player is killed, transfers score data from this life to the game score data
void move_life_scores(){
    score_data.rounds_fired_game += score_data.rounds_fired_life;
    score_data.rounds_fired_life = 0;
    score_data.total_hits_game += score_data.total_hits_life;
    score_data.total_hits_life = 0;
    score_data.killer_name = nullptr;
    score_data.assist_name = nullptr;
    for (uint_least8_t i = 0; i < MT2_MAX_PLAYERS; i++) {
        score_data.hits_from_players_game += score_data.hits_from_players_life[i];
        score_data.hits_from_players_life[i] = 0;
        score_data.damage_from_players_game[i] += score_data.damage_from_players_life[i];
        score_data.damage_from_players_life[i] = 0;
    }
    score_data.alive_time = 0;
}

// Clear all scoring data for the current game
void clear_scores(){
    // If no memory has been allocated to the score arrays, initialize them
    for (uint_least8_t i = 0; i < MT2_MAX_PLAYERS; i++) {
        score_data.killed_by_game[i] = 0;
        score_data.hits_from_players_game[i] = 0;
        score_data.hits_from_players_life[i] = 0;
        score_data.damage_from_players_game[i] = 0;
        score_data.damage_from_players_life[i] = 0;
    }
    score_data.total_hits_game = 0;
    score_data.last_killed_by = 0;
    audio_ptr->play_sound(audio_interface::SOUND_BEEP);
}

void new_game(){
    game_state.health = game_state.max_health;
    game_state.last_shot = 0;
    game_state.last_hit = 0;
    game_state.max_respawns = game_state.respawns_remaining;
    clear_scores();
    audio_ptr->play_sound(audio_interface::SOUND_BEEP);
}

void end_game(){
    game_state.health = 0;
    game_state.last_shot = 0;
    game_state.last_hit = 0;
    audio_ptr->play_sound(audio_interface::SOUND_BEEP);
    move_life_scores();
    game_state.started = false;
}

void start_game(){
    if (game_state.started) {
        audio_ptr->play_sound(audio_interface::SOUND_RELOADED);
    } else {
        new_game();
    }
}

/**
 * @brief Aka. "Admin respawn"
 * @details This method is called when the tagger receives a full health command from an admin gun
 */
void full_health(){
    game_state.is_zombie = false;
    game_state.ammo_count = game_state.clip_size;
    game_state.clip_count = game_state.currentConfig->number_of_clips;
    game_state.health = game_state.max_health;
    game_state.last_shot = 0;
    game_state.last_hit = 0;
    game_state.reloading = false;
    game_state.reload_time = 0;
    score_data.respawn_time = millis();
    move_life_scores(); // Move the life scores to the game scores
    display::lcdDriver::force_backlight(false); // Return control of the backlight to the user
    audio_ptr->play_sound(audio_interface::SOUND_HEAL);
}

void respawn(){ // Called when a player respawns
    // Called when a player respawns
    if (game_state.zombie_mode){ // If we are in zombie mode, respawn the player as a zombie
        game_state.clip_size = 1;
        game_state.clip_count = 0xFF;
        game_state.health = mt2::HP_10;
        game_state.last_shot = 0;
        game_state.last_hit = 0;
        move_life_scores();
        game_state.team = mt2::TEAM_RED; // Zombies are always on team red
    } else if (game_state.respawns_remaining > 0 ||
    game_state.currentConfig->game_bool_flags_1 & GAME_UNLIMITED_RESPAWN) {
        if (game_state.currentConfig->game_bool_flags_2 & GAME_FULL_AMMO_ON_RESPAWN) {
            game_state.ammo_count = game_state.clip_size;
            game_state.clip_count = game_state.currentConfig->number_of_clips;
        }
        game_state.health = game_state.max_health;
        game_state.last_shot = 0;
        game_state.last_hit = 0;
        game_state.reloading = false;
        game_state.reload_time = 0;
        score_data.respawn_time = millis();
        move_life_scores(); // Move the life scores to the game scores
        if (!(game_state.currentConfig->game_bool_flags_1 & GAME_UNLIMITED_RESPAWN)) game_state.respawns_remaining--;
        display::lcdDriver::force_backlight(false); // Return control of the backlight to the user
    }
}

void admin_kill(){ // Called when an admin kills a player
    score_data.last_killed_by = GAME_ADMIN_ID;
    score_data.last_alive_time = score_data.alive_time;
    score_data.killer_name = mt2::get_player_name(GAME_ADMIN_ID);
    score_data.assist_name = nullptr;
    game_state.health = 0;
    audio_ptr->play_sound(audio_interface::SOUND_DEATH); // Make a scream of death

    if (game_state.currentConfig->respawn_delay > 0 && game_state.respawns_remaining > 0) {
        // Set the respawn timer to the current time + the respawn_delay (in ten second increments)
        game_state.auto_respawn_time = millis() + (game_state.currentConfig->respawn_delay * 10000);
        display_ptr->start_progress_circle(game_state.currentConfig->respawn_delay * 10000,
                                           (String *) "Auto Respawning in");
    }

    display::lcdDriver::force_backlight(true, true); // Force the backlight on to show the death screen

}

void pause_unpause(){
    if (game_state.paused){
        game_state.paused = false;
        audio_ptr->play_sound(audio_interface::SOUND_BEEP);
    } else {
        game_state.paused = true;
        audio_ptr->play_sound(audio_interface::SOUND_BEEP);
    }
}

void stunned(){
    game_state.last_shot = millis() + 3000; // Set the last shot time to 3 seconds in the future to prevent shooting
}

void test_sensors(){
    digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_ACTIVE);
    digitalWriteFast(MUZZLE_RED_FLASH, MUZZLE_FLASH_ACTIVE);
    digitalWriteFast(MUZZLE_BLU_FLASH, MUZZLE_FLASH_ACTIVE);
    sendShot(0, mt2::teams::TEAM_GREEN, 0);
    delayMicroseconds(1000);
    digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_INACTIVE);
    digitalWriteFast(MUZZLE_RED_FLASH, MUZZLE_FLASH_INACTIVE);
    digitalWriteFast(MUZZLE_BLU_FLASH, MUZZLE_FLASH_INACTIVE);
    hit_led_timer = 2000;
}

void restart_gun(){
    SCB_AIRCR = 0x05FA0004; // Reset the MCU
}

// End event handlers


/**
 * @brief Main tagger processing loop
 * @details This function is called repeatedly by the main loop. It is responsible for processing received IR data, and
 * for timed events that don't have an interval timer or associated interrupt.
 */
void tagger_loop(){
    // Called by loop() this is where main code logic goes

    if (game_state.reloading) {
        // If we are reloading, check if we are done
        if (millis() >= game_state.reload_time) {
            // We are done reloading
            game_state.reloading = false;
            // Reset the reload_time
            game_state.reload_time = 0l;

            // Reload the ammo
            game_state.ammo_count = game_state.currentConfig->clip_size;
            if ((game_state.currentConfig->game_bool_flags_1 & GAME_UNLIMITED_CLIPS) == false) {
                game_state.clip_count--;
            }
            audio_ptr->play_sound(audio_interface::SOUND_RELOADED);
            game_state.last_shot = 0;
        }
    }

    if(game_state.barrel_temp > 0){
        game_state.barrel_temp -= 0.05;
    }

//    if(game_state.barrel_temp > game_state.max_barrel_temp){
//        game_state.jammed = true;
//    }

//    if (game_state.auto_respawn_time){
//        respawn();
//    }

    if (game_state.zombie_mode && game_state.health == 0) { // Flash the hit LED's at .5hz
        if (hit_led_timer <= millis()) {
            digitalToggleFast(HIT_LED_PIN_NUMBER);
            hit_led_timer = millis() + 500;
        }
    }

    if (hit_led_timer <= millis() && game_state.health > 0 && hit_led_timer != 0) {
        if (game_state.is_zombie){ // If we are a zombie the hit LEDs are inverted
            digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_ACTIVE);
        } else {
            digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_INACTIVE);
        }
        hit_led_timer = 0;
    }

    if (muzzle_flash_timer <= millis() && muzzle_flash_timer != 0) {
        digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_INACTIVE);
        muzzle_flash_timer = 0;
    }

    // Check for IR input
    signalScan();
}

/**
 * @brief Provides a pointer to the current game state
 * @return A pointer to the tagger_state_struct struct
 */
FLASHMEM tagger_state_struct* get_tagger_data_ptr(){
    return &game_state;
}

/**
 * @breief Provides a pointer to the event_handlers struct
 * @return A pointer to the event_handlers struct
 */
FLASHMEM event_handlers* get_event_handler_ptr(){
    return &handles;
}

/**
 * @brief Provides a pointer to the score_data_struct struct
 * @return A pointer to the score data struct
 */
FLASHMEM score_data_struct* get_score_data_ptr(){
    return &score_data;
}

/**
 * @brief This function is called to initialize the tagger
 * @details This function is called once at the beginning of the program. It is responsible for initializing the tagger
 * and setting up the event handlers and other data structures.
 * @param audioPtr - Pointer to the audio interface
 */
FLASHMEM void tagger_init(audio_interface::audio_interface* audioPtr, display::lcdDriver *lcdPtr){
    IR_init();
    audio_ptr = audioPtr;
    display_ptr = lcdPtr;
    game_state.started = true;
    clear_scores();

    trigger_down = 0;

    // Provide the method pointers of the event handlers, so they can get called by tag_communicator
    handles.on_hit =               on_hit;
    handles.on_clone =             on_clone;
    handles.on_respawn =           respawn;
    handles.on_admin_kill =        admin_kill;
    handles.on_full_health =       full_health;
    handles.on_clear_scores =      clear_scores;
    handles.on_start_game =        start_game;
    handles.on_instant_new_game =  new_game;
    handles.on_new_game =          new_game;
    handles.on_end_game =          end_game;
    handles.on_pause_unpause =     pause_unpause;
    handles.on_stun =              stunned;
    handles.on_init_player =       restart_gun; // Crashes the program and forces a restart of the teensy
    handles.on_test_sensors =      test_sensors;
    handles.on_add_health =        add_health;
    handles.on_add_rounds =        add_ammo;
    handles.on_flag_pickup =       flag_pickup;
    handles.on_clip_pickup =       clip_pickup;
    handles.on_health_pickup =     health_pickup;
    handles.on_reset_clock =       reset_clock;
    handles.on_full_armor =        full_armor;
    handles.on_disarm_player =     disarm_player;
    handles.on_stun =              stun_player;
    handles.on_explode =           explode_player;
    handles.on_restore_defaults =  restore_defaults;

    pass_handlers(&handles);

    pinMode(HIT_LED_PIN_NUMBER, HIT_LED_PIN_MODE);

    // Load the current configuration
    configure_from_clone(load_preset(0));
}

void restore_defaults() {
    configure_from_clone(load_preset(0));
}

/**
 * @brief Causes the tagger to explode
 */
void explode_player() {

}

void stun_player() {

}

void disarm_player() {
    game_state.disarmed = !game_state.disarmed;
}

void full_armor() {
    game_state.shield_health = game_state.currentConfig->armour_value;
    audio_ptr->play_sound(audio_interface::SOUND_CLONE_OK);
}

void reset_clock() {
    score_data.game_elapsed_time = 0;
    audio_ptr->play_sound(audio_interface::SOUND_CLONE_OK);
}

void health_pickup(uint8_t value) {
    if (game_state.health < game_state.currentConfig->respawn_health) {
        game_state.health += value;
        if (game_state.health > game_state.currentConfig->respawn_health) {
            game_state.health = game_state.currentConfig->respawn_health;
        }

    }
    audio_ptr->play_sound(audio_interface::SOUND_ADD_HEALTH);
}

void clip_pickup(uint8_t value) {
    if (game_state.clip_count < game_state.currentConfig->number_of_clips) {
        game_state.clip_count++;
    }
    audio_ptr->play_sound(audio_interface::SOUND_ADD_AMMO);
}

/**
 * @note Not Implemented
 * @param value Flag ID
 */
void flag_pickup(uint8_t value) {
//    audio_ptr.play_sound(audio_interface::SOUND_);
}

void add_ammo(uint8_t value) {
    game_state.ammo_count += value;
    if (game_state.ammo_count > game_state.currentConfig->clip_size) {
        game_state.ammo_count = game_state.currentConfig->clip_size;
    }
    audio_ptr->play_sound(audio_interface::SOUND_ADD_AMMO);
}

void add_health(uint8_t value) {
    game_state.health += mt2::health_lookup(static_cast<respawn_health>(value));
    if (game_state.health > game_state.currentConfig->respawn_health) {
        game_state.health = game_state.currentConfig->respawn_health;
    }
    audio_ptr->play_sound(audio_interface::SOUND_ADD_HEALTH);
}

