//
// Created by Jay on 4/25/2022.
//

#ifndef SMGMAIN_AUDIO_PLAYER_H
#define SMGMAIN_AUDIO_PLAYER_H

#include <cstdio>

namespace audio_player {

    class AudioPlayer {

    private:
//        unsigned int* sample_locations;
//        bool* in_memory; // An array of booleans that indicate whether the sample is in memory

//        void load_sample_to_memory(u_int8_t index);

    public:
        void init();
        static void play(unsigned char index);
        AudioPlayer ();

        static void set_volume(float volume);
    };

} // audio_player

#endif //SMGMAIN_AUDIO_PLAYER_H
