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
#include "base/base.hpp"
#include "detail.hpp"

namespace rgm::rmxp {
struct init_music {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      static VALUE bgm_play(VALUE, VALUE path_, VALUE volume_,
                            VALUE position_) {
        RGMLOAD(path, const char*);
        RGMLOAD(volume, int);
        RGMLOAD(position, double);

        worker << base::bgm_play{path, position, volume * MIX_MAX_VOLUME / 100};
        return Qnil;
      }

      static VALUE me_play(VALUE, VALUE path_, VALUE volume_) {
        RGMLOAD(path, const char*);
        RGMLOAD(volume, int);

        worker << base::me_play{path, volume * MIX_MAX_VOLUME / 100};
        return Qnil;
      }

      static VALUE bgm_stop(VALUE, VALUE fade_time_) {
        RGMLOAD(fade_time, int);

        worker << base::bgm_stop{fade_time};
        return Qnil;
      }

      static VALUE me_stop(VALUE, VALUE fade_time_) {
        RGMLOAD(fade_time, int);

        worker << base::me_stop{fade_time};
        return Qnil;
      }

      static VALUE bgm_pos(VALUE) {
        double position = -1;
        worker << base::bgm_pos{&position};
        RGMWAIT(2);
        return DBL2NUM(position);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "audio_bgm_play", wrapper::bgm_play,
                              3);
    rb_define_module_function(rb_mRGM_BASE, "audio_bgm_stop", wrapper::bgm_stop,
                              1);
    rb_define_module_function(rb_mRGM_BASE, "audio_bgm_pos", wrapper::bgm_pos,
                              0);
    rb_define_module_function(rb_mRGM_BASE, "audio_me_play", wrapper::me_play,
                              2);
    rb_define_module_function(rb_mRGM_BASE, "audio_me_stop", wrapper::me_stop,
                              1);
  }
};

struct init_sound {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      static VALUE bgs_play(VALUE, VALUE path_, VALUE volume_, VALUE pitch_) {
        RGMLOAD(path, const char*);
        RGMLOAD(volume, int);
        RGMLOAD(pitch, int);

        float speed = pitch / 100.0;

        worker << base::bgs_play{path, volume, speed};
        return Qnil;
      }

      static VALUE se_play(VALUE, VALUE path_, VALUE volume_, VALUE pitch_) {
        RGMLOAD(path, const char*);
        RGMLOAD(volume, int);
        RGMLOAD(pitch, int);

        float speed = pitch / 100.0;

        worker << base::se_play{path, volume, speed};
        return Qnil;
      }

      static VALUE bgs_stop(VALUE, VALUE fade_time_) {
        RGMLOAD(fade_time, int);

        worker << base::bgs_stop{fade_time};
        return Qnil;
      }

      static VALUE se_stop(VALUE) {
        worker << base::se_stop{};
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "audio_bgs_play", wrapper::bgs_play,
                              3);
    rb_define_module_function(rb_mRGM_BASE, "audio_bgs_stop", wrapper::bgs_stop,
                              1);
    rb_define_module_function(rb_mRGM_BASE, "audio_se_play", wrapper::se_play,
                              3);
    rb_define_module_function(rb_mRGM_BASE, "audio_se_stop", wrapper::se_stop,
                              0);
  }
};
}  // namespace rgm::rmxp