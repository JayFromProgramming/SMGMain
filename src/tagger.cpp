//
// Created by Jay on 4/18/2022.
//
#include <tagger.h>
#include "audio/audio_interface.h"
//#include "mt2Library/tag_communicator.h"
#include <eeprom_handler.h>
#include <Bounce.h>

tagger_state *game_state = nullptr;

event_handlers* handles = nullptr;

score_data *score_data_ptr = nullptr;

audio_interface::audio_interface *audio_ptr = nullptr;

uint32_t hit_led_timer = 0;
uint32_t muzzle_flash_timer = 0;

uint8_t current_flash_bulb_pin = MUZZLE_RED_FLASH_PIN_NUMBER;


void move_life_scores();

// 1 = Just pulled, 0 = Released, -1 = Not released but is being held
short trigger_down = 0; // Flag to indicate if the trigger_pull_interrupt has been called

FLASHMEM void configure_from_clone(mt2::clone* newClone){
    game_state->currentConfig = newClone;

    // The config firerate is stored as Rounds per minute, so we need to calculate the interval in milliseconds
    game_state->shot_interval = (mt2::fire_rate_table_lookup(game_state->currentConfig->cyclic_rpm) * 60) * 1000;

    game_state->hit_delay_ms = mt2::hit_delay_to_micros(game_state->currentConfig->hit_delay);

    game_state->max_health = mt2::health_lookup(game_state->currentConfig->respawn_health);
    game_state->health = game_state->max_health;
    game_state->clip_size = game_state->currentConfig->clip_size;
    game_state->clip_count = game_state->currentConfig->number_of_clips;
    game_state->ammo_count = game_state->clip_size;
    game_state->started = false;
}

void on_reload(){
    // Clear the interrupt flag
    if (game_state->reloading) {
        return;
    }

    if (game_state->clip_count >= 1){
        game_state->reloading = true;
        // Set the reload_time to the current time + the reload_time (in seconds)
        game_state->reload_time = millis() + (game_state->currentConfig->reload_time * 1000);
        audio_ptr->play_sound(audio_interface::SOUND_RELOAD);
    }
}

void shot_check(Bounce *bounce_ptr){

    if (bounce_ptr->fallingEdge()) {
        trigger_down = 1;
    } else if (bounce_ptr->risingEdge()) {
        trigger_down = 0; // Reset the trigger_down flag
    } else {
        if (trigger_down == 1) trigger_down = -1;
    }
    game_state->currentConfig->fire_selector = mt2::FIRE_MODE_AUTO;
    if (game_state->last_shot < game_state->shot_interval && !game_state->reloading) {
        if (game_state->ammo_count > 0) {
            // Check what fire mode we are in
            switch (game_state->currentConfig->fire_selector) {
                case mt2::FIRE_MODE_SINGLE: // Check if trigger_down is 1 and not -1
                    if (trigger_down == 1) {
                        if (shoot()) {
                            audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                            game_state->last_shot = 0;
                            game_state->ammo_count--;
                            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                            muzzle_flash_timer = millis() + 250;
                        }
                    }
                    break;
                case mt2::FIRE_MODE_BURST:
                    if (trigger_down == 1) {
                        if (shoot()) {
                            game_state->current_burst_count = game_state->currentConfig->burst_size;
                            audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                            game_state->last_shot = 0;
                            game_state->ammo_count--;
                            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                            muzzle_flash_timer = millis() + 250;
                        }
                    } else if (trigger_down == -1) {
                        if (game_state->current_burst_count > 0) {
                            if (shoot()) {
                                game_state->current_burst_count--;
                                audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                                game_state->last_shot = 0;
                                game_state->ammo_count--;
                                digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                                muzzle_flash_timer = millis() + 250;
                            }
                        }
                    }
                    break;
                case mt2::FIRE_MODE_AUTO:
                    if (trigger_down != 0) {
                        if (shoot()) { // If the IR transmitter is not busy preform shot actions
                            audio_ptr->play_sound(audio_interface::SOUND_SHOOT);
                            game_state->last_shot = 0;
                            game_state->ammo_count--;
                            digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_ACTIVE);
                            muzzle_flash_timer = millis() + 250;
                        }
                    }
                    break;
            }
        } else if (trigger_down == 1) {
            audio_ptr->play_sound(audio_interface::SOUND_EMPTY);
        }
    }
}

void on_killed(uint_least8_t killer_id) {
    score_data_ptr->killed_by_game[killer_id]++; // Increment the kills by player
    score_data_ptr->last_killed_by = killer_id; // Set the killed by player
    score_data_ptr->killer_name = mt2::get_player_name(score_data_ptr->last_killed_by); // Set the killer name

    audio_ptr->play_sound(audio_interface::SOUND_DEATH); // Make a scream of death

    // Calculate who assisted the killer (whoever did the most amount of damage to the player not including the killer)
    uint_least16_t max_damage = 0;
    uint_least8_t max_damage_id = 0;
    for (uint_least8_t i = 0; i < MT2_MAX_PLAYERS; i++) {
        if (i != killer_id) {
            if (score_data_ptr->damage_from_players_life[i] > max_damage) {
                max_damage = score_data_ptr->damage_from_players_life[i];
                max_damage_id = i;
            }
        }
    }

    move_life_scores();

}

