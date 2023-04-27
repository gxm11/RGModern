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
#include "base/base.hpp"
#include "word.hpp"

namespace rgm::rmxp {
/// @brief 弹出窗口，显示提示信息
/// @name task
struct message_show {
  /// @brief 提示信息内容
  std::string_view text;

  void run(auto&) {
    cen::message_box::show(config::game_title, text.data(),
                           cen::message_box_type::information);
  }
};

/// @brief 提示信息相关的初始化类
/// @name task
struct init_message {
  static void before(auto& this_worker) {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Base#message_show -> message_show */
      static VALUE show(VALUE, VALUE text_) {
        RGMLOAD(text, std::string_view);
        worker >> message_show{text};

        /* 需等待窗口事件处理完毕 */
        RGMWAIT(1);

        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Base = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_Base, "message_show", wrapper::show, 1);
  }
};
}  // namespace rgm::rmxp