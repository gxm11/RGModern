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
#include <string>

#include "cen_library.hpp"
#include "core/core.hpp"

namespace rgm::base {
struct music_info {
  std::string path;
  std::unique_ptr<cen::music> data;
  double position;
  int volume;

  explicit music_info() : path(), data(), position(0), volume(0) {}

  music_info& operator<<(const char* str) {
    path = str;
    data = std::make_unique<cen::music>(str);
    position = 0;
    return *this;
  }

  void save_state() {
    position = Mix_GetMusicPosition(data->get());
    if (position < 0) position = 0;
    volume = cen::music::volume();
  }

  void clear() {
    path.clear();
    data.reset();
    position = 0;
  }
};

struct music_manager {
  // me 优先级比 bgm 高，且不可被 bgm 打断
  // 先修改state，再触发callback，在callback里判断当前的state，再决定如何跳转

  // 0 = stop，无任何音乐播放
  // 1 = bgm_fade，bgm在fade out
  // 2 = bgm_play，bgm正在播放
  // 3 = me_fade，me在fade_out
  // 4 = me_play，me在播放

  // 目前有如下 7 个指令：
  // bgm_play / bgm_fade / bgm_stop / me_play / me_fade / me_stop / music_finish
  // 状态跳转规则：
  // 初始状态 = 0
  // bgm_play:
  // 0 -> 2，播放bgm
  // 1，修改预约的bgm，等待fade out结束的callback触发
  // 2 -> 1，修改预约的bgm，fade out当前bgm
  // 4, 5，修改预约的bgm
  // bgm_fade:
  // 2 -> 1，fade_out当前bgm
  // 0, 1, 4, 5，无操作
  // bgm_stop:
  // 0，无操作
  // 1 -> 0，停止当前bgm，清除预约bgm
  // 2 -> 1，停止当前bgm，清除预约bgm
  // 3, 4，清除预约bgm
  // me_play:
  // 0 -> 4，播放me
  // 1 -> 4，播放me
  // 2 -> 4，播放me，当前bgm转为预约bgm
  // 3 -> 4，停止当前me，播放新me
  // 4 -> 4，停止当前me，播放新me
  // me_fade:
  // 0,1,2,3，无操作
  // 4 -> 3，fade out当前me
  // me_stop:
  // 0,1,2，无操作
  // 3，停止当前me
  // 4 -> 3，停止当前me
  // music_finish:
  // 0, 2, 4，无操作
  // 1，停止当前music(-> 0)，如果有bgm_wait，播放bgm_wait -> 2
  // 3. 停止当前music(-> 0)，如果有bgm_wait，播放bgm_wait -> 2

  enum class state { stop = 0, bgm_fade, bgm_play, me_fade, me_play };

  enum class event {
    bgm_play,
    bgm_fade,
    bgm_stop,
    me_play,
    me_fade,
    me_stop,
    music_finish
  };

  static constexpr cen::music::ms_type default_fade_time{500};

  music_info bgm;
  music_info bgm_wait;
  music_info me;
  state current_state;

  music_manager() : bgm(), bgm_wait(), me(), current_state(state::stop) {}

  template <event music_manager_event>
  state update(const char* path, int volume, double position = 0)
    requires(music_manager_event == event::bgm_play)
  {
    using enum state;

    switch (current_state) {
      case stop:
        bgm << path;
        bgm.data->fade_in(default_fade_time, -1);
        cen::music::set_volume(volume);
        current_state = bgm_play;
        break;
      case bgm_play:
        if (bgm.path != path) {
          bgm.save_state();
          bgm_wait << path;
          bgm_wait.volume = volume;
          bgm_wait.position = position;
          cen::music::fade_out(default_fade_time);
          current_state = bgm_fade;
        } else {
          cen::music::set_position(position);
          cen::music::set_volume(volume);
        }
        break;
      default:
      case bgm_fade:
      case me_fade:
      case me_play:
        bgm_wait << path;
        bgm_wait.volume = volume;
        bgm_wait.position = position;
        break;
    }

    return current_state;
  }

  template <event music_manager_event>
  state update(const char* path, int volume,
               [[maybe_unused]] double position = 0)
    requires(music_manager_event == event::me_play)
  {
    using enum state;

    if (me.path != path) {
      cen::music::pause();
      me << path;
      me.data->fade_in(default_fade_time, 1);
    }
    me.volume = volume;
    cen::music::set_volume(volume);
    switch (current_state) {
      case bgm_play:
        bgm_wait.clear();
        bgm.save_state();
        std::swap(bgm, bgm_wait);
        break;
      default:
      case stop:
      case bgm_fade:
      case me_fade:
      case me_play:
        break;
    }
    current_state = me_play;

    return current_state;
  }

  template <event music_manager_event>
  state update(int fade_time)
    requires(music_manager_event == event::bgm_fade)
  {
    using enum state;

    switch (current_state) {
      case bgm_play:
        cen::music::fade_out(cen::music::ms_type{fade_time});
        bgm_wait.clear();
        current_state = bgm_fade;
        break;
      default:
        break;
    }

    return current_state;
  }

