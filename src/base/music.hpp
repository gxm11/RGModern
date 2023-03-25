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

namespace rgm::base {
using musics = std::map<uint64_t, cen::music>;

struct music_create {
  using data = std::tuple<musics>;

  uint64_t id;
  const char* path;

  void run(auto& worker) {
    cen::log_info("musics, id: %lld, path: %s", id, path);
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
  }
};

struct music_fade_in {
  uint64_t id;
  int iteration;
  int duration;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.at(id).fade_in(cen::music::ms_type{duration}, iteration);
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

struct music_pause {
  void run(auto&) { cen::music::pause(); }
};

struct music_halt {
  void run(auto&) { cen::music::halt(); }
};

struct music_fade_out {
  int duration;

  void run(auto&) { cen::music::fade_out(cen::music::ms_type{duration}); }
};

struct music_is_playing {
  bool* is_playing;

  void run(auto&) { *is_playing = cen::music::is_playing(); }
};

struct init_music {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      // static VALUE music_create(VALUE, VALUE id_, VALUE path_) {
      //   RGMLOAD(id, uint64_t);
      //   RGMLOAD(path, const char*);

      //   worker >> base::music_create{id, path};
      //   return Qnil;
      // }

      static VALUE music_dispose(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        worker >> base::music_dispose{id};
        return Qnil;
      }

      static VALUE music_play(VALUE, VALUE id_, VALUE iteration_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(iteration, int);

        worker >> base::music_play{id, iteration};
        return Qnil;
      }

      static VALUE music_fade_in(VALUE, VALUE id_, VALUE iteration_,
                                 VALUE duration_) {
        RGMLOAD(id, uint64_t);
        RGMLOAD(iteration, int);
        RGMLOAD(duration, int);

        worker >> base::music_fade_in{id, iteration, duration};
        return Qnil;
      }

      static VALUE music_set_volume(VALUE, VALUE volume_) {
        RGMLOAD(volume, int);

        worker >> base::music_set_volume{volume};
        return Qnil;
      }

      static VALUE music_set_position(VALUE, VALUE position_) {
        RGMLOAD(position, double);

        worker >> base::music_set_position{position};
        return Qnil;
      }

      static VALUE music_pause(VALUE) {
        worker >> base::music_pause{};
        return Qnil;
      }

      static VALUE music_halt(VALUE) {
        worker >> base::music_halt{};
        return Qnil;
      }

      static VALUE music_fade_out(VALUE, VALUE duration_) {
        RGMLOAD(duration, int);

        worker >> base::music_fade_out{duration};
        return Qnil;
      }

      static VALUE music_is_playing(VALUE) {
        bool is_playing = false;

        worker >> base::music_is_playing{&is_playing};
        RGMWAIT(2);

        return is_playing ? Qtrue : Qfalse;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    // rb_define_module_function(rb_mRGM_Ext, "music_create",
    //                           wrapper::music_create, 2);
    {
      ruby_wrapper w(this_worker);
      w.template create_sender<music_create, uint64_t, const char*>(
          rb_mRGM_Ext, "music_create");
    }

    rb_define_module_function(rb_mRGM_Ext, "music_dispose",
                              wrapper::music_dispose, 1);
    rb_define_module_function(rb_mRGM_Ext, "music_play", wrapper::music_play,
                              2);
    rb_define_module_function(rb_mRGM_Ext, "music_fade_in",
                              wrapper::music_fade_in, 3);
    rb_define_module_function(rb_mRGM_Ext, "music_set_volume",
                              wrapper::music_set_volume, 1);
    rb_define_module_function(rb_mRGM_Ext, "music_set_position",
                              wrapper::music_set_position, 1);
    rb_define_module_function(rb_mRGM_Ext, "music_pause", wrapper::music_pause,
                              0);
    rb_define_module_function(rb_mRGM_Ext, "music_halt", wrapper::music_halt,
                              0);
    rb_define_module_function(rb_mRGM_Ext, "music_fade_out",
                              wrapper::music_fade_out, 1);
    rb_define_module_function(rb_mRGM_Ext, "music_is_playing",
                              wrapper::music_is_playing, 0);
  }
};
}  // namespace rgm::base