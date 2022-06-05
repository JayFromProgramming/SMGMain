//
// Created by Jay on 4/25/2022.
//

#include "audio_player.h"

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>

// GUItool: begin automatically generated code
AudioPlayMemory          memPlayer;      //xy=621,376
AudioAmplifier           amp;           //xy=802,374
AudioOutputI2S           board_out;      //xy=995,375
AudioConnection          patchCord1(memPlayer, amp);
AudioConnection          patchCord2(amp, 0, board_out, 0);
AudioConnection          patchCord3(amp, 0, board_out, 1);
// GUItool: end automatically generated code

#include "samples/index.h"

// Each audio memory block stores 128 samples or approximately 2.9 ms of audio.

namespace audio_player {

    void AudioPlayer::init() {
        AudioStopUsingSPI();
        AudioMemory(16);
        set_volume(0.5);
//        sample_locations = static_cast<unsigned int *>
//                (malloc(sizeof (unsigned int*) * total_samples));
//        in_memory = static_cast<bool *>
//                (malloc(sizeof(bool *) * total_samples));
//        for (int i = 0; i < total_samples; i++) {
//            sample_locations[i] = i;
//        }
//        load_sample_to_memory(0);
    }

    /**
     * Sets the output volume for all sounds
     * @param volume The volume to set the output to. (0.0 - 2.0)
     * @warning Any value above 0.5 may blow up the speaker. Use with caution.
     */
    void AudioPlayer::set_volume(float volume) {
        volume = volume * 0.27f; // Reduce the volume by 27% so we don't blow up the speaker.
        if (volume > 0.3) {
            volume = 0.3;
        }
        amp.gain(volume);
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

//    void AudioPlayer::load_sample_to_memory(u_int8_t sample_index) {
//        auto* sample = static_cast<unsigned int *>
//                (malloc(sample_sizes[sample_index] * sizeof(unsigned int)));
//        memcpy(sample, samples[sample_index], sample_sizes[sample_index]);
//        sample_locations[sample_index] = *sample;
//        in_memory[sample_index] = sample;
//    }

    AudioPlayer::AudioPlayer() = default;

} // audio_player