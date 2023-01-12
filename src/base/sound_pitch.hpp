// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.

/*
 * sound_pitching_example.cpp
 *
 *  Created on: 27 de dez de 2017
 *      Author: Carlos Faruolo
 */
#pragma once
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "core/core.hpp"

namespace rgm::base {
struct sound_pitch {
  static Uint16 audioFormat;  // current audio format constant
  static int audioFrequency;  // frequency rate of the current audio format
  static int
      audioChannelCount;  // number of channels of the current audio format
  static int
      audioAllocatedMixChannelsCount;  // number of mix channels allocated

  static inline Uint16 formatSampleSize(Uint16 format) {
    return (format & 0xFF) / 8;
  }

  // Get chunk time length (in ms) given its size and current audio format
  static int computeChunkLengthMillisec(int chunkSize) {
    // bytes / samplesize == sample points
    const Uint32 points = chunkSize / formatSampleSize(audioFormat);
    // sample points / channels == sample frames
    const Uint32 frames = (points / audioChannelCount);
    // (sample frames * 1000) / frequency == play length, in ms
    return ((frames * 1000) / audioFrequency);
  }

  // Custom handler object to control which part of the Mix_Chunk's audio data
  // will be played, with which pitch-related modifications. This needed to be a
  // template because the actual Mix_Chunk's data format may vary (AUDIO_U8,
  // AUDIO_S16, etc) and the data type varies with it (Uint8, Sint16, etc). The
  // AudioFormatType should be the data type that is compatible with the current
  // SDL_mixer-initialized audio format.
  template <typename AudioFormatType>
  struct PlaybackSpeedEffectHandler {
    const AudioFormatType* const
        chunkData;             // pointer to the chunk sample data (as array)
    const float* speedFactor;  // the playback speed factor
    float position;            // current position of the sound, in ms
    const int duration;        // the duration of the sound, in ms
    // the size of the sound, as a number of indexes (or sample points).
    // thinks of this as a array size when using the proper array type (instead
    // of just Uint8*).
    const int chunkSize;
    // flags whether playback should stay looping
    const bool loop;
    // flags whether playback should be halted by this callback when playback is
    // finished
    const bool attemptSelfHalting;
    // true if this playback has been pitched by this handler
    bool altered;

    PlaybackSpeedEffectHandler(const Mix_Chunk& chunk, const float* speed,
                               bool loop, bool trySelfHalt)
        : chunkData(reinterpret_cast<AudioFormatType*>(chunk.abuf)),
          speedFactor(speed),
          position(0),
          duration(computeChunkLengthMillisec(chunk.alen)),
          chunkSize(chunk.alen / formatSampleSize(audioFormat)),
          loop(loop),
          attemptSelfHalting(trySelfHalt),
          altered(false) {}

    // processing function to be able to change chunk speed/pitch.
    void modifyStreamPlaybackSpeed(int mixChannel, void* stream, int length) {
      AudioFormatType* buffer = static_cast<AudioFormatType*>(stream);
      const int bufferSize =
          length / sizeof(AudioFormatType);  // buffer size (as array)
      const float speedFactor =
          *this->speedFactor;  // take a "snapshot" of speed factor

      // if there is still sound to be played
      if (position < duration || loop) {
        // normal duration of each sample
        const float delta = 1000.0 / audioFrequency;
        // virtual stretched duration, scaled by 'speedFactor'
        const float vdelta = delta * speedFactor;

        // if playback is unaltered and pitch is required (for the first time)
        if (!altered && speedFactor != 1.0f) {
          // flags playback modification and proceed to the pitch routine.
          altered = true;
        }

        if (altered)  // if unaltered, this pitch routine is skipped
        {
          for (int i = 0; i < bufferSize; i += audioChannelCount) {
            // j goes from 0 to size/channelCount, incremented 1 by 1
            const int j = i / audioChannelCount;
            // get "virtual" index. its corresponding value will be
            // interpolated.
            const float x = position + j * vdelta;
            // get left index to interpolate from original chunk data
            // (right index will be this plus 1)
            const int k = floor(x / delta);
            // get the proportion of the right value (left will be 1.0 minus
            // this)
            const float prop = (x / delta) - k;

            // usually just 2 channels: 0 (left) and 1 (right), but who knows...
            for (int c = 0; c < audioChannelCount; c++) {
              // check if k will be within bounds
              if (k * audioChannelCount + audioChannelCount - 1 < chunkSize ||
                  loop) {
                AudioFormatType v0 =
                    chunkData[(k * audioChannelCount + c) % chunkSize];
                AudioFormatType v1 =
                    chunkData[((k + 1) * audioChannelCount + c) % chunkSize];

                // put interpolated value on 'data'
                // linear interpolation
                buffer[i + c] = (1 - prop) * v0 + prop * v1;
              } else {
                // if k will be out of bounds (chunk bounds),
                // it means we already finished;
                // thus, we'll pass silence
                buffer[i + c] = 0;
              }
            }
          }
        }
        // update position
        position += (bufferSize / audioChannelCount) * vdelta;
        // reset position if looping
        if (loop) {
          while (position > duration) {
            position -= duration;
          }
        }
      }
      // if we already played the whole sound but finished earlier than expected
      // by SDL_mixer (due to faster playback speed)
      else {
        // set silence on the buffer since Mix_HaltChannel() poops out some of
        // it for a few ms.
        for (int i = 0; i < bufferSize; i++) {
          buffer[i] = 0;
        }

        if (attemptSelfHalting) {
          // XXX unsafe call, since it locks audio;
          // but no safer solution was found yet...
          Mix_HaltChannel(mixChannel);
        }
      }
    }

