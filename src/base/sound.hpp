// zlib License
//
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include "core/core.hpp"
#include "detail.hpp"
#include "ruby_wrapper.hpp"
#include "sound_pitch.hpp"

namespace rgm::base {
using sounds = std::map<uint64_t, cen::sound_effect>;
using sound_speeds = std::array<float, 32>;

struct sound_create {
  using data = std::tuple<sounds>;

  uint64_t id;
  const char* path;

  void run(auto& worker) {
    sounds& data = RGMDATA(sounds);
    data.emplace(id, cen::sound_effect(path));
  }
};

struct sound_dispose {
  uint64_t id;

  void run(auto& worker) {
    sounds& data = RGMDATA(sounds);
    data.erase(id);
  }
};

struct sound_play {
  uint64_t id;
  int iteration;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.play(iteration);
  }
};

struct sound_stop {
  uint64_t id;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.stop();
  }
};

struct sound_fade_in {
  uint64_t id;
  int duration;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.fade_in(cen::music::ms_type{duration});
  }
};

struct sound_fade_out {
  uint64_t id;
  int duration;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.fade_out(cen::music::ms_type{duration});
  }
};

struct sound_set_volume {
  uint64_t id;
  int volume;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.set_volume(volume);
  }
};

struct sound_set_pitch {
  using data = std::tuple<sound_speeds>;

  uint64_t id;
  int pitch;
  bool loop;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);

    auto channel = s.channel();
    if (channel.has_value()) {
      int index = channel.value();
      float* p_speed = &RGMDATA(sound_speeds).at(index);
      *p_speed = pitch / 100.f;
      sound_pitch::setupPlaybackSpeedEffect(s.get(), p_speed, index, loop);
    }
  }
};

struct sound_get_state {
  uint64_t id;
  int* p_state;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    *p_state = 0;
    if (s.is_playing()) p_state += 1;
    if (s.is_fading()) p_state += 2;
  }
};

struct sound_get_channel {
  uint64_t id;
  int* channel;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    *channel = s.channel().value_or(-1);
  }
};

struct init_sound {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE sound_get_state(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        int state;
        worker >> base::sound_get_state{id, &state};
        return INT2FIX(state);
      }

      static VALUE sound_get_channel(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        int channel;
        worker >> base::sound_get_channel{id, &channel};
        return INT2FIX(channel);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "sound_get_state",
                              wrapper::sound_get_state, 1);
    rb_define_module_function(rb_mRGM_Base, "sound_get_channel",
                              wrapper::sound_get_channel, 1);

    RGMBIND(rb_mRGM_Base, "sound_create", sound_create, 2);
    RGMBIND(rb_mRGM_Base, "sound_dispose", sound_dispose, 1);
    RGMBIND(rb_mRGM_Base, "sound_play", sound_play, 2);
    RGMBIND(rb_mRGM_Base, "sound_stop", sound_stop, 1);
    RGMBIND(rb_mRGM_Base, "sound_fade_in", sound_fade_in, 2);
    RGMBIND(rb_mRGM_Base, "sound_fade_out", sound_fade_out, 2);
    RGMBIND(rb_mRGM_Base, "sound_set_volume", sound_set_volume, 2);
    RGMBIND(rb_mRGM_Base, "sound_set_pitch", sound_set_pitch, 3);
  }
};
}  // namespace rgm::base