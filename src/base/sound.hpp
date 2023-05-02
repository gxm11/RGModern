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
/// @brief 存储所有 cen::sound_effect，即音效对象的类
using sounds = std::unordered_map<uint64_t, cen::sound_effect>;

/// @brief 存储所有音效的 channel 的速度的容器
using sound_speeds = std::array<float, 32>;

/// @brief 创建 RGM::Sound 对象对应的 C++ 对象
struct sound_create {
  using data = std::tuple<sounds>;
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 音效对象的文件路径
  std::string_view path;

  void run(auto& worker) {
    sounds& data = RGMDATA(sounds);
    data.emplace(id, cen::sound_effect(path.data()));
  }
};

/// @brief 释放 RGM::Music 对象对应的 C++ 对象
struct sound_dispose {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  void run(auto& worker) {
    sounds& data = RGMDATA(sounds);
    data.erase(id);
  }
};

/// @brief 播放音效
struct sound_play {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 播放次数，-1 表示循环播放，0或1 表示播放一次
  int iteration;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.play(iteration);
  }
};

/// @brief 停止音效
struct sound_stop {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.stop();
  }
};

/// @brief 淡入音效
struct sound_fade_in {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 淡入时间，单位是毫秒（ms）
  int duration;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.fade_in(cen::music::ms_type{duration});
  }
};

/// @brief 淡出音效
struct sound_fade_out {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 淡入时间，单位是毫秒（ms）
  int duration;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.fade_out(cen::music::ms_type{duration});
  }
};

/// @brief 设置音效的音量大小
struct sound_set_volume {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 要设置的音量大小
  int volume;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    s.set_volume(volume);
  }
};

/// @brief 设置音效的频率高低
struct sound_set_pitch {
  using data = std::tuple<sound_speeds>;

  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 要设置的音调比例，100 为不变
  int pitch;

  /// @brief 设置是否为循环播放音效（如 BGS）
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

/// @brief 获取音效的播放状态
/// 每一个比特位代表了不同的状态：
/// 1 -> is_playing
/// 2 -> is_fading
struct sound_get_state {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 储存状态的变量的指针
  int* p_state;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    *p_state = 0;
    if (s.is_playing()) p_state += 1;
    if (s.is_fading()) p_state += 2;
  }
};

/// @brief 获取音效所属的通道
struct sound_get_channel {
  /// @brief 音效对象的 id，在 sounds 中作为键使用
  uint64_t id;

  /// @brief 储存通道的变量的指针
  int* channel;

  void run(auto& worker) {
    cen::sound_effect& s = RGMDATA(sounds).at(id);
    *channel = s.channel().value_or(-1);
  }
};

/// @brief 音效播放相关的初始化类
struct init_sound {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#sound_get_state -> sound_get_state */
      static VALUE sound_get_state(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);

        int state = 0;
        worker >> base::sound_get_state{id, &state};
        RGMWAIT(2);
        return INT2FIX(state);
      }

      /* ruby method: Base#sound_get_channel -> sound_get_channel */
      static VALUE sound_get_channel(VALUE, VALUE id_) {
        RGMLOAD(id, uint64_t);

        int channel = -1;
        worker >> base::sound_get_channel{id, &channel};
        RGMWAIT(2);
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