    // Mix_EffectFunc_t callback that redirects to handler method (handler
    // passed via userData)
    static void mixEffectFuncCallback(int channel, void* stream, int length,
                                      void* userData) {
      static_cast<PlaybackSpeedEffectHandler*>(userData)
          ->modifyStreamPlaybackSpeed(channel, stream, length);
    }

    // Mix_EffectDone_t callback that deletes the handler at the end of the
    // effect usage (handler passed via userData)
    static void mixEffectDoneCallback(int, void* userData) {
      delete static_cast<PlaybackSpeedEffectHandler*>(userData);
    }

    // function to register a handler to this channel for the next playback.
    static void registerEffect(int channel, const Mix_Chunk& chunk,
                               const float* speed, bool loop,
                               bool trySelfHalt) {
      Mix_RegisterEffect(
          channel, mixEffectFuncCallback, mixEffectDoneCallback,
          new PlaybackSpeedEffectHandler(chunk, speed, loop, trySelfHalt));
    }
  };

  // Register playback speed effect handler according to the current audio
  // format; effect valid for a single playback; if playback is looped, lasts
  // until it's halted.
  static void setupPlaybackSpeedEffect(const Mix_Chunk* const chunk,
                                       const float* speed, int channel,
                                       bool loop = false,
                                       bool trySelfHalt = false) {
    // select the register function for the current audio format
    // and register the effect using the compatible handlers
    // XXX is it correct to behave the same way to all S16 and U16 formats?
    // Should we create case statements for AUDIO_S16SYS, AUDIO_S16LSB,
    // AUDIO_S16MSB, etc, individually?
    switch (audioFormat) {
#define BRANCH(type)                                                       \
  PlaybackSpeedEffectHandler<type>::registerEffect(channel, *chunk, speed, \
                                                   loop, trySelfHalt)
      case AUDIO_U8:
        BRANCH(Uint8);
        break;
      case AUDIO_S8:
        BRANCH(Sint8);
        break;
      case AUDIO_U16:
        BRANCH(Uint16);
        break;
      default:
      case AUDIO_S16:
        BRANCH(Sint16);
        break;
      case AUDIO_S32:
        BRANCH(Sint32);
        break;
      case AUDIO_F32:
        BRANCH(float);
        break;
#undef BRANCH
    }
  }

  // setup and bind
  static void setup() {
    // query specs
    Mix_QuerySpec(&audioFrequency, &audioFormat, &audioChannelCount);
    audioAllocatedMixChannelsCount = Mix_AllocateChannels(MIX_CHANNELS);
  }
};
Uint16 sound_pitch::audioFormat;
int sound_pitch::audioFrequency;
int sound_pitch::audioChannelCount;
int sound_pitch::audioAllocatedMixChannelsCount;
}  // namespace rgm::base