// This section contains the event handlers for the game

void on_clone(mt2::clone* clone){
    save_preset(0, clone);
    configure_from_clone(clone);
}

// Called when the player is hit
void on_hit(uint_least8_t playerID, mt2::teams teamID, mt2::damage_table dmg){

    // Check if we are currently still in the hit delay period and if so, ignore the hit
    if (millis() - game_state->last_hit < game_state->hit_delay_ms) {
        return;
    }

    // Check if friendly fire is enabled, if so check if the hit was on a friendly team and if so ignore the hit
    if (game_state->currentConfig->game_bool_flags_1 & GAME_FRIENDLY_FIRE || teamID != game_state->currentConfig->team_id) {
        game_state->health = max(0, game_state->health - mt2::damage_table_lookup(dmg));
        game_state->last_hit = millis(); // Set the last hit time
        score_data_ptr->hits_from_players_life[playerID]++;;
        score_data_ptr->damage_from_players_life[playerID] +=
                mt2::damage_table_lookup(dmg);
        score_data_ptr->total_hits_life++;
    }

    // Check if we are dead
    if (game_state->health <= 0) {
        on_killed(playerID);
    } else {
        audio_ptr->play_sound(audio_interface::SOUND_HIT);
        digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_ACTIVE); // Turn on the hit led
        hit_led_timer = millis() + 250; // Set the hit led timer
    }
}
// Called when the player is killed, transfers score data from this life to the game score data
void move_life_scores(){
    for (uint_least8_t i = 0; i < MT2_MAX_PLAYERS; i++) {
        score_data_ptr->hits_from_players_game += score_data_ptr->hits_from_players_life[i];
        score_data_ptr->hits_from_players_life[i] = 0;
        score_data_ptr->damage_from_players_game[i] += score_data_ptr->damage_from_players_life[i];
        score_data_ptr->damage_from_players_life[i] = 0;
    }
    score_data_ptr->rounds_fired_game += score_data_ptr->rounds_fired_life;
    score_data_ptr->rounds_fired_life = 0;
    score_data_ptr->total_hits_game += score_data_ptr->total_hits_life;
    score_data_ptr->total_hits_life = 0;
    score_data_ptr->last_alive_time = score_data_ptr->alive_time;
    score_data_ptr->alive_time = 0;
    score_data_ptr->killer_name = nullptr;
    score_data_ptr->assist_name = nullptr;
}

// Clear all scoring data for the current game
void clear_scores(){
    // If no memory has been allocated to the score arrays, initialize them
    if (score_data_ptr->killed_by_game == nullptr) {
        score_data_ptr->killed_by_game = new uint_least16_t[MT2_MAX_PLAYERS];
    }
    if (score_data_ptr->hits_from_players_game == nullptr) {
        score_data_ptr->hits_from_players_game = new uint_least16_t[MT2_MAX_PLAYERS];
    }
    if (score_data_ptr->hits_from_players_life == nullptr) {
        score_data_ptr->hits_from_players_life = new uint_least16_t[MT2_MAX_PLAYERS];
    }
    if (score_data_ptr->damage_from_players_game == nullptr) {
        score_data_ptr->damage_from_players_game = new uint_least32_t[MT2_MAX_PLAYERS];
    }
    if (score_data_ptr->damage_from_players_life == nullptr) {
        score_data_ptr->damage_from_players_life = new uint_least32_t[MT2_MAX_PLAYERS];
    }

    for (uint_least8_t i = 0; i < MT2_MAX_PLAYERS; i++) {
        score_data_ptr->killed_by_game[i] = 0;
        score_data_ptr->hits_from_players_game[i] = 0;
        score_data_ptr->hits_from_players_life[i] = 0;
        score_data_ptr->damage_from_players_game[i] = 0;
        score_data_ptr->damage_from_players_life[i] = 0;
    }
    score_data_ptr->total_hits_game = 0;
    score_data_ptr->last_killed_by = 0;
    audio_ptr->play_sound(audio_interface::SOUND_BEEP);
}

void new_game(){
    game_state->health = game_state->max_health;
    game_state->last_shot = 0;
    game_state->last_hit = 0;
    clear_scores();
    audio_ptr->play_sound(audio_interface::SOUND_BEEP);
}

void end_game(){
    game_state->health = 0;
    game_state->last_shot = 0;
    game_state->last_hit = 0;
    audio_ptr->play_sound(audio_interface::SOUND_BEEP);
}

