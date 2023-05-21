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
#include <SDL_syswm.h>

#include "controller.hpp"
#include "core/core.hpp"
#include "sound_pitch.hpp"

namespace rgm::base {
/// @brief 辅助设定 SDL Hint 的类，在 cen_library 中使用
struct sdl_hint {
  cen::window::window_flags window_flag;

  sdl_hint() : window_flag{cen::window::window_flags::hidden} {
    /* 提示 SDL 选择合适的渲染器 */
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, config::driver_name.data());

    /* 显示输入法的 UI */
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    /* 对于 opengl 渲染器，需要给 window_flags 添加 opengl 项 */
    if (config::opengl) {
      window_flag = static_cast<cen::window::window_flags>(
          static_cast<size_t>(window_flag) |
          static_cast<size_t>(cen::window::window_flags::opengl));
    }
  }
};

/// @brief 封装了 SDL2 的数据的类
/// 此类还会利用 RAII 机制管理 SDL2 初始化和退出
struct cen_library {
  /// @brief SDL 库
  cen::sdl sdl;

  /// @brief SDL Hints
  sdl_hint hint;

  /// @brief SDL_IMAGE
  cen::img img;

  /// @brief SDL_TTF
  cen::ttf ttf;

  /// @brief SDL_MIXER
  cen::mix mix;

  /// @brief 当前的窗口
  cen::window window;

  /// @brief 当前的渲染器
  cen::renderer renderer;

  /// @brief 管理所有窗口事件回调的类，不在这里注册类型就不能定义事件回调
  using event_dispatcher_t = cen::event_dispatcher<
      cen::quit_event, cen::window_event, cen::keyboard_event,
      cen::mouse_button_event, cen::mouse_motion_event, cen::mouse_wheel_event,
      cen::text_editing_event, cen::text_input_event,
      cen::controller_axis_event, cen::controller_button_event>;

  /// @brief 管理事件回调的分配器
  event_dispatcher_t event_dispatcher;

  /// @brief 储存窗口的信息
  SDL_SysWMinfo window_info;

  /// @brief 储存渲染器的信息
  SDL_RendererInfo renderer_info;

  /// @brief 管理所有控制器的容器
  /// joystick_index -> controller
  std::map<int, cen::controller> controllers;

  /// @brief screen 绘制到 window 上的缩放模式
  int scale_mode;

  /// @brief 初始化 SDL2 运行环境，创建并显示窗口
  explicit cen_library()
      : sdl(),
        hint(),
        img(),
        ttf(),
        mix(),
        /* 构造 cen::window */
        window(config::game_title,
               cen::iarea{config::window_width, config::window_height},
               hint.window_flag),
        /* 构造 cen::renderer */
        renderer(window.make_renderer()),
        event_dispatcher(),
        scale_mode(0) {
    /* 绘制透明的初始画面 */
    renderer.reset_target();
    renderer.clear_with(cen::colors::transparent);
    renderer.present();

    /* 显示窗口 */
    window.show();

    /* 设置 SDL_MIXER 的频率调制器 */
    sound_pitch::setup();

    /*
     * 处理当前堆积的窗口事件
     * 实际上此时没有任何窗口事件的回调函数被定义，所以什么也不会发生
     */
    event_dispatcher.poll();

    /* 读取窗口和渲染器的信息 */
    SDL_VERSION(&window_info.version);
    SDL_GetWindowWMInfo(window.get(), &window_info);
    SDL_GetRendererInfo(renderer.get(), &renderer_info);
  }
};

/// @brief 任务：处理 SDL 事件
struct poll_event {
  void run(auto& worker) {
    /* 处理当前堆积的窗口事件 */
    RGMDATA(cen_library).event_dispatcher.poll();

    /* 处理 controller 的插入和拔出事件 */
    std::map<int, cen::controller>& cs = RGMDATA(cen_library).controllers;

    for (int i = 0; i < static_cast<int>(cs.size()); ++i) {
      if (!cen::controller::supported(i)) {
        cs.erase(i);
        worker >> controller_disconnect{i};
      }
    }

    const int joystick_numbers = SDL_NumJoysticks();
    for (int i = 0; i < joystick_numbers; ++i) {
      if (cs.find(i) != cs.end()) continue;
      cs.emplace(i, cen::controller(i));
      worker >> controller_connect{i};
    }
  }
};

/// @brief 数据类 cen_library 相关的初始化类
struct init_sdl2 {
  using data = std::tuple<cen_library>;

  static void before(auto& worker) {
    SDL_RendererInfo& info = RGMDATA(cen_library).renderer_info;
    cen::log_warn("[Driver] use %s for rendering\n", info.name);
    cen::log_warn("[Driver] max texture %d x %d\n", info.max_texture_width,
                  info.max_texture_height);
  }

  static void after(auto& worker) { RGMDATA(cen_library).window.hide(); }
};
}  // namespace rgm::base