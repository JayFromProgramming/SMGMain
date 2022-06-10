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
#include <lcdDisplay/lcdDriver.h>

#include <mt2Library/state_structs.h>

void shot_check(Bounce *bounce);

void on_reload();

void on_fire_select(bool value);

void on_mode_select();

void on_held_trigger();

void tagger_init(audio_interface::audio_interface* audio_interface, display::lcdDriver *lcdPtr);

void tagger_loop();

tagger_state_struct* get_tagger_data_ptr();

event_handlers* get_event_handler_ptr();

score_data_struct* get_score_data_ptr();

#endif //SMGMAIN_TAGGER_H