void start_game(){
    score_data_ptr->game_time = 0;
    clear_scores();
    game_state->health = game_state->max_health;
    game_state->last_shot = 0;
    game_state->last_hit = 0;

}

void full_health(){
    audio_ptr->play_sound(audio_interface::SOUND_HEAL);
    game_state->health = game_state->max_health;
}

void respawn(){ // Called when a player respawns
    // Called when a player respawns
    game_state->health = game_state->max_health;
    score_data_ptr->respawn_count++;
    score_data_ptr->last_killed_by = 0;
    game_state->clip_count = game_state->currentConfig->number_of_clips;
    game_state->last_shot = 0;
    game_state->last_hit = 0;
    game_state->ammo_count = game_state->clip_size;
    game_state->reloading = false;
    game_state->reload_time = 0;
    score_data_ptr->respawn_time = millis();
}

void admin_kill(){ // Called when an admin kills a player
    game_state->health = 0;
    score_data_ptr->last_killed_by = GAME_ADMIN_ID;
}

void pause_unpause(){
    if (game_state->paused){
        game_state->paused = false;
        audio_ptr->play_sound(audio_interface::SOUND_BEEP);
    } else {
        game_state->paused = true;
        audio_ptr->play_sound(audio_interface::SOUND_BEEP);
    }
}

void stunned(){
    game_state->last_shot = millis() + 3000; // Set the last shot time to 3 seconds in the future to prevent shooting
}

void test_sensors(){
    digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_ACTIVE);
    hit_led_timer = millis() + 2000;
}

void restart_gun(){
    SCB_AIRCR = 0x05FA0004; // Reset the MCU
}

// End event handlers

void start(){
    // To start the game the player must hold the trigger for 2 seconds

}

void tagger_loop(){
    // Called by loop() this is where main code logic goes

    if (game_state->reloading) {
        // If we are reloading, check if we are done
        if (millis() >= game_state->reload_time) {
            // We are done reloading
            game_state->reloading = false;
            // Reset the reload_time
            game_state->reload_time = 0l;

            // Reload the ammo
            game_state->ammo_count = game_state->currentConfig->clip_size;
            if ((game_state->currentConfig->game_bool_flags_1 & GAME_UNLIMITED_CLIPS) == false) {
                game_state->clip_count--;
            }
        }
    }

    if (hit_led_timer < millis() && hit_led_timer != 0) {
        digitalWriteFast(HIT_LED_PIN_NUMBER, HIT_LED_PIN_INACTIVE);
        hit_led_timer = 0;
    }

    if (muzzle_flash_timer < millis() && muzzle_flash_timer != 0) {
        digitalWriteFast(current_flash_bulb_pin, MUZZLE_FLASH_INACTIVE);
        muzzle_flash_timer = 0;
    }

    // Check for IR input
    signalScan();
}

FLASHMEM tagger_state* get_tagger_data_ptr(){
    return game_state;
}

FLASHMEM event_handlers* get_event_handler_ptr(){
    return handles;
}

FLASHMEM score_data* get_score_data_ptr(){
    return score_data_ptr;
}

FLASHMEM void tagger_init(audio_interface::audio_interface* audioPtr){
    IR_init();
    audio_ptr = audioPtr;
    game_state = new tagger_state();

    score_data_ptr = new score_data();
    clear_scores();

    handles = get_handlers();
    trigger_down = 0;

    // Provide the method pointers of the event handlers, so they can get called by tag_communicator
    handles->on_hit =           on_hit;
    handles->on_clone =         on_clone;
    handles->on_respawn =       respawn;
    handles->on_admin_kill =    admin_kill;
    handles->on_full_health =   full_health;
    handles->on_clear_scores =  clear_scores;
    handles->on_start_game =    start_game;
    handles->on_new_game =      new_game;
    handles->on_end_game =      end_game;
    handles->on_pause_unpause = pause_unpause;
    handles->on_stun =          stunned;
    handles->on_init_player =   restart_gun; // Crashes the program and forces a restart of the teensy
    handles->on_test_sensors =  test_sensors;

//    pinMode(TRIGGER_PIN_NUMBER, TRIGGER_PIN_MODE);
//    pinMode(RELOAD_PIN_NUMBER, RELOAD_PIN_MODE);

    pinMode(HIT_LED_PIN_NUMBER, HIT_LED_PIN_MODE);

    // Attach interrupts on the trigger and reload pins
//    attachInterrupt(digitalPinToInterrupt(RELOAD_PIN_NUMBER), reload_interrupt, RELOAD_INTERRUPT_MODE);

    #ifdef USE_INTERRUPT_ON_TRIGGER
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN_NUMBER), trigger_interrupt, TRIGGER_INTERRUPT_MODE);
    #endif

    // Load the current configuration
    configure_from_clone(load_preset(0));


}

