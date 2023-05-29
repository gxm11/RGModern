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
#include "init_sdl2.hpp"
#include "ruby_wrapper.hpp"

namespace rgm::base {
/// @brief 设置窗口的标题
struct set_title {
  /// @brief 窗口的标题，这里复制了一份 ruby 中的字符串
  std::string title;

  void run(auto& worker) {
    cen::window& window = RGMDATA(cen_library).window;

    window.set_title(title);
  }
};

/// @brief 设置窗口的全屏模式
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

/// @brief 获取窗口大小，在 resize_window 和 set_fullscreen 中用到
struct get_display_bounds {
  /// @brief 存储宽度的变量的指针
  int* p_width;

  /// @brief 存储高度的变量的指针
  int* p_height;

  void run(auto&) {
    auto opt = cen::display_bounds();
    if (opt) {
      *p_width = opt->width();
      *p_height = opt->height();
    }
  }
};

/// @brief 获取窗口的 HWND 值
struct get_hwnd {
  /// @brief 存储 HWND 变量的指针
  uint64_t* p_handle;

  void run(auto& worker) {
#ifdef __WIN32
    SDL_SysWMinfo& info = RGMDATA(cen_library).window_info;
    *p_handle = reinterpret_cast<uint64_t>(info.info.win.window);
#endif
  }
};

/// @brief window 相关类型的初始化类
struct init_window {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      static VALUE display_bounds(VALUE) {
        int width = 0;
        int height = 0;

        worker >> get_display_bounds{&width, &height};
        RGMWAIT(1);

        /* 一般情况下这两个数都不会太大，以防万一还是夹一下 */
        width = std::clamp(width, 0, 65535);
        height = std::clamp(height, 0, 65535);

        /* 将高和宽打包到一起传过去 */
        return INT2FIX(height * 65536 + width);
      }

      static VALUE window_handle(VALUE) {
        uint64_t hwnd = 0;

        worker >> get_hwnd{&hwnd};
        RGMWAIT(1);

        return ULL2NUM(hwnd);
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");

    rb_define_module_function(rb_mRGM_Base, "get_display_bounds",
                              wrapper::display_bounds, 0);
    rb_define_module_function(rb_mRGM_Base, "get_hwnd", wrapper::window_handle,
                              0);
    RGMBIND(rb_mRGM_Base, "set_title", base::set_title, 1);
    RGMBIND(rb_mRGM_Base, "set_fullscreen", base::set_fullscreen, 1);
  }
};
}  // namespace rgm::base