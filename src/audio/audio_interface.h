//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_AUDIO_INTERFACE_H
#define SMGMAIN_AUDIO_INTERFACE_H


#define AUDIO Serial2

#define AUDIO_EOC 0xFF


#include "audio_player.h"

namespace audio_interface {

    enum sound_samples{
        SOUND_NONE,
        SOUND_SHOOT,
        SOUND_HIT,
        SOUND_DEATH,
        SOUND_PICKUP,
        SOUND_HEAL,
        SOUND_RELOAD,
        SOUND_RELOADED,
        SOUND_EMPTY,
        SOUND_BEEP
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

    private:
        audio_player::AudioPlayer *player;

    public:

        void init(){
            player = new audio_player::AudioPlayer();

        }

        void play_sound(sound_samples sound){
            player->play(sound);
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
