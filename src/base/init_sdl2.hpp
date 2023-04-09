// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include <SDL_syswm.h>

#include <map>
#include <memory>

#include "cen_library.hpp"
#include "controller.hpp"
#include "core/core.hpp"
#include "sound_pitch.hpp"
#include "driver.hpp"

namespace rgm::base {
struct sdl_hint {
  cen::window::window_flags window_flag;

  sdl_hint() : window_flag{cen::window::window_flags::hidden} {
    switch (driver) {
      default:
        break;
      case driver_type::opengl:
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

        window_flag = static_cast<cen::window::window_flags>(
            static_cast<size_t>(window_flag) |
            static_cast<size_t>(cen::window::window_flags::opengl));
        break;
      case driver_type::direct3d9:
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d9");
        break;
      case driver_type::direct3d11:
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
        break;
    }

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
  }
};

/** @brief 利用 RAII 机制管理 SDL2 初始化和退出的类 */
struct cen_library {
  cen::sdl sdl;
  sdl_hint hint;

  cen::img img;
  cen::ttf ttf;
  cen::mix mix;

  /** @brief 当前的窗口 */
  cen::window window;
  /** @brief 当前的渲染器 */
  cen::renderer renderer;
  /** @brief 管理事件的分配器 */
  using event_dispatcher_t = cen::event_dispatcher<
      cen::quit_event, cen::window_event, cen::keyboard_event,
      cen::mouse_button_event, cen::text_editing_event, cen::text_input_event,
      cen::controller_axis_event, cen::controller_button_event>;

  event_dispatcher_t event_dispatcher;

  SDL_SysWMinfo window_info;
  SDL_RendererInfo renderer_info;
  /** @brief joystick_index -> controller */
  std::map<int, cen::controller> controllers;
  /** @brief 初始化 SDL2 运行环境，创建并显示窗口 */
  explicit cen_library()
      : sdl(),
        hint(),
        img(),
        ttf(),
        mix(),
        window(config::game_title,
               cen::iarea{config::window_width, config::window_height},
               hint.window_flag),
        renderer(window.make_renderer()),
        event_dispatcher() {
    if (config::build_mode <= 0) {
      cen::set_priority(cen::log_priority::debug);
    } else if (config::build_mode == 1) {
      cen::set_priority(config::debug ? cen::log_priority::debug
                                      : cen::log_priority::info);
    } else {
      cen::set_priority(config::debug ? cen::log_priority::info
                                      : cen::log_priority::warn);
    }

    renderer.reset_target();
    renderer.clear_with(cen::colors::transparent);
    renderer.present();

    window.show();

    sound_pitch::setup();

    event_dispatcher.poll();

    SDL_VERSION(&window_info.version);
    SDL_GetWindowWMInfo(window.get(), &window_info);
    SDL_GetRendererInfo(renderer.get(), &renderer_info);
  }
};

/** @brief 任务：处理 SDL 事件 */
struct poll_event {
  void run(auto& worker) {
    RGMDATA(cen_library).event_dispatcher.poll();
    // 处理 controller
    std::map<int, cen::controller>& cs = RGMDATA(cen_library).controllers;

    bool changed = false;
    for (int i = 0; i < static_cast<int>(cs.size()); ++i) {
      if (!cen::controller::supported(i)) {
        cen::log_warn("[Input] Controller %d is disconnected.", i);
        cs.erase(i);
        changed = true;
      }
    }
    const int joystick_numbers = SDL_NumJoysticks();
    for (int i = 0; i < joystick_numbers; ++i) {
      if (cs.find(i) != cs.end()) continue;
      cen::log_warn("[Input] Controller %d is connected.", i);
      cs.emplace(i, cen::controller(i));
      changed = true;
    }
    if (changed) worker >> controller_axis_reset{};
  }
};

struct set_title {
  std::string title;

  void run(auto& worker) {
    cen::window& window = RGMDATA(cen_library).window;

    window.set_title(title);
  }
};

struct set_fullscreen {
  int mode;

  void run(auto& worker) {
    cen::window& window = RGMDATA(cen_library).window;

    switch (mode) {
      case 1:
        SDL_SetWindowFullscreen(window.get(), SDL_WINDOW_FULLSCREEN);
        return;
      case 2:
        SDL_SetWindowFullscreen(window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
        return;
      default:
      case 0:
        SDL_SetWindowFullscreen(window.get(), 0);
        return;
    }
  }
};

/** @brief 将 cen_library 类型的变量添加到 worker 的 datalist 中 */
struct init_sdl2 {
  using data = std::tuple<cen_library>;

  static void before(auto& worker) {
    SDL_RendererInfo& info = RGMDATA(cen_library).renderer_info;
    cen::log_info("[Driver] use %s for rendering\n", info.name);
    cen::log_info("[Driver] max texture %d x %d\n", info.max_texture_width,
                  info.max_texture_height);
  }

  static void after(auto& worker) { RGMDATA(cen_library).window.hide(); }
};
}  // namespace rgm::base