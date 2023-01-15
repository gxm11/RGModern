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
#include <string>

#include "cen_library.hpp"
#include "core/core.hpp"
#include "sound_pitch.hpp"

namespace rgm::base {
struct sound_info {
  std::string path;
  std::unique_ptr<cen::sound_effect> data;
  float speed;

  explicit sound_info() : path(), data(), speed(1.0) {}

  sound_info& operator<<(const char* str) {
    path = str;
    data = std::make_unique<cen::sound_effect>(str);
    speed = 1.0;
    return *this;
  }

  void clear() {
    path.clear();
    data.reset();
    speed = 1.0;
  }

  void set_speed(float new_speed, bool loop = false) {
    speed = new_speed;
    auto channel = data->channel();
    if (channel.has_value()) {
      sound_pitch::setupPlaybackSpeedEffect(data->get(), &speed,
                                            channel.value(), loop);
    }
  }
};

struct sound_manager {
  enum class event { bgs_play, bgs_fade, bgs_stop, se_play, se_stop };

  static constexpr cen::music::ms_type default_fade_time{500};

  sound_info bgs;
  std::map<uint64_t, sound_info> sounds;
  uint64_t index;

  explicit sound_manager() : bgs(), sounds(), index(0) {}

  void clean() {
    if (bgs.data && !bgs.data->is_playing()) bgs.clear();

    std::erase_if(sounds,
                  [](auto& pair) { return !pair.second.data->is_playing(); });
  }

  template <event sound_manager_event>
  void update(const char* path, int volume, float speed) {
    clean();

    if constexpr (sound_manager_event == event::bgs_play) {
      if (bgs.path.empty() || bgs.path != path) {
        bgs << path;
        bgs.data->play(-1);
      }
      if (bgs.data->channel().has_value()) {
        bgs.data->set_volume(volume);
        bgs.set_speed(speed, true);
      }
    }

    if constexpr (sound_manager_event == event::se_play) {
      sound_info se;
      se << path;
      ++index;
      sounds.emplace(index, std::move(se));

      sound_info& se2 = sounds[index];
      se2.data->play(0);
      if (se2.data->channel().has_value()) {
        se2.data->set_volume(volume);
        se2.set_speed(speed, false);
      }
    }
  }

  template <event sound_manager_event>
  void update(int fade_time) {
    if constexpr (sound_manager_event == event::bgs_fade) {
      if (!bgs.path.empty()) {
        bgs.path.clear();
        bgs.data->fade_out(cen::music::ms_type{fade_time});
      }
    }
  }

  template <event sound_manager_event>
  void update() {
    if constexpr (sound_manager_event == event::bgs_stop) {
      bgs.clear();
    }

    if constexpr (sound_manager_event == event::se_stop) {
      sounds.clear();
    }
  }
};

struct bgs_play {
  const char* path;
  int volume;
  float speed;

  void run(auto& worker) {
    cen::log_info(cen::log_category::audio, "[Audio] BGS Play %s", path);

    sound_manager& sm = RGMDATA(sound_manager);
    sm.update<sound_manager::event::bgs_play>(path, volume, speed);
  }
};

struct bgs_stop {
  int fade_time;

  void run(auto& worker) {
    sound_manager& sm = RGMDATA(sound_manager);
    if (fade_time > 0) {
      cen::log_debug(cen::log_category::audio, "[Audio] BGS Fade in %d ms",
                     fade_time);
      sm.update<sound_manager::event::bgs_fade>(fade_time);
    } else {
      cen::log_debug(cen::log_category::audio, "[Audio] BGS Stop");
      sm.update<sound_manager::event::bgs_stop>();
    }
  }
};

struct se_play {
  const char* path;
  int volume;
  float speed;

  void run(auto& worker) {
    cen::log_info(cen::log_category::audio, "[Audio] SE Play %s", path);

    sound_manager& sm = RGMDATA(sound_manager);
    sm.update<sound_manager::event::se_play>(path, volume, speed);
  }
};

struct se_stop {
  void run(auto& worker) {
    cen::log_debug(cen::log_category::audio, "[Audio] SE Stop");

    sound_manager& sm = RGMDATA(sound_manager);
    sm.update<sound_manager::event::se_stop>();
  }
};

struct init_sound_manager {
  using data = rgm::data<sound_manager>;
};
}  // namespace rgm::base