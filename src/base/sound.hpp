// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.

#pragma once
#include <map>

#include "cen_library.hpp"
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
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");
    rb_define_module_function(rb_mRGM_Ext, "sound_get_state",
                              wrapper::sound_get_state, 1);
    rb_define_module_function(rb_mRGM_Ext, "sound_get_channel",
                              wrapper::sound_get_channel, 1);
    // simple bindings
    base::ruby_wrapper w(this_worker);
    w.template create_sender<sound_create, uint64_t, const char*>(
        rb_mRGM_Ext, "sound_create");
    w.template create_sender<sound_dispose, uint64_t>(rb_mRGM_Ext,
                                                      "sound_dispose");
    w.template create_sender<sound_play, uint64_t, int>(rb_mRGM_Ext,
                                                        "sound_play");
    w.template create_sender<sound_stop, uint64_t>(rb_mRGM_Ext, "sound_stop");
    w.template create_sender<sound_fade_in, uint64_t, int>(rb_mRGM_Ext,
                                                           "sound_fade_in");
    w.template create_sender<sound_fade_out, uint64_t, int>(rb_mRGM_Ext,
                                                            "sound_fade_out");
    w.template create_sender<sound_set_volume, uint64_t, int>(
        rb_mRGM_Ext, "sound_set_volume");
    w.template create_sender<sound_set_pitch, uint64_t, int, bool>(
        rb_mRGM_Ext, "sound_set_pitch");
  }
};
}  // namespace rgm::base