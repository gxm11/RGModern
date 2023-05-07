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

namespace rgm::ext {
/// @brief 启动输入法
/// @see ./src/script/textbox.rb
/// 输入法在 ruby 层有高级封装，请查阅并按照范例代码使用。
struct textinput_start {
  /// @brief 输入框的横坐标
  int x;

  /// @brief 输入框的纵坐标
  int y;

  /// @brief 输入框的宽度
  int width;

  /// @brief 输入框的高度
  int height;

  void run(auto&) {
    cen::log_warn("[Input] text input is started");

    SDL_StartTextInput();
    SDL_Rect r{x, y, width, height};
    SDL_SetTextInputRect(&r);
  }
};

/// @brief 关闭输入法
struct textinput_stop {
  void run(auto&) {
    cen::log_warn("[Input] text input is stopped");

    SDL_SetTextInputRect(NULL);
    SDL_StopTextInput();
  }
};

/// @brief 储存输入法状态的数据类
struct textinput_state {
  /// @brief 输入法显示的临时字符，即输入内容
  std::string text;

  /// @brief 输入法光标的位置
  int position;

  /// @brief ruby 中类 RGM::Ext::TextBox 对应的 VALUE
  VALUE rb_cTextBox;

  /// @brief ruby 中类 RGM::Ext::TextBox 的类变量 @@need_refresh 对应的 VALUE
  ID rb_sym_need_refresh;

  void setup() noexcept {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    rb_cTextBox = rb_define_class_under(rb_mRGM_Ext, "TextBox", rb_cObject);
    rb_sym_need_refresh = rb_intern("@@need_refresh");
  }

  void refresh() {
    /* 设置类 RGM::Ext::TextBox 的类变量 @@need_refresh 为 true */
    rb_cvar_set(rb_cTextBox, rb_sym_need_refresh, Qtrue);
  }
};

/// @brief 修改输入内容
/// 此事件由 SDL 事件自动触发
struct text_edit {
  std::string text;
  int pos;

  void run(auto& worker) {
    textinput_state& ci = RGMDATA(textinput_state);
    ci.text = text;
    ci.position = pos;
    ci.refresh();
  }
};

/// @brief 确认输入内容
/// 此事件由 SDL 事件自动触发
struct text_input {
  std::string text;

  void run(auto& worker) {
    textinput_state& ci = RGMDATA(textinput_state);
    ci.text = text;
    ci.position = -1;

    /* 触发 refresh 事件 */
    ci.refresh();
  }
};

/// @brief 输入法相关的初始化类
struct init_text_event {
  static void before(auto& worker) noexcept {
    base::cen_library::event_dispatcher_t& d =
        RGMDATA(base::cen_library).event_dispatcher;

    /* 绑定输入法的编辑事件 */
    d.bind<cen::text_editing_event>().to(
        [&worker](const cen::text_editing_event& e) {
          cen::log_debug("[Input] text edit\n");

          worker >> text_edit{std::string{e.text()}, e.start()};
        });

    /* 绑定输入法的输入事件 */
    d.bind<cen::text_input_event>().to(
        [&worker](const cen::text_input_event& e) {
          cen::log_info("[Input] text input '%s'\n", e.text_utf8().data());

          worker >> text_input{std::string{e.text_utf8()}};
        });
  }
};

/// @brief 数据类 textinput_state 相关的初始化类
struct init_textinput {
  using data = std::tuple<textinput_state>;

  static void before(auto& this_worker) noexcept {
    /* 静态的 worker 变量供函数的内部类 wrapper 使用 */
    static decltype(auto) worker = this_worker;

    /* wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      /* ruby method: Ext#textinput_edit_text -> textinput_state::text */
      static VALUE edit_text(VALUE) {
        textinput_state& ci = RGMDATA(textinput_state);

        return rb_utf8_str_new(ci.text.data(), ci.text.length());
      }

      /* ruby method: Ext#textinput_edit_pos -> textinput_state::position */
      static VALUE edit_pos(VALUE) {
        textinput_state& ci = RGMDATA(textinput_state);

        return INT2FIX(ci.position);
      }

      /* ruby method: Ext#textinput_edit_clear -> textinput_state */
      static VALUE edit_clear(VALUE) {
        textinput_state& ci = RGMDATA(textinput_state);
        ci.text.clear();
        ci.position = 0;
        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_Ext = rb_define_module_under(rb_mRGM, "Ext");

    rb_define_module_function(rb_mRGM_Ext, "textinput_edit_text",
                              wrapper::edit_text, 0);
    rb_define_module_function(rb_mRGM_Ext, "textinput_edit_pos",
                              wrapper::edit_pos, 0);
    rb_define_module_function(rb_mRGM_Ext, "textinput_edit_clear",
                              wrapper::edit_clear, 0);

    RGMBIND(rb_mRGM_Ext, "textinput_start", textinput_start, 4);
    RGMBIND(rb_mRGM_Ext, "textinput_stop", textinput_stop, 0);

    /* 定义 ruby 中的相关类和变量 */
    RGMDATA(textinput_state).setup();
  }
};
}  // namespace rgm::ext