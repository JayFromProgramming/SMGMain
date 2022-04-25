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

    }

    void AudioPlayer::play(unsigned char index) {
        if (memPlayer.isPlaying()) {
            memPlayer.stop();
        }
        memPlayer.play(samples[index - 1]);
    }

} // audio_player