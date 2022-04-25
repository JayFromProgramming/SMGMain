//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_AUDIO_INTERFACE_H
#define SMGMAIN_AUDIO_INTERFACE_H


#define AUDIO Serial2

#define AUDIO_EOC 0xFF

#include <Arduino.h>

namespace sounds {

    enum sound_samples{
        SOUND_NONE = 0,
        SOUND_SHOOT = 1,
        SOUND_HIT = 2,
        SOUND_DEATH = 3,
        SOUND_PICKUP = 4,
        SOUND_HEAL = 5,
        SOUND_RELOAD = 6,
        SOUND_EMPTY = 7,
        SOUND_BEEP = 8,
    };

    enum actions{
        ACTION_STOP = 0,
        ACTION_PLAY_SOUND = 1,
        ACTION_PLAY_SOUND_LOOP = 2,
        ACTION_VOLUME = 3,
        ACTION_CHANGE_SOUND_SET = 4,
    };

    enum sound_sets{
        AUDIO_SOUND_SET_MILSIM = 0,
        AUDIO_SOUND_SET_SIFI = 1,
        AUDIO_SOUND_SET_SILENCED = 2
    };


    class audio_interface {

    public:

        static void init(){
            AUDIO.begin(9600);
            AUDIO.write(ACTION_STOP);
            AUDIO.write(AUDIO_EOC);
        }

        static void play_sound(sound_samples sound){
            AUDIO.write(ACTION_PLAY_SOUND);
            AUDIO.write(sound);
            AUDIO.write(AUDIO_EOC);
        }

        FLASHMEM static void change_sound_set(sound_sets sound_set){
            AUDIO.write(ACTION_CHANGE_SOUND_SET);
            AUDIO.write(sound_set);
            AUDIO.write(AUDIO_EOC);
        }

        FLASHMEM static void change_volume(int volume){
            AUDIO.write(ACTION_VOLUME);
            AUDIO.write(volume);
            AUDIO.write(AUDIO_EOC);
        }
    };

} // audio_interface

#endif //SMGMAIN_AUDIO_INTERFACE_H
