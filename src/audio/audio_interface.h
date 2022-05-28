//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_AUDIO_INTERFACE_H
#define SMGMAIN_AUDIO_INTERFACE_H

#include "audio_player.h"

namespace audio_interface {

    enum sound_samples{
        SOUND_NONE,
        SOUND_SHOOT,
        SOUND_HIT,
        SOUND_DEATH,
        SOUND_HEAL,
        SOUND_RELOAD,
        SOUND_RELOADED,
        SOUND_EMPTY,
        SOUND_BEEP,
        SOUND_CLONE_OK,
        SOUND_EXPLOSION,
        SOUND_ADD_HEALTH,
        SOUND_ADD_AMMO,
        SOUND_ADD_SHIELD,
        SOUND_SHIELD_HIT,
        SOUND_SENSOR_FAIL,
        SOUND_LOW_BATTERY,
        SOUND_ZOMBIE_1,
        SOUND_ZOMBIE_2,
        SOUND_STUNNED,
        SOUND_BUZZ,

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
        sound_sets current_sound_set;

    public:

        void init(){
            player = new audio_player::AudioPlayer();

        }

        void play_sound(sound_samples sound){
            switch (current_sound_set){
                case AUDIO_SOUND_SET_MILSIM:
                    switch(sound){
                        case SOUND_SHOOT: player->play(0); break;
                        case SOUND_HIT: player->play(5); break;
                        case SOUND_DEATH: player->play(6); break;
                        case SOUND_HEAL: player->play(10); break;
                        case SOUND_RELOAD: player->play(2); break;
                        case SOUND_RELOADED: player->play(3); break;
                        case SOUND_EMPTY: player->play(1); break;
                        case SOUND_BEEP: player->play(8); break;
                        case SOUND_SENSOR_FAIL: player->play(19); break;
                        case SOUND_LOW_BATTERY: player->play(26); break;
                        case SOUND_ZOMBIE_1: player->play(27); break;
                        case SOUND_ZOMBIE_2: player->play(28); break;
                        case SOUND_STUNNED: player->play(29); break;
                        case SOUND_BUZZ: player->play(30); break;
                        case SOUND_EXPLOSION: player->play(14); break;
                        case SOUND_ADD_HEALTH: player->play(10); break;
                        case SOUND_ADD_AMMO: player->play(12); break;
                        case SOUND_ADD_SHIELD: player->play(13); break;
                        case SOUND_SHIELD_HIT: player->play(15); break;
                        case SOUND_CLONE_OK: player->play(18); break;
                        default: break;
                    }
                    break;
                case AUDIO_SOUND_SET_SIFI:
                    switch(sound){
                        case SOUND_SHOOT: player->play(0); break;
                        case SOUND_HIT: player->play(5); break;
                        case SOUND_DEATH: player->play(6); break;
                        case SOUND_HEAL: player->play(10); break;
                        case SOUND_RELOAD: player->play(2); break;
                        case SOUND_RELOADED: player->play(3); break;
                        case SOUND_EMPTY: player->play(1); break;
                        case SOUND_BEEP: player->play(18); break;
                        default: break;
                    }
                    break;
                case AUDIO_SOUND_SET_SILENCED:
                    switch (sound) {
                        case SOUND_SHOOT: player->play(0); break;
                        case SOUND_HIT: player->play(5); break;
                        case SOUND_DEATH: player->play(6); break;
                        case SOUND_HEAL: player->play(10); break;
                        case SOUND_RELOAD: player->play(2); break;
                        case SOUND_RELOADED: player->play(3); break;
                        case SOUND_EMPTY: player->play(1); break;
                        case SOUND_BEEP: player->play(18); break;
                        default: break;
                    }
                    break;
            }
        }

    };

} // audio_interface

#endif //SMGMAIN_AUDIO_INTERFACE_H
