//
// Created by Jay on 4/18/2022.
//

#ifndef SMGMAIN_TAGGER_H
#define SMGMAIN_TAGGER_H

#define TRIGGER_PIN_NUMBER 2
#define TRIGGER_PIN_MODE INPUT_PULLDOWN
#define TRIGGER_PIN_ACTIVE HIGH
#define TRIGGER_PIN_RELEASED LOW
#define TRIGGER_INTERRUPT_MODE RISING

#define RELOAD_PIN_NUMBER 3
#define RELOAD_PIN_MODE INPUT_PULLUP
#define RELOAD_INTERRUPT_MODE RISING

#define HIT_LED_PIN_NUMBER 4
#define HIT_LED_PIN_MODE OUTPUT
#define HIT_LED_PIN_ACTIVE HIGH
#define HIT_LED_PIN_RELEASED LOW

#define IR_RECEIVER_PIN_NUMBER 5

#include <Arduino.h>

#include <mt2Library/mt2_protocol.h>

struct tagger_state {
    mt2::clone *currentConfig = nullptr;
    uint32_t last_shot = 0;
    uint32_t last_hit = 0;
    long shot_interval = 0;
    unsigned short max_health = 0;
    volatile unsigned short health = 0;
    unsigned char player_id = 0;
    volatile unsigned char ammo_count = 0;
    unsigned char clip_count = 0;
    unsigned char current_burst_count = 0;
    volatile bool reloading = false;
    volatile unsigned long reload_time = 0.0;
};

void tagger_init();

void tagger_loop();

tagger_state* get_tagger_data_ptr();

#endif //SMGMAIN_TAGGER_H
