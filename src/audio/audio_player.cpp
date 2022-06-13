//
// Created by Jay on 4/25/2022.
//

#include "audio_player.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


// GUItool: begin automatically generated code
AudioPlayMemory          memPlayer;      //xy=621,376
AudioAmplifier           amp;           //xy=802,374
AudioOutputI2S2          board_out;         //xy=1012,371
AudioConnection          patchCord1(memPlayer, amp);
AudioConnection          patchCord2(amp, 0, board_out, 0);
AudioConnection          patchCord3(amp, 0, board_out, 1);
// GUItool: end automatically generated code

#include "samples/index.h"

// Each audio memory block stores 128 samples or approximately 2.9 ms of audio.
// 2.9 * 32 = 96.8 ms of audio. Which should get through a shot cycle. (~20.2 ms)

namespace audio_player {

    void AudioPlayer::init() {
        AudioStopUsingSPI();
        AudioMemory(32);
        set_volume(0.5);
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

    AudioPlayer::AudioPlayer() = default;

} // audio_player