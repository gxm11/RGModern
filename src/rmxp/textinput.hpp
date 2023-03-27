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
#include "base/base.hpp"
#include "detail.hpp"

namespace rgm::rmxp {
struct textinput_start {
  int x;
  int y;
  int width;
  int height;

  void run(auto&) {
    cen::log_warn(cen::log_category::system, "[Input] text input is started");

    SDL_StartTextInput();
    SDL_Rect r{x, y, width, height};
    SDL_SetTextInputRect(&r);
  }
};

struct textinput_stop {
  void run(auto&) {
    cen::log_warn(cen::log_category::system, "[Input] text input is stopped");

    SDL_SetTextInputRect(NULL);
    SDL_StopTextInput();
  }
};

struct textinput_state {
  std::string text;
  int position;

  VALUE rb_cTextBox;
  ID rb_sym_need_refresh;

  void setup() {
    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_EXT = rb_define_module_under(rb_mRGM, "Ext");
    rb_cTextBox = rb_define_class_under(rb_mRGM_EXT, "TextBox", rb_cObject);
    rb_sym_need_refresh = rb_intern("@@need_refresh");
  }

  void refresh() { rb_cvar_set(rb_cTextBox, rb_sym_need_refresh, Qtrue); }
};

struct text_input {
  std::string text;

  void run(auto& worker) {
    textinput_state& ci = RGMDATA(textinput_state);
    ci.text = text;
    ci.position = -1;
    ci.refresh();
  }
};

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

struct init_textinput {
  using data = std::tuple<textinput_state>;

  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    /** wrapper 类，创建静态方法供 ruby 的模块绑定 */
    struct wrapper {
      static VALUE edit_text(VALUE) {
        textinput_state& ci = RGMDATA(textinput_state);

        return rb_utf8_str_new(ci.text.data(), ci.text.length());
      }

      static VALUE edit_pos(VALUE) {
        textinput_state& ci = RGMDATA(textinput_state);

        return INT2FIX(ci.position);
      }

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

    // RGMBIND(rb_mRGM_Ext, textinput_start, int, int, int, int);
    RGMBIND4(rb_mRGM_Ext, "textinput_start", textinput_start, 4);
    // RGMBIND(rb_mRGM_Ext, textinput_stop);
    RGMBIND4(rb_mRGM_Ext, "textinput_stop", textinput_stop, 0);
    RGMDATA(textinput_state).setup();
  }
};
}  // namespace rgm::rmxp