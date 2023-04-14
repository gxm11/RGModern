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

namespace rgm::base {
using musics = std::map<uint64_t, cen::music>;

struct music_finish_callback {
  void run(auto&) {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_funcall(rb_mRGM_Base, rb_intern("music_finish_callback"), 0);
  }
};

struct music_create {
  using data = std::tuple<musics>;

  uint64_t id;
  const char* path;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.emplace(id, cen::music(path));
  }
};

struct music_dispose {
  uint64_t id;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.erase(id);
  }
};

struct music_play {
  uint64_t id;
  int iteration;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.at(id).play(iteration);

    static decltype(auto) this_worker(worker);
    struct wrapper {
      static void callback() { this_worker >> music_finish_callback{}; }
    };

    Mix_HookMusicFinished(wrapper::callback);
  }
};

struct music_fade_in {
  uint64_t id;
  int iteration;
  int duration;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.at(id).fade_in(cen::music::ms_type{duration}, iteration);

    static decltype(auto) this_worker(worker);
    struct wrapper {
      static void callback() { this_worker >> music_finish_callback{}; }
    };

    Mix_HookMusicFinished(wrapper::callback);
  }
};

struct music_get_position {
  uint64_t id;
  double* p_position;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    *p_position = data.at(id).position().value_or(-1);
  }
};

struct music_set_volume {
  int volume;

  void run(auto&) { cen::music::set_volume(volume); }
};

struct music_set_position {
  double position;

  void run(auto&) { cen::music::set_position(position); }
};

struct music_resume {
  void run(auto&) { cen::music::resume(); }
};

struct music_pause {
  void run(auto&) { cen::music::pause(); }
};

struct music_halt {
  void run(auto&) { cen::music::halt(); }
};

struct music_rewind {
  void run(auto&) { cen::music::rewind(); }
};

struct music_fade_out {
  int duration;

  void run(auto&) { cen::music::fade_out(cen::music::ms_type{duration}); }
};

struct music_get_volume {
  int* p_volume;

  void run(auto&) { *p_volume = cen::music::volume(); }
};

struct music_get_state {
  int* p_state;

  void run(auto&) {
    *p_state = 0;
    if (cen::music::is_fading()) *p_state += 1;
    if (cen::music::is_fading_in()) *p_state += 2;
    if (cen::music::is_fading_out()) *p_state += 4;
    if (cen::music::is_paused()) *p_state += 8;
    if (cen::music::is_playing()) *p_state += 16;
  }
};

struct init_music {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE music_get_state(VALUE) {
        int state;
        worker >> base::music_get_state{&state};
        RGMWAIT(2);
        return INT2FIX(state);
      }

      static VALUE music_get_volume(VALUE) {
        int volume;
        worker >> base::music_get_volume{&volume};
        RGMWAIT(2);
        return INT2FIX(volume);
      }

      static VALUE music_get_position(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        double position;
        worker >> base::music_get_position{id, &position};
        RGMWAIT(2);
        return DBL2NUM(position);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "music_get_state",
                              wrapper::music_get_state, 0);
    rb_define_module_function(rb_mRGM_Base, "music_get_volume",
                              wrapper::music_get_volume, 0);
    rb_define_module_function(rb_mRGM_Base, "music_get_position",
                              wrapper::music_get_position, 1);

    RGMBIND(rb_mRGM_Base, "music_create", music_create, 2);
    RGMBIND(rb_mRGM_Base, "music_dispose", music_dispose, 1);
    RGMBIND(rb_mRGM_Base, "music_play", music_play, 2);
    RGMBIND(rb_mRGM_Base, "music_fade_in", music_fade_in, 3);
    RGMBIND(rb_mRGM_Base, "music_set_volume", music_set_volume, 1);
    RGMBIND(rb_mRGM_Base, "music_set_position", music_set_position, 1);
    RGMBIND(rb_mRGM_Base, "music_resume", music_resume, 0);
    RGMBIND(rb_mRGM_Base, "music_pause", music_pause, 0);
    RGMBIND(rb_mRGM_Base, "music_halt", music_halt, 0);
    RGMBIND(rb_mRGM_Base, "music_rewind", music_rewind, 0);
    RGMBIND(rb_mRGM_Base, "music_fade_out", music_fade_out, 1);
  }
};
}  // namespace rgm::base