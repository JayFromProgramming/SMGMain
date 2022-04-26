//
// Created by Jay on 4/25/2022.
//

#include "audio_player.h"

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>

// GUItool: begin automatically generated code
AudioPlayMemory          memPlayer;            //xy=415,90
AudioOutputI2S           board_out;           //xy=682.0000076293945,90.00000762939453
AudioConnection          patchCord1(memPlayer, 0, board_out, 0);
AudioConnection          patchCord2(memPlayer, 0, board_out, 1);
// GUItool: end automatically generated code

#include "samples/index.h"

// Each audio memory block stores 128 samples or approximately 2.9 ms of audio.

namespace audio_player {

    void AudioPlayer::init() {
        AudioStopUsingSPI();
        AudioMemory(16);
        sample_locations = static_cast<unsigned int *>
                (malloc(sizeof (unsigned int*) * total_samples));
        in_memory = static_cast<bool *>
                (malloc(sizeof(bool *) * total_samples));
        for (int i = 0; i < total_samples; i++) {
            sample_locations[i] = i;
        }
//        load_sample_to_memory(0);
    }

    void AudioPlayer::play(uint8_t index) {
        if (index >= total_samples) {
            return;
        }
        if (memPlayer.isPlaying()) {
            memPlayer.stop();
        }
        memPlayer.play(samples[index - 1]);
    }

    void AudioPlayer::load_sample_to_memory(u_int8_t sample_index) {
        auto* sample = static_cast<unsigned int *>
                (malloc(sample_sizes[sample_index] * sizeof(unsigned int)));
        memcpy(sample, samples[sample_index], sample_sizes[sample_index]);
        sample_locations[sample_index] = *sample;
        in_memory[sample_index] = sample;
    }

} // audio_player