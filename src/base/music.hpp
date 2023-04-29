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
/// @brief 存储所有 cen::music，即音乐对象的类
/// @name data
/// SDL_MIXER 里，播放音乐会使当前播放的音乐停止，同时只能有 1 个音乐在播放
using musics = std::map<uint64_t, cen::music>;

/// @brief 音乐播放结束后，会自动回调此函数
/// @name task
/// 在 Audio 模块中重新定义以处理 BGM 和 ME 之间的切换
struct music_finish_callback {
  void run(auto&) {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");

    rb_funcall(rb_mRGM_Base, rb_intern("music_finish_callback"), 0);
  }
};

/// @brief 创建 RGM::Music 对象对应的 C++ 对象
/// @name task
struct music_create {
  using data = std::tuple<musics>;

  /// @brief 音乐对象的 id，在 musics 中作为键使用
  uint64_t id;

  /// @brief 音乐对象的文件路径
  std::string_view path;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.emplace(id, cen::music(path.data()));
  }
};

/// @brief 释放 RGM::Music 对象对应的 C++ 对象
/// @name task
struct music_dispose {
  /// @brief 音乐对象的 id，在 musics 中作为键使用
  uint64_t id;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.erase(id);
  }
};

/// @brief 播放音乐
/// @name task
struct music_play {
  /// @brief 音乐对象的 id，在 musics 中作为键使用
  uint64_t id;

  /// @brief 播放次数，-1 表示循环播放，0或1 表示播放一次
  int iteration;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.at(id).play(iteration);

    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) this_worker(worker);

    /* wrapper 类，创建静态方法供 Mix_HookMusicFinished 绑定 */
    struct wrapper {
      static void callback() { this_worker >> music_finish_callback{}; }
    };

    /* 音乐结束时发送 music_finish_callback 任务 */
    Mix_HookMusicFinished(wrapper::callback);
  }
};

/// @brief 淡入音乐
/// @name task
struct music_fade_in {
  /// @brief 音乐对象的 id，在 musics 中作为键使用
  uint64_t id;

  /// @brief 播放次数，-1 表示循环播放，0或1 表示播放一次
  int iteration;

  /// @brief 淡入时间，单位是毫秒（ms）
  int duration;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    data.at(id).fade_in(cen::music::ms_type{duration}, iteration);

    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) this_worker(worker);

    /* wrapper 类，创建静态方法供 Mix_HookMusicFinished 绑定 */
    struct wrapper {
      static void callback() { this_worker >> music_finish_callback{}; }
    };

    /* 音乐结束时发送 music_finish_callback 任务 */
    Mix_HookMusicFinished(wrapper::callback);
  }
};

/// @brief 获取当前播放的音乐的位置
/// @name task
struct music_get_position {
  /// @brief 音乐对象的 id，在 musics 中作为键使用
  uint64_t id;

  /// @brief 存储位置的变量的指针
  double* p_position;

  void run(auto& worker) {
    musics& data = RGMDATA(musics);
    *p_position = data.at(id).position().value_or(-1);
  }
};

/* 以下方法不需要音乐对象的 id */

/// @brief 获取当前播放的音乐的音量
/// @name task
struct music_get_volume {
  /// @brief 存储音量的变量的指针
  int* p_volume;

  void run(auto&) { *p_volume = cen::music::volume(); }
};

/// @brief 设置当前播放的音乐的音量大小
/// @name task
struct music_set_volume {
  /// @brief 要设置的音量大小
  int volume;

  void run(auto&) { cen::music::set_volume(volume); }
};

/// @brief 设置当前播放的音乐的播放位置
/// @name task
struct music_set_position {
  /// @brief 要设置的当前音乐的播放位置
  double position;

  void run(auto&) { cen::music::set_position(position); }
};

/// @brief 恢复当前音乐的播放
/// @name task
struct music_resume {
  void run(auto&) { cen::music::resume(); }
};

/// @brief 暂停当前音乐的播放
/// @name task
struct music_pause {
  void run(auto&) { cen::music::pause(); }
};

/// @brief 停止当前音乐的播放
/// @name task
struct music_halt {
  void run(auto&) { cen::music::halt(); }
};

/// @brief 从头播放当前音乐
/// @name task
struct music_rewind {
  void run(auto&) { cen::music::rewind(); }
};

/// @brief 淡出当前音乐
/// @name task
struct music_fade_out {
  /// @brief 淡出时间，单位是毫秒（ms）
  int duration;

  void run(auto&) { cen::music::fade_out(cen::music::ms_type{duration}); }
};

/// @brief 获取当前音乐的播放状态
/// @name task
/// 每一个比特位代表了不同的状态：
/// 1 -> is_fading
/// 2 -> is_fading_in
/// 3 -> is_fading_out
/// 4 -> is_paused
/// 5 -> is_playing
struct music_get_state {
  /// @brief 储存状态的变量的指针
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

/// @brief 设置 MIDI 音乐的 SoundFont 库
/// @name task
struct music_set_soundfonts {
  /// @brief soundfonts 的路径
  std::string_view path;

  void run(auto&) {
    cen::log_info("Soundfonts is loaded from: %s", path.data());

    Mix_SetSoundFonts(path.data());
  }
};

/// @brief 音乐播放相关的初始化类
/// @name task
struct init_music {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#music_get_state -> music_get_state */
      static VALUE music_get_state(VALUE) {
        int state = 0;
        worker >> base::music_get_state{&state};
        RGMWAIT(2);
        return INT2FIX(state);
      }

      /* ruby method: Base#music_get_volume -> music_get_volume */
      static VALUE music_get_volume(VALUE) {
        int volume = 0;
        worker >> base::music_get_volume{&volume};
        RGMWAIT(2);
        return INT2FIX(volume);
      }

      /* ruby method: Base#music_get_position -> music_get_position */
      static VALUE music_get_position(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);
        double position = 0;
        worker >> base::music_get_position{id, &position};
        RGMWAIT(2);
        return DBL2NUM(position);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "music_finish_callback",
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
    RGMBIND(rb_mRGM_Base, "music_set_soundfonts", music_set_soundfonts, 1);
  }
};
}  // namespace rgm::base