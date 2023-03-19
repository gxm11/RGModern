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
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>

#include "base/base.hpp"
namespace rgm::rmxp {
struct message_show {
  const char* text;

  void run(auto& worker) {
#if 0
    cen::message_box::show(config::game_title, text,
                           cen::message_box_type::information);
#else
    printf("show form\n");

    SDL_SysWMinfo& info = RGMDATA(base::cen_library).window_info;
    HWND hwnd = info.info.win.window;

    nana::appearance opt;
    opt.taskbar = true;
    opt.floating = false;
    opt.no_activate = false;
    opt.minimize = true;
    opt.maximize = false;
    opt.sizable = false;
    opt.decoration = true;

    nana::form fm(reinterpret_cast<nana::window>(hwnd), {300, 200}, opt);
    fm.caption("RGModern Nana");

    nana::label msg_label(fm, nana::rectangle{10, 10, 280, 40});
    msg_label.caption(text);

    nana::button close_button(fm, nana::rectangle{90, 140, 120, 40});
    close_button.caption("OK");
    close_button.events().click([&fm] { fm.close(); });

    fm.show();
    fm.modality();
    printf("close form\n");
#endif
  }
};

struct init_message {
  static void before(auto& this_worker) {
    static decltype(auto) worker = this_worker;

    struct wrapper {
      static VALUE show(VALUE, VALUE text_) {
        RGMLOAD(text, const char*);
        worker >> message_show{text};
        RGMWAIT(1);

        return Qnil;
      }
    };

    VALUE rb_mRGM = rb_define_module("RGM");
    VALUE rb_mRGM_BASE = rb_define_module_under(rb_mRGM, "Base");
    rb_define_module_function(rb_mRGM_BASE, "message_show", wrapper::show, 1);
  }
};
}  // namespace rgm::rmxp