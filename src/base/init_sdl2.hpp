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
#include <SDL_syswm.h>

#include <memory>

#include "cen_library.hpp"
#include "core/core.hpp"
#include "sound_manager.hpp"

namespace rgm::base {
struct sdl_hint {
  cen::window::window_flags window_flag;

  sdl_hint() : window_flag{cen::window::window_flags::hidden} {
    switch (shader::driver) {
      default:
        break;
      case shader::opengl:
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

        window_flag = static_cast<cen::window::window_flags>(
            static_cast<size_t>(window_flag) |
            static_cast<size_t>(cen::window::window_flags::opengl));
        break;
      case shader::direct3d9:
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d9");
        break;
      case shader::direct3d11:
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
  using event_dispatcher_t =
      cen::event_dispatcher<cen::quit_event, cen::window_event,
                            cen::keyboard_event, cen::mouse_button_event,
                            cen::text_editing_event, cen::text_input_event>;

  event_dispatcher_t event_dispatcher;

  SDL_SysWMinfo window_info;
  SDL_RendererInfo renderer_info;
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
#if RGM_BUILDMODE <= 0
    cen::set_priority(cen::log_priority::debug);
#elif RGM_BUILDMODE == 1
    cen::set_priority(cen::log_priority::info);
#else
    cen::set_priority(config::debug ? cen::log_priority::info
                                    : cen::log_priority::warn);
#endif
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
  void run(auto& worker) { RGMDATA(cen_library).event_dispatcher.poll(); }
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
    cen::log_info(cen::log_category::video, "[Driver] use %s for rendering\n",
                  info.name);
    cen::log_info(cen::log_category::video, "[Driver] max texture %d x %d\n",
                  info.max_texture_width, info.max_texture_height);
  }

  static void after(auto& worker) { RGMDATA(cen_library).window.hide(); }
};
}  // namespace rgm::base