  template <event music_manager_event>
  state update(int fade_time)
    requires(music_manager_event == event::me_fade)
  {
    using enum state;

    switch (current_state) {
      case me_play:
        cen::music::fade_out(cen::music::ms_type{fade_time});
        current_state = me_fade;
        break;
      default:
      case stop:
      case bgm_fade:
      case bgm_play:
      case me_fade:
        break;
    }

    return current_state;
  }

  template <event music_manager_event>
  state update()
    requires(music_manager_event == event::bgm_stop)
  {
    using enum state;

    switch (current_state) {
      case bgm_fade:
        cen::music::halt();
        current_state = stop;
        break;
      case bgm_play:
        cen::music::halt();
        bgm_wait.clear();
        current_state = bgm_fade;
        break;
      default:
      case stop:
      case me_fade:
      case me_play:
        break;
    }
    bgm_wait.clear();

    return current_state;
  }

  template <event music_manager_event>
  state update()
    requires(music_manager_event == event::me_stop)
  {
    using enum state;

    switch (current_state) {
      case me_fade:
        cen::music::halt();
        me.clear();
        break;
      case me_play:
        cen::music::halt();
        current_state = me_fade;
        break;
      default:
      case stop:
      case bgm_fade:
      case bgm_play:
        break;
    }

    return current_state;
  }

  template <event music_manager_event>
  state update()
    requires(music_manager_event == event::music_finish)
  {
    using enum state;

    switch (current_state) {
      case bgm_fade:
      case me_fade:
      case me_play:
        if (bgm_wait.data && !cen::music::is_playing()) {
          cen::music::pause();
          me.clear();
          bgm.clear();
          std::swap(bgm, bgm_wait);

          bgm.data->fade_in(default_fade_time, -1);
          cen::music::set_position(bgm.position);
          cen::music::set_volume(bgm.volume);
          current_state = bgm_play;
        } else {
          me.clear();
          current_state = stop;
        }
        break;
      default:
      case stop:
      case bgm_play:
        break;
    }

    return current_state;
  }
};

struct music_finish {
  void run(auto& worker) {
    music_manager& mm = RGMDATA(music_manager);

    music_manager::state s = mm.update<music_manager::event::music_finish>();

    static const decltype(worker) this_worker(worker);
    struct wrapper {
      static void callback() { this_worker >> music_finish{}; }
    };

    if (s == music_manager::state::bgm_play) {
      Mix_HookMusicFinished(wrapper::callback);
    }
  }
};

struct bgm_play {
  const char* path;
  double position;
  int volume;

  void run(auto& worker) {
    cen::log_info(cen::log_category::audio, "[Audio] BGM Play %s", path);

    music_manager& mm = RGMDATA(music_manager);
    mm.update<music_manager::event::bgm_play>(path, volume, position);

    static const decltype(worker) this_worker(worker);
    struct wrapper {
      static void callback() { this_worker >> music_finish{}; }
    };

    Mix_HookMusicFinished(wrapper::callback);
  }
};

struct bgm_stop {
  int fade_time;

  void run(auto& worker) {
    music_manager& mm = RGMDATA(music_manager);
    if (fade_time > 0) {
      cen::log_debug(cen::log_category::audio, "[Audio] BGM Fade in %d ms",
                     fade_time);

      mm.update<music_manager::event::bgm_fade>(fade_time);
    } else {
      cen::log_debug(cen::log_category::audio, "[Audio] BGM Stop");

      mm.update<music_manager::event::bgm_stop>();
    }
  }
};

struct me_play {
  const char* path;
  int volume;

  void run(auto& worker) {
    cen::log_info(cen::log_category::audio, "[Audio] ME Play %s", path);

    music_manager& mm = RGMDATA(music_manager);
    mm.update<music_manager::event::me_play>(path, volume);

    static const decltype(worker) this_worker(worker);
    struct wrapper {
      static void callback() { this_worker >> music_finish{}; }
    };

    Mix_HookMusicFinished(wrapper::callback);
  }
};

struct me_stop {
  int fade_time;

  void run(auto& worker) {
    music_manager& mm = RGMDATA(music_manager);
    if (fade_time > 0) {
      cen::log_debug(cen::log_category::audio, "[Audio] ME Fade in %d ms",
                     fade_time);

      mm.update<music_manager::event::me_fade>(fade_time);
    } else {
      cen::log_debug(cen::log_category::audio, "[Audio] ME Stop");

      mm.update<music_manager::event::me_stop>();
    }
  }
};

struct bgm_pos {
  double* p_position;

  void run(auto& worker) {
    music_manager& mm = RGMDATA(music_manager);
    mm.bgm.save_state();
    *p_position = mm.bgm.position;
  }
};

struct init_music_manager {
  using data = rgm::data<music_manager>;

  static void after(auto& worker) {
    music_manager& mm = RGMDATA(music_manager);
    mm.bgm.clear();
    mm.bgm_wait.clear();
    mm.me.clear();
  }
};
}  // namespace rgm::base