//
// Created by Jay on 4/18/2022.
//
#include <tagger.h>
#include <audio_interface.h>
#include "mt2Library/tag_communicator.h"
#include <eeprom_handler.h>

tagger_state *tagger_data = nullptr;

// 1 = Just pulled, 0 = Released, -1 = Not released but is being held
short trigger_down = 0; // Flag to indicate if the trigger_pull_interrupt has been called

FLASHMEM void configure_from_clone(mt2::clone* newClone){
    tagger_data->currentConfig = newClone;

    // The config firerate is stored as Rounds per minute, so we need to calculate the interval in milliseconds
    tagger_data->shot_interval = (60 / tagger_data->currentConfig->cyclic_rpm) * 1000;

    tagger_data->max_health = mt2::heath_lookup(tagger_data->currentConfig->respawn_health);
    tagger_data->health = tagger_data->max_health;
}

void reload_interrupt(){
    // Clear the interrupt flag
    cli();
    if (tagger_data->reloading) {
        sei();
        return;
    }

    if (tagger_data->clip_count >= 1){
        tagger_data->reloading = true;
        // Set the reload_time to the current time + the reload_time (in seconds)
        tagger_data->reload_time = millis() + tagger_data->currentConfig->reload_time * 1000;
        sounds::audio_interface::play_sound(sounds::SOUND_RELOAD);
    }

    // Re-enable interrupts
    sei();
}

void shot_check(){

    uint8_t trigger_state = digitalReadFast(TRIGGER_PIN_NUMBER);

    if (trigger_state == TRIGGER_PIN_ACTIVE) {
        if (trigger_down == 0) {
            trigger_down = 1; // Just pulled
        }
    } else {
        trigger_down = 0; // Reset the trigger_down flag
    }

    if (tagger_data->last_shot + tagger_data->shot_interval < millis() && !tagger_data->reloading) {
        if (tagger_data->ammo_count > 0) {
            // Check what fire mode we are in
            switch (tagger_data->currentConfig->fire_selector) {
                case mt2::FIRE_MODE_SINGLE: // Check if trigger_down is 1 and not -1
                    if (trigger_down == 1) {
                        trigger_down = -1;
                        sounds::audio_interface::play_sound(sounds::SOUND_SHOOT);
                        shoot();
                        tagger_data->last_shot = millis();
                    }
                    break;
                case mt2::FIRE_MODE_BURST:
                    if (trigger_down == 1) {
                        tagger_data->current_burst_count = tagger_data->currentConfig->burst_size;
                        trigger_down = -1;
                        sounds::audio_interface::play_sound(sounds::SOUND_SHOOT);
                        shoot();
                        tagger_data->last_shot = millis();
                    } else if (trigger_down == -1) {
                        if (tagger_data->current_burst_count > 0) {
                            tagger_data->current_burst_count--;
                            sounds::audio_interface::play_sound(sounds::SOUND_SHOOT);
                            shoot();
                            tagger_data->last_shot = millis();
                        }
                    }
                    break;
                case mt2::FIRE_MODE_AUTO:
                    if (trigger_down != 0) {
                        trigger_down = -1;
                        sounds::audio_interface::play_sound(sounds::SOUND_SHOOT);
                        shoot();
                        tagger_data->last_shot = millis();
                    }
                    break;
            }
        } else if (trigger_down == 1) {
            trigger_down = -1;
            sounds::audio_interface::play_sound(sounds::SOUND_EMPTY);
        }
    }
}

void trigger_interrupt(){
    cli();
    shot_check();
    sei();
}

void tagger_loop(){
    // Called by loop() this is where main code logic goes

    if (tagger_data->reloading) {
        // If we are reloading, check if we are done
        if (millis() >= tagger_data->reload_time) {
            // We are done reloading
            tagger_data->reloading = false;
            // Reset the reload_time
            tagger_data->reload_time = 0l;

            // Reload the ammo
            tagger_data->ammo_count = tagger_data->currentConfig->clip_size;
            if ((tagger_data->currentConfig->game_bool_flags_1 & GAME_UNLIMITED_CLIPS) == false) {
                tagger_data->clip_count--;
            }
        }
    }

    shot_check(); // Check if user wants to shoot


    // Check for IR input
    signalScan();
}

// This section contains the event handlers for the game

void on_clone(mt2::clone* clone){
    save_preset(0, clone);
    configure_from_clone(clone);
}

void on_hit(uint_least8_t playerID, uint_least8_t teamID, uint_least8_t dmg){

    if (millis() - tagger_data->last_hit < tagger_data->currentConfig->hit_delay) {
        return;
    }

    // Called when a player is hit
    if (tagger_data->currentConfig->game_bool_flags_1 & GAME_FRIENDLY_FIRE){

    } else if (teamID != tagger_data->currentConfig->team_id) {
        // If friendly fire is disabled and the player is on our team, don't take damage
        return;
    }
}

void on_respawn(){

}

void on_admin_kill(){

}


// End event handlers

FLASHMEM tagger_state* get_tagger_data_ptr(){
    return tagger_data;
}


FLASHMEM void tagger_init(){
    IR_init();
    tagger_data = new tagger_state();
    event_handlers* handles = get_handlers();

    handles->on_hit = on_hit;
    handles->on_clone = on_clone;
    handles->on_respawn = on_respawn;
    handles->on_admin_kill = on_admin_kill;

    pinMode(TRIGGER_PIN_NUMBER, TRIGGER_PIN_MODE);
    pinMode(RELOAD_PIN_NUMBER, RELOAD_PIN_MODE);

    // Attach interrupts on the trigger and reload pins
    attachInterrupt(digitalPinToInterrupt(RELOAD_PIN_NUMBER), reload_interrupt, RELOAD_INTERRUPT_MODE);

    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN_NUMBER), trigger_interrupt, TRIGGER_INTERRUPT_MODE);

    // Load the current configuration
    configure_from_clone(load_preset(0));

}

