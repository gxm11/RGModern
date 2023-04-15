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
#include "ruby_wrapper.hpp"

namespace rgm::base {
/// @brief 设置窗口的标题
/// @name task
struct set_title {
  std::string title;

  void run(auto& worker) {
    cen::window& window = RGMDATA(cen_library).window;

    window.set_title(title);
  }
};

/// @brief 设置窗口的全屏模式
/// @name task
struct set_fullscreen {
  /// @brief 全屏模式
  /// 0: 关闭全屏，使用窗口模式
  /// 1: 全屏模式
  /// 2: 窗口全屏模式，在绘制时窗口的分辨率被视为屏幕的分辨率
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

/// @brief window 相关类型的初始化类
/// @name task
struct init_window {
  static void before(auto& worker) {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");

    RGMBIND(rb_mRGM_Base, "set_title", base::set_title, 1);
    RGMBIND(rb_mRGM_Base, "set_fullscreen", base::set_fullscreen, 1);
  }
};
}  // namespace rgm::base